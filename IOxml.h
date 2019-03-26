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

#include <string>
#include <fstream>
#include <vector>
#include <math.h>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <iomanip>

#include "Node.h"
#include "Edge.h"
#include "xmlParser.h"
#include "PressurePoint.h"
#include "Pipe.h"
//#include "Pump.h"
#include "Valve.h"
#include "Pool.h"
//#include "Channel.h"
//#include "Weir.h"
//#include "CheckValve.h"

class IOxml
{

public:
  IOxml(const char *xmlFileName);
  void loadSystem(vector<Node *> &nodes, vector<Edge *> &edges);
  void loadInitialValue(vector<Node *> &nodes, vector<Edge *> &edges);
  void saveResult(double fluidVolume, double sum_of_inflow, double sum_of_demand, vector<Node *> nodes, vector<Edge *> edges, bool conv_reached, int staci_debug_level);
  string readSetting(string which);

private:
  XMLNode xmlMain; // main XMLNode
  XMLNode nodeSetting;
  XMLNode nodeNodes;
  XMLNode nodesEdges;
  const char *xmlFileName;
  bool debug;
  int nodeNumber, edgeNumber;
  void curveReader(const string id, const XMLNode elem, vector<double> &px, vector<double> &py);
  void writingTagValue(XMLNode node, string tag_name, double tag_value);
};
