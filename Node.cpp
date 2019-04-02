#include "Node.h"

Node::Node(const string a_name, const double a_xPosition, const double a_yPosition, const double a_geodeticHeight, const double a_demand, const double a_head, const double a_density)
{
  density = a_density;
  demand = a_demand / 3600. * density;
  geodeticHeight = a_geodeticHeight;
  xPosition = a_xPosition;
  yPosition = a_yPosition;
  name = a_name;
  head = a_head;
  user1 = 0.;
  user2 = 0.;
}

//--------------------------------------------------------------
Node::~Node()
{
}

//--------------------------------------------------------------
void Node::initialization(int mode, double value)
{
  if (mode == 0)
    head = 300. - geodeticHeight;
  else
    head = value - geodeticHeight;
}

//--------------------------------------------------------------
void Node::setProperty(string prop, double value)
{
  if(prop == "demand")
    demand = value;
  else if(prop == "head")
    head = value;
  else if(prop == "density")
    density = value;
  else if(prop == "height" || prop == "geodeticHeight")
    geodeticHeight = value;
  else if(prop == "xPosition")
    xPosition = value;
  else if(prop == "yPosition")
    yPosition = value;
  else if(prop == "user1")
    user1 = value;
  else if(prop == "user2")
    user2 = value;
  else
  {
    cout << endl << endl << "Node::getProperty() wrong argument:" << prop;
    cout << ", right values: demand|head|pressure|density|height|xPosition|yPosition|user1|user2" << endl << endl;
  }
}

//--------------------------------------------------------------
double Node::getProperty(string prop)
{
  double out = 0.0;

  if(prop == "demand")
    out = demand;
  else if(prop == "pressure")
    out = head / density / 9.81;
  else if(prop == "head")
    out = head;
  else if(prop == "density")
    out = density;
  else if(prop == "height" || prop == "geodeticHeight")
    out = geodeticHeight;
  else if(prop == "xPosition")
    out = xPosition;
  else if(prop == "yPosition")
    out = yPosition;
  else if(prop == "user1")
    out = user1;
  else if(prop == "user2")
    out = user2;
  else
  {
    cout << endl << endl << "Node::getProperty() wrong argument:" << prop;
    cout << ", right values: demand|head|pressure|density|height|xPosition|yPosition|user1|user2" << endl << endl;
  }
  return out;
}

//--------------------------------------------------------------
string Node::info(bool check_if_lonely)
{
  ostringstream strstrm;
  strstrm << "\n       Node name: " << name;
  strstrm << "\n          height: " << geodeticHeight << " m";
  strstrm << "\n            head: " << head << " m (=p[Pa]/density/g)";
  strstrm << "\n        pressure: " << head*density * 9.81 << " Pa";
  strstrm << "\n         desnity: " << density << " kg/m3";
  strstrm << "\n     consumption: " << demand << " kg/s = " << demand * 3600 / density << " m3/h";
  strstrm << "\n  incoming edges: ";
  for (vector<int>::iterator it = edgeIn.begin(); it != edgeIn.end(); it++)
      strstrm << *it << " ";
  strstrm << "\n  outgoing edges: ";
  for (vector<int>::iterator it = edgeOut.begin(); it != edgeOut.end(); it++)
      strstrm << *it << " ";
  strstrm << endl;

  if (check_if_lonely && ((edgeIn.size() + edgeOut.size()) == 0))
  {
    strstrm << "\n!!! ERROR !!! Lonely node: " << name << " !!!\n";
    cout << strstrm.str();
    exit(-1);
  }

  return strstrm.str();
}
