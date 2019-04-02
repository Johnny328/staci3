#include "SVDCalibration.h"

using namespace std;
using namespace Eigen;

SVDCalibration::SVDCalibration(string spr_filename) : Calibration(spr_filename){
}
SVDCalibration::~SVDCalibration(){
}

int SVDCalibration::calibrate(const vector<double> &fric_est, double tol){

  int numberNodes = nodes.size(), numberEdges = edges.size();

  // Filling up fric var, that will contain the friction at each step
  vector<double> fric(numberEdges,0.), demandNominal(numberNodes,0.);
  for (int k=0;k<numberEdges;k++)
    if(edges[k]->getEdgeStringProperty("type") == "Pipe")
      fric[k] = fric_est[k];
  // Saving the nominal demands
  for (int i=0; i<numberNodes;i++)
    demandNominal[i] = nodes[i]->getProperty("demand");

  string caseFileName = getDefinitionFile();
  loadMeasurement(caseFileName,true);

  // Start of main iteration
  if(debugLevelSVD>0)
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
    VectorXd errorPressure = VectorXd::Zero(numberOperationalPoint*numberPressure);

    MatrixXd calculatedPressure;
    while(!better && j<j_max){
      // calculating the reduced sensitivity matrix for every case when measurement occured, moreover it gives back the output pressur for the measured nodes in calculatedPressure
      SMred = constructSensitivity(fric, calculatedPressure);

      for(int k=0; k<numberOperationalPoint; k++)
        for(int l=0; l<numberPressure; l++)
          errorPressure(l+k*numberPressure) = measuredPressure(k,l) - calculatedPressure(k,l);

      if(debugLevelSVD>1)
        cout << "e_p_norm_r: " << e_p_norm << "   e_p_norm: " << errorPressure.norm() << endl;

      if(e_p_norm > errorPressure.norm())
        better = true;
      else{
        relax /= 2.;
        for(int k=0; k<numberEdges; k++)
          if(edges[k]->getEdgeStringProperty("type") == "Pipe")
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
    eigenSVD(SMred,tol,U,S,V);
    MatrixXd Sr_pinv = pinvSVD(U,S,V);
    dlambda = Sr_pinv*errorPressure;

    // Checking the largest relativa change
    double lth=0.;
    for(int k=0; k<numberEdges; k++)
      if(edges[k]->getEdgeStringProperty("type") == "Pipe")
        if(abs(relax*dlambda(k)/fric[k])>max_dlambda)
          lth = abs(dlambda(k)/fric[k]);

    satur=1.;
    if(lth>max_dlambda)
      satur = max_dlambda/lth;

    for(int k=0; k<numberEdges; k++)
      if(edges[k]->getEdgeStringProperty("type") == "Pipe")
        fric[k] += relax_nom*satur*dlambda(k);

    if(debugLevelSVD>1){
      cout << "\nfrictions:\n";
      for(int k=0; k<numberEdges; k++)
        if(edges[k]->getEdgeStringProperty("type") == "Pipe")
          cout << fric[k] << endl;
      cout << endl;
    }

    if(debugLevelSVD>0){
      cout << "\n\n**********Calibration*************";
      printf("\niter: %2i", i);
      printf("  dlambda: %2.4e",dlambda.norm()/pow((double)numberEdges,.5));
      printf("  e_p: %2.4e", errorPressure.norm()/pow((double)numberNodes,.5));
      printf("  relax:  %1.2f",relax);
      printf("  satur:  %1.2f",satur);
      cout << "\n**********Calibration*************\n\n";
    }
    relax = relax_nom;

    if(errorPressure.norm()/pow((double)numberNodes,.5) < 1e-5 || dlambda.norm()/pow((double)numberEdges,.5) < 1e-5){
      konv = 1;
      break;
    }
    if(i==iterMax-1 && debugLevelSVD>0)
        cout << "\nWARNING!! Convergence is not sufficient!";
  }
  if(debugLevelSVD>0)
    cout << "\n\n---------------------------------\n******Ending Calibration******\n---------------------------------\n\n";
  if(konv != 0){
    for(int k=0; k<numberEdges; k++)
      if(edges[k]->getEdgeStringProperty("type")=="Pipe")
        edges[k]->setDoubleProperty("roughness",fric[k]); 
  }else{
  	for(int k=0; k<numberEdges; k++)
      if(edges[k]->getEdgeStringProperty("type")=="Pipe")
        edges[k]->setDoubleProperty("roughness",fric_est[k]); 
  }
  for(int k=0; k<numberNodes; k++)
    nodes[k]->setProperty("demand",demandNominal[k]);
  calculateSensitivity("friction_coeff");

  return konv;
}

MatrixXd SVDCalibration::constructSensitivity(const vector<double> &fric, MatrixXd &calculatedPressure){

  int numberNodes = nodes.size(), numberEdges = edges.size();

  double demandSumNominal = 0.;
  vector<double> demandNominal(numberNodes,0.);
  for(int i=0; i<numberNodes; i++){
  	demandNominal[i] = nodes[i]->getProperty("demand");
    if(demandNominal[i]>0.)
      demandSumNominal += nodes[i]->getProperty("demand");
  }
  
  double demandSumMeasuredNominal=0.;
  for(int i=0; i<numberDemand; i++)
   demandSumMeasuredNominal += demandNominal[measuredDemandIndex[i]];

  MatrixXd Sr(numberOperationalPoint*numberPressure,numberEdges);
  // Sens analysis on measurement case
  calculatedPressure = MatrixXd::Zero(numberOperationalPoint,numberPressure);
  for(int i=0; i<numberOperationalPoint; i++){
    // Setting frictions
    for(int k=0; k<numberEdges; k++)
      if(edges[k]->getEdgeStringProperty("type") == "Pipe")
        edges[k]->setDoubleProperty("roughness",fric[k]);
    // Setting measured demands
    double demandSumMeasuredReal=0.;
    for(int k=0; k<numberDemand; k++){
      nodes[measuredDemandIndex[k]]->setProperty("demand",measuredDemand(i,k));
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
      if(nodes[k]->getProperty("demand") > 0. && !measured)
        nodes[k]->setProperty("demand",(measuredDemandSum[i]*demandSumNominal-demandSumMeasuredReal)/(demandSumNominal-demandSumMeasuredNominal)*demandNominal[k]);
    }
    // Steady state + sensitivity analysis
    calculateSensitivity("friction_coeff");
    for(int k=0; k<numberPressure; k++)
      calculatedPressure(i,k) = nodes[measuredPressureIndex[k]]->getProperty("head");
    // Filling up the reduced sensitivity matrix
    for(int k=0; k<numberPressure; k++)
      Sr.row(i*numberPressure+k) = pressureSensitivity.row(measuredPressureIndex[k]);
  }
  if(debugLevelSVD>1)
    cout << "\n*Sr:\n" << Sr << endl << endl;

  for(int i=0; i<numberNodes; i++)
    nodes[i]->setProperty("demand",demandNominal[i]);

  return Sr;
}

void SVDCalibration::eigenSVD(const MatrixXd &A, double &tol, MatrixXd &U, MatrixXd &S, MatrixXd &V){
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

  if(debugLevelSVD>0){
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

MatrixXd SVDCalibration::pinvSVD(const MatrixXd &U, const MatrixXd &S, const MatrixXd &V){

  MatrixXd S_inv = MatrixXd::Constant(S.rows(),S.cols(),0.);
  for(int i=0; i<S.rows(); i++){
    if(S(i,i)>1e-15)
      S_inv(i,i) = 1./S(i,i);
    else
      S_inv(i,i) = 0.0;
  }

  return V*(S_inv*U.transpose()); //See Numerical Recipes, Press, Teukolsky
}

int SVDCalibration::rankSVD(const MatrixXd &S){
  int rank=0;
  for(int i=0; i<S.rows(); i++)
    if(S(i,i)>1e-15)
      rank++;

  return rank;
}

double SVDCalibration::detSVD(const MatrixXd &S){
  double det=1.;
  for(int i=0; i<S.rows(); i++)
    if(S(i,i)>1e-10)
      det *= S(i,i);
    
  return det;
}

double SVDCalibration::traceSVD(const MatrixXd &S){
  double tr=0.;
  for(int i=0; i<S.rows(); i++)
    tr += S(i,i);
    
  return tr;
}