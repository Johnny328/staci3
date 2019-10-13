#include "HydraulicSolver.h"

//--------------------------------------------------------------
HydraulicSolver::HydraulicSolver(string spr_filename) : Staci(spr_filename)
{
  maxIterationNumber = 100;
  maxPressureError = 0.001;
  maxMassFlowError = 0.01;
  relaxationFactor = 1.0;
  relaxationFactorIncrement = 1.5;
  massFlowInitial = 100.;
  pressureInitial = 5.0;

  // setting the number of equations
  numberEquations = numberEdges + numberNodes;

  // Setting the heights of the edges
  for(int i=0; i<numberEdges; i++)
  {
    if(edges[i]->numberNode == 2)
    {
      int startIndex = edges[i]->startNodeIndex;
      edges[i]->startHeight = nodes[startIndex]->geodeticHeight;

      int endIndex = edges[i]->endNodeIndex;
      edges[i]->endHeight = nodes[endIndex]->geodeticHeight;
    }
    if(edges[i]->numberNode == 1)
    {
      int startIndex = edges[i]->startNodeIndex;
      edges[i]->startHeight = nodes[startIndex]->geodeticHeight;
    }
  }

  // giving initial values to head and volume flow rates
  initialization();

  // calculating the max rank of the jacobian matrix
  maxRank = 4;
  for(int i=0; i<numberNodes; i++)
    if(nodes[i]->edgeIn.size() + nodes[i]->edgeOut.size() > maxRank)
      maxRank = nodes[i]->edgeIn.size() + nodes[i]->edgeOut.size();

  // resiying Eigen vectors
  x.resize(numberEquations); f.resize(numberEquations);
}

HydraulicSolver::~HydraulicSolver(){}

//--------------------------------------------------------------
bool HydraulicSolver::solveSystem()
{
  double e_mp, e_p, e_mp_r, e_p_r;
  int iter;
  bool isConv;
  
  for(int i=0; i<numberEdges; i++){
    if(edges[i]->type == "ValveFCV")
      edges[i]->status = 1;
  }

  e_mp = 1e10, e_p = 1e10, e_mp_r = 1e10, e_p_r = 1e10;
  iter = 0;
  isConv = false;

  // Setting initial conditions to x vector
  for(int i=0; i<numberEdges; i++)
    x(i) = edges[i]->volumeFlowRate;
  for(int i=0; i<numberNodes; i++)
    x(numberEdges + i) = nodes[i]->head;

  buildJacobian(x, f); // updating Jacobian matrix

  computeError(f, e_mp, e_p, e_mp_r, e_p_r, isConv);
  while((iter<maxIterationNumber+1) && (!isConv))
  {
    linearSolver(x, f); // Solving Jac*dx = f

    // updating active valves (FCV, PRV) | should be indexed more efficiently
    //for(int i=0; i<numberEdges; i++)
    //  if(edges[i]->type == "ValveFCV"){
    //    if(x[i] >= edges[i]->setting)
    //      edges[i]->status = 1;
    //    else
    //      edges[i]->status = 2;
    //  }
    //bool changed = prvStatusUpdate();

    bool changed = edgeStatusUpdate();

    computeError(f, e_mp, e_p, e_mp_r, e_p_r, isConv);
    //cout << iterInfo(iter, e_mp, e_p);
    if(isConv && !changed)
      break;

    updateJacobian(x,f);
    if(iter>1)
      updateRelaxationFactor(e_mp, e_p, e_mp_r, e_p_r);
    //cout << iterInfoDetail(x, f);
    //cin.get();

    iter++;
  }

  for(int i=0; i<numberEdges; i++)
    edges[i]->volumeFlowRate = x(i);
  for(int i=0; i<numberNodes; i++)
    nodes[i]->head = x(numberEdges + i);

  //valveChanged = valveStatusUpdate();
  //if(valveChanged)
   // updateOpenElements();

  if(iter > maxIterationNumber)
  {
    cout << iterInfo(iter, e_mp, e_p);
    //cout << iterInfoDetail(x, f);
  }
  else
  {
    for(int i=0; i<numberEdges; i++)
      edges[i]->volumeFlowRate = x(i);
    for(int i=0; i<numberNodes; i++)
    { 
      nodes[i]->head = x(numberEdges + i);
      if(isPressureDemand)
      {
        double cons = -nodes[i]->function(x(numberEdges + i));
        nodes[i]->consumption = cons;
      }
      else
      {
        nodes[i]->consumption = nodes[i]->demand;
      }
    }
    relaxationFactor = 1.0;
  }

  // Checking the pumps
  checkPumpOperatingPoint();

  return isConv;
}

//--------------------------------------------------------------
void HydraulicSolver::linearSolver(VectorXd &x, VectorXd &f)
{
  VectorXd dx = VectorXd::Zero(numberEquations);
  solver.analyzePattern(jacobianMatrix);
  solver.factorize(jacobianMatrix); // performing LU decomposition
  dx = solver.solve(-f); // Solving Jac*dx = -f
  x += relaxationFactor*dx; // x_i+1 = x_i + relax*dx (~0.1<relax<~1.0)
}

//--------------------------------------------------------------
void HydraulicSolver::buildJacobian(VectorXd &x, VectorXd &f)
{
  int n = x.rows();

  //jacobianMatrix.setZero();
  jacobianMatrix.resize(n,n);
  jacobianMatrix.reserve(VectorXi::Constant(n,maxRank));

  for(int i=0; i<numberEdges; i++)
  {
    int startIndex, endIndex=0;
    startIndex = edges[i]->startNodeIndex;

    // todo: this can be done much more efficiently
    vector<double> ppq(3, 0.0); // contains [Pstart,Pend,Vf]
    ppq[0] = x(numberEdges + startIndex);
    if(edges[i]->numberNode == 2)
    {
      endIndex = edges[i]->endNodeIndex;
      ppq[1] = x(numberEdges + endIndex);
    }
    ppq[2] = x(i);
    f(i) = edges[i]->function(ppq);

    vector<double> jv = edges[i]->functionDerivative(ppq);

    jacobianMatrix.insert(i,numberEdges + startIndex) = jv[0];
    if(edges[i]->numberNode == 2)
      jacobianMatrix.insert(i,numberEdges + endIndex) = jv[1];
    jacobianMatrix.insert(i,i) = jv[2];
  }

  for(int i=0; i<numberNodes; i++)
  { 
    if(isPressureDemand)// If the demands are depending on the pressure
    {
      jacobianMatrix.insert(numberEdges + i, numberEdges + i) = nodes[i]->functionDerivative(x(numberNodes+i));
      f(numberEdges + i) = nodes[i]->function(x(numberEdges+i));
    }
    else// If the demands are NOT depending on the pressure
    {
      f(numberEdges + i) = -nodes[i]->demand;
    }

    for(int j=0; j<nodes[i]->edgeIn.size(); j++)
    { 
      //int idx = getVectorIndex(openEdges,nodes[openNodes[i]]->edgeIn[j]);
      int idx = nodes[i]->edgeIn[j];
      f(numberEdges + i) += x(idx);
      jacobianMatrix.insert(numberEdges + i, idx) = 1.0;
    }
    for(int j=0; j<nodes[i]->edgeOut.size(); j++)
    {
      //int idx = getVectorIndex(openEdges,nodes[openNodes[i]]->edgeOut[j]);
      int idx = nodes[i]->edgeOut[j];
      f(numberEdges + i) -= x(idx);
      jacobianMatrix.insert(numberEdges + i, idx) = -1.0;
    }
  }

  //solver.analyzePattern(jacobianMatrix);
}

//--------------------------------------------------------------
void HydraulicSolver::updateJacobian(VectorXd &x, VectorXd &f)
{
  for(int i=0; i<numberEdges; i++)
  {
    int startIndex, endIndex=0;
    startIndex = edges[i]->startNodeIndex;

    vector<double> ppq(3, 0.0); // contains [Pstart,Pend,Vf]
    ppq[0] = x(numberEdges + startIndex);
    if(edges[i]->numberNode == 2)
    {
      endIndex = edges[i]->endNodeIndex;
      ppq[1] = x(numberEdges + endIndex);
    }
    ppq[2] = x(i);
    f(i) = edges[i]->function(ppq);

    vector<double> jv = edges[i]->functionDerivative(ppq);
    jacobianMatrix.coeffRef(i,numberEdges + startIndex) = jv[0];
    if(edges[i]->numberNode == 2)
      jacobianMatrix.coeffRef(i,numberEdges + endIndex) = jv[1];
    jacobianMatrix.coeffRef(i,i) = jv[2];
  }

  for(int i=0; i<numberNodes; i++)
  { 
    if(isPressureDemand)// If the demands are depending on the pressure
    {
      jacobianMatrix.coeffRef(numberEdges + i, numberEdges + i) = nodes[i]->functionDerivative(x(numberEdges+i));
      f(numberEdges + i) = nodes[i]->function(x(numberEdges+i));
    }
    else// If the demands are NOT depending on the pressure
    {
      f(numberEdges + i) = -nodes[i]->demand;
    }

    for(int j=0; j<nodes[i]->edgeIn.size(); j++)
    { 
      //int idx = getVectorIndex(openEdges,nodes[openNodes[i]]->edgeIn[j]);
      int idx = nodes[i]->edgeIn[j];
      f(numberEdges + i) += x(idx);
    }
    for(int j=0; j<nodes[i]->edgeOut.size(); j++)
    {
      //int idx = getVectorIndex(openEdges,nodes[openNodes[i]]->edgeOut[j]);
      int idx = nodes[i]->edgeOut[j];
      f(numberEdges + i) -= x(idx);
    }
  }
}

//--------------------------------------------------------------
void HydraulicSolver::updateRelaxationFactor(double e_mp, double e_p, double & e_mp_r, double & e_p_r)
{
  double error, error_old;
  if (e_p > e_mp) {
    error = e_p;
    error_old = e_p_r;
  } else {
    error = e_mp;
    error_old = e_mp_r;
  }
  if (error <= error_old)
    relaxationFactor = relaxationFactor * relaxationFactorIncrement;
  else
    relaxationFactor = relaxationFactor / 2.;
  if (relaxationFactor < minRelaxationFactor)
    relaxationFactor = minRelaxationFactor;
  if (relaxationFactor > maxRelaxationFactor)
    relaxationFactor = maxRelaxationFactor;

  e_mp_r = e_mp;
  e_p_r = e_p;
}

//--------------------------------------------------------------
void HydraulicSolver::computeError(const VectorXd &f, double & e_mp, double & e_p, double & e_mp_r, double & e_p_r, bool & convergence)
{
  // Saving old errors
  e_mp_r = e_mp;
  e_p_r = e_p;

  // Determining new errors
  e_p = (f.head(numberEdges)).norm();
  e_mp = (f.tail(numberNodes)).norm();

  // Checking the convergence
  if ((e_p < maxPressureError) && (e_mp < maxMassFlowError)) 
    convergence = true;
}

//--------------------------------------------------------------
void HydraulicSolver::initialization()
{
  double baseGeodeticHeight = 0.;
  int i=0;
  while(baseGeodeticHeight == 0. && i<numberNodes){
    string type = edges[i]->getEdgeStringProperty("type");
    if(type == "PressurePoint" || type == "Pool"){
      int idx = edges[i]->getEdgeIntProperty("startNodeIndex");
      baseGeodeticHeight = nodes[idx]->getProperty("height");
    }
    i++;
  }

  // Filling up the nodes
  for(int i = 0; i < numberNodes; i++){
    double head = 50. + baseGeodeticHeight;
    nodes[i]->initialization(0, head);
  }

  // Filling up the edges
  for(int i = 0; i < numberEdges; i++){
    if(edges[i]->getEdgeStringProperty("type") == "Pipe"){
      double v = 0.3;
      double D = edges[i]->getDoubleProperty("diameter");
      edges[i]->initialization(1., v*density*D*D*M_PI/4.);
    }else
      edges[i]->initialization(1., massFlowInitial);
  }
}

//--------------------------------------------------------------
string HydraulicSolver::iterInfo(int iter, double e_mp, double e_p)
{
  ostringstream consolePrint;
  consolePrint.str("");
  consolePrint.setf(ios::dec);
  consolePrint.unsetf(ios::showpos);
  consolePrint << " iter. # " << iter << "./" << maxIterationNumber;
  consolePrint << setprecision(2) << scientific;  // << number << std::endl;
  consolePrint << " e_mp=" << e_mp << ",  e_p=" << e_p;
  consolePrint << setprecision(2) << fixed;
  consolePrint << "  relax.=" << relaxationFactor << ", relax.mul.=" << relaxationFactorIncrement << endl;

  return consolePrint.str();
}

//--------------------------------------------------------------
string HydraulicSolver::iterInfoDetail(const VectorXd &x,const VectorXd &f)
{
  ostringstream consolePrint;
  consolePrint.str("");
  for(int i = 0; i < numberEdges; i++)
    consolePrint << endl << "\t" << edges[i]->name << ": \tmp=" << x[i] << ", \tf=" << f[i];
  for(int i = 0; i < numberNodes; i++)
    consolePrint << endl << "\t" << nodes[i]->name << ": \tp =" << x[numberEdges + i] << ", \tf=" << f[numberEdges + i];
  return consolePrint.str();
}

//--------------------------------------------------------------
void HydraulicSolver::listResult()
{
  cout << scientific << setprecision(3) << showpos;
  cout << endl << endl << "RESULTS:";
  for(int i = 0; i < numberEdges; i++){
    printf("\n %-12s:",edges[i]->name.c_str());
    cout << "  mp = " << edges[i]->volumeFlowRate << " kg/s" << "   Q = " << edges[i]->volumeFlowRate << " l/s" << "   v = " << edges[i]->getDoubleProperty("velocity") << " m/s";
  }
  cout << endl << "\t" << "-----------------------------------------------------------------";
  for(int i = 0; i < numberNodes; i++){
    printf("\n %-12s:", nodes[i]->getName().c_str());
    cout << "  p = " << nodes[i]->head * 1000. * 9.81 / 1.e5 << " bar" << "     H = " << nodes[i]->head << " m" << "      H+height=" << nodes[i]->head + nodes[i]->geodeticHeight << " m";
  }
  cout << endl << endl;
}

//--------------------------------------------------------------
void HydraulicSolver::checkPumpOperatingPoint()
{
  for(int i=0; i<numberEdges; i++){
    if(edges[i]->getEdgeStringProperty("type") == "Pump"){
      edges[i]->checkPump();
    }
  }
}

//--------------------------------------------------------------
bool HydraulicSolver::edgeStatusUpdate()
{
  bool changed = false;
  for(int i=0; i<numberEdges; i++)
  {
    if(edges[i]->status != -1) // the edge is not permanently closed
    { 
      if(edges[i]->type == "Pipe") // Pipe
      {
        double pi = x[numberEdges + edges[i]->startNodeIndex];
        double pj = x[numberEdges + edges[i]->endNodeIndex];
        double hi = nodes[edges[i]->startNodeIndex]->geodeticHeight;
        double hj = nodes[edges[i]->endNodeIndex]->geodeticHeight;
        double vf = x[i];
        int status = edges[i]->status;
        if(edges[i]->isCheckValve) // && (!isCheckValve || (isCheckValve && x[2] > 0.))
        {
          if(status == 1) // OPEN
          {
            if(vf < 0.)
              edges[i]->status = 0;
          }
          else // CLOSED
          {
            if(pi + (hi-hj) > pj)
              edges[i]->status = 1;
          }
        }
        if(status != edges[i]->status) // Checking for change
          changed = true;
      }
      else if(edges[i]->type == "ValvePRV") // PRV: Pressure Reduction Valve
      { 
        double pi = x[numberEdges + edges[i]->startNodeIndex];
        double pj = x[numberEdges + edges[i]->endNodeIndex];
        double hi = nodes[edges[i]->startNodeIndex]->geodeticHeight;
        double hj = nodes[edges[i]->endNodeIndex]->geodeticHeight;
        double vf = x[i];
        int status = edges[i]->status;
        double setting = edges[i]->setting;
        if(edges[i]->status == 2) // ACTIVE
        {
          if(vf<-volumeFlowRateTolerance){
            edges[i]->status = 0;
          } 
          else if(pi + hi < setting + hj - headTolerance)// todo: minor loss
          {
            edges[i]->status = 1;
          }
          else
          {
            edges[i]->status = 2;
          }
        }
        else if(edges[i]->status == 1) // OPEN
        {
          if(vf<-volumeFlowRateTolerance)
          {
            edges[i]->status = 0;
          } 
          else if(pj + hj >= setting + hj + headTolerance)// todo: minor loss
          {
            edges[i]->status = 2;
          }
          else
          {
            edges[i]->status = 1;
          }
        }
        else // CLOSED
        {
          if(pi + hi >= setting + hj + headTolerance && pj + hj < setting + hj - headTolerance)
          {
            edges[i]->status = 2;
          }
          else if(pi + hi < setting + hj - headTolerance && pi + hi > pj + hj + headTolerance)
          {
            edges[i]->status = 1;
          }
        }
        if(status != edges[i]->status) // Checking for change
          changed = true;
      }
      else if(edges[i]->type == "ValvePSV") // PRV: Pressure Reduction Valve
      { 
        double pi = x[numberEdges + edges[i]->startNodeIndex];
        double pj = x[numberEdges + edges[i]->endNodeIndex];
        double hi = nodes[edges[i]->startNodeIndex]->geodeticHeight;
        double hj = nodes[edges[i]->endNodeIndex]->geodeticHeight;
        double vf = x[i];
        int status = edges[i]->status;
        double setting = edges[i]->setting;
        if(edges[i]->status == 2) // ACTIVE
        {
          if(vf<-volumeFlowRateTolerance){
            edges[i]->status = 0;
          } 
          else if(pj + hj > setting + hi + headTolerance)// todo: minor loss
          {
            edges[i]->status = 1;
          }
          else
          {
            edges[i]->status = 2;
          }
        }
        else if(edges[i]->status == 1) // OPEN
        {
          if(vf<-volumeFlowRateTolerance)
          {
            edges[i]->status = 0;
          } 
          else if(pi + hi < setting + hi - headTolerance)// todo: minor loss
          {
            edges[i]->status = 2;
          }
          else
          {
            edges[i]->status = 1;
          }
        }
        else // CLOSED
        {
          if(pj + hj >= setting + hi + headTolerance && pi + hi > setting + hi + headTolerance)
          {
            edges[i]->status = 1;
          }
          else if(pi + hi >= setting + hi + headTolerance && pi + hi > pj + hj + headTolerance)
          {
            edges[i]->status = 1;
          }
        }
        if(status != edges[i]->status) // Checking for change
          changed = true;
      }
      else if(edges[i]->type == "ValveFCV"){
        int status = edges[i]->status;
        if(x[i] >= edges[i]->setting)
          edges[i]->status = 2; // ACTIVE
        else
          edges[i]->status = 1; // OPEN

        if(status != edges[i]->status) // Checking for change
          changed = true;
      }
    }
  }
  return changed;
}