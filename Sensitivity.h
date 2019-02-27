#include "HydraulicSolver.h"

class Sensitivity : public HydraulicSolver
{
public:
  MatrixXd massFlowRateSensitivity; // Sensitivity Matrix
  MatrixXd pressureSensitivity; // Sensitivity Matrix
  VectorXd functionDerivative; // derivative of eqs. w.r.t. parameter
  VectorXd resultDerivative; // derivative of eqs. w.r.t. parameter
  
	Sensitivity(string spr_filename);
	~Sensitivity();
	bool calculateSensitivity(string parameter, int scale);
	void calculateFunctionDerivative();
};