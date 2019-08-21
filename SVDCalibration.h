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
	*  fric_est:		starting point of the calibration
	*  tol:				NEW: tol determine the part of trace that will be counted || OLD: if tol>0, under tol every singular value will be 0; but if tol<0, the cut is under the largest singular value diveded with (-tol)
	*  return:     0: no convergence, 1: full convergence (errorPressure~0), 2: local convergence (d(errorPressure)>0)*/
	int calibrate(const vector<double> &fric_est, double tol);

	/*! WR: Singular value decomposition using Eigen
	*  A:    matrix for SVD
	*  tol:  NEW: tol determine the part of trace that will be counted || OLD: if tol>0, under tol every singular value will be 0; but if tol<0, the cut is under the largest singular value diveded with (-tol)
	*  U,S,V: returnung matrices*/
  void eigenSVD(const MatrixXd &A, double &tol, MatrixXd &U, MatrixXd &S, MatrixXd &V);

	/*! WR: Pseudoinvere (or normal inverse) using the SVD method (see Numerical Recipes)
  *U,S,V:   coming from as an output from eigenSVD() function
  *return:  Pseudoinverse matrix*/
  MatrixXd pinvSVD(const MatrixXd &U, const MatrixXd &S, const MatrixXd &V);

  /*! WR: Determines the rank (i.e. linearly independent vectors ) of a matrix using SVD (see Numerical Recipes)
  *S:       S matrix coming from eigenSVD()
  *return:  rank of the matrix*/
  int rankSVD(const MatrixXd &S);

	/*! WR: Determinant (i.e. product of non-zero eigen values) of a matrix
  *S:       coming from eigenSVD()
  *return:  det of the matrix*/
  double detSVD(const MatrixXd &S);

  /*! WR: Trace (i.e. sum of the eigen values) of a matrix
  *S:       coming from eigenSVD()
  *return:  det of the matrix*/
  double traceSVD(const MatrixXd &S);

  //========================
  //GETSETGETSETGETSETGETSET
  //========================
  void setDebugLevelSVD(int a)
  {
    debugLevelSVD = a;
  }

private:
		/*! WR: Building up the reduced sensitivity matrix for SVDCalibrate
	*  calculatedPressure:		giving back the calculated pressures for SVDCalibrate
	*	return:		reduced sensitivity matrix*/
  MatrixXd constructSensitivity(const vector<double> &fric, MatrixXd &calculatedPressure);

  /// Debugging the SVDCalibration class
  int debugLevelSVD;
};

#endif
