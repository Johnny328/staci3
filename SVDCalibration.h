/*===================================================================*\
                            SVDCalibration
                            ---------------

  Derived from Calibration class. Calibration technique on a per-pipe
  based i.e. the pipes are used independently from each other.
  See journal paper XY TODO
 
  staci3 is using Eigen, see http://eigen.tuxfamily.org

    staci3
    Cs. Hos, R. Weber, T. Huzsvar
    https://github.com/weberrichard/staci3
\*==================================================================*/

#ifndef SVDCALIBRATION_H
#define SVDCALIBRATION_H

#include "Calibration.h"

using namespace Eigen;

class SVDCalibration : public Calibration
{
public:
	SVDCalibration(string spr_filename);
	~SVDCalibration();

	/*! WR: Calibration technique using SVD
	*  return:     0: no convergence, 1: full convergence (errorPressure~0), 2: local convergence*/
	int calibrate();

  /// indicies of the measurement that were actually used for SVD calibration
  vector<int> measuredUsedIndex;
  vector<double> measuredUsedTime;
  /// every queue-th measurement is used
  int numberMeasuredUsed;

  /// parameters of the iteration
  int iterMax = 1e2; // max iteration number
  double SVDTolerance = 0.9; // how much part of singular values will be used
  double errorPressureStop = 1e-2; // if max eP change is smaller than this will cause iteration stop
  double dlambdaStop = 1e-2; // if max dlambda change is smaller than this will cause iteration stop
  double maxChange = 10.; // max change allowed in dlambda, above that will rescale vector

	/*! WR: Singular value decomposition using Eigen
	*  A:    matrix for SVD
	*  tol:  NEW: tol determine the part of trace that will be counted || OLD: if tol>0, under tol every singular value will be 0; but if tol<0, the cut is under the largest singular value diveded with (-tol)
	*  U,S,V: returnung matrices*/
  void eigenSVD(const MatrixXd &A, double &tol, MatrixXd &U, MatrixXd &S, MatrixXd &V);

	/*! WR: Pseudoinvere (or normal inverse) using the SVD method (see Numerical Recipes)
  *U,S,V:   coming from as an output from eigenSVD() function
  *return:  Pseudoinverse matrix*/
  MatrixXd pinvSVD(const MatrixXd &U, const MatrixXd &S, const MatrixXd &V);

  /// Debugging the SVDCalibration class
  int printLevelSVD = 0;

  /// struct to control the series sensitivity calculation, see SeriesHydraulis.h
  ssc seriesSensitivityControl;
};

#endif
