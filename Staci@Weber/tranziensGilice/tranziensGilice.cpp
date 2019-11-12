#include <iostream>
#include <fstream>
#include <random>
#include <iomanip>
#include <string>
#include <algorithm>
#include <stdio.h>
#include <sys/stat.h>

#include "../../SeriesHydraulics.h"
#include "../../Shutdown.h"

using namespace std;
using namespace Eigen;

int main(int argc, char* argv[]){

  // Name of containing folder of staci file
  string caseFolder = "../../../../Tranziens Gilice/";

  string caseName;
  if(argc == 1){
    //caseName = "C-town.inp";
    //caseName = "Anytown.inp";
    caseName = "gilice.inp";
    //caseName =  "grid_9.inp";
    //caseName =  "ky2.inp";
    //caseName =  "Net1.inp";
  }else if(argc == 2){
    caseName = argv[1];
  }

  cout << endl << "Case: " << caseName << endl;

  SeriesHydraulics *wds;
  wds = new SeriesHydraulics(caseFolder + caseName);
  cout << "LOADING OK" << endl;

  /* For checking the disconnected network areas
  Shutdown *wdsShutdown;
  wdsShutdown = new Shutdown(caseFolder + caseName);
  int nSegments = wdsShutdown->getNumberSegment();
  cout << "Number of segments: " << nSegments << endl;
  wdsShutdown->saveResult("segment","All");*/

  wds->seriesSolve();

  cout << endl << endl;
  return 0;
}