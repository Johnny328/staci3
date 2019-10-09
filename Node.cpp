#include "Node.h"

Node::Node(const string a_name, const double a_xPosition, const double a_yPosition, const double a_geodeticHeight, const double a_demand, const double a_head, const double a_density)
{
  density = a_density;
  demand = a_demand;
  geodeticHeight = a_geodeticHeight;
  xPosition = a_xPosition;
  yPosition = a_yPosition;
  name = a_name;
  head = a_head;
}

//--------------------------------------------------------------
Node::~Node(){}

//--------------------------------------------------------------
void Node::initialization(int mode, double value)
{
  if (mode == 0)
    head = 50.;
  else
    head = value - geodeticHeight;
}

//--------------------------------------------------------------
double Node::function(double head){ // for head dependent demnads
  double out;
  if(head<pdMinPressure)
    out = 0.0;
  else if(head<pdDesiredPressure && head>pdMinPressure)
    out = -demand*pow((head-pdMinPressure)/(pdDesiredPressure-pdMinPressure),1./pdExponent);
  else
    out = -demand;

  return out;
}

//--------------------------------------------------------------
double Node::functionDerivative(double head){ // for head dependent demnads
  double out;
  if(head<pdMinPressure)
    out = 0.0;
  else if(head<pdDesiredPressure && head>pdMinPressure)
    out = -demand*pow((head-pdMinPressure)/(pdDesiredPressure-pdMinPressure),1./pdExponent-1)/(pdDesiredPressure-pdMinPressure);
  else
    out = 0.0;

  return out;
}

//--------------------------------------------------------------
void Node::setProperty(string prop, double value)
{
  if(prop == "demand")
    demand = value;
  else if(prop == "consumption"){
    consumption = value;
    if(demand == 0.0)
      consumptionPercent = 0.0;
    else
      consumptionPercent = 100.*consumption/demand;
  }
  else if(prop == "consumptionPercent"){
    consumptionPercent = value;
    consumption = consumptionPercent/100.*demand;
  }
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
  else
  {
    cout << endl << endl << "Node::getProperty() wrong argument:" << prop;
    cout << ", right values: demand|head|pressure|density|height|xPosition|yPosition" << endl << endl;
  }
}

//--------------------------------------------------------------
double Node::getProperty(string prop)
{
  double out = 0.0;

  if(prop == "demand")
    out = demand;
  else if(prop == "consumption")
    out = consumption;
  else if(prop == "consumptionPercent")
    out = consumptionPercent;
  else if(prop == "pressure")
    out = head * density * 9.81;
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
  else if(prop == "segment")
    out = (double)segment;
  else
  {
    cout << endl << endl << "Node::getProperty() wrong argument:" << prop;
    cout << ", right values: demand|head|pressure|density|height|xPosition|yPosition|userOutput" << endl << endl;
  }
  return out;
}

//--------------------------------------------------------------
string Node::info(bool check_if_lonely)
{
  ostringstream strstrm;
  strstrm << "\n Node name       : " << name;
  strstrm << "\n open/closed     : ";
  strstrm << "\n height          : " << geodeticHeight << " m";
  strstrm << "\n head            : " << head << " m";
  strstrm << "\n pressure        : " << head*density * 9.81 << " Pa";
  strstrm << "\n desnity         : " << density << " kg/m3";
  strstrm << "\n demand          : " << demand << " l/s = " << demand * 3.6 << " m3/h";
  strstrm << "\n consumption     : " << consumption << " l/s = " << consumption * 3.6 << " m3/h";
  strstrm << "\n segment         : " << segment;
  strstrm << "\n incoming edges  : ";
  for(vector<int>::iterator it = edgeIn.begin(); it != edgeIn.end(); it++)
      strstrm << *it << " ";
  strstrm << "\n outgoing edges  : ";
  for(vector<int>::iterator it = edgeOut.begin(); it != edgeOut.end(); it++)
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
