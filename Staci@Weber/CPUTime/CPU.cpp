#include <iostream>
#include "../../Sensitivity.h"
#include <fstream>
#include <random>
#include <iomanip>
#include <string>
#include <algorithm>
#include <stdio.h>

using namespace std;

int main(int argc, char* argv[]){

  // Name of containing folder of staci file
  //string case_folder = "/mnt/d/Linux/Staci_On_Windows/staci-master/Halozatok/sopron_halozatok/";
  string case_folder = "/home/rweber/0_PhD/Halozatok/sopron_halozatok/";
  //string case_folder = "/home/rweber/0_PhD/Halozatok/general_alap/";

  string case_name;
  if(argc == 1){
    //case_name = "VIZ-SOPTVR-O-68-input_mod_94_2";
    //case_name = "VIZ-SOPTVR-O-68-input_mod";
    //case_name = "VIZ-SOPTVR-J-55-input_mod";
    case_name = "VIZ-SOPTVR-M-input_mod";
    //case_name =  "lin_alap_9";
    //case_name =  "racs_alap_9";
  }else if(argc == 2){
    case_name = argv[1];
  }

  cout << endl << "Case: " << case_name << endl;
  srand((unsigned int) time(0));

  Sensitivity *wds;
  clock_t ido = clock();
  wds = new Sensitivity(case_folder + case_name + ".spr");
  cout << "Nodes: " << wds->nodes.size() << endl << "Edges: " << wds->edges.size() << endl;
  wds->setDebugLevel(2);
  cout << "\nKonst:   " << double(clock()-ido)/ CLOCKS_PER_SEC << " s" << endl;
  ido = clock();
  wds->initialization();
  cout << "\nInit:    " << double(clock()-ido)/ CLOCKS_PER_SEC << " s" << endl;
  ido = clock();
  wds->buildSystem();
  cout << "\nBuild:   " << double(clock()-ido)/ CLOCKS_PER_SEC << " s" << endl;
  ido = clock();
  wds->solveSystem();
  cout << "\nSolver:  " << double(clock()-ido)/ CLOCKS_PER_SEC << " s" << endl;
  wds->initialization();
  ido = clock();
  wds->calculateSensitivity("friction_coeff",0);
  cout << "\nS fric:  " << double(clock()-ido)/ CLOCKS_PER_SEC << " s" << endl;
  wds->initialization();
  ido = clock();
  wds->calculateSensitivity("demand",0);
  cout << "\nS demand: " << double(clock()-ido)/ CLOCKS_PER_SEC << " s" << endl;

  cout << endl << endl;
  return 0;
}