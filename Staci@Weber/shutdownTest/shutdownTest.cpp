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
  //string caseFolder = "/home/rweber/0_PhD/Halozatok/sopron_halozatok/";

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

  Shutdown *Sd = new Shutdown(caseFolder + caseName);
  //vector<string> valves = Sd->closeSegment(0);
  //vector<string> valves = Sd->shutdownPlan("pipe_3");

  //cout << "\nvalves to close: ";
 // for(int i=0; i<valves.size(); i++)
  //  cout << "\n" << valves[i];

  //for(int i=0; i<Sd->edges.size(); i++)
  //  if(Sd->edges[i]->getEdgeStringProperty("type") == "Valve")
  //    cout << endl << Sd->edges[i]->getEdgeStringProperty("name") << "  " << Sd->edges[i]->getIntProperty("startSegment") << " -> " << Sd->edges[i]->getIntProperty("endSegment");

  Sd->saveResult("segment","All");

  HydraulicSolver *HS = new HydraulicSolver(caseFolder + caseName);

  for(int i=0; i<HS->edges.size(); i++)
    if(Sd->edges[i]->isClosed)
      HS->changeEdgeStatus(Sd->edges[i]->getEdgeStringProperty("name"),false);

  //cout << "\nHS valves:\n";
  //for(int i=0; i<HS->edges.size(); i++)
  //  if(HS->edges[i]->getEdgeStringProperty("type") == "Valve")
  //    cout << HS->edges[i]->getEdgeStringProperty("name") << "  " << HS->edges[i]->isClosed << endl;

  HS->solveSystem();

  //HS->saveResult("head","Node");

  cout << endl << endl;
  return 0;
}