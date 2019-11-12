/*===================================================================*\
                                 Staci
                            ---------------

  Main Staci class. Contains basic variables (e.g. vector for Node
  and Edge) and functions (e.g. building the system topology for
  solving the equations).
 
  staci3 is using Eigen, see http://eigen.tuxfamily.org

    staci3
    Cs. Hos, R. Weber, T. Huzsvar
    https://github.com/weberrichard/staci3
\*==================================================================*/

#ifndef STACI_H
#define STACI_H

#include "BasicFileIO.h"
#include "Edge.h"
#include "Graph.h"
#include "Node.h"
#include "Pipe.h"
#include "PressurePoint.h"
#include "Pump.h"
#include "Pool.h"
#include "Statistic.h"
#include "Valve.h"
#include "ValveISO.h"
#include "ValveFCV.h"
#include "ValvePRV.h"
#include "ValvePSV.h"
#include "ValveTCV.h"

#include "Eigen/Eigen/Eigen"

#include <string>
#include <iomanip>
#include <vector>
#include <cmath>
#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <sys/stat.h> //mkdir

using namespace Eigen;

class Staci
{
public:
  Staci(string spr_filename);
  ~Staci();

  // Basic Node and Edge list
  vector<Node *> nodes;
  vector<Edge *> edges;

  // indicies vector for edge types, avoiding the for cycles
  // e.g. poolIndex contains the indicies of the pools in the edges list
  /* typeCode values for edges
  -2: pool
  -1: pressure point
  0:  pipe with check valve
  1:  normal pipe
  2:  pump
  3:  pressure reduction valve (PRV)
  4:  pressure sustaining valve (PSV)
  5:  pressure breaker valve (PBV)
  6:  flow control valve (FCV)
  7:  throttle control valve (TCV)
  8:  general purpose valve (GPV)
  9:  isolation valve (ISO)
  */
  vector<int> poolIndex, presIndex, pumpIndex, valveIndex, valveISOIndex, pipeIndex, pipeCVIndex;

  // Prints everything 
  void listSystem();

  // Converts node names (IDs) to indicies i.e. finds the node name in the list of the nodes
  vector<int> ID2Index(const vector<string> &id);

  // Checking the IDs of the edges, if one has identical ones drops exit(-1)
  void checkSystem();

  // Closing/opening edges (close: state = false | open: state = true)
  //void changeEdgeStatus(string ID, bool state); // with ID (calls the idx version)
  //void changeEdgeStatus(int idx, bool state); // with index

  /// Containing indicies of open edges and nodes for HydSolver
  //vector<int> openEdges, openNodes;

  /// Saving results to file
  void saveResult(string property, string element);

  /// Loading the system from INP | IOinp.cpp
  void loadSystem();
  /// Saving the system to INP | IOinp.cpp
  void saveSystem(vector<Node *> &nodes, vector<Edge *> &edges, string frictionModel);

  /// ************************************************ ///
  /// GETSET GETSET GETSET GETSET GETSET GETSET GETSET ///
  /// ************************************************ ///
  string getDefinitionFile()
  {
      return definitionFile;
  }
  bool getIsInitialization()
  {
      return isInitialization;
  }
  void setIsInitialization(const bool var)
  {
      isInitialization = var;
  }
  string getFrictionModel(){
      return frictionModel;
  }

protected:
  /// Finds the index of value in the v vector
  //int getVectorIndex(const vector<int> &v, int value);
  vector<string> line2sv(string line); // cutting string line to pieces
  int nodeIDtoIndex(string ID);
  int edgeIDtoIndex(string ID);

  // Checking lonely nodes and newly connected ones
  // void checkNodeStatus(int index);

  // UNITS
  double demandUnit; // LPS, GPM etc. to LPS
  string unit; // SI or US

  // number of edges and nodes
  int numberEdges, numberNodes;

private:
  bool isInitialization = false;
  string definitionFile, caseName;
  string frictionModel;
  // Creates the indicies for the nodes of edges and edges of nodes i.e. indexing of the sparse Jacobian
  void buildSystem();
  // Create the indexing, making the code more efficient
  void buildIndexing();
};

#endif //STACI_H