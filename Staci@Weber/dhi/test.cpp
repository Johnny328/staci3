#include <iostream>
#include <fstream>
#include <random>
#include <iomanip>
#include <string>
#include <algorithm>
#include <stdio.h>

#include "../../Sensitivity.h"

using namespace std;
using namespace Eigen;

int main(int argc, char* argv[]){

  // Name of containing folder of staci file
  string case_folder = "";
  //string case_folder = "/home/rweber/0_PhD/Halozatok/sopron_halozatok/";

  string case_name;
  if(argc == 1){
    case_name = "MUModel.inp";
    //case_name = "C-town.inp";
    //case_name = "Anytown.inp";
    //case_name = "VIZ-SOPTVR-J-55-input_mod.inp";
    //case_name =  "linear_9.inp";
    //case_name =  "grid_9.inp";
    //case_name =  "ky2.inp";
    //case_name =  "Net1.inp";
  }else if(argc == 2){
    case_name = argv[1];
  }

  cout << endl << "Case: " << case_name << endl;
  srand((unsigned int) time(0));

  HydraulicSolver *wds;
  wds = new HydraulicSolver(case_folder + case_name);
  cout << "OK" << endl;
  wds->initialization();

  wds->solveSystem();


  /*cout << endl << "PRES";
  for(int i=0; i<wds->nodes.size(); i++)
  	cout << endl << wds->nodes[i]->getName() << "\t" << wds->nodes[i]->getProperty("head");
  cout << "\n\nFLOW";
  for(int i=0; i<wds->edges.size(); i++)
  	cout << endl << wds->edges[i]->getEdgeStringProperty("name") << "\t" << 3.6*wds->edges[i]->getEdgeDoubleProperty("massFlowRate");

  wds->initialization();
  wds->calculateSensitivity("demand");
  cout << "\npr - dem\n" << wds->pressureSensitivity << endl;
  cout << "\nmf - dem\n" << wds->massFlowRateSensitivity << endl;

  wds->initialization();
  wds->calculateSensitivity("friction_coeff");
  cout << "\npr - fric\n" << wds->pressureSensitivity << endl;
  cout << "\nmf - fric\n" << wds->massFlowRateSensitivity << endl;*/

  cout << endl << endl;
  return 0;
}