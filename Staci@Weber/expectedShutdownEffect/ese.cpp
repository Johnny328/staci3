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

  vector<string> everyCase;
  //everyCase.push_back("villasor");
  //everyCase.push_back("ferto");
  //everyCase.push_back("sanchegy");
  everyCase.push_back("buk");
  //everyCase.push_back("lovo");
  //everyCase.push_back("nagycenk");
  //everyCase.push_back("vashegy");
  //everyCase.push_back("varis");
  //everyCase.push_back("becsidomb");
  //everyCase.push_back("tomalom");
  //everyCase.push_back("jerevan");
  //everyCase.push_back("balf");
  //everyCase.push_back("hermes");

  int nCases = everyCase.size();
  cout << endl << "   CASES\n***********\n";
  for(int i=0; i<nCases; i++)
    cout << everyCase[i] << endl;


  // topological stuff
  vector<double> n1n3p(nCases,0.0);
  vector<double> L1L3p(nCases,0.0);
  vector<double> V1V3p(nCases,0.0);
  // one hydraulic simulation stuff
  vector<double> globalAlfa(nCases,0.0);

  //========================
  //       MAIN CYCLE       
  //========================
  cout << endl << "   PROGRESS\n**************\n";
  for(int i=0; i<nCases; i++)
  {
    string caseName = everyCase[i];
    Shutdown *sd = new Shutdown(caseFolder + caseName + ".inp");
    int nSegments = sd->getNumberSegment();

    // CALCULATING TOPOLOGICAL STUFF
    //*******************************
    vector<int> rank(nSegments,0);
    for(int j=0; j<sd->edges.size(); j++)
    {
      if(sd->edges[j]->getEdgeStringProperty("type") == "Valve")
      {
        rank[sd->edges[j]->getIntProperty("startSegment")]++;
        rank[sd->edges[j]->getIntProperty("endSegment")]++;
      }
    }

    int n1=0, n3p=0;
    for(int j=0; j<nSegments; j++)
    {
      if(rank[j]==1)
        n1++;
      if(rank[j]>=3)
        n3p++;
    }
    n1n3p[i] = (double)n1/(double)n3p;

    double L1=0,L3p=0;
    double V1=0,V3p=0;
    for(int j=0; j<sd->edges.size(); j++)
    {
      if(sd->edges[j]->getEdgeStringProperty("type") == "Pipe")
      {
        if(rank[sd->edges[j]->getEdgeIntProperty("segment")] == 1)
        {
          L1 += sd->edges[j]->getDoubleProperty("length");
          V1 += sd->edges[j]->getDoubleProperty("volume");
        }
        if(rank[sd->edges[j]->getEdgeIntProperty("segment")] >= 3)
        {
          L3p += sd->edges[j]->getDoubleProperty("length");
          V3p += sd->edges[j]->getDoubleProperty("volume");
        }
      }
    }
    L1L3p[i] = L1/L3p;
    V1V3p[i] = V1/V3p;

    // CALCULATING ALFA | ONE HYDRAULIC SIMULATION
    //*********************************************
    HydraulicSolver *hs = new HydraulicSolver(caseFolder + caseName + ".inp");
    hs->solveSystem();

    vector<double> localAlfa(nSegments,0.0);
    vector<double> length(nSegments,0.0);
    for(int j=0; j<hs->edges.size(); j++)
    {
      // Calculating the lengths of the segments
      if(hs->edges[j]->getEdgeStringProperty("type") == "Pipe")
      {
        length[sd->edges[j]->getEdgeIntProperty("segment")] += hs->edges[j]->getDoubleProperty("length");
      }
      // Calculating the volume flow rate in the segments
      double vf = hs->edges[j]->getEdgeDoubleProperty("volumeFlowRate");
      if(hs->edges[j]->getEdgeStringProperty("type") == "Valve")
      {
        if(vf>0.)
          localAlfa[sd->edges[j]->getIntProperty("endSegment")] += vf;
        if(vf<0.)
          localAlfa[sd->edges[j]->getIntProperty("startSegment")] += -vf;
      }
      if(hs->edges[j]->getEdgeStringProperty("type") == "PressurePoint" || hs->edges[j]->getEdgeStringProperty("type") == "Pool")
      { 
        if(vf<0.)
          localAlfa[sd->edges[j]->getEdgeIntProperty("segment")] += -vf;
      }
    }

    // reranging alfa to [0,1] by deviding with the sum inflow
    double sumDemand=0.0;
    for(int j=0; j<hs->nodes.size(); j++)
        sumDemand += hs->nodes[j]->getProperty("demand");
    for(int j=0; j<localAlfa.size(); j++)
      localAlfa[j] /= sumDemand;

    double sumLength=0.0;
    for(int j=0; j<nSegments; j++)
      sumLength += length[j];
    for(int j=0; j<nSegments; j++)
      globalAlfa[i] += localAlfa[j]*length[j];
    globalAlfa[i] /= sumLength;

    for(int k=0; k<nSegments; k++)
    {
      for(int j=0; j<sd->edges.size(); j++)
      {
        if(k == sd->edges[j]->getEdgeIntProperty("segment"))
          hs->edges[j]->setEdgeDoubleProperty("userOutput",localAlfa[k]);
      }
      for(int j=0; j<sd->nodes.size(); j++)
      {
        if(k == sd->nodes[j]->getProperty("segment"))
          hs->nodes[j]->setProperty("userOutput",localAlfa[k]);
      }
    }
    //hs->saveResult("userOutput","Pipe");
    //hs->saveResult("userOutput","Node");
    //hs->saveResult("volumeFlowRate","Pipe");
    //hs->saveResult("userOutput","Pool");
    //hs->saveResult("userOutput","PressurePoint");
    //sd->saveResult("segment","All");

    //printf("\n%-2s  %-8s  %-8s","j","Alfa_loc","length");
    //for(int j=0; j<localAlfa.size(); j++)
    //  printf("\n%2i  %8.3f  %8.3f",j,localAlfa[j],length[j]);

    //printf("\n%16s: %6.4f", everyCase[i].c_str(), globalAlfa[i]);
    //cout << endl;

    // CALCULATING BETA NEW
    //*******************************
    /*for(int j=0; j<nSegments; j++)
    {
      sd->closeSegment(j);

      for(int k=0; k<hs->edges.size(); k++){
        if(sd->edges[k]->isClosed)
          hs->changeEdgeStatus(k,false);
        else
          hs->changeEdgeStatus(k,true);
      }

      hs->solveSystem();

      double sumCons=0., sumDem=0.;
      for(int k=0; k<hs->nodes.size(); k++)
      {
        sumCons = hs->nodes[k]->getProperty("consumption");
        sumDem = hs->nodes[k]->getProperty("demand");
      }
    }*/

    cout << everyCase[i] << ": OK" << endl;
  }

  cout << "\n   RESULTS   ";
  cout << "\n*************\n" << endl;
  printf("%16s | %2s | %8s | %8s | %8s | %8s\n", "case name", "i", "n1/n3+", "L1/L3+", "V1/V3+", "alfa");
  cout << "-----------------------------------------------------------------\n";
  for(int i=0; i<nCases; i++)
  {
    printf("%16s | %2i | %8.4f | %8.4f | %8.4f | %8.4f\n", everyCase[i].c_str(), i+1, n1n3p[i], L1L3p[i], V1V3p[i], globalAlfa[i]);
  }

  cout << "\n CORRELATIONS";
  cout << "\n**************\n" << endl;
  cout << "alfa - n1n3p: " << correlCoefficient(n1n3p,globalAlfa) << endl;
  cout << "alfa - L1L3p: " << correlCoefficient(L1L3p,globalAlfa) << endl;
  cout << "alfa - V1V3p: " << correlCoefficient(V1V3p,globalAlfa) << endl;

  cout << endl << endl;
  return 0;
}