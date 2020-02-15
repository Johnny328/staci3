#include <iostream>
#include <fstream>
#include <random>
#include <iomanip>
#include <string>
#include <algorithm>
#include <stdio.h>
#include <sys/stat.h>

#include "../../SVDCalibration.h"

using namespace std;
using namespace Eigen;

int main(int argc, char* argv[])
{
  // Name of containing folder of staci file
  string caseFolder = "../../Networks/Net1/";
  //string caseFolder = "/home/rweber/0_PhD/Halozatok/sopron_halozatok/";

  string caseName;
  if(argc == 1){
    //caseName = "MUModel.inp";
    caseName = "Net1Calib.inp";
    //caseName = "linear_3Calib.inp";
  }else if(argc == 2){
    caseName = argv[1];
  }

  cout << endl << " Case name: " << caseName << endl;

  SVDCalibration *calib;
  calib = new SVDCalibration(caseFolder + caseName);
  calib->printLevel = 3;
  calib->printLevelSVD = 2;
  calib->SVDTolerance = 0.9;

  calib->calibrate();

  cout << endl << endl;
  return 0;
}
