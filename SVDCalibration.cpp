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
     *  return:     0: no convergence, 1: full convergence (e_p~0), 2: local convergence (d(e_p)>0)
     */
int SVDCalibration::Calibrate(const vector<double> &fric_est, double tol){

  int n_nodes = nodes.size(), n_edges = edges.size();

  // Filling up fric var, that will contain the friction at each step
  vector<double> fric(n_edges,0.), d_orig(n_nodes,0.);
  for (int k=0;k<n_edges;k++)
    if(edges[k]->getType() == "Pipe")
      fric[k] = fric_est[k];
  // Saving the nominal demands
  for (int i=0; i<n_nodes;i++)
    d_orig[i] = nodes[i]->getDemand();

  LoadMeas(getDefinitionFile());
  /*cout << endl << "DEBUG" << endl;
  cout << endl << "d_meas_sum: " << endl;
  for(int i=0; i<d_meas_sum.size(); i++)
    cout << d_meas_sum[i] << endl;
  
  cout << endl << "p_meas: " << endl;
  for(int i=0; i<p_meas.size(); i++){
    for(int j=0; j<p_meas[i].size(); j++)
      cout << p_meas[i][j] << " | ";
    cout << endl;
  }
  cout << endl << "d_meas_ids: " << endl;
  for(int i=0; i<d_meas_ids.size(); i++)
    cout << d_meas_ids[i] << endl;
  
  cout << endl << "d_meas: " << endl;
  for(int i=0; i<d_meas.size(); i++){
    for(int j=0; j<d_meas[i].size(); j++)
      cout << d_meas[i][j] << " | ";
    cout << endl;
  }*/

  // Start of main iteration
  if(getDebugLevel()>0)
    cout << "\n\n---------------------------------\n******Starting Calibration******\n---------------------------------\n\n";
  int iter_max = 1e2; // max iteration number
  double relax = .5, satur = 1.; // relaxation factor, saturation factor
  double max_dlambda = .5; // saturation for dlambda with relaxation
  int konv = 0;
  VectorXd dlambda;
  double e_p_norm=1e10;
  double relax_nom = relax;

  for(int i=0;i<iter_max;i++){
    bool better = false;
    int j = 0;
    int j_max = 4; // max no. of inner iteration
    MatrixXd SMred;
    VectorXd e_p(n_op*n_p);

    vector<vector<double> > p_calc;
    while(!better && j<j_max){
      // calculating the reduced sensitivity matrix for every case when measurement occured, moreover it gives back the output pressur for the measured nodes in p_calc
      SMred = ConstructSM4Calibration(fric, p_calc);

      for(int k=0; k<n_op; k++)
        for(int l=0; l<n_p; l++)
          e_p(l+k*n_p) = p_meas[k][l] - p_calc[k][l];
      if(getDebugLevel()>1)
        cout << "e_p_norm_r: " << e_p_norm << "   e_p_norm: " << e_p.norm() << endl;

      if(e_p_norm > e_p.norm())
        better = true;
      else{
        relax /= 2.;
        for(int k=0; k<n_edges; k++)
          if(edges[k]->getType() == "Pipe")
            fric[k] -= relax*satur*dlambda(k);
      }
      j++;
    }
    e_p_norm = e_p.norm();
    if(j==j_max){
      konv = 2;
      break;
    }

    // performing SVD and calculate dlambda
    MatrixXd U,S,V;
    EigenSVD(SMred,tol,U,S,V);
    MatrixXd Sr_pinv = PinvSVD(U,S,V);
    dlambda = Sr_pinv*e_p;

    // Checking the largest relativa change
    double lth=0.;
    for(int k=0; k<n_edges; k++)
      if(edges[k]->getType() == "Pipe")
        if(abs(relax*dlambda(k)/fric[k])>max_dlambda)
          lth = abs(dlambda(k)/fric[k]);

    satur=1.;
    if(lth>max_dlambda)
      satur = max_dlambda/lth;

    for(int k=0; k<n_edges; k++)
      if(edges[k]->getType() == "Pipe")
        fric[k] += relax_nom*satur*dlambda(k);

    if(getDebugLevel()>1){
      cout << "\nfrictions:\n";
      for(int k=0; k<n_edges; k++)
        if(edges[k]->getType() == "Pipe")
          cout << fric[k] << endl;
      cout << endl;
    }

    if(getDebugLevel()>0){
      cout << "\n\n**********Calibration*************";
      printf("\niter: %2i", i);
      printf("  df_n: %2.4e",dlambda.norm()/pow((double)n_edges,.5));
      printf("  e_p: %2.4e", e_p.norm()/pow((double)n_nodes,.5));
      printf("  relax:  %1.2f",relax);
      printf("  satur:  %1.2f",satur);
      cout << "\n**********Calibration*************\n\n";
    }
    relax = relax_nom;

    if(e_p.norm()/pow((double)n_nodes,.5)<1e-8 || dlambda.norm()/pow((double)n_edges,.5)<1e-8){
      konv = 1;
      break;
    }
    if(i==iter_max-1 && getDebugLevel()>0)
        cout << "\nWARNING!! Convergence is not sufficient!";
  }
  if(getDebugLevel()>0)
    cout << "\n\n---------------------------------\n******Ending Calibration******\n---------------------------------\n\n";
  if(konv != 0){
    for(int k=0; k<n_edges; k++)
      if(edges[k]->getType()=="Pipe")
        edges[k]->setProperty("roughness",fric[k]); 
  }else{
  	for(int k=0; k<n_edges; k++)
      if(edges[k]->getType()=="Pipe")
        edges[k]->setProperty("roughness",fric_est[k]); 
  }
  for(int k=0; k<n_nodes; k++)
    nodes[k]->setDemand(d_orig[k]);
  calculateSensitivity("friction_coeff",0);

  return konv;
}

/*! WR: Building up the reduced sensitivity matrix for SVDCalibrate
     *  p_calc:		giving back the calculated pressures for SVDCalibrate
     *	return:		reduced sensitivity matrix
     */
MatrixXd SVDCalibration::ConstructSM4Calibration(const vector<double> &fric, vector<vector<double > > &p_calc){

  int n_nodes = nodes.size(), n_edges = edges.size();

  //double d_sum_nom = Get_dem_nom_sum();
  double d_sum_nom = 0.;
  vector<double> d_orig(n_nodes,0.);
  for(int i=0; i<n_nodes; i++){
  	d_orig[i] = nodes[i]->getDemand();
    if(d_orig[i]>0.)
      d_sum_nom += nodes[i]->getDemand();
  }
  
  double d_sum_meas_nom=0.;
  for(int i=0; i<n_d; i++)
   d_sum_meas_nom += d_orig[d_meas_idx[i]];

  MatrixXd Sr(n_op*n_p,n_edges);
  // Sens analysis on measurement case  A(dimension, vector<int>(dimension));
  p_calc.clear();
  p_calc.resize(n_op,vector<double>(n_p));
  for(int i=0; i<n_op; i++){
    // Setting frictions
    for(int k=0; k<n_edges; k++)
      if(edges[k]->getType() == "Pipe")
        edges[k]->setProperty("roughness",fric[k]);
    // Setting measured demands
    double d_meas_real=0.;
    for(int k=0; k<n_d; k++){
      nodes[d_meas_idx[k]]->setDemand(d_meas[i][k]);
      d_meas_real += d_meas[i][k];
    }
    // Setting the rest of the demands
    for(int k=0; k<n_nodes; k++){
      bool measured=false;
      int l=0;
      while(!measured && l<n_d){
        if(k == d_meas_idx[l])
          measured = true;
        l++;
      }
      if(nodes[k]->getDemand() > 0. && !measured)
        nodes[k]->setDemand((d_meas_sum[i]*d_sum_nom-d_meas_real)/(d_sum_nom-d_sum_meas_nom)*d_orig[k]); // TODO: d_sum_nom -> d_sum_nom*d_meas_sum[i] ???
    }
    // Steady state + sensitivity analysis
    calculateSensitivity("friction_coeff",0.);
    for(int k=0; k<n_p; k++)
      p_calc[i][k] = nodes[p_meas_idx[k]]->getHead();
    // Filling up the reduced sensitivity matrix
    for(int k=0; k<n_p; k++)
      for(int l=0; l<n_edges; l++)
        Sr.row(i*n_p+k) = pressureSensitivity.row(p_meas_idx[k]);
        //Sr(i*n_p+k,l) = pressureSensitivity(p_meas_idx[k],l);
  }
  if(getDebugLevel()>1)
    cout << "\n*Sr:\n" << Sr << endl << endl;

  for(int i=0; i<n_nodes; i++)
    nodes[i]->setDemand(d_orig[i]);

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