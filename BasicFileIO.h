#include <vector>
#include <iostream>
#include <fstream>
#include "Eigen/Eigen/Eigen"

using namespace std;
using namespace Eigen;

// Reading doubles from file, separeted with ','
vector<string> readVectorString(string fileName);

// Reading doubles from file to Eigen MatrixXd, separeted with "separator"
MatrixXd readMatrixDouble(string fileName, char separator);

// Reading doubles from file to Eigen VectorXd
VectorXd readVectorDouble(string fileName);

// Counting the rows for Eigen Vectors
int countRows(string fileName);

// Counting the rows and cols for Eigen Vectors
vector<int> countRowsCols(string fileName, char separator);