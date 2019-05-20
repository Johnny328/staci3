#include "Sensitivity.h"

Sensitivity::Sensitivity(string spr_filename) : HydraulicSolver(spr_filename){}
Sensitivity::~Sensitivity(){}

//-----------------------------------------------------------------
/// NEW sensitivity calculation: as a side product of solveSystem
bool Sensitivity::calculateSensitivity(string parameter){
  bool convergence = solveSystem();

  if(convergence){

    int numberNodes = openNodes.size(), numberEdges = openEdges.size();
    int n = numberNodes + numberEdges;
    functionDerivative.resize(n);

    if(strcmp(parameter.c_str(), "diameter") == 0 || strcmp(parameter.c_str(), "friction_coeff") == 0)
    { 
      massFlowRateSensitivity = MatrixXd::Zero(numberEdges,numberEdges);
      pressureSensitivity = MatrixXd::Zero(numberNodes,numberEdges);
      for(int i=0; i<numberEdges; i++){
        functionDerivative.setZero();
        functionDerivative.coeffRef(i) = edges[openEdges[i]]->functionParameterDerivative(parameter);
        resultDerivative = solver.solve(-functionDerivative);
        massFlowRateSensitivity.col(i) = resultDerivative.head(numberEdges);
        pressureSensitivity.col(i) = resultDerivative.tail(numberNodes);
      }
    }
    else if(strcmp(parameter.c_str(), "demand") == 0)
    {
      massFlowRateSensitivity = MatrixXd::Zero(numberEdges,numberNodes);
      pressureSensitivity = MatrixXd::Zero(numberNodes,numberNodes);
      for(int i=0; i<numberNodes; i++){
        functionDerivative.setZero();
        functionDerivative.coeffRef(numberEdges + i) = -1.0;
        resultDerivative = solver.solve(-functionDerivative);
        massFlowRateSensitivity.col(i) = resultDerivative.head(numberEdges);
        pressureSensitivity.col(i) = resultDerivative.tail(numberNodes);
      }
    }
    else
    {
      cout << "\n\n !!!WARNING!!! calculateSensitivity(string parameter) -> unknown parameter: " << parameter;
      cout << "\n Available values: diameter | friction_coeff | demand";
      cout << "\n Skipping sensitivity calculations, then continouing ..." << endl << endl;
    }

  } // end of if(convergence)
  else
    cout << endl << "[*] Sensitivity (" << parameter << "): hydraulic solver has NOT convergenved :(" << endl;

  return convergence;
}

/*    functionDerivative.resize(n,m);
    functionDerivative.reserve(VectorXi::Constant(n,1));

    //resultDerivative = MatrixXd::Zero(n,n);

    // Edge property was selected
    if(is_edge_prop == 0)
    {
      for(int i=0; i<numberEdges; i++)
      {
        functionDerivative.coeffRef(i,i) = edges[openEdges[i]]->functionParameterDerivative(parameter);
      }
    }

    // Node property was selected
    if(is_edge_prop == 1)
    {
      for (int i = 0; i < numberNodes; i++)
      {
        functionDerivative.coeffRef(numberEdges+i,i) = -1.0;
      }
    }
    resultDerivative = solver.solve(-functionDerivative);

    massFlowRateSensitivity = resultDerivative.topRows(numberEdges);
    pressureSensitivity = resultDerivative.bottomRows(numberNodes);
*/