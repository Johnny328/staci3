#include "HydraulicSolver.h"

struct val_and_ID {
  double val, x;
  string ID;
  bool operator>(const val_and_ID& rhs) { return val > rhs.val; }
  bool operator>=(const val_and_ID& rhs) { return val >= rhs.val; }
  bool operator<(const val_and_ID& rhs) { return val < rhs.val; }
  bool operator<=(const val_and_ID& rhs) { return val <= rhs.val; }
};

bool comparison_function1(const val_and_ID& lhs, const val_and_ID& rhs ) { return lhs.val > rhs.val;
}

HydraulicSolver::HydraulicSolver(string spr_filename) : Staci(spr_filename){
  //maxIterationNumber = atoi(IOxml.read_setting("maxIterationNumber").c_str());
  //maxPressureError = atof(IOxml.read_setting("maxPressureError").c_str());
  //maxMassFlowError = atof(IOxml.read_setting("maxMassFlowError").c_str());
  //relaxationFactor = atof(IOxml.read_setting("relax").c_str());
  //relaxationFactorIncrement = atof(IOxml.read_setting("relax_mul").c_str());
  //massFlowInitial = atof(IOxml.read_setting("massFlowInitial").c_str());
  //pressureInitial = atof(IOxml.read_setting("pressureInitial").c_str());
  //frictionModel = IOxml.read_setting("frictionModel").c_str();
  maxIterationNumber = 1e2;
  maxPressureError = 0.0001;
  maxMassFlowError = 0.01;
  relaxationFactor = 1.;
  relaxationFactorIncrement = 1.2;
  massFlowInitial = 100.0;
  pressureInitial = 5.0;
  frictionModel = "HW";

  if (frictionModel == "nincs ilyen node!") {
    frictionModel = "DW";
    cout << endl << endl << "******** WARNING! ********";
    cout << endl << "<frictionModel>DW|HW</frictionModel> node not found!";
    cout << endl
         << "Setting DW friction model for all pipes." << endl
         << endl;
  }
}

HydraulicSolver::~HydraulicSolver(){}

//--------------------------------------------------------------
void HydraulicSolver::updateJacobian(VectorXd &x, VectorXd &f, int iter) {
  int n = nodes.size() + edges.size();

  if(iter == 0)
  {
    jacobianMatrix.resize(n,n);
    jacobianMatrix.reserve(VectorXi::Constant(n,10));
  }

  for (int i = 0; i < edges.size(); i++) {
    x(i) = edges[i]->getMassFlowRate();
  }
  for (int i = 0; i < nodes.size(); i++) {
    x(edges.size() + i) = nodes[i]->getHead();
  }
  // f es Jacobi kiertekelese az aktualis adatokkal
  int Q_indx, pe_indx, pv_indx;
  vector<double> pevhev(4, 0.0);
  vector<double> jv;

  for (unsigned int i = 0; i < edges.size(); i++) {
    if (edges[i]->getNumberNode() == 1)
    {
      pevhev[0] = nodes[edges[i]->getStartNodeIndex()]->getHead();
      pevhev[1] = 0.0;
      pevhev[2] = nodes[edges[i]->getStartNodeIndex()]->getGeodeticHeight();
      pevhev[3] = 0.0;
    } 
    else 
    {
      pevhev[0] = nodes[edges[i]->getStartNodeIndex()]->getHead();
      pevhev[1] = nodes[edges[i]->getEndNodeIndex()]->getHead();
      pevhev[2] = nodes[edges[i]->getStartNodeIndex()]->getGeodeticHeight();
      pevhev[3] = nodes[edges[i]->getEndNodeIndex()]->getGeodeticHeight();
    }
    f(i) = edges[i]->function(pevhev);
    jv = edges[i]->functionDerivative(pevhev);

    Q_indx = i;
    pe_indx = edges[i]->getStartNodeIndex();
    pv_indx = edges[i]->getEndNodeIndex();

    if(iter>0)
      jacobianMatrix.coeffRef(i,edges.size() + pe_indx)=jv.at(0);
    else
      jacobianMatrix.insert(i,edges.size() + pe_indx)=jv.at(0);

    if (edges[i]->getNumberNode() == 2) {
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

  for (int i = 0; i < nodes.size(); i++)
  {
    f(edges.size() + i) = -nodes[i]->getDemand();
    for (int j = 0; j < nodes[i]->edgeIn.size(); j++)
    {
      f(edges.size() + i) += edges[nodes[i]->edgeIn.at(j)]->getMassFlowRate();
      if(iter>0)
        jacobianMatrix.coeffRef(edges.size() + i,nodes[i]->edgeIn.at(j)) = +1.0;
      else
      jacobianMatrix.insert(edges.size() + i,nodes[i]->edgeIn.at(j)) = +1.0;
    }
    for (int j = 0; j < nodes[i]->edgeOut.size(); j++)
    {
      f(edges.size() + i) -= edges[nodes[i]->edgeOut.at(j)]->getMassFlowRate();
      if(iter>0)
        jacobianMatrix.coeffRef(edges.size() + i,nodes[i]->edgeOut.at(j)) = -1.0;
      else
        jacobianMatrix.insert(edges.size() + i,nodes[i]->edgeOut.at(j)) = -1.0;
    }
  }

  if(getDebugLevel() > 6)
    printJacobian();

}

//--------------------------------------------------------------
bool HydraulicSolver::solveSystem() {
  const int n = nodes.size() + edges.size();
  VectorXd b(n), x(n), dx(n), f(n), xu(n);
  int iter = 0;
  double e_mp = 1e10, e_p = 1e10, e_mp_r = 1e10, e_p_r = 1e10;
  bool konv_ok = false;

  ostringstream consolePrint;
  consolePrint.str("");
  consolePrint << "\n\nSolving system...\n====================================" << endl;

  if (getDebugLevel() > 0)
    writeLogFile(consolePrint.str(), 3);

  bool comp_ok = true;
  while ((iter < maxIterationNumber + 1) && (!konv_ok)) {

    updateJacobian(x, f, iter);

    computeError(f, e_mp, e_p, e_mp_r, e_p_r, konv_ok);

    writeLogFile(iterInfo(x, f, iter, e_mp, e_p), 3);

    printWorstIter(x, f, 3);

    updateRelaxationFactor(e_mp, e_p, e_mp_r, e_p_r);

    comp_ok = linearSolver(x, f); // TODO handling comp_ok

    if ((e_mp < maxMassFlowError) && (e_p < maxPressureError))
      konv_ok = true;

    consolePrint.str("");
    for (unsigned int i = 0; i < edges.size(); i++)
      consolePrint << endl << "\t" << edges.at(i)->getName() << ": \tmp=" << x[i] << ", \tf=" << f[i];
    for (unsigned int i = 0; i < nodes.size(); i++)
      consolePrint << endl << "\t" << nodes.at(i)->getName() << ": \tp =" << x[edges.size() + i] << ", \tf=" << f[edges.size() + i];
    writeLogFile(consolePrint.str(), 4);

    if(iter > 100)
      exit(-1);
    iter++;
  }

  if (!konv_ok)
    printWorstIter(x, f, 1);

  return konv_ok;
}

//--------------------------------------------------------------
bool HydraulicSolver::linearSolver(VectorXd &x, VectorXd &f) {
  int n = edges.size() + nodes.size();
  VectorXd dx(n);
  dx = VectorXd::Zero(n);
  SparseLU<SparseMatrix<double, ColMajor> > solver;
  solver.analyzePattern(jacobianMatrix);
  solver.factorize(jacobianMatrix);
  dx = solver.solve(f);

  bool success = true;
  for(int i=0; i<n; i++)
    if(isnan(dx[i])) {
      success = false;
      break;
    }

  if(success)
  {
    for(unsigned int i=0; i<edges.size(); i++)
      edges[i]->setMassFlowRate(x(i) - relaxationFactor * dx(i));
    for(unsigned int i=0; i<nodes.size(); i++)
      nodes[i]->setHead(x(edges.size() + i) - relaxationFactor*dx(edges.size()+i));
  }

  return success;
}

// -----------------------------------------
void HydraulicSolver::printWorstIter(const VectorXd &x, const VectorXd &f , const int a_debug_level) {

  vector<val_and_ID> v_edges;
  vector<val_and_ID> v_nodes;
  val_and_ID s;

  for (int i = 0 ; i < edges.size(); i++) {
    s.val =  fabs(f(i));
    s.x =  x(i);
    s.ID = edges.at(i)->getName().c_str();
    v_edges.push_back(s);
  }

  for (int i = 0 ; i < nodes.size(); i++) {
    s.val =  fabs(f(i + edges.size()));
    s.x =  x(i + edges.size());
    s.ID = nodes.at(i)->getName().c_str();
    v_nodes.push_back(s);
  }

  sort(v_edges.begin(), v_edges.end(), comparison_function1);
  sort(v_nodes.begin(), v_nodes.end(), comparison_function1);

  int N_to_print = 10;
  if (N_to_print > edges.size())
    N_to_print = edges.size();
  if (N_to_print > nodes.size())
    N_to_print = nodes.size();

  ostringstream consolePrint;
  consolePrint << "\n\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\n";
  consolePrint << "The " << N_to_print << " worst edges:";
  for (int i = 0; i < N_to_print; i++)
    consolePrint << "\n\t " << v_edges.at(i).ID.c_str() << ": mp=" << v_edges.at(i).x << ", f=" << v_edges.at(i).val;
  consolePrint << "\nThe " << N_to_print << " worst nodes:";
  for (int i = 0; i < N_to_print; i++)
    consolePrint << "\n\t " << v_nodes.at(i).ID.c_str() << ": p =" << v_nodes.at(i).x << ", f=" << v_nodes.at(i).val;
  consolePrint << "\n\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";

  writeLogFile(consolePrint.str(), a_debug_level);

}

//--------------------------------------------------------------
void HydraulicSolver::initialization() {
  ostringstream strstrm;
  if (!getIsInitialization()) {
    strstrm << endl << " Automatic inicialization...";

    strstrm << endl << "\tnodal pressures:\t" << pressureInitial << " mwc ";
    for (unsigned int i = 0; i < nodes.size(); i++)
      nodes.at(i)->initialization(1., pressureInitial);

    strstrm << endl << "\tedge mass flow rates:\t" << massFlowInitial << " kg/s ...";
    for (unsigned int i = 0; i < edges.size(); i++)
      edges.at(i)->initialization(1., massFlowInitial);

  } else {
    strstrm << endl
            << endl
            << "loading initial values from file " << getInitializationFile() << endl;
    IOxml IOxml(getInitializationFile().c_str());
    IOxml.load_ini_values(nodes, edges);
  }

  if (getDebugLevel() > 0) {
    writeLogFile(strstrm.str(), 1);
  }

}

//--------------------------------------------------------------
void HydraulicSolver::initialization(const Staci * IniStaci) {
  for (unsigned int i = 0; i < nodes.size(); i++)
    nodes.at(i)->setHead(IniStaci->nodes.at(i)->getHead());

  for (unsigned int i = 0; i < edges.size(); i++)
    edges.at(i)->setMassFlowRate(IniStaci->edges.at(i)->getMassFlowRate());
}

//--------------------------------------------------------------
void HydraulicSolver::printJacobian(){
  ostringstream strstrm;
  string strstrm_nev;
  const unsigned int MAX_NEV_HOSSZ = 15;

  strstrm.str("");
  strstrm << scientific << setprecision(3) << showpos;
  strstrm << endl << "\nJACOBI:" << endl << "          ";

  for (unsigned int i = 0; i < edges.size(); i++) {
    strstrm_nev = edges[i]->getName();
    while (strstrm_nev.size() < MAX_NEV_HOSSZ) strstrm_nev.append(" ");
    strstrm << "\tmp," << strstrm_nev;
  }
  for (unsigned int i = 0; i < nodes.size(); i++) {
    strstrm_nev = nodes[i]->getName();
    while (strstrm_nev.size() < MAX_NEV_HOSSZ) strstrm_nev.append(" ");
    strstrm << "\t p," << strstrm_nev;
  }
  strstrm << endl;

  for (unsigned int i = 0; i < edges.size(); i++) {
    strstrm_nev = edges[i]->getName();
    while (strstrm_nev.size() < MAX_NEV_HOSSZ) strstrm_nev.append(" ");
    strstrm << strstrm_nev;
    for (unsigned int j = 0; j < edges.size() + nodes.size(); j++)
      //strstrm << "; " << jacobianMatrix(i,j);
    strstrm << endl;
  }
  for (unsigned int i = 0; i < nodes.size(); i++) {
    strstrm_nev = nodes[i]->getName();
    while (strstrm_nev.size() < MAX_NEV_HOSSZ) strstrm_nev.append(" ");
    strstrm << strstrm_nev;
    for (unsigned int j = 0; j < edges.size() + nodes.size(); j++)
      //strstrm << ";" << jacobianMatrix(edges.size() + i,j);
    strstrm << endl;
  }

  ofstream JacFile;
  JacFile.open("dfdx.txt");
  JacFile << strstrm.str();
  JacFile.close();
  //  writeLogFile(strstrm.str(), 3);
}

//--------------------------------------------------------------
string HydraulicSolver::iterInfo(const VectorXd &x,const VectorXd &f, int iter, double e_mp, double e_p) {
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
void HydraulicSolver::computeError(const VectorXd &f, double & e_mp, double & e_p, double & e_mp_r, double & e_p_r, bool & konv_ok) {
  e_mp_r = e_mp;
  e_p_r = e_p;
  e_mp = 0.0;
  e_p = 0.0;
  for (unsigned int i = 0; i < edges.size(); i++) e_p += f[i] * f[i];
  for (unsigned int i = 0; i < nodes.size(); i++)
    e_mp += f(edges.size() + i) * f(edges.size() + i);

  e_mp = pow(e_mp, 0.5);
  e_p = pow(e_p, 0.5);

  if ((e_p < maxPressureError) && (e_mp < maxMassFlowError)) konv_ok = true;
}

//--------------------------------------------------------------
void HydraulicSolver::updateRelaxationFactor(double e_mp, double e_p, double & e_mp_r, double & e_p_r) {
  minRelaxationFactor = 0.01;
  maxRelaxationFactor = 1.0;
  double hiba, hiba_r;
  if (e_p > e_mp) {
    hiba = e_p;
    hiba_r = e_p_r;
  } else {
    hiba = e_mp;
    hiba_r = e_mp_r;
  }
  if (hiba < hiba_r)
    relaxationFactor = relaxationFactor * relaxationFactorIncrement;
  else
    relaxationFactor = relaxationFactor / 10.;
  if (relaxationFactor < minRelaxationFactor) relaxationFactor = minRelaxationFactor;
  if (relaxationFactor > maxRelaxationFactor) relaxationFactor = maxRelaxationFactor;

  e_mp_r = e_mp;
  e_p_r = e_p;
}

//--------------------------------------------------------------
string HydraulicSolver::listResult() {
  ostringstream strstrm;

  // string pot;
  // const
  // int string_hossz = 10;

  strstrm << scientific << setprecision(3) << showpos;
  strstrm << endl << endl << "EREDMENYEK:";
  for (unsigned int i = 0; i < edges.size(); i++)
    strstrm << endl
            << "\t" << edges[i]->getName()
            << ":\tmp=" << edges[i]->getMassFlowRate() << " kg/s"
            << "\tQ=" << (3600 * (edges[i]->getVolumeFlowRate())) << " m3/h"
            << "\tv=" << edges[i]->getVelocity() << " m/s";
  strstrm
      << endl
      << "\t"
      << "-----------------------------------------------------------------";
  for (unsigned int i = 0; i < nodes.size(); i++)
    strstrm << endl
            << "\t" << nodes[i]->getName()
            << ":\t p=" << nodes[i]->getHead() * 1000 * 9.81 / 1e5 << " bar"
            << "\tH=" << nodes[i]->getHead() << " m"
            << ",   H+magassag=" << nodes[i]->getHead() + nodes[i]->getGeodeticHeight()
            << " m";
  strstrm << endl << endl;
  return strstrm.str();
}