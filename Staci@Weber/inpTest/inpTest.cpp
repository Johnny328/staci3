#include <iostream>
#include <fstream>
#include <random>
#include <iomanip>
#include <string>
#include <algorithm>
#include <stdio.h>
#include <sys/stat.h>

#include "../../Sensitivity.h"
#include "../../Shutdown.h"

using namespace std;
using namespace Eigen;

int main(int argc, char* argv[]){

  // Name of containing folder of staci file
  string caseFolder = "../../Networks/";
  //string caseFolder = "/home/rweber/0_PhD/Halozatok/sopron_halozatok/";

  string caseName;
  if(argc == 1){
    //caseName = "C-town.inp";
    caseName = "Anytown.inp";
    //caseName =  "grid_9.inp";
    //caseName =  "ky2.inp";
    //caseName =  "Net1.inp";
  }else if(argc == 2){
    caseName = argv[1];
  }

  cout << endl << "Case: " << caseName << endl;
  srand((unsigned int) time(0));

  Sensitivity *wds;
  //wds = new Sensitivity(caseFolder + caseName);
  wds = new Sensitivity(caseFolder + caseName);
  wds->initialization();
  wds->solveSystem();
  //wds->saveResult("head","Node");

  Shutdown *wds2;
  wds2 = new Shutdown(caseFolder + caseName);
  wds2->buildSegmentGraph();
  wds2->saveResult("segment","All");

  cout << endl << endl;
  return 0;
}