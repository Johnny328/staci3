/*===================================================================*\
                              BasicFileIO
                            ---------------

  Reading and writing data from and to files.

  Can be used independently from staci3.
  
  staci3 is using Eigen, see http://eigen.tuxfamily.org

    staci3
    Cs. Hos, R. Weber, T. Huzsvar
    https://github.com/weberrichard/staci3
\*==================================================================*/

#ifndef BASICFILEIO_H
#define BASICFILEIO_H

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

#endif
