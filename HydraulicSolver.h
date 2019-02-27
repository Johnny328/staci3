#include "Staci.h"

class HydraulicSolver : public Staci
{
public:
  double pressureInitial, massFlowInitial;
  int maxIterationNumber;
  double maxMassFlowError, maxPressureError;
  double relaxationFactor, relaxationFactorIncrement, minRelaxationFactor, maxRelaxationFactor;
  string frictionModel;
  SparseMatrix<double, ColMajor> jacobianMatrix;

	HydraulicSolver(string spr_filename);
	~HydraulicSolver();
	void updateJacobian(VectorXd &x, VectorXd &f, int iter);
	bool solveSystem();
	void printWorstIter(const VectorXd &x, const VectorXd &f , const int a_debug_level);
	void initialization();
	void initialization(const Staci * IniStaci);
	bool linearSolver(VectorXd &x, VectorXd &f);
	void printJacobian();
	string iterInfo(const VectorXd &x,const VectorXd &f, int iter, double e_mp, double e_p);
	void computeError(const VectorXd &f, double & e_mp, double & e_p, double & e_mp_r, double & e_p_r, bool & konv_ok);
	void updateRelaxationFactor(double e_mp, double e_p, double & e_mp_r, double & e_p_r);
	string listResult();
	void saveResult(bool conv_reached);
};