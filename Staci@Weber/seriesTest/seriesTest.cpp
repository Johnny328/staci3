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
  string caseFolder = "../../Networks/";
  //string caseFolder = "/home/rweber/0_PhD/Halozatok/sopron_halozatok/";

  string caseName;
  if(argc == 1){
    //caseName = "C-town.inp";
    //caseName = "Anytown.inp";
    caseName = "MUModel.inp";
    //caseName =  "grid_9.inp";
    //caseName =  "ky2.inp";
    //caseName =  "Net1.inp";
  }else if(argc == 2){
    caseName = argv[1];
  }

  cout << endl << "Case: " << caseName << endl;

  SeriesHydraulics *sh;
  sh = new SeriesHydraulics(caseFolder + caseName);
  //for(int i=0; i<sh->edges.size(); i++)
  //  if(sh->edges[i]->getEdgeStringProperty("type") == "Pump")
  //    sh->edges[i]->setStringProperty("curveType", "linear");
  //sh->solveSystem();
  sh->seriesSolve();

    // For checking the disconnected network areas
  /*Shutdown *sd;
  sd = new Shutdown(caseFolder + caseName);

  for(int i=0; i<sd->edges.size(); i++)
    sd->edges[i]->status = sh->edges[i]->status;

  sd->buildSegmentGraph();
  int nSegments = sd->getNumberSegment();
  cout << "Number of segments: " << nSegments << endl;
  sd->saveResult("segment","All");
  cout << endl << "SEGMENT GRAPH OK " << endl;

  vector<int> disc = sd->closeDisconnectedParts();
  cout << "\ndisc: " << disc.size() << endl;
  for(int i=0; i<disc.size(); i++)
  {
    cout << disc[i] << endl;
  }*/

  /*cout << endl << "edges";
  for(int i=0; i<sh->edges.size(); i++)
    cout << endl << sh->edges[i]->name << "   " << sh->edges[i]->volumeFlowRate;
  cout << endl << "nodes";
  for(int i=0; i<sh->nodes.size(); i++)
    cout << endl << sh->nodes[i]->name << "   " << sh->nodes[i]->head;*/

  vector<string> tableEdge;
  tableEdge.push_back("9");
  tableEdge.push_back("21");
  tableEdge.push_back("22");
  tableEdge.push_back("PRV");
  tableEdge.push_back("FCV");
  tableEdge.push_back("valve");
  sh->timeTableEdge(tableEdge,"gpm");

  vector<string> tableNode;
  tableNode.push_back("2");
  tableNode.push_back("23");
  tableNode.push_back("32");
  tableNode.push_back("33");
  sh->timeTableNode(tableNode,"psi");

  //for(int i=0; i<wds->edges.size(); i++)
    //if(wds->edges[i]->getEdgeStringProperty("type") == "PressurePoint" || wds->edges[i]->getEdgeStringProperty("type") == "Pool")
      //cout << wds->edges[i]->getEdgeStringProperty("name") << "  " << wds->edges[i]->getDoubleProperty("volumeFlowRate") << endl;
  //wds->listSystem();
  //wds->saveResult("head","Node");
  //wds->saveResult("volumeFlowRate","Pipe");

  //cout << endl << "NODES" << endl;
  //for(int i=0; i<3; i++){
  //  cout << endl << wds->nodes[i]->getName() << endl;
  //  for(int j=0; j<wds->nodes[i]->vectorDemand.size(); j++){
  //    cout << wds->nodes[i]->vectorDemand[j] << "  " << wds->nodes[i]->vectorPatternID[j] << '\t' << wds->nodes[i]->vectorPatternIndex[j] << endl;
  //  }
  //}

 // HydraulicSolver *wds2;
 // wds2 = new HydraulicSolver(caseFolder + caseName);
 // vector<string> nodeCon;
 // nodeCon.push_back("node_1");
 // nodeCon.push_back("node_7");
 // nodeCon.push_back("node_12");
 // for(int i=0; i<3; i++){
 //   wds2->edges.push_back(new Pipe("PIPE_NEW", nodeCon[i], "node_8", 1000., 100., 0.1, -0.02, 0.0));
 //   wds2->buildSystem();
 //   wds2->solveSystem();
 // }

  cout << endl << endl;
  return 0;
}