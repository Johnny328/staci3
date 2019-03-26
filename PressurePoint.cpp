#include <cmath>
#include <vector>
#include <iostream>
#include <sstream>
#include "Edge.h"
#include "PressurePoint.h"

PressurePoint::PressurePoint(const string a_name, const double a_referenceCrossSection, const string a_startNodeName, const double a_density, const double a_head, const double a_massFlowRate) :

    Edge(a_name, a_referenceCrossSection, a_massFlowRate, a_density) {
    type = "PressurePoint";
    numberNode = 1;

    endNodeName = "<nincs>";
    startNodeName = a_startNodeName;
    head = a_head;
}

//--------------------------------------------------------------
PressurePoint::~PressurePoint() {
}

//--------------------------------------------------------------
string PressurePoint::info() {
    ostringstream strstrm;
    strstrm << Edge::info();
    strstrm << endl << "  kapcsolodas : " << startNodeName << "(index:" << startNodeIndex << ")\n";
    return strstrm.str();
}

//--------------------------------------------------------------
double PressurePoint::function(vector<double> x) {
    double result = x[0] - head / density / gravity;
    return result;
}

//--------------------------------------------------------------
vector<double> PressurePoint::functionDerivative(vector<double> x) {
    vector<double> result;
    result.push_back(1);
    result.push_back(0);
    result.push_back(0);
    result.push_back(-head / density / gravity);

    return result;
}

//--------------------------------------------------------------
void PressurePoint::initialization(int mode, double value) {
    if (mode == 0)
        massFlowRate = 0.01;
    else
        massFlowRate = value;
}


//--------------------------------------------------------------
double PressurePoint::getDoubleProperty(string prop){
  double out = 0.;
  if(prop == "head")
    out = head;
  else if(prop == "pressure")
    out = head * gravity * density;
  else if(prop == "massFlowRate" || prop == "mass_flow_rate")
    out = massFlowRate;
  else if(prop == "volumeFlowRate" || prop == "volume_flow_rate")
    out = massFlowRate / density;
  else if(prop == "velicoty")
    out = massFlowRate / density / referenceCrossSection;
  else if(prop == "density")
    out = density;
  else if(prop == "referenceCrossSection" || prop == "reference_cross_section" || prop == "Aref")
    out = referenceCrossSection;
  else if(prop == "user1")
    out = user1;
  else if(prop == "user2")
    out = user2;
  else
  {
    cout << endl << endl << "DOUBLE PressurePoint::getDoubleProperty() wrong argument:" << prop;
    cout << ", right values: head | pressure | massFlowRate | velocity | density | referenceCrossSection | user1 | user2" << endl << endl;
  }
  return out;
}

//--------------------------------------------------------------
void PressurePoint::setDoubleProperty(string prop, double value){
  if(prop == "head")
    head = value;
  else if(prop == "pressure")
    head = value / density / gravity;
  else if(prop == "massFlowRate" || prop == "mass_flow_rate")
    massFlowRate = value;
  else if(prop == "density")
    density = value;
  else if(prop == "referenceCrossSection" || prop == "reference_cross_section")
    referenceCrossSection = value;
  else if(prop == "user1")
    user1 = value;
  else if(prop == "user2")
    user2 = value;
  else
  {  
    cout << endl << endl << "PressurePoint::setProperty( DOUBLE ) wrong argument:" << prop;
    cout << ", right values: head | massFlowRate | velocity | density | referenceCrossSection | user1 | user2" << endl << endl;
  }
}
