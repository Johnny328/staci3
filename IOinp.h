#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <math.h>

#include "Node.h"
#include "Edge.h"
#include "xmlParser.h"
#include "PressurePoint.h"
#include "Pipe.h"
#include "Pump.h"
#include "Valve.h"
#include "Pool.h"

class IOinp
{
public:
	IOinp();
	~IOinp();
  void loadSystem(vector<Node *> &nodes, vector<Edge *> &edges);

private:
	string inpFileName;
	vector<string> line2sv(string line);
};