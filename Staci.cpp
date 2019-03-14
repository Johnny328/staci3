#include "Staci.h"

using namespace Eigen;

Staci::Staci(string spr_filename) {
  definitionFile = spr_filename;
  setOutputFile(definitionFile + ".log");

  stringstream consolePrint;
  consolePrint.str("");
  consolePrint << endl << "======================================";
  consolePrint << endl << " STACI v.3.0";
  consolePrint << endl << " (c) BME Dept. of Hydrodynamic Systems";
  consolePrint << endl << " (c) Cs. Hos (cshos@hds.bme.hu), R. Weber (rweber@hds.bme.hu), T. Huzsvar (thuzsvar@hds.bme.hu)";
  consolePrint << endl << " info: www.hds.bme.hu\\staci_web";
  consolePrint << endl << "======================================" << endl;
  time_t ido = time(0);
  consolePrint << endl << " date: " << ctime(&ido);
  consolePrint << endl << " input file: " << definitionFile << endl;
  writeLogFile(consolePrint.str(), 1);
  consolePrint.str("");

  IOxml IOxmlObj(definitionFile.c_str());
  IOxmlObj.load_system(nodes, edges);

  debugLevel = atoi(IOxmlObj.read_setting("debug_level").c_str());
  frictionModel = IOxmlObj.read_setting("friction_model");
}

//--------------------------------------------------------------
Staci::~Staci() { 
}

//--------------------------------------------------------------
void Staci::buildSystem() {
  ostringstream msg1;
  writeLogFile("\n\n Building system  ", 3);
  bool startGotIt = false, endGotIt = false;
  int j = 0;
  int startNode = -1, endNode = -1;
  ostringstream consolePrint;

  for(int i=0; i<edges.size(); i++){
    startGotIt = false;
    j = 0;
    while((j < nodes.size()) && (!startGotIt)){
      if((edges[i]->getStartNodeName()).compare(nodes[j]->getName()) == 0){
        startGotIt = true;
        startNode = j;
        nodes[j]->edgeOut.push_back(i);
      }
      j++;
    }
    if(!startGotIt){
      consolePrint.str("");
      consolePrint << "\n!!! Edge name: " << edges[i]->getName().c_str() << ", startnode not found !!!";
      writeLogFile(consolePrint.str(), 0);
      exit(-1);
    } 

    if(edges[i]->getNumberNode() == 2){
      endGotIt = false;
      j = 0;
      while ((j < nodes.size()) && (!endGotIt)) {
        if ((edges[i]->getEndNodeName()).compare(nodes[j]->getName()) == 0) {
          endGotIt = true;
          endNode = j;
          nodes[j]->edgeIn.push_back(i);
        }
        j++;
      }
      if (!endGotIt) {
        consolePrint.str("");
        consolePrint << "\n!!! Edge name: " << edges[i]->getName().c_str() << ", endnode not found !!!";
        writeLogFile(consolePrint.str(), 0);
        exit(-1);
      }
    }

    if(edges[i]->getNumberNode() == 2) 
      edges[i]->addNodes(startNode, endNode);
    else
      edges[i]->addNodes(startNode, -1);
  }

  for(int i = 0; i < edges.size(); i++){
    if(edges[i]->getType() == "Pipe")
      edges[i]->setFrictionModel(frictionModel);
  }

  writeLogFile("\t ok.", 3);
  msg1.str("");
  msg1 << " ready." << endl;
  writeLogFile(msg1.str(), 3);
}

//--------------------------------------------------------------
void Staci::writeLogFile(string msg, int msg_debugLevel) {
  if (debugLevel >= msg_debugLevel) {
    ofstream outfile(outputFile.c_str(), ios::app);
    outfile << msg;
    outfile.close();
    cout << msg << flush;
  }
}

//--------------------------------------------------------------
void Staci::listSystem() {
  writeLogFile("\n\n Nodes:\n--------------------------", 0);
  for (int i = 0; i < nodes.size(); i++)
    writeLogFile(nodes[i]->info(true), 0);
  writeLogFile("\n\n Edges:\n--------------------------", 0);
  for (int i = 0; i < edges.size(); i++)
    writeLogFile(edges[i]->info(), 0);
}


/*! WR: find the the given node IDs and gives back the Indicies
  *id:  vector containing the IDs
  IMPORTANT:  if only one ID not found, drops ERROR with exit(-1)
*/
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
    if(gotIt == false){
      cout << "\n!!!!! ERROR !!!!!\nStaci:ID2Indicies function\nNode is not existing, id: " << id[j] << endl<< "\nexiting...";
      exit(-1);
    }
  }
  return idx;
}

//--------------------------------------------------------------
void Staci::checkSystem()
{
  ostringstream msg1;
  bool stop = false;

  msg1 << endl << " [*] Checking System  ";
  writeLogFile(msg1.str(), 1);
  msg1.str("");

  writeLogFile("\n [*] Looking for identical IDs  ", 3);
  string name1, name2;
  for(int i = 0; i < edges.size(); i++){
    name1 = edges.at(i)->getName();
    for(int j = 0; j < edges.size(); j++){
      name2 = edges.at(j)->getName();
      if(i != j){
        if(name1 == name2){
          ostringstream msg;
          msg << "\n !!!ERROR!!! edge #" << i << ": " << name1 << " and edge #" << j << ": " << name2 << " with same ID!" << endl;
          writeLogFile(msg.str(), 1);
          stop = true;
        }
      }
    }

    for(int j = 0; j < nodes.size(); j++){
      name2 = nodes.at(j)->getName();
      if(i != j){
        if(name1 == name2){
          ostringstream msg;
          msg << "\n !!!ERROR!!! edge #" << i << ": " << name1 << " and node #" << j << ": " << name2 << " with same ID!" << endl;
          writeLogFile(msg.str(), 1);
          stop = true;
        }
      }
    }
  }

  if (stop)
    exit(-1);
  else
    writeLogFile("\t ok.", 3);
}