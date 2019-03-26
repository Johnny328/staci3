/*===================================================================*\
                            HydraulicSolver
                            ---------------

  This class is derived from the basic Staci class and provides
  single steady-state hydraulic calculations by solving the 
  continouity and energy equations in the form of a non-linear,
  algebraic f(x) = 0, where x contains the unknown pressures and
  flow rates. We are using Newton iteration, see 
  https://en.wikipedia.org/wiki/Newton_method
 
  staci3 is using Eigen, see http://eigen.tuxfamily.org

    staci3
    Cs. Hos, R. Weber, T. Huzsvar
    https://github.com/weberrichard/staci3
\*==================================================================*/

#include "Staci.h"

class HydraulicSolver : public Staci
{
public:

	HydraulicSolver(string spr_filename);
	~HydraulicSolver();

	/// Fills up the unknown variables pressure, velovity
	bool solveSystem();

	/*! Giving initial values for unknows (i.e. x vector)
			If isInitialization (found in Staci class) variable is false
			pressureInitial and massFlowInitial will be used, if it is true,
			the initialization (found in Staci class) file will be used.*/
	void initialization();

	/*! Giving initial values for unknows (i.e. x vector) using another
			Staci type objective.*/
	void initialization(const Staci * IniStaci);

	/// Printing the solution to consol and log file (for debugging)
	string listResult();

protected:
	/// Jacobian matrix in a sparse Eigen type container
	/// Used in f(x) = 0, and also in Sensitivity class
  SparseMatrix<double, ColMajor> jacobianMatrix; 

private:
  double relaxationFactor, relaxationFactorIncrement, minRelaxationFactor, maxRelaxationFactor;
  double maxMassFlowError, maxPressureError;
  double pressureInitial, massFlowInitial;
  int maxIterationNumber;
  string frictionModel; // Darcy-Weisbach or Hazen-Williams

  /// Printing the basic iteration informaition to console and log file
	string iterInfo(const VectorXd &x,const VectorXd &f, int iter, double e_mp, double e_p);

	/// Linear solver with Newton's technique
  bool linearSolver(VectorXd &x, VectorXd &f);

  /// Updating the Jacobian matrix and relaxation factor
	void updateJacobian(VectorXd &x, VectorXd &f, int iter);
	void updateRelaxationFactor(double e_mp, double e_p, double & e_mp_r, double & e_p_r);

	/// Computing the change in x, explicitly in pressure and massflowrate
	void computeError(const VectorXd &f, double & e_mp, double & e_p, double & e_mp_r, double & e_p_r, bool & konv_ok);

	/// For debugging, debugLevel > TODO
	void printJacobian();
	/// For debugging, debugLevel > TODO
	void printWorstIter(const VectorXd &x, const VectorXd &f , const int a_debug_level);
};