#include "HydraulicSolver.h"

HydraulicSolver::HydraulicSolver(string spr_filename) : Staci(spr_filename){

  string fileFormat = getDefinitionFile().substr(getDefinitionFile().length()-3,3); // SPR or INP
  
  if(fileFormat == "spr")
  {
    IOxml IOxmlObj(getDefinitionFile().c_str());
    maxIterationNumber = stoi(IOxmlObj.readSetting("iter_max"));
    maxPressureError = stod(IOxmlObj.readSetting("e_p_max"));
    maxMassFlowError = stod(IOxmlObj.readSetting("e_mp_max"));
    //relaxationFactor = stod(IOxmlObj.readSetting("relax"));
    relaxationFactor = 0.5;
    relaxationFactorIncrement = stod(IOxmlObj.readSetting("relax_mul"));
    massFlowInitial = stod(IOxmlObj.readSetting("mp_init"));
    pressureInitial = stod(IOxmlObj.readSetting("p_init"));
    //frictionModel = IOxmlObj.readSetting("friction_model");
  }
  else if(fileFormat == "inp")
  {
    maxIterationNumber = 100;
    maxPressureError = 0.00001;
    maxMassFlowError = 0.001;
    relaxationFactor = 1.0;
    relaxationFactorIncrement = 1.2;
    massFlowInitial = 100.;
    pressureInitial = 5.0;
  }
  else
  {
    cout << endl << "Unkown file format: " << fileFormat << endl << "Available file formats are: inp | spr" << endl;
    exit(-1);
  }
  // Setting the heights of the edges
  for(int i=0; i<edges.size(); i++)
  {
    string type = edges[i]->getEdgeStringProperty("type");
    if(type == "Pipe" || type == "Pump" || type == "Valve")
    {
      int startIndex = edges[i]->getEdgeIntProperty("startNodeIndex");
      double startHeight = nodes[startIndex]->getProperty("height");
      edges[i]->setDoubleProperty("startHeight",startHeight);

      int endIndex = edges[i]->getEdgeIntProperty("endNodeIndex");
      double endHeight = nodes[endIndex]->getProperty("height");
      edges[i]->setDoubleProperty("endHeight",endHeight);

    }
    if(type == "Pool" || type == "PressurePoint")
    {
      int index = edges[i]->getEdgeIntProperty("startNodeIndex");
      double height = nodes[index]->getProperty("height");
      edges[i]->setDoubleProperty("height",height);
    }
  }
  initialization();
}

HydraulicSolver::~HydraulicSolver(){}

//--------------------------------------------------------------
bool HydraulicSolver::solveSystem() {

  updateOpenElements();
  const int n = openEdges.size() + openNodes.size();
  VectorXd x(n), f(n);
  int iter = 0;
  double e_mp = 1e10, e_p = 1e10, e_mp_r = 1e10, e_p_r = 1e10;
  bool isConv = false;

  // Setting initial conditions to x vector
  for(int i=0; i<openEdges.size(); i++)
    x(i) = edges[openEdges[i]]->getEdgeDoubleProperty("volumeFlowRate");
  for(int i=0; i<openNodes.size(); i++)
    x(openEdges.size() + i) = nodes[openNodes[i]]->getProperty("head");

  buildJacobian(x, f); // updating Jacobian matrix

  computeError(f, e_mp, e_p, e_mp_r, e_p_r, isConv);
  while((iter < maxIterationNumber + 1) && (!isConv))
  {
    linearSolver(x, f); // Solving Jac*dx = f

    computeError(f, e_mp, e_p, e_mp_r, e_p_r, isConv);
    if(isConv)
      break;

    updateJacobian(x,f);
    updateRelaxationFactor(e_mp, e_p, e_mp_r, e_p_r);

    iter++;
  }

  if(iter > maxIterationNumber)
  {
    cout << iterInfo(iter, e_mp, e_p);
    cout << iterInfoDetail(x, f);
  }
  else
  { 
    for(int i=0; i<openEdges.size(); i++)
      edges[openEdges[i]]->setEdgeDoubleProperty("volumeFlowRate",x(i));
    for(int i=0; i<openNodes.size(); i++)
    { 
      nodes[openNodes[i]]->setProperty("head",x(openEdges.size() + i));
      if(isPressureDemand)
      {
        vector<double> parameters{pdExponent,pdDesiredPressure,pdMinPressure};
        double cons = -nodes[openNodes[i]]->function(x(openEdges.size() + i),parameters);
        nodes[openNodes[i]]->setProperty("consumption",cons);
      }
      else
      {
        nodes[openNodes[i]]->setProperty("consumption",nodes[openNodes[i]]->getProperty("demand"));
      }
    }
  }

  return isConv;
}

//--------------------------------------------------------------
void HydraulicSolver::linearSolver(VectorXd &x, VectorXd &f) {

  int n = f.rows();
 
  VectorXd dx = VectorXd::Zero(n);
  solver.factorize(jacobianMatrix); // performing LU decomposition
  dx = solver.solve(-f); // Solving Jac*dx = -f
  x += relaxationFactor*dx; // x_i+1 = x_i + relax*dx (~0.1<relax<~1.2)

}

//--------------------------------------------------------------
void HydraulicSolver::buildJacobian(VectorXd &x, VectorXd &f) {

  int n = x.rows();
  int maxRank = 4;
  for(int i=0; i<openNodes.size(); i++)
    if(nodes[openNodes[i]]->edgeIn.size() + nodes[openNodes[i]]->edgeOut.size() > maxRank)
      maxRank = nodes[openNodes[i]]->edgeIn.size() + nodes[openNodes[i]]->edgeOut.size();
  //jacobianMatrix.setZero();
  jacobianMatrix.resize(n,n);
  jacobianMatrix.reserve(VectorXi::Constant(n,maxRank));

  for(int i=0; i<openEdges.size(); i++)
  {
    int startIndex, endIndex=0;
    startIndex = edges[openEdges[i]]->getEdgeIntProperty("startNodeIndex");
    startIndex = getVectorIndex(openNodes,startIndex);

    vector<double> ppq(3, 0.0); // contains [Pstart,Pend,Mp]
    ppq[0] = x(openEdges.size() + startIndex);
    if(edges[openEdges[i]]->getEdgeIntProperty("numberNode") == 2)
    {
      endIndex = edges[openEdges[i]]->getEdgeIntProperty("endNodeIndex");
      endIndex = getVectorIndex(openNodes,endIndex);
      ppq[1] = x(openEdges.size() + endIndex);
    }
    ppq[2] = x(i);
    f(i) = edges[openEdges[i]]->function(ppq);

    vector<double> jv = edges[openEdges[i]]->functionDerivative(ppq);

    jacobianMatrix.insert(i,openEdges.size() + startIndex) = jv[0];
    if(edges[openEdges[i]]->getEdgeIntProperty("numberNode") == 2)
      jacobianMatrix.insert(i,openEdges.size() + endIndex) = jv[1];
    jacobianMatrix.insert(i,i) = jv[2];
  }

  for(int i=0; i<openNodes.size(); i++)
  { 
    
    if(isPressureDemand)// If the demands are depending on the pressure
    {
      vector<double> parameters{pdExponent,pdDesiredPressure,pdMinPressure};
      jacobianMatrix.insert(openEdges.size() + i, openEdges.size() + i) = nodes[openNodes[i]]->functionDerivative(x(openEdges.size()+i), parameters);
      f(openEdges.size() + i) = nodes[openNodes[i]]->function(x(openEdges.size()+i), parameters);
    }
    else// If the demands are NOT depending on the pressure
    {
      f(openEdges.size() + i) = -nodes[openNodes[i]]->getProperty("demand");
    }

    for(int j=0; j<nodes[openNodes[i]]->edgeIn.size(); j++)
    { 
      int idx = getVectorIndex(openEdges,nodes[openNodes[i]]->edgeIn[j]);
      f(openEdges.size() + i) += x(idx);
      jacobianMatrix.insert(openEdges.size() + i,idx) = 1.0;
    }
    for(int j=0; j<nodes[openNodes[i]]->edgeOut.size(); j++)
    {
      int idx = getVectorIndex(openEdges,nodes[openNodes[i]]->edgeOut[j]);
      f(openEdges.size() + i) -= x(idx);
      jacobianMatrix.insert(openEdges.size() + i,idx) = -1.0;
    }
  }

  solver.analyzePattern(jacobianMatrix);
}

//--------------------------------------------------------------
void HydraulicSolver::updateJacobian(VectorXd &x, VectorXd &f) {

  for(int i=0; i<openEdges.size(); i++)
  {
    int startIndex, endIndex=0;
    startIndex = edges[openEdges[i]]->getEdgeIntProperty("startNodeIndex");
    startIndex = getVectorIndex(openNodes,startIndex);

    vector<double> ppq(3, 0.0); // contains [Pstart,Pend,Mp]
    ppq[0] = x(openEdges.size() + startIndex);
    if(edges[openEdges[i]]->getEdgeIntProperty("numberNode") == 2)
    {
      endIndex = edges[openEdges[i]]->getEdgeIntProperty("endNodeIndex");
      endIndex = getVectorIndex(openNodes,endIndex);
      ppq[1] = x(openEdges.size() + endIndex);
    }
    ppq[2] = x(i);
    f(i) = edges[openEdges[i]]->function(ppq);

    vector<double> jv = edges[openEdges[i]]->functionDerivative(ppq);
    jacobianMatrix.coeffRef(i,i) = jv[2];
  }

  for(int i=0; i<openNodes.size(); i++)
  { 
    if(isPressureDemand)// If the demands are depending on the pressure
    {
      vector<double> parameters{pdExponent,pdDesiredPressure,pdMinPressure};
      jacobianMatrix.coeffRef(openEdges.size() + i, openEdges.size() + i) = nodes[openNodes[i]]->functionDerivative(x(openEdges.size()+i), parameters);
      f(openEdges.size() + i) = nodes[openNodes[i]]->function(x(openEdges.size()+i), parameters);
    }
    else// If the demands are NOT depending on the pressure
    {
      f(openEdges.size() + i) = -nodes[openNodes[i]]->getProperty("demand");
    }

    for(int j=0; j<nodes[openNodes[i]]->edgeIn.size(); j++)
    { 
      int idx = getVectorIndex(openEdges,nodes[openNodes[i]]->edgeIn[j]);
      f(openEdges.size() + i) += x(idx);
    }
    for(int j=0; j<nodes[openNodes[i]]->edgeOut.size(); j++)
    {
      int idx = getVectorIndex(openEdges,nodes[openNodes[i]]->edgeOut[j]);
      f(openEdges.size() + i) -= x(idx);
    }
  }
}

//--------------------------------------------------------------
void HydraulicSolver::updateRelaxationFactor(double e_mp, double e_p, double & e_mp_r, double & e_p_r) {
  minRelaxationFactor = 0.1;
  maxRelaxationFactor = 1.2;
  double hiba, hiba_r;
  if (e_p > e_mp) {
    hiba = e_p;
    hiba_r = e_p_r;
  } else {
    hiba = e_mp;
    hiba_r = e_mp_r;
  }
  if (hiba <= hiba_r)
    relaxationFactor = relaxationFactor * relaxationFactorIncrement;
  else
    relaxationFactor = relaxationFactor / 5.;
  if (relaxationFactor < minRelaxationFactor)
    relaxationFactor = minRelaxationFactor;
  if (relaxationFactor > maxRelaxationFactor)
    relaxationFactor = maxRelaxationFactor;

  e_mp_r = e_mp;
  e_p_r = e_p;
}

//--------------------------------------------------------------
void HydraulicSolver::computeError(const VectorXd &f, double & e_mp, double & e_p, double & e_mp_r, double & e_p_r, bool & convergence) {
  // Saving old errors
  e_mp_r = e_mp;
  e_p_r = e_p;

  // Determining new errors
  e_p = (f.head(openEdges.size())).norm();
  e_mp = (f.tail(openNodes.size())).norm();

  // Checking the convergence
  if ((e_p < maxPressureError) && (e_mp < maxMassFlowError)) 
    convergence = true;
}

//--------------------------------------------------------------
void HydraulicSolver::initialization() {
  if(!getIsInitialization()) // Automatic initialization
  {
    // Finding the geodetic height of the first pressure point
    double baseGeodeticHeight = 0.;
    int i=0;
    while(baseGeodeticHeight == 0. && i<nodes.size()){
      string type = edges[i]->getEdgeStringProperty("type");
      if(type == "PressurePoint" || type == "Pool"){
        int idx = edges[i]->getEdgeIntProperty("startNodeIndex");
        baseGeodeticHeight = nodes[idx]->getProperty("height");
      }
      i++;
    }

    // Filling up the nodes
    for(int i = 0; i < nodes.size(); i++){
      double head = 50. + baseGeodeticHeight;
      nodes[i]->initialization(1., head);
    }

    // Filling up the edges
    for(int i = 0; i < edges.size(); i++){
      if(edges[i]->getEdgeStringProperty("type") == "Pipe"){
        double v = 0.3;
        double D = edges[i]->getDoubleProperty("diameter");
        edges[i]->initialization(1., v*density*D*D*M_PI/4.);
      }else
        edges[i]->initialization(1., massFlowInitial);
    }
  }
  else // From file
  {
    IOxml IOxml(getInitializationFile().c_str());
    IOxml.loadInitialValue(nodes, edges);
  }
}

//--------------------------------------------------------------
void HydraulicSolver::initialization(const Staci * inStaci) {
  for(int i = 0; i < nodes.size(); i++)
    nodes.at(i)->setProperty("head",inStaci->nodes.at(i)->getProperty("head"));

  for(int i = 0; i < edges.size(); i++)
    edges.at(i)->setEdgeDoubleProperty("volumeFlowRate",inStaci->edges.at(i)->getEdgeDoubleProperty("volumeFlowRate"));
}

//--------------------------------------------------------------
string HydraulicSolver::iterInfo(int iter, double e_mp, double e_p) {
  ostringstream consolePrint;
  consolePrint.str("");
  consolePrint.setf(ios::dec);
  consolePrint.unsetf(ios::showpos);
  consolePrint << endl << " iter. # " << iter << "./" << maxIterationNumber;
  consolePrint << setprecision(2) << scientific;  // << number << std::endl;
  consolePrint << " e_mp=" << e_mp << ",  e_p=" << e_p;
  consolePrint << setprecision(2) << fixed;
  consolePrint << "  relax.=" << relaxationFactor << ", relax.mul.=" << relaxationFactorIncrement;

  return consolePrint.str();
}

//--------------------------------------------------------------
string HydraulicSolver::iterInfoDetail(const VectorXd &x,const VectorXd &f){
  ostringstream consolePrint;
  consolePrint.str("");
  for(int i = 0; i < openEdges.size(); i++)
    consolePrint << endl << "\t" << edges[openEdges[i]]->getEdgeStringProperty("name") << ": \tmp=" << x[i] << ", \tf=" << f[i];
  for(int i = 0; i < openNodes.size(); i++)
    consolePrint << endl << "\t" << nodes[openNodes[i]]->getName() << ": \tp =" << x[openEdges.size() + i] << ", \tf=" << f[openEdges.size() + i];
  return consolePrint.str();
}

//--------------------------------------------------------------
void HydraulicSolver::listResult() {
 
  cout << scientific << setprecision(3) << showpos;
  cout << endl << endl << "RESULTS:";
  for(int i = 0; i < edges.size(); i++){
    printf("\n %-12s:",edges[i]->getEdgeStringProperty("name").c_str());
    cout << "  mp = " << edges[i]->getEdgeDoubleProperty("massFlowRate") << " kg/s" << "   Q = " << (3600 * (edges[i]->getEdgeDoubleProperty("volumeFlowRate"))) << " m3/h" << "   v = " << edges[i]->getEdgeDoubleProperty("velocity") << " m/s";
  }
  cout << endl << "\t" << "-----------------------------------------------------------------";
  for(int i = 0; i < nodes.size(); i++){
    printf("\n %-12s:", nodes[i]->getName().c_str());
    cout << "  p = " << nodes[i]->getProperty("head") * 1000 * 9.81 / 1e5 << " bar" << "     H = " << nodes[i]->getProperty("head") << " m" << "      H+height=" << nodes[i]->getProperty("head") + nodes[i]->getProperty("height") << " m";
  }
  cout << endl << endl;
}

//--------------------------------------------------------------
void HydraulicSolver::updateOpenElements(){
  openEdges.clear();
  openNodes.clear();
  for(int i=0; i<edges.size(); i++)
    if(!edges[i]->isClosed)
      openEdges.push_back(i);
  for(int i=0; i<nodes.size(); i++)
    if(!nodes[i]->isClosed)
      openNodes.push_back(i);
}