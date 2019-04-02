#include "Staci.h"

using namespace Eigen;

Staci::Staci(string spr_filename) {
  definitionFile = spr_filename;

  IOxml IOxmlObj(definitionFile.c_str());
  IOxmlObj.loadSystem(nodes, edges);

  frictionModel = IOxmlObj.readSetting("friction_model");
}

//--------------------------------------------------------------
Staci::~Staci() { 
}

//--------------------------------------------------------------
void Staci::buildSystem() {
  bool startGotIt = false, endGotIt = false;
  int j = 0;
  int startNode = -1, endNode = -1;

  for(int i=0; i<edges.size(); i++){
    startGotIt = false;
    j = 0;
    while((j < nodes.size()) && (!startGotIt)){
      if((edges[i]->getEdgeStringProperty("startNodeName")).compare(nodes[j]->getName()) == 0){
        startGotIt = true;
        startNode = j;
        nodes[j]->edgeOut.push_back(i);
      }
      j++;
    }
    if(!startGotIt){
      cout << "\n!!! ERROR !!! Edge name: " << edges[i]->getEdgeStringProperty("name").c_str() << ", startnode not found !!!";
      cout << endl << "startNode: " << startNode << endl << "Exiting..." << endl;
      exit(-1);
    } 

    if(edges[i]->getEdgeIntProperty("numberNode") == 2){
      endGotIt = false;
      j = 0;
      while ((j < nodes.size()) && (!endGotIt)) {
        if ((edges[i]->getEdgeStringProperty("endNodeName")).compare(nodes[j]->getName()) == 0) {
          endGotIt = true;
          endNode = j;
          nodes[j]->edgeIn.push_back(i);
        }
        j++;
      }
      if(!endGotIt) {
        cout << "\n!!! ERROR !!! Edge name: " << edges[i]->getEdgeStringProperty("name").c_str() << ", startnode not found !!!";
        cout << endl << "startNode: " << startNode << endl << "Exiting..." << endl;
        exit(-1);
      }
    }

    if(edges[i]->getEdgeIntProperty("numberNode") == 2){
      edges[i]->setEdgeIntProperty("startNodeIndex", startNode);
      edges[i]->setEdgeIntProperty("endNodeIndex", endNode);
    }
    else{
      edges[i]->setEdgeIntProperty("startNodeIndex", startNode);
      edges[i]->setEdgeIntProperty("endNodeIndex", -1);
    }
  }

  for(int i = 0; i < edges.size(); i++){
    if(edges[i]->getEdgeStringProperty("type") == "Pipe")
      edges[i]->setFrictionModel(frictionModel);
  }
}

//--------------------------------------------------------------
void Staci::listSystem() {
  cout << "\n\n Nodes:\n--------------------------";
  for (int i = 0; i < nodes.size(); i++)
    cout << nodes[i]->info(true);
  cout << "\n\n Edges:\n--------------------------";
  for (int i = 0; i < edges.size(); i++)
    cout << edges[i]->info();
}


/*! WR: find the the given node IDs and gives back the Indicies
  *id:  vector containing the IDs
  IMPORTANT:  if only one ID not found, drops ERROR with exit(-1)*/
vector<int> Staci::ID2Index(const vector<string> &id){
  int n_id = id.size();
  bool gotIt = false;
  vector<int> idx(n_id);
  for(int j=0;j<n_id;j++){
    int i=0;
    gotIt = false;
    while(!gotIt && i<nodes.size()){
      if(id[j] == nodes[i]->getName()){
        idx[j] = i;
        gotIt = true;
      }
      i++;
    }
    if(gotIt == false)
      cout << "\n!!!WARNING!!!\nStaci:ID2Indicies function\nNode is not existing, id: " << id[j] << endl<< "\ncontinouing...";
  }
  return idx;
}

//--------------------------------------------------------------
void Staci::checkSystem()
{
  ostringstream msg1;
  bool stop = false;

  cout << endl << " [*] Checking System  ";

  cout << "\n [*] Looking for identical IDs  ";
  string name1, name2;
  for(int i = 0; i < edges.size(); i++){
    name1 = edges.at(i)->getEdgeStringProperty("name");
    for(int j = 0; j < edges.size(); j++){
      name2 = edges.at(j)->getEdgeStringProperty("name");
      if(i != j){
        if(name1 == name2){
          cout << "\n !!!ERROR!!! edge #" << i << ": " << name1 << " and edge #" << j << ": " << name2 << " with same ID!" << endl;
          stop = true;
        }
      }
    }

    for(int j = 0; j < nodes.size(); j++){
      name2 = nodes.at(j)->getName();
      if(i != j){
        if(name1 == name2){
          cout << "\n !!!ERROR!!! edge #" << i << ": " << name1 << " and node #" << j << ": " << name2 << " with same ID!" << endl;
          stop = true;
        }
      }
    }
  }

  if (stop)
    exit(-1);
  else
    cout << endl << " [*] Checking System:  OK";
}