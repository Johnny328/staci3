/*===================================================================*\
                              staci2epanet  
                            ----------------

  This code converts a STACI file (.spr) to an EPANET file (.net).
  Exporting nodes, pipelines, valves, pumps, pools, pressure points.

  HOW TO USE: 

      - CASE_NAME is the name of the file without .spr extension

      - SET:  nothing :)

      - OUTPUT: CASE_NAME.net            File for EPANET

      - RUN: ./staci2epanet CASE_NAME

      - COMPILE: make -f make_staci2epanet.mk

    staci3
    Cs. Hos, R. Weber, T. Huzsvar
    https://github.com/weberrichard/staci3
\*==================================================================*/

#include <iostream>
#include <fstream>
#include <random>
#include <iomanip>
#include <string>
#include <algorithm>
#include <stdio.h>

#include "../../Staci.h"

using namespace std;

Staci *wds;

int main(int argc, char* argv[]){

  string case_name;
  if(argc == 1){
    case_name = "Anytown";
  }else if(argc == 2){
    case_name = argv[1];
  }

  cout << endl << "Case: " << case_name << endl;

  // Creating the case, loading the data
  wds = new Staci(case_name + ".spr");
  wds->buildSystem();
  int n_nodes = wds->nodes.size(), n_edges=wds->edges.size();

  // Start of writing the file
  string file_name = case_name + ".inp";
  FILE *wfile;
  wfile = fopen(file_name.c_str(),"w");

  fprintf(wfile, "[TITLE]\n");
  fprintf(wfile, "%s\n", case_name.c_str());
  fprintf(wfile, "%s\n", ("Converted from Epanet to Staci, case name: " + case_name + "\n").c_str());

  // TODO: Demand pattern
  fprintf(wfile, "\n[JUNCTIONS]\n");
  fprintf(wfile, ";ID              \tElev        \tDemand      \tPattern         \n");
  for(int i=0; i<n_nodes; i++){
    fprintf(wfile, " %-16s\t%-12.2f\t%-12.2f\t%-16s\t;\n", wds->nodes[i]->getName().c_str(),wds->nodes[i]->getProperty("geodeticHeight"),wds->nodes[i]->getProperty("demand"),"");
  }

  fprintf(wfile, "\n[RESERVOIRS]\n");
  fprintf(wfile, ";ID              \tHead        \tPattern         \n");
  for(int i=0; i<n_edges; i++){
    if(wds->edges[i]->getEdgeStringProperty("type")=="PressurePoint")
      fprintf(wfile," %-16s\t%-12.2f\t%-16s\n",wds->edges[i]->getEdgeStringProperty("name").c_str(),wds->edges[i]->getDoubleProperty("pressure"),"");
  }

  // TODO: min/max level
  fprintf(wfile, "\n[TANKS]\n");
  fprintf(wfile, ";ID             \tElevation   \tInitLevel   \tMinLevel    \tMaxLevel    \tDiameter    \tMinVol      \tVolCurve\n");
  for(int i=0; i<n_edges; i++){
    if(wds->edges[i]->getEdgeStringProperty("type")=="Pool")
      fprintf(wfile, " %-16s\t%-12.2f\t%-12.2f\t%-12.2f\t%-12.2f\t%-12.2f\t%-12.2f\t%-16s\t\n", wds->edges[i]->getEdgeStringProperty("name").c_str(),wds->edges[i]->getDoubleProperty("bottomLevel"),wds->edges[i]->getDoubleProperty("waterLevel"),0.0,1000.0,pow(wds->edges[i]->getDoubleProperty("referenceCrossSection"),.5)*4./M_PI,0.,"");
  }

  // TODO: STATUS
  fprintf(wfile, "\n[PIPES]\n");
  fprintf(wfile, ";ID             \tNode1           \tNode2           \tLength      \tDiameter    \tRoughness   \tMinorLoss   \tStatus\n");
  for(int i=0; i<n_edges; i++){
    if(wds->edges[i]->getEdgeStringProperty("type") == "Pipe")
      fprintf(wfile, " %-16s\t%-16s\t%-16s\t%-12.2f\t%-12.2f\t%-12.2f\t%-12.2f\t%-6s;\n",wds->edges[i]->getEdgeStringProperty("name").c_str(),wds->edges[i]->getEdgeStringProperty("startNodeName").c_str(),wds->edges[i]->getEdgeStringProperty("endNodeName").c_str(),wds->edges[i]->getDoubleProperty("length"),wds->edges[i]->getDoubleProperty("diameter"),wds->edges[i]->getDoubleProperty("roughness"),0.0,"Open");
  }

  // TODO: Pump curves
  fprintf(wfile, "\n[PUMPS]\n");
  fprintf(wfile, ";ID             \tNode1           \tNode2           \tParameters\n");
  int pump_counter=0;
  for(int i=0; i<n_edges; i++){
    if(wds->edges[i]->getEdgeStringProperty("type") == "Pump"){
      fprintf(wfile, " %-16s\t%-16s\t%-16s\t%-6s;\n", wds->edges[i]->getEdgeStringProperty("name").c_str(), wds->edges[i]->getEdgeStringProperty("startNodeName").c_str(), wds->edges[i]->getEdgeStringProperty("endNodeName").c_str(), ("HEAD "+to_string(pump_counter)).c_str());
      pump_counter++;
    }
  }

  // TODO: Different type of valves
  fprintf(wfile, "\n[VALVES]\n");
  fprintf(wfile, ";ID             \tNode1           \tNode2           \tDiameter    \tType\tSetting     \tMinorLoss   \n");
  for(int i=0; i<n_edges; i++){
    if(wds->edges[i]->getEdgeStringProperty("type") == "Valve")
      fprintf(wfile, " %-16s,%-16s,%-16s,%-12.2f,%-4s,%-12.2f,%-12.2f\n", wds->edges[i]->getEdgeStringProperty("name").c_str(), wds->edges[i]->getEdgeStringProperty("startNodeName").c_str(), wds->edges[i]->getEdgeStringProperty("endNodeName").c_str(), pow(wds->edges[i]->getDoubleProperty("referenceCrossSection"),.5)*4/M_PI, "TCV",0.0,0.0);
  }

  fprintf(wfile, "\n[TAGS]\n");

  fprintf(wfile, "\n[DEMANDS]\n");
  fprintf(wfile, ";Junction       \tDemand      \tPattern         \tCategory\n");

  // TODO: ADD STATUS i.e. pumps/valves are closed or open
  fprintf(wfile, "\n[STATUS]\n");
  fprintf(wfile, ";ID              \tStatus/Setting\n");

  // TODO: ADD PATTERNS
  fprintf(wfile, "\n[PATTERNS]\n");
  fprintf(wfile, ";ID             \tMultipliers\n");

  // TODO: PUMP CURVES
  fprintf(wfile, "\n[CURVES]\n");
  fprintf(wfile, ";ID             \tX-Value     \tY-Value\n");
  pump_counter = 0;
  for(int i=0; i<n_edges; i++){
    if(wds->edges[i]->getEdgeStringProperty("type") == "Pump"){
      vector <double> pumpCurveVolumeFlowRate = wds->edges[i]->getVectorProperty("volumeFlowRate");
      vector <double> pumpCurveHead = wds->edges[i]->getVectorProperty("head");
      fprintf(wfile, " %-16i\t%-12.4f\t%-12.4f\n", pump_counter, pumpCurveVolumeFlowRate[i], pumpCurveHead[i]);
      pump_counter++;
    }
  }

  fprintf(wfile, "\n[CONTROLS]\n");

  fprintf(wfile, "\n[RULES]\n");

  fprintf(wfile, "\n[ENERGY]\n");

  fprintf(wfile, "\n[EMITTERS]\n");

  fprintf(wfile, "\n[QUALITY]\n");

  fprintf(wfile, "\n[SOURCES]\n");

  fprintf(wfile, "\n[REACTIONS]\n");

  fprintf(wfile, "\n[MIXING]\n");

  // TODO: for extended calculations
  fprintf(wfile, "\n[TIMES]\n");

  fprintf(wfile, "\n[REPORT]\n");

  fprintf(wfile, "\n[OPTIONS]\n");
  fprintf(wfile, " Units              \tLPS\n");
  if(wds->getFrictionModel() == "HW")
    fprintf(wfile, " Headloss           \tH-W\n");
  if(wds->getFrictionModel() == "DW")
    fprintf(wfile, " Headloss           \tD-W\n");
  fprintf(wfile, " Specific Gravity   \t1.0\n");
  fprintf(wfile, " Viscosity          \t1.0\n");
  fprintf(wfile, " Trials             \t40\n");
  fprintf(wfile, " Accuracy           \t0.001\n");
  fprintf(wfile, " CHECKFREQ          \t2\n");
  fprintf(wfile, " MAXCHECK           \t10\n");
  fprintf(wfile, " DAMPLIMIT          \t0\n");
  fprintf(wfile, " Unbalanced         \tContinue 10\n");
  fprintf(wfile, " Pattern            \t1\n");
  fprintf(wfile, " Demand Multiplier  \t1.0\n");
  fprintf(wfile, " Emitter Exponent   \t0.5\n");
  fprintf(wfile, " Quality            \tChlorine mg/L\n");
  fprintf(wfile, " Diffusivity        \t1.0\n");
  fprintf(wfile, " Tolerance          \t0.01\n");

  // TODO: xpos or xcoord??
  fprintf(wfile, "\n[COORDINATES]\n");
  fprintf(wfile, ";Node           \tX-Coord         \tY-Coord\n");
  for(int i=0; i<n_nodes; i++){
    fprintf(wfile, " %-16s\t%-16.3f\t%-16.3f\n", wds->nodes[i]->getName().c_str(), wds->nodes[i]->getProperty("xPosition"), wds->nodes[i]->getProperty("yPosition"));
  }

  fprintf(wfile, "\n[VERTICES]\n");
  fprintf(wfile, "\n[LABELS]\n");
  fprintf(wfile, "\n[BACKDROP]\n");

  // End of writing the file
  fprintf(wfile, "\n[END]\n");
  fclose(wfile);

  cout << endl << endl;
  return 0;
}