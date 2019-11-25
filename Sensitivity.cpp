#include "Sensitivity.h"

Sensitivity::Sensitivity(string fileName) : HydraulicSolver(fileName){}
Sensitivity::~Sensitivity(){}

//-----------------------------------------------------------------
bool Sensitivity::calculateSensitivity(string parameter)
{
  bool convergence = solveSystem();

  if(convergence){

    if(parameter == "diameter" || parameter == "roughness")
    {
      int parNumber;
      if(parameter == "roughness")
        parNumber = 0;
      else if(parameter == "diameter")
        parNumber = 1;

      massFlowRateSensitivity = MatrixXd::Zero(numberEdges,numberEdges);
      pressureSensitivity = MatrixXd::Zero(numberNodes,numberEdges);
      for(int i=0; i<numberEdges; i++)
      {
        SparseVector<double> funcParDer;
        funcParDer.resize(numberNodes + numberEdges);
        funcParDer.coeffRef(i) = edges[i]->functionParameterDerivative(parNumber);
        VectorXd resultDerivative = solver.solve(-funcParDer);
        massFlowRateSensitivity.col(i) = resultDerivative.head(numberEdges);
        pressureSensitivity.col(i) = resultDerivative.tail(numberNodes);
      }
    }
    else if(strcmp(parameter.c_str(), "demand") == 0)
    {
      massFlowRateSensitivity = MatrixXd::Zero(numberEdges,numberNodes);
      pressureSensitivity = MatrixXd::Zero(numberNodes,numberNodes);
      for(int i=0; i<numberNodes; i++)
      {
        SparseVector<double> funcParDer;
        funcParDer.resize(numberNodes + numberEdges);
        funcParDer.coeffRef(numberEdges + i) = nodes[i]->functionParameterDerivative(isPressureDemand);
        VectorXd resultDerivative = solver.solve(-funcParDer);
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