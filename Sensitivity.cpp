#include "Sensitivity.h"

Sensitivity::Sensitivity(string spr_filename) : HydraulicSolver(spr_filename){}
Sensitivity::~Sensitivity(){}

//-----------------------------------------------------------------
/// NEW sensitivity calculation: as a side product of solveSystem
bool Sensitivity::calculateSensitivity(string parameter){
  bool convergence = solveSystem();

  if(convergence){

    int is_edge_prop = -1;
    int numberNodes = nodes.size(), numberEdges = edges.size();

    if(strcmp(parameter.c_str(), "diameter") == 0 || strcmp(parameter.c_str(), "friction_coeff") == 0)
    {
      is_edge_prop = 0;
      massFlowRateSensitivity = MatrixXd::Zero(numberEdges,numberEdges);
      pressureSensitivity = MatrixXd::Zero(numberNodes,numberEdges);
    }
    else if(strcmp(parameter.c_str(), "demand") == 0)
    {
      is_edge_prop = 1;
      massFlowRateSensitivity = MatrixXd::Zero(numberEdges,numberNodes);
      pressureSensitivity = MatrixXd::Zero(numberNodes,numberNodes);
    }
    else
    {
      cout << "\n\n !!!WARNING!!! calculateSensitivity(string parameter) -> unknown parameter: " << parameter;
      cout << "\n Available values: diameter | friction_coeff | demand";
      cout << "\n Skipping sensitivity calculations, then continouing ..." << endl << endl;
    }

    int numberEquations = numberNodes + numberEdges;

    functionDerivative = VectorXd::Zero(numberEquations);
    SparseLU<SparseMatrix<double, ColMajor> > solver;
    solver.analyzePattern(jacobianMatrix);
    solver.factorize(jacobianMatrix);

    // Edge property was selected
    if(is_edge_prop == 0)
    {
      for(int i=0; i<numberEdges; i++)
      {
        string elementID = edges[i]->getEdgeStringProperty("name");

        functionDerivative = calculateFunctionDerivative(parameter, elementID);
        
        resultDerivative = VectorXd::Zero(numberEquations);
        resultDerivative = solver.solve(-functionDerivative);

        massFlowRateSensitivity.col(i) = resultDerivative.head(numberEdges);
        pressureSensitivity.col(i) = resultDerivative.tail(numberNodes);
      }
    }

    // Node property was selected
    if(is_edge_prop == 1)
    {
      for (int i = 0; i < numberNodes; i++)
      {
        string elementID = nodes.at(i)->getName();

        functionDerivative = calculateFunctionDerivative(parameter, elementID);
        
        resultDerivative = VectorXd::Zero(numberEquations);
        resultDerivative = solver.solve(-functionDerivative);

        massFlowRateSensitivity.col(i) = resultDerivative.head(numberEdges);
        pressureSensitivity.col(i) = resultDerivative.tail(numberNodes);
      }
    }

  } // end of if(convergence)
  else
    cout << endl << "[*] Sensitivity (" << parameter << "): hydraulic solver has NOT convergenved :(" << endl;

  return convergence;
}

//--------------------------------------------------------------
VectorXd Sensitivity::calculateFunctionDerivative(string parameter, string elementID){

  functionDerivative = VectorXd::Zero(nodes.size()+edges.size());

  for(int i=0; i<edges.size(); i++)
  {
    if ((parameter == "diameter" || parameter == "friction_coeff") && elementID == edges.at(i)->getEdgeStringProperty("name"))
    {
      double dfdmu = edges.at(i)->functionParameterDerivative(parameter);
      functionDerivative(i) = dfdmu;
    }
  }

  for(int i=0; i<nodes.size(); i++)
  {
    if(parameter == "demand" && elementID == nodes.at(i)->getName())
    {
      functionDerivative(edges.size() + i) = -1.0;
    }
  }

  return functionDerivative;
}