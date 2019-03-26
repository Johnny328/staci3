/*===================================================================*\
                                GetNew
                            ---------------                            

  This class provides the opportunity to dynamically modify the
  topology of a water distribution network, e.g. addig/deleting
  Pipes.

  staci3 is using Eigen, see http://eigen.tuxfamily.org

    staci3
    Cs. Hos, R. Weber, T. Huzsvar
    https://github.com/weberrichard/staci3
\*==================================================================*/

//#include "/mnt/d/Linux/Staci_30/staci_3.0/Staci.cpp"
//#include "/mnt/d/Linux/Staci_30/staci_3.0/Staci.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <cmath>
#include <ctime>
#include <stdlib.h>
#include "Pipe.h"
#include "Node.h"

using namespace std;

class GetNew
{
public:
	GetNew(vector<Node *> &nodes, vector<Edge *> &edges);
	~GetNew();
	vector<Node *> newnodes;
	vector<Edge *> newedges;
	void PiPe(const string nodeOfInflow, const string nodeOfOutflow, const double density, const double D, const double erdesseg);
	double readMassFlow();
	void ListPipes();
	bool HaveExtraItems(bool Used);
};