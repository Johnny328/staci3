#include "Staci.h"

using namespace Eigen;

Staci::Staci(string spr_filename) {
  definitionFile = spr_filename;

  IOxml IOxmlObj(definitionFile.c_str());
  IOxmlObj.loadSystem(nodes, edges);

  frictionModel = IOxmlObj.readSetting("friction_model");
  for(int i=0; i<edges.size(); i++)
  {
    if(edges[i]->getEdgeStringProperty("type") == "Pipe")
      edges[i]->setFrictionModel(frictionModel);
  }

  // Finding the indicies of nodes for the edges and vica versa
  buildSystem();

  for(int i=0; i<edges.size(); i++)
  {
    string type = edges[i]->getEdgeStringProperty("type");
    if(type == "Pipe" || type == "Pump" || type == "Valve")
    {
      int startIndex = edges[i]->getEdgeIntProperty("startNodeIndex");
      double startHeight = nodes[startIndex]->getProperty("height");
      edges[i]->setDoubleProperty("startHeight",startHeight);

      int endIndex = edges[i]->getEdgeIntProperty("endNodeIndex");
      double endHeight = nodes[endIndex]->getProperty("height");
      edges[i]->setDoubleProperty("endHeight",endHeight);
    }
    if(type == "Pool" || type == "PressurePoint"){
      int index = edges[i]->getEdgeIntProperty("startNodeIndex");
      double height = nodes[index]->getProperty("height");
      edges[i]->setDoubleProperty("height",height);
    }
  }

  openEdges.clear();
  openNodes.clear();
  for(int i=0; i<edges.size(); i++)
    if(!edges[i]->isClosed)
      openEdges.push_back(i);
  for(int i=0; i<nodes.size(); i++)
    if(!nodes[i]->isClosed)
      openNodes.push_back(i);
}

//--------------------------------------------------------------
Staci::~Staci() { 
}

//--------------------------------------------------------------
void Staci::buildSystem(){
  bool startGotIt = false, endGotIt = false;
  int j = 0;
  int startNode = -1, endNode = -1;

  // Clearing the in/out going edges from nodes
  for(int i=0; i<nodes.size(); i++){
    nodes[i]->edgeIn.clear();
    nodes[i]->edgeOut.clear();
  }

  for(int i=0; i<edges.size(); i++)
  {
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
  *id:  vector containing the IDs*/
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
      cout << "\n!!!WARNING!!!\nStaci:ID2Indicies function\nNode is not existing, id: " << id[j] << endl<< "\nContinouing...";
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

//--------------------------------------------------------------
void Staci::changeEdgeStatus(string ID, bool state){

  if(state == false) // Closing an edge
  {
    int i=0, idx=-1;
    bool gotIt=false;
    while(i<edges.size() && !gotIt){
      if(ID.compare(edges[i]->getEdgeStringProperty("name")) == 0){
        edges[i]->isClosed = true;
        gotIt = true;
        idx = i;
      }
      i++;
    }
    if(idx == -1)
      cout << "\n!!!WARNING!!!\nStaci:changeEdgeStatus function\nEdge is not existing, id: " << ID << endl<< "\nContinouing...";
    else
    {
      int nodeFrom = edges[idx]->getEdgeIntProperty("startNodeIndex");
      int nodeTo = edges[idx]->getEdgeIntProperty("endNodeIndex");

      for(int i=0; i<nodes[nodeFrom]->edgeOut.size(); i++)
        if(nodes[nodeFrom]->edgeOut[i] == idx)
          nodes[nodeFrom]->edgeOut.erase(nodes[nodeFrom]->edgeOut.begin() + i);

      if(nodes[nodeFrom]->edgeOut.size() + nodes[nodeFrom]->edgeIn.size() == 0){
        nodes[nodeFrom]->isClosed = true;
        int i=getVectorIndex(openNodes,nodeFrom);
        openNodes.erase(openNodes.begin() + i);
      }

      for(int i=0; i<nodes[nodeTo]->edgeIn.size(); i++)
        if(nodes[nodeTo]->edgeIn[i] == idx)
          nodes[nodeTo]->edgeIn.erase(nodes[nodeTo]->edgeIn.begin() + i);

      if(nodes[nodeTo]->edgeOut.size() + nodes[nodeTo]->edgeIn.size() == 0){
        nodes[nodeTo]->isClosed = true;
        int i=getVectorIndex(openNodes,nodeTo);
        openNodes.erase(openNodes.begin() + i);
      }

      int i=getVectorIndex(openEdges,idx);
      openEdges.erase(openEdges.begin() + i);
    }
  }
  else // Opening an edge
  {
    int i=0, idx=-1;
    bool gotIt=false;
    while(i<edges.size() && !gotIt){
      if(ID.compare(edges[i]->getEdgeStringProperty("name")) == 0){
        edges[i]->isClosed = false;
        gotIt = true;
        idx = i;
      }
      i++;
    }
    if(idx == -1)
      cout << "\n!!!WARNING!!!\nStaci:changeEdgeStatus function\nEdge is not existing, id: " << ID << endl<< "\nContinouing...";
    else
    {
      int nodeFrom = edges[idx]->getEdgeIntProperty("startNodeIndex");
      int nodeTo = edges[idx]->getEdgeIntProperty("endNodeIndex");

      nodes[nodeFrom]->edgeOut.push_back(idx);

      if(nodes[nodeFrom]->edgeOut.size() + nodes[nodeFrom]->edgeIn.size() == 1){
        nodes[nodeFrom]->isClosed = false;
        openNodes.push_back(nodeFrom);
      }

      nodes[nodeTo]->edgeIn.push_back(idx);

      if(nodes[nodeTo]->edgeOut.size() + nodes[nodeTo]->edgeIn.size() == 1){
        nodes[nodeTo]->isClosed = false;
        openNodes.push_back(nodeTo);
      }

      openEdges.push_back(idx);
    }
  }
}

//--------------------------------------------------------------
int Staci::getVectorIndex(const vector<int> &v, int value){
  int idx = -1;
  for(int i=0; i<v.size(); i++)
    if(v[i] == value){
      idx = i;
      break;
    }
  if(idx == -1)
    cout << endl << "!!! WARNING !!! Staci::getVectorIndex did not found value " << value << endl;

  return idx;
}

