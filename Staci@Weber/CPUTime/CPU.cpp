#include <iostream>
#include "../../Sensitivity.h"
#include <fstream>
#include <random>
#include <iomanip>
#include <string>
#include <algorithm>
#include <stdio.h>

using namespace std;
using namespace Eigen;

int main(int argc, char* argv[]){

  // Name of containing folder of staci file
  string case_folder = "../../Networks/";

  string case_name;
  if(argc == 1){
    case_name = "Anytown";
    //case_name =  "linear_9";
    //case_name =  "grid_9";
    //case_name =  "ky2";
    //case_name =  "Net1";
  }else if(argc == 2){
    case_name = argv[1];
  }

  cout << endl << "Case: " << case_name << endl;
  srand((unsigned int) time(0));

  Sensitivity *wds;
  clock_t ido = clock();
  wds = new Sensitivity(case_folder + case_name + ".spr");
  cout << "Nodes: " << wds->nodes.size() << endl << "Edges: " << wds->edges.size() << endl;
  cout << endl << "\nKonst:   " << double(clock()-ido)/ CLOCKS_PER_SEC << " s" << endl;
  ido = clock();
  wds->initialization();
  cout << endl << "\nInit:    " << double(clock()-ido)/ CLOCKS_PER_SEC << " s" << endl;
  ido = clock();
  wds->buildSystem();
  cout << endl << "\nBuild:   " << double(clock()-ido)/ CLOCKS_PER_SEC << " s" << endl;
  ido = clock();
  wds->solveSystem();
  cout << endl << "\nSolver:  " << double(clock()-ido)/ CLOCKS_PER_SEC << " s" << endl;
  wds->initialization();
  ido = clock();
  wds->calculateSensitivity("friction_coeff");
  cout << endl << "\nSensitivity, fric:  " << double(clock()-ido)/ CLOCKS_PER_SEC << " s" << endl;
  wds->initialization();
  ido = clock();
  wds->calculateSensitivity("demand");
  cout << endl << "\nSensitivity, demand:  " << double(clock()-ido)/ CLOCKS_PER_SEC << " s" << endl;

  cout << endl << endl;
  return 0;
}