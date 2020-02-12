/*===================================================================*\
                                  IOxml
                            ---------------

  Loading/saving staci3 files from/to xml file format.
  xml files can be loaded with the graphical interface of staci3.
 
  staci3 is using Eigen, see http://eigen.tuxfamily.org

    staci3
    Cs. Hos, R. Weber, T. Huzsvar
    https://github.com/weberrichard/staci3
\*==================================================================*/

#ifndef IOXML_H
#define IOXML_H

#include <string>
#include <fstream>
#include <vector>
#include <math.h>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <iomanip>

#include "xmlParser.h"
#include "Node.h"
#include "Edge.h"
#include "PressurePoint.h"
#include "Pool.h"
#include "Pipe.h"
#include "Valve.h"
#include "ValveISO.h"

class IOxml
{

public:
  IOxml(const char *xmlFileName);
  void loadSystem(vector<Node *> &nodes, vector<Edge *> &edges);
  void loadInitialValue(vector<Node *> &nodes, vector<Edge *> &edges);
  string readSetting(string which);

private:
  XMLNode xmlMain;
  XMLNode nodeSetting;
  XMLNode nodeNodes;
  XMLNode nodesEdges;
  const char *xmlFileName;
  bool debug;
  int nodeNumber, edgeNumber;
  void curveReader(const string id, const XMLNode elem, vector<double> &px, vector<double> &py);
  void writingTagValue(XMLNode node, string tag_name, double tag_value);
};

#endif
