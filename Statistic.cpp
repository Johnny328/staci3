#include "Statistic.h"

using namespace std;
using namespace Eigen;

//--------------------------------------------------------------
double average(const vector<double> &x){
  double a=0.;
  for(int i=0; i<x.size() ;i++)
    a += x[i];
  return a/(double)x.size();
}
double average(const vector<int> &x){
  double a=0.;
  for(int i=0; i<x.size() ;i++)
    a += x[i];
  return a/(double)x.size();
}

//--------------------------------------------------------------
double absoluteMax(const vector<double> &x){
  double m=0.;
  for(int i=0; i<x.size(); i++)
    if((x[i]>m))
      m = abs(x[i]);
  return m;
}

//--------------------------------------------------------------
double standardDeviation(const vector<double> &x){
  double s=0.;
  double m = average(x);
  for(int i=0; i<x.size(); i++)
    s += pow(x[i]-m,2.);
  return pow(s/((double)x.size()-1),.5);
}

//--------------------------------------------------------------
double normalDistribution(double m, double szig){
  random_device rd;
  default_random_engine generator( rd() );
  normal_distribution<double> dist(m,szig);
  return dist(generator);
}

//--------------------------------------------------------------
double uniformDistribution(double min, double max)
{
    double f = (double)rand() / RAND_MAX;
    return min + f * (max - min);
}

//--------------------------------------------------------------
bool chiSquaredTest(const vector<double> &x, double m, double szig){
  bool r=false;
  int no_intervals = 6;
  int n_data = x.size();
  vector<double> limits;
  double sqrt2 = sqrt(2.);
  // boundaries between the intervals using standard normal distribution function: FI^-1(0.1),FI^-1(0.2)...FI^-1(0.9)
  for(int i=0; i<no_intervals-1;i++){
    limits.push_back( -sqrt2*erfinv(1-2*(i+1)/(double)no_intervals) );
    //cout << "limits: " << limits.at(i) << "\t FI: " << .5*erfc(-limits.at(i)/sqrt2) << endl;
  }

  vector<int> counters(no_intervals);
  for(int k=0; k<n_data; k++){
    double x_st = (x.at(k)-m)/szig; // Standardizing the x vector with m and szig
      for(int i=0; i<no_intervals-2; i++){
          if(limits.at(i)<x_st && limits.at(i+1)>x_st) 
              counters.at(i+1)++;
      }
      if(x_st<limits.at(0))
              counters.at(0)++;
      if(x_st>limits.at(no_intervals-2))
              counters.at(no_intervals-1)++;
  }

    double khi=0;
    for(int i=0; i<no_intervals; i++){
      //cout << "counters: " << counters.at(i);
      khi += pow(counters.at(i)-n_data/no_intervals,2)/(n_data/no_intervals);
      //cout << "\t khi: " << khi << endl;
    }
    /*p: probability(0.95,0.98,0.99)
      k: degrees of freedom (no. of intervals(6,10) - 1) TODO: using vector,vector; then search for p and k, save i and j, if not found ERROR
      |------------------------------|
      |1-p|  0.05  |  0.02  |  0.01  |
      |k -+--------+--------+--------|
      |5  | 11.070 | 13.388 | 15.086 |
      |9  | 16.919 | 19.679 | 21.666 |
      |------------------------------|*/
    if(khi<=11.070)
      r = true;

  return r;
}

//--------------------------------------------------------------
bool twoSampleUTest(double m1, double s1, int n1, double m2, double s2, int n2, double p){
  bool test_ok = false;
  double u_akt, u_krit;

  u_akt = (m1-m2)/pow(pow(s1,2.)/(double)n1 + pow(s2,2.)/(double)n2,.5);
  u_krit = sqrt(2)*erfinv(p);
  if(abs(u_akt)<u_krit)
    test_ok = true;
  return test_ok;
}

//--------------------------------------------------------------
double erfinv(double x)
{
  double a3 = -0.140543331, a2 = 0.914624893, a1 = -1.645349621, a0 = 0.886226899;
  double b4 = 0.012229801, b3 = -0.329097515, b2 = 1.442710462, b1 = -2.118377725, b0 = 1;
  double c3 = 1.641345311, c2 = 3.429567803, c1 = -1.62490649, c0 = -1.97084045;
  double d2 = 1.637067800, d1 = 3.543889200, d0 = 1;
  double x2, r, y;
  int sign_x;

  if (x < -1 || x > 1){
    cout << "ERROR: erfinv(x) function called with fail x, x must be >=-1 AND <=1" << endl;
    return -1;
  }

  if (x == 0)
    return 0;

  if (x > 0)
    sign_x = 1;
  else {
    sign_x = -1;
    x = -x;
  }
 
  if (x <= 0.7) {
    x2 = x * x;
    r = x * (((a3 * x2 + a2) * x2 + a1) * x2 + a0);
    r /= (((b4 * x2 + b3) * x2 + b2) * x2 + b1) * x2 + b0;
  }
  else {
    y = sqrt(-log ((1 - x) / 2));
    r = (((c3 * y + c2) * y + c1) * y + c0);
    r /= ((d2 * y + d1) * y + d0);
  }
 
  r = r*sign_x;
  x = x*sign_x;
 
  r -= (erf(r) - x) / (2 / sqrt(M_PI) * exp(-r * r));
  r -= (erf(r) - x) / (2 / sqrt(M_PI) * exp(-r * r));
 
  return r;
}

//--------------------------------------------------------------
double correlCoefficient(const vector<double> &x, const vector<double> &y){
  if(x.size() != y.size()){
    cout << "\n\n !!!!! ERROR !!!!!\n Size of x and y are not matching in CorrelCoef, Statistic.cpp\n";
    return 0;
  }
  int n = x.size();
  double szam=0.0,sx=0.0,sy=0.0,ax=0.0,ay=0.0;
  for(int i=0;i<n;i++){
    ax += x.at(i);
    ay += y.at(i);
  }
  ax /= n; ay /= n;
  for(int i=0;i<n;i++)
    szam += (x.at(i)-ax)*(y.at(i)-ay);
  for(int i=0;i<n;i++){
    sx += pow(x.at(i)-ax,2.);
    sy += pow(y.at(i)-ay,2.);
  }
  sx = pow(sx,0.5);
  sy = pow(sy,0.5);
  return szam/sx/sy;
}

//--------------------------------------------------------------
vector<double> grubbsTest(const vector<double> &x){
  double a,m,s;
  a = average(x);
  m = absoluteMax(x);
  s = standardDeviation(x);
  vector<double> y;
  for(int i=0; i<x.size(); i++)
    if(x.at(i) > m-3.*s && x.at(i) < m+3.*s)
      y.push_back(x.at(i));
  return y;
}

//--------------------------------------------------------------
double interpolate(vector<double> x, vector<double> y, double xp){

  double yp = 0.0;
  int nx=x.size(), ny=y.size();
  if(nx == 0 || ny == 0 || nx == 1 || ny ==1)
  {
    cout << endl << " !!!ERROR!!! interpolate: Number of x/y coordinates (" << nx << "/" << ny << " is equal to 0 or 1!" << endl << " Exiting..." << endl;
    exit(0);
  }
  if(nx == ny)
  {
    int idx;
    if(xp<min(x,idx)){
      cout << endl << " !!!WARNING!! interpolate: xp = " << xp << " is smaller than min(x) = " << min(x,idx) << endl << " Overriding xp = min(x), then Continouing..." << endl;
      xp = min(x,idx);
      yp = y[idx];
    }
    else if(xp>max(x,idx))
    {
      cout << endl << " !!!WARNING!! xp = " << xp << " is larger than max(x) = " << max(x,idx) << endl << " Overriding xp = max(x), then Continouing..." << endl;
      xp = max(x,idx);
      yp = y[idx];
    }else
    {
      if(nx == 2) // Fitting a linear
      {
        double a = (y[1]-y[0])/(x[1]-x[0]);
        double b = y[1] - a*x[1];
        yp = a*xp + b;
      }
      if(nx>=3)// Above 3 points, fitting a parabole with the three closest
      {
        vector<double> xdist(nx);
        vector<int> idx;
        for(int i=0; i<nx; i++)
          xdist[i] = abs(x[i] - xp);
        quickSortWithIndicies(xdist,idx,0,nx-1);
        yp = (xp-x[idx[1]])*(xp-x[idx[2]])/(x[idx[0]]-x[idx[1]])/(x[idx[0]]-x[idx[2]])*y[idx[0]] + (xp-x[idx[0]])*(xp-x[idx[2]])/(x[idx[1]]-x[idx[0]])/(x[idx[1]]-x[idx[2]])*y[idx[1]] + (xp-x[idx[0]])*(xp-x[idx[1]])/(x[idx[2]]-x[idx[0]])/(x[idx[2]]-x[idx[1]])*y[idx[2]]; // Lagrange polinomial
      }
    }
  }
  else
  {
    cout << endl << " !!!ERROR!!! Number of x coordinates (" << nx << ") NOT equal to number of y coordinates(" << ny << ") !" << endl;
    exit(0);
  }

  return yp;
}

//-------------------------------------------------------------- 
void quickSortWithIndicies(vector<double> &x, vector<int> &idx, int low, int high){

  idx.clear();
  idx.resize(x.size());
  for(int i=0; i<idx.size(); i++)
    idx[i] = i;
  quickSort(x,idx,low,high);
}


/* The main function that implements QuickSort 
 arr[] --> Array to be sorted, 
  low  --> Starting index, 
  high  --> Ending index */
void quickSort(vector<double> &x, vector<int> &idx, int low, int high) 
{
  if (low < high) 
  { 
    /* pi is partitioning index, arr[p] is now 
       at right place */
    int pi = partition(x, idx, low, high); 

    // Separately sort elements before 
    // partition and after partition 
    quickSort(x, idx, low, pi - 1); 
    quickSort(x, idx, pi + 1, high); 
  } 
} 

/* This function takes last element as pivot, places 
   the pivot element at its correct position in sorted 
    array, and places all smaller (smaller than pivot) 
   to left of pivot and all greater elements to right 
   of pivot */
int partition(vector<double> &x, vector<int> &idx, int low, int high) 
{ 
  int pivot = x[high];    // pivot 
  int i = (low - 1);  // Index of smaller element 

  for (int j = low; j <= high- 1; j++) 
  { 
    // If current element is smaller than or 
    // equal to pivot 
    if (x[j] <= pivot) 
    { 
      i++;    // increment index of smaller element 
      swap(x,i,j);
      if(idx.size() == x.size())
        swap(idx,i,j);
    } 
  } 
  swap(x,i+1,high); 
  if(idx.size() == x.size())
    swap(idx,i+1,high);
  return (i + 1); 
}

//--------------------------------------------------------------
void swap(vector<double> &x, int i, int j){
  double temp = x[i];
  x[i] = x[j];
  x[j] = temp;
}

//--------------------------------------------------------------
void swap(vector<int> &x, int i, int j){
  int temp = x[i];
  x[i] = x[j];
  x[j] = temp;
}

//--------------------------------------------------------------
double min(vector<double> x, int &idx){
  double min=x[0];
  idx = 0;
  for(int i=1; i<x.size(); i++){
    if(min > x[i]){
      min = x[i];
      idx = i;
    }
  }
  return min;
}
//--------------------------------------------------------------
double min(vector<double> x){
  int idx;
  return min(x,idx);
}

//--------------------------------------------------------------
double max(vector<double> x, int &idx){
  double max=x[0];
  idx = 0;
  for(int i=1; i<x.size(); i++){
    if(max < x[i]){
      max = x[i];
      idx = i;
    }
  }
  return max;
}
//--------------------------------------------------------------
double max(vector<double> x){
  int idx;
  return max(x,idx);
}

VectorXd leastSquaresPolynomial(const VectorXd &x, const VectorXd &y, int order){

  if(x.rows() != y.rows()){
    cout << endl << "!!!ERROR!!! Statistic::leastSquaresPolynomial(): size of X is not equal to size of Y" << endl << "Exiting..." << endl;
    exit(0);
  }
  if(order>x.rows()-1){
    cout << endl << "!!!WARNING!!! Statistic::leastSquaresPolynomial(): order(" << order << ") is larger or equal than number of points(" << x.rows() << ")!" << endl << "order is decreased to x.rows()-1 (" << x.rows()-1 << "), then continouing..." << endl;
    order = x.rows()-1;
  }

  VectorXd out = VectorXd::Zero(order+1);
  MatrixXd Gram = MatrixXd::Zero(order+1,order+1);

  for(int i=0; i<Gram.rows(); i++){
    for(int j=i; j<Gram.cols(); j++){
      Gram(i,j) = (eigenVectorXdPow(x,i).transpose()*eigenVectorXdPow(x,j)).sum(); // <fi,fj> = [x1^i, ... xm^i]*[x1^j, ... xm^j]^T
      if(j!=i)
        Gram(j,i) = Gram(i,j); // Since Gram matrix is symmetric
    }
  }

  VectorXd b = VectorXd::Zero(order+1);
  for(int i=0; i<b.rows(); i++)
    b(i) = (y.transpose()*eigenVectorXdPow(x,i)).sum();

  out = Gram.ldlt().solve(b); //LDL decomposition is suitable, since Gram is positive definite (also symmetric)

  return out;
}

VectorXd eigenVectorXdPow(const VectorXd &x, int order){
  int n=x.rows();
  VectorXd out;
  if(order==0)
    out = VectorXd::Constant(n,1.);
  else
    out = x;

  if(order>=2 || order <=-2){
    for(int i=0; i<n; i++)
      for(int j=0; j<order-1; j++)
        out(i) = out(i)*out(i);
  }

  if(order<0){
    for(int i=0; i<n; i++)
      out(i) = 1./out(i);
  }
  return out;
}