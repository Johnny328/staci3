#include "Calibration.h"

using namespace Eigen;

class SVDCalibration : public Calibration
{
public:
	SVDCalibration(string spr_filename);
	~SVDCalibration();
	int Calibrate(const vector<double> &fric_est, double tol);
  MatrixXd ConstructSM4Calibration(const vector<double> &fric, MatrixXd &calculatedPressure);
  void EigenSVD(const MatrixXd &A, double &tol, MatrixXd &U, MatrixXd &S, MatrixXd &V);
  MatrixXd PinvSVD(const MatrixXd &U, const MatrixXd &S, const MatrixXd &V);
  int RankSVD(const MatrixXd &S);
  double DetSVD(const MatrixXd &S);
  double TraceSVD(const MatrixXd &S);
};