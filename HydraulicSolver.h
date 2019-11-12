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

#ifndef HYDRAULICSOLVER_H
#define HYDRAULICSOLVER_H

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
	void initialization(const Staci * inStaci);

	/// Printing the solution to consol (for debugging)
	void listResult();

  /// density of the fluid
  double density = 1000.0;

  /// Is the demands depending on pressure
  bool isPressureDemand = false;

protected:
	/// Jacobian matrix in a sparse Eigen type container
	/// Used in f(x) = 0, and also in Sensitivity class
  SparseMatrix<double, ColMajor> jacobianMatrix;
  int maxRank; // max rank of jacobian
  
  // Eigen Sparse LU decomposition solver
  SparseLU<SparseMatrix<double, ColMajor> > solver;

  /// Maximal accepted error of pressure and mass flow rate
  /// in case of hydraulic solver (2* in case of SVD calibration)
  double maxMassFlowError, maxPressureError;

  /// Storing the number of edges and nodes, and their sum as numberEquations
  int numberEquations;

  /// numeric tolerance for status updates
  double volumeFlowRateTolerance, headTolerance;

  /// Building up the Jacobian matrix (also calculating the function f(x)) before the Newtonian iteration
  void buildJacobian();
  // Eigen vectors for the solver
  VectorXd x, f;
  
private:
  double relaxationFactor, relaxationFactorIncrement, relaxationFactorDecrement;
  double minRelaxationFactor = 0.1, maxRelaxationFactor = 1.0;
  int maxIterationNumber;
  string frictionModel; // Darcy-Weisbach (DW) or Hazen-Williams (HW)

	/// Linear solver Jac*x = -f
  void linearSolver();

  /// Updateing up the Jacobian matrix (also calculating the function f(x)) before the Newtonian iteration
	void updateJacobian();
  VectorXd getEdgeFunction(int i);
  VectorXd getNodeFunction(int i);

  /// Updating relaxation factor for the Newtonian iteration
	void updateRelaxationFactor(double e_mp, double e_p, double & e_mp_r, double & e_p_r);

	/// Computing the change in x, explicitly in pressure and massflowrate
	void computeError(const VectorXd &f, double & e_mp, double & e_p, double & e_mp_r, double & e_p_r, bool & konv_ok);

	/// Printing the basic iteration informaition to console and log file
	void iterInfo(int iter, double e_mp, double e_p, vector<int> idx);
  /// Printing the detailed iteration informaition to console and log file
  void iterInfoDetail(const VectorXd &f);

  /// Checking the pump operational points, whether they are on the curve
  void checkPumpOperatingPoint();
  
  // Updating the status of edges (pipe with CV, flow control valves, etc.)
  vector<int> edgeStatusUpdate(); // Calls all the below functions
  // If node is only connected with closed edges, the node will be closed as well
  void closeIsolatedNode(int idx);

  // Checking whether there are a zero norm column in the jacobianMatrix
  // Can be used for debugging of the Eigen errors
  void checkJacobianMatrix();
};

#endif