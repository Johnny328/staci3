#include <vector>
#include <string>
#include <cmath>
#include <vector>
#include <random>
#include <iostream>

using namespace std;

// WR Calculates average, max, standard deviation of a vector x
void Avr_absmax_stddev(const vector<double> &x, double &a, double &m, double &s);

// WR average of a vector
double Average(const vector<double> &x);
double Average(const vector<int> &x);

// WR maximum of a vector in absolute value
double Absmax(const vector<double> &x);

// WR standard deviation of a vector
double Stddev(const vector<double> &x);

// Normal distribution double with average of m and deviation of szig
double NormalDist(double m, double szig);

// Uniform distribution double betweein min and max
double UniformDist(double min, double max);

// Chi squrad test to x vector with expected value of m and deviation of szig, only normal distribution
bool ChiSquared_test(const vector<double> &x, double m, double szig);

// 2-sample U test wit averages, stddevs, sizes and p is the significanse
bool TwoSampleU_test(double m1, double s1, int n1, double m2, double s2, int n2, double p);

// Approximation of the inverse function of the error function //really good approximation
double erfinv(double x);

// Sample correlation coefficient between x and y, x.size() must be equal to y.size()
double CorrelCoef(const vector<double> &x, const vector<double> &y);

// Simple Grubbs test, returns a vector without the element under x_mean-3*x_dev and abov x_mean+3*x_dev
vector<double> GrubbsTest(const vector<double> &x);