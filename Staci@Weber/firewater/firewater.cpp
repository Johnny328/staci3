#include <iostream>
#include "../../HydraulicSolver.h"
#include "../../PressurePoint.h"
#include "../../Node.h"
#include "../../Valve.h"
#include <fstream>
#include <random>
#include <iomanip>
#include <string>
#include <algorithm>
#include <stdio.h>

using namespace std;

int main(int argc, char* argv[]){

  double dzeta = 0.5;

  // Name of containing folder of staci file
  //string case_folder = "/home/rweber/0_PhD/Halozatok/sopron_halozatok/";
  //string case_folder = "/home/rweber/0_PhD/Halozatok/general_alap/";

  string case_name;
  if(argc == 1){
    //case_name = "VIZ-SOPTVR-O-68-input_mod_94_2";
    //case_name = "VIZ-SOPTVR-O-68-input_mod";
    case_name =  "lin_alap_9";
  }else if(argc == 2){
    case_name = argv[1];
  }

  cout << endl << "Case: " << case_name << endl;
  srand((unsigned int) time(0));

  ifstream ifile;
  //ifile.open("nodelist_sop.txt");
  ifile.open("nodelist_lin.txt");
  vector<string> hydrant_nodes;
  string line, temp;
  while(getline(ifile,line)){
    hydrant_nodes.push_back(line);
  }
  ifile.close();

  vector<double> firewater;
  ofstream wfile;
  wfile.open("firewater_"+case_name+".txt");
  time_t ido=time(0);
  for (int i=0; i<hydrant_nodes.size(); i++){
    
    HydraulicSolver *wds;

    wds = new HydraulicSolver(case_name + ".spr");

    int hydrant_idx = -1, idx=0;
    while(!(hydrant_idx+1) && idx<wds->nodes.size()){
      if(hydrant_nodes[i] == wds->nodes[idx]->getName())
        hydrant_idx = idx;
      idx++;
    }

    wds->nodes.push_back(new Node("Hout", 0., 0., wds->nodes[hydrant_idx]->getProperty("height"), 0., 0., 1000.));
    vector<double> e,zeta;
    e.push_back(0);  e.push_back(100);
    zeta.push_back(0);  zeta.push_back(dzeta);
    wds->edges.push_back(new Valve("Hvalve","Hout", hydrant_nodes[i], 1000., 1., e, zeta, 100.,0.));
    wds->edges.push_back(new PressurePoint("Hpres", 1., hydrant_nodes[i], 1000., 100000., 0.));

    wds->initialization();
    wds->buildSystem();
    wds->solveSystem();

    firewater.push_back(wds->edges[wds->edges.size()-1]->getDoubleProperty("massFlowRate")*60.);
    wfile << firewater[i] << '\n';

    delete wds;
  }

  wfile.close();

  cout << "\nCPU : " << time(0)-ido << " s" << endl;
  cout << endl << endl;
  return 0;
}