/*===================================================================*\
                              staci2epanet  
                            ----------------

  This code converts a STACI file (.spr) to an EPANET file (.inp).
  Exporting nodes, pipelines, valves, pumps, pools, pressure points.

  HOW TO USE: 

      - CASE_NAME is the name of the file without .spr extension

      - SET:  nothing :)

      - OUTPUT: CASE_NAME.inp            File for EPANET

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

  string case_folder = "../../Networks/";
  string case_name;
  if(argc == 1){
    case_name = "linear_3.spr";
  }else if(argc == 2){
    case_name = argv[1];
  }

  // Creating the case, loading the data
  wds = new Staci(case_folder + case_name);
  int n_nodes = wds->nodes.size(), n_edges=wds->edges.size();
  cout << endl << "[*] File loaded successfully: " << case_folder + case_name << endl;

  // Adding the elevation to pressure points
  for(int i=0; i<wds->edges.size(); i++){
    if(wds->edges[i]->getEdgeStringProperty("type") == "PressurePoint"){
      int idx=-1;
      for(int j=0; j<wds->nodes.size(); j++)
        if(wds->nodes[j]->getName()==wds->edges[i]->getEdgeStringProperty("startNodeName"))
          idx = j;

    wds->edges[i]->setDoubleProperty("head", (wds->edges[i]->getDoubleProperty("head") + wds->nodes[idx]->getProperty("height"))); // meter to Pascal minus elevation
    }
  }

  // Warning about the isolation valves if the connecting node does not have a rank of 2
  vector<int> nodeRank(wds->nodes.size(), 0);
  for(int i=0; i<wds->edges.size(); i++)
    if(wds->edges[i]->getEdgeStringProperty("type") == "Pipe" || wds->edges[i]->getEdgeStringProperty("type") == "Pump"){
      nodeRank[wds->edges[i]->getEdgeIntProperty("startNodeIndex")]++;
      nodeRank[wds->edges[i]->getEdgeIntProperty("endNodeIndex")]++;
    }

  vector<int> isolationNode;
  vector<string> isolationNodeEnd;
  for(int i=0; i<wds->edges.size(); i++){
    if(wds->edges[i]->getEdgeStringProperty("type") == "Valve"){
      isolationNode.push_back(wds->edges[i]->getEdgeIntProperty("startNodeIndex"));
      isolationNodeEnd.push_back(wds->edges[i]->getEdgeStringProperty("endNodeName"));
      // if the rank is higher than two, then do something...
      if(nodeRank[isolationNode.back()] > 2)
        cout << "\n!WARNING! Node " << wds->nodes[isolationNode.back()]->getName() << " has rank of " << nodeRank[isolationNode.back()] << "\n Do something about it..." << endl;
      // Deleting the node and the valve if the rank is one
      if(nodeRank[isolationNode.back()] == 1){
        cout << "\n Node " << wds->nodes[isolationNode.back()]->getName() << " has rank of " << nodeRank[isolationNode.back()] << ", thus it the end node and the valve will be deleted from INP" << endl;
        wds->edges.erase(wds->edges.begin()+i);
        for(int j=0; j<wds->nodes.size(); j++)
          if(j == wds->edges[i]->getEdgeIntProperty("endNodeIndex"))
            wds->nodes.erase(wds->nodes.begin()+j);
      }
    }
  }

  // Reconnecting the pipes with including the isolation valve
  for(int j=0; j<isolationNode.size(); j++){
    for(int i=0; i<wds->edges.size(); i++){
      if(wds->edges[i]->getEdgeStringProperty("type") == "Pipe" || wds->edges[i]->getEdgeStringProperty("type") == "Pump"){
        if(wds->edges[i]->getEdgeIntProperty("startNodeIndex") == isolationNode[j]){
          wds->edges[i]->setEdgeStringProperty("startNodeName",isolationNodeEnd[j]);
          break;
        }
        if(wds->edges[i]->getEdgeIntProperty("endNodeIndex") == isolationNode[j]){
          wds->edges[i]->setEdgeStringProperty("endNodeName",isolationNodeEnd[j]);
          break;
        }
      }
    }
  }

  // Renaming everything
 for(int i=0; i<wds->nodes.size(); i++)
  {
    string oldName = wds->nodes[i]->getName();
    string newName = "node_"+to_string(i);
    wds->nodes[i]->setName(newName);
    for(int j=0; j<wds->edges.size(); j++)
    {
      if(wds->edges[j]->getEdgeStringProperty("startNodeName") == oldName)
        wds->edges[j]->setEdgeStringProperty("startNodeName",newName);
      if(wds->edges[j]->getEdgeStringProperty("endNodeName") == oldName)
        wds->edges[j]->setEdgeStringProperty("endNodeName",newName);
    }
  }

  int presCounter=0, poolCounter=0, valveCounter=0, pipeCounter=0, pumpCounter=0;
  for(int i=0; i<wds->edges.size(); i++)
  {
    string type = wds->edges[i]->getEdgeStringProperty("type");
    if(type == "PressurePoint")
    {
      wds->edges[i]->setEdgeStringProperty("name","pres_"+to_string(presCounter));
      presCounter++;
    }
    if(type == "Pool")
    {
      wds->edges[i]->setEdgeStringProperty("name","pool_"+to_string(poolCounter));
      poolCounter++;
    }
    if(type == "Valve")
    {
      wds->edges[i]->setEdgeStringProperty("name","valve_"+to_string(valveCounter));
      valveCounter++;
    }
    if(type == "Pipe")
    {
      wds->edges[i]->setEdgeStringProperty("name","pipe_"+to_string(pipeCounter));
      pipeCounter++;
    }
    if(type == "Pump")
    {
      wds->edges[i]->setEdgeStringProperty("name","pump_"+to_string(pumpCounter));
      pumpCounter++;
    }
  }

  // finding the nodes with pressurepoint or pool
  vector<bool> isRealNode(wds->nodes.size(),true);
  for(int i=0; i<wds->nodes.size(); i++)
  {
    for(int j=0; j<wds->edges.size(); j++)
    {
      string type = wds->edges[j]->getEdgeStringProperty("type");
      if(type == "PressurePoint" || type == "Pool")
      {
        if(wds->nodes[i]->getName() == wds->edges[j]->getEdgeStringProperty("startNodeName"))
        {
          isRealNode[i] = false;
          if(wds->nodes[i]->getProperty("demand") != 0)
            cout << "!WARNING! Pressure point " << wds->nodes[i]->getName() << " DOES have non-zero demand " << wds->nodes[i]->getProperty("demand") << endl;
          string node_name = wds->nodes[i]->getName();
          wds->nodes[i]->setName(wds->edges[j]->getEdgeStringProperty("name"));
          for(int k=0; k<wds->edges.size(); k++)
          {
            if(node_name == wds->edges[k]->getEdgeStringProperty("startNodeName"))
              wds->edges[k]->setEdgeStringProperty("startNodeName",wds->nodes[i]->getName());
            if(node_name == wds->edges[k]->getEdgeStringProperty("endNodeName"))
              wds->edges[k]->setEdgeStringProperty("endNodeName",wds->nodes[i]->getName());
          }
        }
      }
    }
  }

  // Start of writing the file
  string file_name = case_folder + case_name.substr(0,case_name.length()-4) + ".inp";
  FILE *wfile;
  wfile = fopen(file_name.c_str(),"w");

  fprintf(wfile, "[TITLE]\n");
  fprintf(wfile, "%s\n", ("Converted from Epanet to Staci, case name: " + case_name + "\n").c_str());

  fprintf(wfile, "\n[JUNCTIONS]\n");
  fprintf(wfile, ";ID              \tElev        \tDemand      \tPattern         \n");
  for(int i=0; i<n_nodes; i++){
    if(isRealNode[i])
      fprintf(wfile, " %-16s\t%-12.4f\t%-12.6f\t%-16s\t;\n", wds->nodes[i]->getName().c_str(),wds->nodes[i]->getProperty("geodeticHeight"),wds->nodes[i]->getProperty("demand"),"");
  }

  fprintf(wfile, "\n[RESERVOIRS]\n");
  fprintf(wfile, ";ID              \tHead        \tPattern         \n");
  for(int i=0; i<n_edges; i++){
    if(wds->edges[i]->getEdgeStringProperty("type")=="PressurePoint"){
      fprintf(wfile," %-16s\t%-12.4f\t%-16s\n",wds->edges[i]->getEdgeStringProperty("name").c_str(),wds->edges[i]->getDoubleProperty("height") + wds->edges[i]->getDoubleProperty("head"),"");
    }
  }

  // TODO: min/max level
  fprintf(wfile, "\n[TANKS]\n");
  fprintf(wfile, ";ID             \tElevation   \tInitLevel   \tMinLevel    \tMaxLevel    \tDiameter    \tMinVol      \tVolCurve\n");
  for(int i=0; i<n_edges; i++){
    if(wds->edges[i]->getEdgeStringProperty("type")=="Pool")
      fprintf(wfile, " %-16s\t%-12.4f\t%-12.4f\t%-12.4f\t%-12.4f\t%-12.4f\t%-12.4f\t%-16s\t\n", wds->edges[i]->getEdgeStringProperty("name").c_str(),wds->edges[i]->getDoubleProperty("bottomLevel"),wds->edges[i]->getDoubleProperty("waterLevel"),0.0,1000.0,pow(wds->edges[i]->getDoubleProperty("referenceCrossSection"),.5)*4./M_PI,0.,"");
  }

  fprintf(wfile, "\n[PIPES]\n"); // diameter is in mm in INP, while it is in m in SPR
  fprintf(wfile, ";ID             \tNode1           \tNode2           \tLength      \tDiameter    \tRoughness   \tMinorLoss   \tStatus\n");
  for(int i=0; i<n_edges; i++){
    if(wds->edges[i]->getEdgeStringProperty("type") == "Pipe")
      fprintf(wfile, " %-16s\t%-16s\t%-16s\t%-12.4f\t%-12.4f\t%-12.4f\t%-12.4f\t%-6s;\n",wds->edges[i]->getEdgeStringProperty("name").c_str(),wds->edges[i]->getEdgeStringProperty("startNodeName").c_str(),wds->edges[i]->getEdgeStringProperty("endNodeName").c_str(),wds->edges[i]->getDoubleProperty("length"),wds->edges[i]->getDoubleProperty("diameter")*1000.,wds->edges[i]->getDoubleProperty("roughness"),0.0,"Open");
    //if(wds->edges[i]->getEdgeStringProperty("type") == "PressurePoint")
    //  fprintf(wfile, " %-16s\t%-16s\t%-16s\t%-12.4f\t%-12.4f\t%-12.4f\t%-12.4f\t%-6s;\n",("PIPE_" + wds->edges[i]->getEdgeStringProperty("name")).c_str(),wds->edges[i]->getEdgeStringProperty("name").c_str(),wds->edges[i]->getEdgeStringProperty("startNodeName").c_str(),0.0,1.0,0.0,0.0,"Open");
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
      fprintf(wfile, " %-16s\t%-16s\t%-16s\t%-12.4f\t%-4s\t%-12.4f\t%-12.4f\n", wds->edges[i]->getEdgeStringProperty("name").c_str(), wds->edges[i]->getEdgeStringProperty("startNodeName").c_str(), wds->edges[i]->getEdgeStringProperty("endNodeName").c_str(), 100., "ISO",0.0,0.0);
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
      for(int j=0; j<pumpCurveVolumeFlowRate.size(); j++)
        fprintf(wfile, " %-16i\t%-12.4f\t%-12.4f\n", pump_counter, pumpCurveVolumeFlowRate[j], pumpCurveHead[j]);
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
  else if(wds->getFrictionModel() == "DW")
    fprintf(wfile, " Headloss           \tD-W\n");
  else
    cout << endl << "WARNING, unkown friction model: " << wds->getFrictionModel() << endl;
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
    fprintf(wfile, " %-16s\t%-16.4f\t%-16.4f\n", wds->nodes[i]->getName().c_str(), wds->nodes[i]->getProperty("xPosition"), wds->nodes[i]->getProperty("yPosition"));
  }
  /*for(int i=0; i<n_edges; i++){
    if(wds->edges[i]->getEdgeStringProperty("type") == "PressurePoint" || wds->edges[i]->getEdgeStringProperty("type") == "Pool"){
      for(int j=0; j<n_nodes; j++){
        if(wds->edges[i]->getEdgeStringProperty("startNodeName") == wds->nodes[j]->getName())
          fprintf(wfile, " %-16s\t%-16.4f\t%-16.4f\n", wds->edges[i]->getEdgeStringProperty("name").c_str(), wds->nodes[j]->getProperty("xPosition"), wds->nodes[j]->getProperty("yPosition"));
      }
    }
  }*/

  fprintf(wfile, "\n[VERTICES]\n");
  fprintf(wfile, "\n[LABELS]\n");
  fprintf(wfile, "\n[BACKDROP]\n");

  // End of writing the file
  fprintf(wfile, "\n[END]\n");
  fclose(wfile);

  cout << endl << "[*] File written successfully: " << file_name << endl;

  cout << endl << endl;
  return 0;
}