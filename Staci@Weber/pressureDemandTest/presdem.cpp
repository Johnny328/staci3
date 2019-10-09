#include <iostream>
#include <fstream>
#include <random>
#include <iomanip>
#include <string>
#include <algorithm>
#include <stdio.h>
#include <sys/stat.h>

#include "../../Shutdown.h"
#include "../../HydraulicSolver.h"

using namespace std;
using namespace Eigen;

int main(int argc, char* argv[]){

  // Name of containing folder of staci file
  string caseFolder = "../../Networks/";

  string caseName;
  if(argc == 1){
    //caseName = "Anytown.inp";
    caseName = "hermes.inp";
    //caseName = "villasor.inp";
    //caseName =  "linear_9.inp";
    //caseName =  "grid_9.inp";
    //caseName =  "ky2.inp";
    //caseName =  "Net1.inp";
  }else if(argc == 2){
    caseName = argv[1];
  }

  HydraulicSolver *HS = new HydraulicSolver(caseFolder + caseName);

  HS->isPressureDemand = true;
  for(int i=0; i<HS->nodes.size(); i++)
    HS->nodes[i]->setProperty("demand",600.*HS->nodes[i]->getProperty("demand"));

  HS->solveSystem();

  HS->saveResult("head","Node");
  HS->saveResult("headloss","Pipe");

  cout << endl << endl;
  return 0;
}