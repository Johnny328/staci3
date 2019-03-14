#include "SVDCalibration.h"

using namespace std;
using namespace Eigen;

SVDCalibration::SVDCalibration(string spr_filename) : Calibration(spr_filename){
}
SVDCalibration::~SVDCalibration(){
}

/*! WR: Calibration technique using SVD
     *  fric_est:		starting point of the calibration
     *  tol:				NEW: tol determine the part of trace that will be counted || OLD: if tol>0, under tol every singular value will be 0; but if tol<0, the cut is under the largest singular value diveded with (-tol)
     *  return:     0: no convergence, 1: full convergence (errorPressure~0), 2: local convergence (d(errorPressure)>0)
     */
int SVDCalibration::Calibrate(const vector<double> &fric_est, double tol){

  int numberNodes = nodes.size(), numberEdges = edges.size();

  // Filling up fric var, that will contain the friction at each step
  vector<double> fric(numberEdges,0.), demandNominal(numberNodes,0.);
  for (int k=0;k<numberEdges;k++)
    if(edges[k]->getType() == "Pipe")
      fric[k] = fric_est[k];
  // Saving the nominal demands
  for (int i=0; i<numberNodes;i++)
    demandNominal[i] = nodes[i]->getDemand();

  loadMeasurement(getDefinitionFile());

  // Start of main iteration
  if(getDebugLevel()>0)
    cout << "\n\n---------------------------------\n******Starting Calibration******\n---------------------------------\n\n";
  int iterMax = 1e2; // max iteration number
  double relax = .5, satur = 1.; // relaxation factor, saturation factor
  double max_dlambda = .5; // saturation for dlambda with relaxation
  int konv = 0;
  VectorXd dlambda;
  double e_p_norm=1e10;
  double relax_nom = relax;

  for(int i=0;i<iterMax;i++){
    bool better = false;
    int j = 0;
    int j_max = 4; // max no. of inner iteration
    MatrixXd SMred;
    VectorXd errorPressure(numberOperationalPoint*numberPressure);

    MatrixXd calculatedPressure;
    while(!better && j<j_max){
      // calculating the reduced sensitivity matrix for every case when measurement occured, moreover it gives back the output pressur for the measured nodes in calculatedPressure
      SMred = ConstructSM4Calibration(fric, calculatedPressure);

      for(int k=0; k<numberOperationalPoint; k++)
        errorPressure.segment(k*(numberPressure-1),k*numberPressure-1) = measuredPressure.row(k) - calculatedPressure.row(k);
        //for(int l=0; l<numberPressure; l++)
          //errorPressure(l+k*numberPressure) = measuredPressure(k,l) - calculatedPressure(k,l);
      if(getDebugLevel()>1)
        cout << "e_p_norm_r: " << e_p_norm << "   e_p_norm: " << errorPressure.norm() << endl;

      if(e_p_norm > errorPressure.norm())
        better = true;
      else{
        relax /= 2.;
        for(int k=0; k<numberEdges; k++)
          if(edges[k]->getType() == "Pipe")
            fric[k] -= relax*satur*dlambda(k);
      }
      j++;
    }
    e_p_norm = errorPressure.norm();
    if(j==j_max){
      konv = 2;
      break;
    }

    // performing SVD and calculate dlambda
    MatrixXd U,S,V;
    EigenSVD(SMred,tol,U,S,V);
    MatrixXd Sr_pinv = PinvSVD(U,S,V);
    dlambda = Sr_pinv*errorPressure;

    // Checking the largest relativa change
    double lth=0.;
    for(int k=0; k<numberEdges; k++)
      if(edges[k]->getType() == "Pipe")
        if(abs(relax*dlambda(k)/fric[k])>max_dlambda)
          lth = abs(dlambda(k)/fric[k]);

    satur=1.;
    if(lth>max_dlambda)
      satur = max_dlambda/lth;

    for(int k=0; k<numberEdges; k++)
      if(edges[k]->getType() == "Pipe")
        fric[k] += relax_nom*satur*dlambda(k);

    if(getDebugLevel()>1){
      cout << "\nfrictions:\n";
      for(int k=0; k<numberEdges; k++)
        if(edges[k]->getType() == "Pipe")
          cout << fric[k] << endl;
      cout << endl;
    }

    if(getDebugLevel()>0){
      cout << "\n\n**********Calibration*************";
      printf("\niter: %2i", i);
      printf("  df_n: %2.4e",dlambda.norm()/pow((double)numberEdges,.5));
      printf("  errorPressure: %2.4e", errorPressure.norm()/pow((double)numberNodes,.5));
      printf("  relax:  %1.2f",relax);
      printf("  satur:  %1.2f",satur);
      cout << "\n**********Calibration*************\n\n";
    }
    relax = relax_nom;

    if(errorPressure.norm()/pow((double)numberNodes,.5)<1e-8 || dlambda.norm()/pow((double)numberEdges,.5)<1e-8){
      konv = 1;
      break;
    }
    if(i==iterMax-1 && getDebugLevel()>0)
        cout << "\nWARNING!! Convergence is not sufficient!";
  }
  if(getDebugLevel()>0)
    cout << "\n\n---------------------------------\n******Ending Calibration******\n---------------------------------\n\n";
  if(konv != 0){
    for(int k=0; k<numberEdges; k++)
      if(edges[k]->getType()=="Pipe")
        edges[k]->setProperty("roughness",fric[k]); 
  }else{
  	for(int k=0; k<numberEdges; k++)
      if(edges[k]->getType()=="Pipe")
        edges[k]->setProperty("roughness",fric_est[k]); 
  }
  for(int k=0; k<numberNodes; k++)
    nodes[k]->setDemand(demandNominal[k]);
  calculateSensitivity("friction_coeff",0);

  return konv;
}

/*! WR: Building up the reduced sensitivity matrix for SVDCalibrate
     *  calculatedPressure:		giving back the calculated pressures for SVDCalibrate
     *	return:		reduced sensitivity matrix
     */
MatrixXd SVDCalibration::ConstructSM4Calibration(const vector<double> &fric, MatrixXd &calculatedPressure){

  int numberNodes = nodes.size(), numberEdges = edges.size();

  double demandSumNominal = 0.;
  vector<double> demandNominal(numberNodes,0.);
  for(int i=0; i<numberNodes; i++){
  	demandNominal[i] = nodes[i]->getDemand();
    if(demandNominal[i]>0.)
      demandSumNominal += nodes[i]->getDemand();
  }
  
  double demandSumMeasuredNominal=0.;
  for(int i=0; i<numberDemand; i++)
   demandSumMeasuredNominal += demandNominal[measuredDemandIndex[i]];

  MatrixXd Sr(numberOperationalPoint*numberPressure,numberEdges);
  // Sens analysis on measurement case  A(dimension, vector<int>(dimension));
  calculatedPressure = MatrixXd::Zero(numberOperationalPoint,numberPressure);
  for(int i=0; i<numberOperationalPoint; i++){
    // Setting frictions
    for(int k=0; k<numberEdges; k++)
      if(edges[k]->getType() == "Pipe")
        edges[k]->setProperty("roughness",fric[k]);
    // Setting measured demands
    double demandSumMeasuredReal=0.;
    for(int k=0; k<numberDemand; k++){
      nodes[measuredDemandIndex[k]]->setDemand(measuredDemand(i,k));
      demandSumMeasuredReal += measuredDemand(i,k);
    }
    // Setting the rest of the demands
    for(int k=0; k<numberNodes; k++){
      bool measured=false;
      int l=0;
      while(!measured && l<numberDemand){
        if(k == measuredDemandIndex[l])
          measured = true;
        l++;
      }
      if(nodes[k]->getDemand() > 0. && !measured)
        nodes[k]->setDemand((measuredDemandSum[i]*demandSumNominal-demandSumMeasuredReal)/(demandSumNominal-demandSumMeasuredNominal)*demandNominal[k]); // TODO: demandSumNominal -> demandSumNominal*measuredDemandSum[i] ???
    }
    // Steady state + sensitivity analysis
    calculateSensitivity("friction_coeff",0.);
    for(int k=0; k<numberPressure; k++)
      calculatedPressure(i,k) = nodes[measuredPressureIndex[k]]->getHead();
    // Filling up the reduced sensitivity matrix
    for(int k=0; k<numberPressure; k++)
      Sr.row(i*numberPressure+k) = pressureSensitivity.row(measuredPressureIndex[k]);
        //Sr(i*numberPressure+k,l) = pressureSensitivity(measuredPressureIndex[k],l);
  }
  if(getDebugLevel()>1)
    cout << "\n*Sr:\n" << Sr << endl << endl;

  for(int i=0; i<numberNodes; i++)
    nodes[i]->setDemand(demandNominal[i]);

  return Sr;
}

/*! WR: Singular value decomposition using Eigen
     *  A:    matrix for SVD
     *  tol:  NEW: tol determine the part of trace that will be counted || OLD: if tol>0, under tol every singular value will be 0; but if tol<0, the cut is under the largest singular value diveded with (-tol)
     *  U,S,V: returnung matrices
     */
void SVDCalibration::EigenSVD(const MatrixXd &A, double &tol, MatrixXd &U, MatrixXd &S, MatrixXd &V){
  JacobiSVD<MatrixXd> svd(A, ComputeThinU | ComputeThinV);
  U = svd.matrixU(), V = svd.matrixV();
  VectorXd v = svd.singularValues();
  S = MatrixXd::Constant(v.rows(),v.rows(),0.);

  if(tol>1. || tol<0.)
    tol = .9;
  double tr=0.;
  for(int i=0; i<v.rows(); i++)
    tr += v(i);
  double sum_tr = 0.;
  for(int i=0; i<v.rows(); i++)
    if(sum_tr/tr<tol){
      S(i,i) = v(i);
      sum_tr += v(i);
    }else
      break;

  if(getDebugLevel()>0){
    printf("\n*Singuar values of Sr (tol: %5.2f %%) : ", tol*100.);
    int i=0;
    while(i<v.rows() && S(i,i)>0.){
      printf("%6.3e | ",v(i));
      i++;
    }
    printf("OFF: ");
    while(i<v.rows()){
      printf("%6.3e | ",v(i));
      i++;
    }
  }
}

/*! WR: Pseudoinvere (or normal inverse) using the SVD method (see Numerical Recipes)
  *U,S,V:   coming from as an output from EigenSVD() function
  *return:  Pseudoinverse matrix
*/
MatrixXd SVDCalibration::PinvSVD(const MatrixXd &U, const MatrixXd &S, const MatrixXd &V){

  MatrixXd S_inv = MatrixXd::Constant(S.rows(),S.cols(),0.);
  for(int i=0; i<S.rows(); i++){
    if(S(i,i)>1e-15)
      S_inv(i,i) = 1./S(i,i);
    else
      S_inv(i,i) = 0.0;
  }

  return V*(S_inv*U.transpose()); //See Numerical Recipes
}

/*! WR: Determines the rank (i.e. linearly independent vectors ) of a matrix using SVD (see Numerical Recipes)
  *S:       S matrix coming from EigenSVD()
  *return:  rank of the matrix
*/
int SVDCalibration::RankSVD(const MatrixXd &S){
  int rank=0;
  for(int i=0; i<S.rows(); i++)
    if(S(i,i)>1e-15)
      rank++;

  return rank;
}

/*! WR: Determinant (i.e. product of non-zero eigen values) of a matrix
  *S:       coming from EigenSVD()
  *return:  det of the matrix
*/
double SVDCalibration::DetSVD(const MatrixXd &S){
  double det=1.;
  for(int i=0; i<S.rows(); i++)
    if(S(i,i)>1e-10)
      det *= S(i,i);
    
  return det;
}

/*! WR: Trace (i.e. sum of the eigen values) of a matrix
  *S:       coming from EigenSVD()
  *return:  det of the matrix
*/
double SVDCalibration::TraceSVD(const MatrixXd &S){
  double tr=0.;
  for(int i=0; i<S.rows(); i++)
    tr += S(i,i);
    
  return tr;
}