#include "Sensitivity.h"

Sensitivity::Sensitivity(string spr_filename) : HydraulicSolver(spr_filename){}
Sensitivity::~Sensitivity(){}

//-----------------------------------------------------------------
/// NEW sensitivity calculation: as a side product of solveSystem
/// made by WR
bool Sensitivity::calculateSensitivity(string parameter, int scale){
  bool konv_ok = solveSystem();

  propertyID = parameter;
  int is_edge_prop = -1;
  int numberNodes = nodes.size(), numberEdges = edges.size();

  if(strcmp(propertyID.c_str(), "diameter") == 0 || strcmp(propertyID.c_str(), "friction_coeff") == 0)
  {
    is_edge_prop = 0;
    massFlowRateSensitivity = MatrixXd::Zero(numberEdges,numberEdges);
    pressureSensitivity = MatrixXd::Zero(numberNodes,numberEdges);
  }

  if(strcmp(propertyID.c_str(), "demand") == 0)
  {
    is_edge_prop = 1;
    massFlowRateSensitivity = MatrixXd::Zero(numberEdges,numberNodes);
    pressureSensitivity = MatrixXd::Zero(numberNodes,numberNodes);
  }

  if(is_edge_prop == -1)
  {
    cout << "\n\n ERROR!\n\ncalculateSensitivity(string parameter) -> unknown parameter: " << parameter;
    cout << "\n possible values: diameter|friction_coeff|demand" << endl << endl;
    exit(-1);
  }

  int numberEquations = numberNodes + numberEdges;

  resultDerivative = VectorXd::Zero(numberEquations);
  functionDerivative = VectorXd::Zero(numberEquations);
  SparseLU<SparseMatrix<double, ColMajor> > solver;
  solver.analyzePattern(jacobianMatrix);
  solver.factorize(jacobianMatrix);

  // Edge property was selected
  if(is_edge_prop == 0)
  {
    for(int i=0; i<numberEdges; i++)
    {
      elementID = edges[i]->getName();

      calculateFunctionDerivative();

      resultDerivative = solver.solve(-functionDerivative);

      for(int i=0; i<numberEquations; i++)
        if(isnan(resultDerivative(i))){
          cout << "\n!!!!\nSensitivity.cpp, dxdmu() -> x[" << i << "]=NaN!!!\nName: " << edges.at(i-nodes.size())->getName() << "   Type: " << edges.at(i-nodes.size())->getType() << endl;
        }

      massFlowRateSensitivity.col(i) = resultDerivative.head(numberEdges);
      pressureSensitivity.col(i) = resultDerivative.tail(numberNodes);
    }
  }

  // Node property was selected
  if(is_edge_prop == 1)
  {
    for (int i = 0; i < nodes.size(); i++)
    {
      elementID = nodes.at(i)->getName();

      calculateFunctionDerivative();

      resultDerivative = solver.solve(-functionDerivative);

      for(int i=0; i<numberEquations; i++)
        if(isnan(resultDerivative(i))){
          cout << "\n\n!!!!\nSensitivity.cpp, dxdmu() -> x[" << i << "]=NaN!!!\n\n";
          break;
        }

      massFlowRateSensitivity.col(i) = resultDerivative.head(numberEdges);
      pressureSensitivity.col(i) = resultDerivative.tail(numberNodes);
    }
  }

  return konv_ok;
}

//--------------------------------------------------------------
void Sensitivity::calculateFunctionDerivative(){
  bool gotit = false;

  for(int i=0; i<edges.size(); i++)
  {
    if ((propertyID == "diameter" || propertyID == "friction_coeff") && elementID == edges.at(i)->getName())
    {
      double dfdmu = edges.at(i)->getFunctionDerivative(propertyID);
      functionDerivative(i) = dfdmu;
      gotit = true;
    }
  }

  for(int i=0; i<nodes.size(); i++)
  {
    if(propertyID == "demand" && elementID == nodes.at(i)->getName())
    {
      functionDerivative(edges.size() + i) = -1.0;
      gotit = true;
    }
  }

  if (!gotit) {
    stringstream strstrm;
    strstrm.str("");
    strstrm << "\nHydraulicSolver::calculateFunctionDerivative(): !!! elementID: " << elementID
            << ", propertyID: " << propertyID
            << " not found (propertyID = demand | diameter)!";
    writeLogFile(strstrm.str(), 1);
    cout << strstrm.str();
    StaciException hiba(strstrm.str());
    throw hiba;
  }
}