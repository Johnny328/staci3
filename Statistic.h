/*===================================================================*\
                               Statistic
                            ---------------

  Statistical calculations, e.g. average, deviation, normal dist.

  Can be used independently from staci3.
 
  staci3 is using Eigen, see http://eigen.tuxfamily.org

    staci3
    Cs. Hos, R. Weber, T. Huzsvar
    https://github.com/weberrichard/staci3
\*==================================================================*/

#ifndef STATISTIC_H
#define STATISTIC_H

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

// Interpolation using x-y points, with two points linear, with three or above parabole with the closest (in x) points
double interpolate(vector<double> x, vector<double> y, double xp);

// Dynamic sorting algorithm from index "low" until index "high" and following the changes and saving in idx
void quickSortWithIndicies(vector<double> &x, vector<int> &idx, int low, int high);

// Dynamic sorting algorithm from index "low" until index "high"
void quickSort(vector<double> &x, vector<int> &idx, int low, int high);

// Function for quickSort
int partition(vector<double> &x, vector<int> &idx, int low, int high);

// Changing two elements in vector double/integer
void swap(vector<double> &x, int i, int j);
void swap(vector<int> &x, int i, int j);

// Finding the min element and index in vector
double min(vector<double> x, int &idx);

// Finding the max element and index in vector
double max(vector<double> x, int &idx);

#endif