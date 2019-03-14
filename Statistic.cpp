#include "Statistic.h"

using namespace std;

void Avr_absmax_stddev(const vector<double> &x, double &a, double &m, double &s){
  a = Average(x);
  s = Stddev(x);
  m = Absmax(x);
}

double Average(const vector<double> &x){
  double a=0.;
  for(int i=0; i<x.size() ;i++)
    a += x[i];
  return a/(double)x.size();
}
double Average(const vector<int> &x){
  double a=0.;
  for(int i=0; i<x.size() ;i++)
    a += x[i];
  return a/(double)x.size();
}

double Absmax(const vector<double> &x){
  double m=0.;
  for(int i=0; i<x.size(); i++)
    if((x[i]>m))
      m = abs(x[i]);
  return m;
}

double Stddev(const vector<double> &x){
  double s=0.;
  double m = Average(x);
  for(int i=0; i<x.size(); i++)
    s += pow(x[i]-m,2.);
  return pow(s/((double)x.size()-1),.5);
}

double NormalDist(double m, double szig){
  random_device rd;
  default_random_engine generator( rd() );
  normal_distribution<double> dist(m,szig);
  return dist(generator);
}

double UniformDist(double min, double max)
{
    double f = (double)rand() / RAND_MAX;
    return min + f * (max - min);
}

bool ChiSquared_test(const vector<double> &x, double m, double szig){
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

bool TwoSampleU_test(double m1, double s1, int n1, double m2, double s2, int n2, double p){
  bool test_ok = false;
  double u_akt, u_krit;

  u_akt = (m1-m2)/pow(pow(s1,2.)/(double)n1 + pow(s2,2.)/(double)n2,.5);
  u_krit = sqrt(2)*erfinv(p);
  if(abs(u_akt)<u_krit)
    test_ok = true;
  return test_ok;
}

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

double CorrelCoef(const vector<double> &x, const vector<double> &y){
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

vector<double> GrubbsTest(const vector<double> &x){
  double a,m,s;
  Avr_absmax_stddev(x,a,m,s);
  vector<double> y;
  for(int i=0; i<x.size(); i++)
    if(x.at(i) > m-3.*s && x.at(i) < m+3.*s)
      y.push_back(x.at(i));
  return y;
}