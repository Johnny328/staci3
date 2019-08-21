#include <cmath>
#include <vector>
#include <iostream>
#include <sstream>
#include "Edge.h"
#include "PressurePoint.h"

PressurePoint::PressurePoint(const string a_name, const double a_referenceCrossSection, const string a_startNodeName, const double a_density, const double a_head, const double a_volumeFlowRate) :
    Edge(a_name, a_referenceCrossSection, a_volumeFlowRate, a_density){
    type = "PressurePoint";
    numberNode = 1;

    endNodeName = "<nincs>";
    startNodeName = a_startNodeName;
    head = a_head / density / gravity;
}

//--------------------------------------------------------------
PressurePoint::~PressurePoint(){
}

//--------------------------------------------------------------
string PressurePoint::info(){
    ostringstream strstrm;
    strstrm << Edge::info();
    strstrm << "\n connection            : " << startNodeName << "(index:" << startNodeIndex << ")";
    strstrm << "\n pressure              : " << head*density*gravity << " [Pa]";
    strstrm << "\n head                  : " << head << " [m]" << endl;
    return strstrm.str();
}

//--------------------------------------------------------------
double PressurePoint::function(vector<double> x){
    return x[0] - head;
}

//--------------------------------------------------------------
vector<double> PressurePoint::functionDerivative(vector<double> x){
    vector<double> result;
    result.push_back(1.);
    result.push_back(0.);
    result.push_back(0.);
    return result;
}

//--------------------------------------------------------------
void PressurePoint::initialization(int mode, double value){
  if (mode == 0)
    volumeFlowRate = 0.01;
  else
    volumeFlowRate = value;
}


//--------------------------------------------------------------
double PressurePoint::getDoubleProperty(string prop){
  double out = 0.;
  if(prop == "head")
    out = head;
  else if(prop == "pressure")
    out = head * gravity * density;
  else if(prop == "massFlowRate")
    out = volumeFlowRate * density/1000.;
  else if(prop == "volumeFlowRate")
    out = volumeFlowRate;
  else if(prop == "velocity")
    out = volumeFlowRate / 1000. / referenceCrossSection;
  else if(prop == "density")
    out = density;
  else if(prop == "referenceCrossSection" || prop == "reference_cross_section" || prop == "Aref")
    out = referenceCrossSection;
  else if(prop == "height")
    out = height;
  else
  {
    cout << endl << endl << "DOUBLE PressurePoint::getDoubleProperty() wrong argument:" << prop;
    cout << ", right values: head | pressure | volumeFlowRate | velocity | density | referenceCrossSection | user1 | user2" << endl << endl;
  }
  return out;
}

//--------------------------------------------------------------
void PressurePoint::setDoubleProperty(string prop, double value){
  if(prop == "head")
    head = value;
  else if(prop == "pressure")
    head = value / density / gravity;
  else if(prop == "volumeFlowRate")
    volumeFlowRate = value;
  else if(prop == "density")
    density = value;
  else if(prop == "referenceCrossSection" || prop == "reference_cross_section")
    referenceCrossSection = value;
  else if(prop == "height")
    height = value;
  else
  {  
    cout << endl << endl << "PressurePoint::setProperty( DOUBLE ) wrong argument:" << prop;
    cout << ", right values: head | volumeFlowRate | velocity | density | referenceCrossSection | user1 | user2" << endl << endl;
  }
}
