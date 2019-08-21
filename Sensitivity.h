/*===================================================================*\
                              Sensitivity
                            ---------------

  Derived from HydraulicSolver class. This class is capable of
  calculating the sensitivity to a paramter on a per pipe base.
  Parameter can be Pipe diameter, roughness or Node demand.
 
  staci3 is using Eigen, see http://eigen.tuxfamily.org

    staci3
    Cs. Hos, R. Weber, T. Huzsvar
    https://github.com/weberrichard/staci3
\*==================================================================*/
#ifndef SENSITIVITY_H
#define SENSITIVITY_H

#include "HydraulicSolver.h"

class Sensitivity : public HydraulicSolver
{
public:
  MatrixXd massFlowRateSensitivity; // Sensitivity Matrix
  MatrixXd pressureSensitivity; // Sensitivity Matrix
  
	Sensitivity(string spr_filename);
	~Sensitivity();

  /*! Calculating the sensitivity on a per-pipe basis.
      Paramter can be either Pipe diameter, roughness or Node demand.
      diameter | friction_coeff | demand */
	bool calculateSensitivity(string parameter);
	
private:
  //SparseMatrix<double, ColMajor> functionDerivative; // derivative of eqs. w.r.t. parameter
  //MatrixXd resultDerivative; // derivative of eqs. w.r.t. parameter
  SparseVector<double> functionDerivative; // derivative of eqs. w.r.t. parameter
  VectorXd resultDerivative; // derivative of eqs. w.r.t. parameter
};

#endif