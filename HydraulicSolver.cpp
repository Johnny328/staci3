#include "HydraulicSolver.h"

HydraulicSolver::HydraulicSolver(string spr_filename) : Staci(spr_filename){

  IOxml IOxmlObj(getDefinitionFile().c_str());

  maxIterationNumber = stoi(IOxmlObj.readSetting("iter_max"));
  maxPressureError = stod(IOxmlObj.readSetting("e_p_max"));
  maxMassFlowError = stod(IOxmlObj.readSetting("e_mp_max"));
  relaxationFactor = stod(IOxmlObj.readSetting("relax"));
  relaxationFactorIncrement = stod(IOxmlObj.readSetting("relax_mul"));
  massFlowInitial = stod(IOxmlObj.readSetting("mp_init"));
  pressureInitial = stod(IOxmlObj.readSetting("p_init"));
  frictionModel = IOxmlObj.readSetting("friction_model");

  if (frictionModel != "DW" && frictionModel != "HW") {
    cout << endl << endl << "******** WARNING! ********" << endl << " Friction model: " << frictionModel << " do NOT exist" << endl << " Options: DW | HW" << endl << " Changing to DW!" << endl;
    frictionModel = "DW";
  }
}

HydraulicSolver::~HydraulicSolver(){}

//--------------------------------------------------------------
bool HydraulicSolver::solveSystem() {
  const int n = nodes.size() + edges.size();
  VectorXd b(n), x(n), dx(n), f(n), xu(n);
  int iter = 0;
  double e_mp = 1e10, e_p = 1e10, e_mp_r = 1e10, e_p_r = 1e10;
  bool convergence = false;

  while ((iter < maxIterationNumber + 1) && (!convergence)) {

    updateJacobian(x, f, iter);
    computeError(f, e_mp, e_p, e_mp_r, e_p_r, convergence);
    if(convergence)
      break;

    linearSolver(x, f);

    updateRelaxationFactor(e_mp, e_p, e_mp_r, e_p_r);

    iter++;
  }

  if(iter > maxIterationNumber){
    cout << iterInfo(iter, e_mp, e_p);
    cout << iterInfoDetail(x, f);
  }

  return convergence;
}

//--------------------------------------------------------------
void HydraulicSolver::initialization() {
  if(!getIsInitialization()) // Automatic initialization
  {
    for(int i = 0; i < nodes.size(); i++)
      nodes.at(i)->initialization(1., pressureInitial);
    for(int i = 0; i < edges.size(); i++)
      edges.at(i)->initialization(1., massFlowInitial);
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
    edges.at(i)->setEdgeDoubleProperty("massFlowRate",inStaci->edges.at(i)->getEdgeDoubleProperty("massFlowRate"));
}

//--------------------------------------------------------------
void HydraulicSolver::linearSolver(VectorXd &x, VectorXd &f) {
  int n = edges.size() + nodes.size();
  VectorXd dx = VectorXd::Zero(n);
  SparseLU<SparseMatrix<double, ColMajor> > solver;
  solver.analyzePattern(jacobianMatrix);
  solver.factorize(jacobianMatrix);
  dx = solver.solve(f);

  for(int i=0; i<edges.size(); i++)
    edges[i]->setEdgeDoubleProperty("massFlowRate",x(i) - relaxationFactor * dx(i));
  for(int i=0; i<nodes.size(); i++)
    nodes[i]->setProperty("head",x(edges.size() + i) - relaxationFactor*dx(edges.size()+i));
}

//--------------------------------------------------------------
string HydraulicSolver::listResult() {
 
  ostringstream strstrm;
  strstrm << scientific << setprecision(3) << showpos;
  strstrm << endl << endl << "RESULTS:";
  for(int i = 0; i < edges.size(); i++)
    strstrm << endl << "\t" << edges[i]->getEdgeStringProperty("name") << ":\tmp=" << edges[i]->getEdgeDoubleProperty("massFlowRate") << " kg/s" << "\tQ=" << (3600 * (edges[i]->getEdgeDoubleProperty("volumeFlowRate"))) << " m3/h" << "\tv=" << edges[i]->getEdgeDoubleProperty("velocity") << " m/s";
  strstrm << endl << "\t" << "-----------------------------------------------------------------";
  for(int i = 0; i < nodes.size(); i++)
    strstrm << endl << "\t" << nodes[i]->getName() << ":\t p=" << nodes[i]->getProperty("head") * 1000 * 9.81 / 1e5 << " bar" << "\tH=" << nodes[i]->getProperty("head") << " m" << ",   H+height=" << nodes[i]->getProperty("head") + nodes[i]->getProperty("height") << " m";
  strstrm << endl << endl;
  return strstrm.str();
}

//--------------------------------------------------------------
void HydraulicSolver::updateJacobian(VectorXd &x, VectorXd &f, int iter) {
  int n = nodes.size() + edges.size();

  if(iter == 0)
  {
    jacobianMatrix.resize(n,n);
    jacobianMatrix.reserve(VectorXi::Constant(n,10));
  }

  for(int i = 0; i < edges.size(); i++) {
    x(i) = edges[i]->getEdgeDoubleProperty("massFlowRate");
  }
  for(int i = 0; i < nodes.size(); i++) {
    x(edges.size() + i) = nodes[i]->getProperty("head");
  }
  // f es Jacobi kiertekelese az aktualis adatokkal
  int Q_indx, pe_indx, pv_indx;
  vector<double> pevhev(4, 0.0);
  vector<double> jv;

  for(int i = 0; i < edges.size(); i++) {
    if (edges[i]->getEdgeIntProperty("numberNode") == 1)
    {
      pevhev[0] = nodes[edges[i]->getEdgeIntProperty("startNodeIndex")]->getProperty("head");
      pevhev[1] = 0.0;
      pevhev[2] = nodes[edges[i]->getEdgeIntProperty("startNodeIndex")]->getProperty("height");
      pevhev[3] = 0.0;
    } 
    else 
    {
      pevhev[0] = nodes[edges[i]->getEdgeIntProperty("startNodeIndex")]->getProperty("head");
      pevhev[1] = nodes[edges[i]->getEdgeIntProperty("endNodeIndex")]->getProperty("head");
      pevhev[2] = nodes[edges[i]->getEdgeIntProperty("startNodeIndex")]->getProperty("height");
      pevhev[3] = nodes[edges[i]->getEdgeIntProperty("endNodeIndex")]->getProperty("height");
    }
    f(i) = edges[i]->function(pevhev);
    jv = edges[i]->functionDerivative(pevhev);

    Q_indx = i;
    pe_indx = edges[i]->getEdgeIntProperty("startNodeIndex");
    pv_indx = edges[i]->getEdgeIntProperty("endNodeIndex");

    if(iter>0)
      jacobianMatrix.coeffRef(i,edges.size() + pe_indx)=jv.at(0);
    else
      jacobianMatrix.insert(i,edges.size() + pe_indx)=jv.at(0);

    if (edges[i]->getEdgeIntProperty("numberNode") == 2) {
      if(iter>0)
        jacobianMatrix.coeffRef(i,edges.size() + pv_indx) = jv.at(1);
      else
        jacobianMatrix.insert(i,edges.size() + pv_indx) = jv.at(1);
    }
    if(iter>0)
      jacobianMatrix.coeffRef(i,Q_indx) = jv.at(2);
    else
      jacobianMatrix.insert(i,Q_indx) = jv.at(2);

    jv.clear();
  }

  for(int i = 0; i < nodes.size(); i++)
  {
    f(edges.size() + i) = -nodes[i]->getProperty("demand");
    for(int j = 0; j < nodes[i]->edgeIn.size(); j++)
    {
      f(edges.size() + i) += edges[nodes[i]->edgeIn.at(j)]->getEdgeDoubleProperty("massFlowRate");
      if(iter>0)
        jacobianMatrix.coeffRef(edges.size() + i,nodes[i]->edgeIn.at(j)) = +1.0;
      else
        jacobianMatrix.insert(edges.size() + i,nodes[i]->edgeIn.at(j)) = +1.0;
    }
    for(int j = 0; j < nodes[i]->edgeOut.size(); j++)
    {
      f(edges.size() + i) -= edges[nodes[i]->edgeOut.at(j)]->getEdgeDoubleProperty("massFlowRate");
      if(iter>0)
        jacobianMatrix.coeffRef(edges.size() + i,nodes[i]->edgeOut.at(j)) = -1.0;
      else
        jacobianMatrix.insert(edges.size() + i,nodes[i]->edgeOut.at(j)) = -1.0;
    }
  }
}

//--------------------------------------------------------------
void HydraulicSolver::updateRelaxationFactor(double e_mp, double e_p, double & e_mp_r, double & e_p_r) {
  minRelaxationFactor = 0.01;
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
  e_p = (f.head(edges.size())).norm();
  e_mp = (f.tail(nodes.size())).norm();

  // Checking the convergence
  if ((e_p < maxPressureError) && (e_mp < maxMassFlowError)) 
    convergence = true;
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
  for(int i = 0; i < edges.size(); i++)
    consolePrint << endl << "\t" << edges.at(i)->getEdgeStringProperty("name") << ": \tmp=" << x[i] << ", \tf=" << f[i];
  for(int i = 0; i < nodes.size(); i++)
    consolePrint << endl << "\t" << nodes.at(i)->getName() << ": \tp =" << x[edges.size() + i] << ", \tf=" << f[edges.size() + i];
  return consolePrint.str();
}
