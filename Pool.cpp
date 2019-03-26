#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <math.h>
#include "Edge.h"
#include "Pool.h"

Pool::Pool(const string a_name, const string a_startNodeName, const double a_density, const double a_referenceCrossSection, const double a_bottomLevel, const double a_waterLevel, const double a_massFlowRate) :
    Edge(a_name, a_referenceCrossSection, a_massFlowRate, a_density) {
    type = "Pool";
    numberNode = 1;
    startNodeName = a_startNodeName;
    endNodeName = "endNothingYet";
    bottomLevel = a_bottomLevel;
    waterLevel = a_waterLevel;
}

//--------------------------------------------------------------
Pool::~Pool() {
}

//--------------------------------------------------------------
string Pool::info() {
    ostringstream strstrm;
    strstrm << Edge::info();
    strstrm << "\n               type : " << type;
    strstrm << "\n         connection : " << startNodeName << "(index:" << startNodeIndex << ")\n";
    strstrm << "     bottom level [m] : " << bottomLevel << endl;
    strstrm << "      water level [m] : " << waterLevel << endl;
    return strstrm.str();
}

//--------------------------------------------------------------
double Pool::function(vector<double> x) { //TODO L,D,A = ????
    double L = 1.;
    double D = 0.5;
    double A = D * D * M_PI / 4.;
    double c = 0.02 * L / D / 2. / gravity / density / density / A / A;
    double result = x[0] + x[2] + 0. * c * massFlowRate * abs(massFlowRate) - (bottomLevel + waterLevel);

    return result;
}

//--------------------------------------------------------------
vector<double> Pool::functionDerivative(vector<double> x) {
    vector<double> result;
    result.push_back(1.0);
    result.push_back(0.0);
    /*double L = 1;
    double D = 0.5;
    double A = D * D * M_PI / 4;
    double c = 0.02 * L / D / 2. / gravity / density / density / A / A;
    result.push_back(0 * 2 * c * fabs(massFlowRate));*/
    //result.push_back(-head / density / gravity);
    result.push_back(0.0);
    result.push_back(-(bottomLevel + waterLevel));

    return result;
}

//--------------------------------------------------------------
void Pool::initialization(int mode, double value) {
  if (mode == 0)
    massFlowRate = 1;
  else
    massFlowRate = value;
}

//--------------------------------------------------------------
double Pool::getDoubleProperty(string prop){
  double out = 0.;
  if(prop == "waterLevel")
    out = waterLevel;
  else if(prop == "bottomLevel")
    out = bottomLevel;
  else if(prop == "head")
    out = (bottomLevel + waterLevel);
  else if(prop == "pressure")
    out = (bottomLevel + waterLevel) * density * gravity;
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
    cout << endl << endl << "DOUBLE Pool::getDoubleProperty() wrong argument:" << prop;
    cout << ", right values: waterLevel | bottomLevel | head | pressure | massFlowRate | velocity | density | referenceCrossSection | user1 | user2" << endl << endl;
  }
  return out;
}

//--------------------------------------------------------------
void Pool::setDoubleProperty(string prop, double value){
  if(prop == "waterLevel")
    waterLevel = value;
  else if(prop == "bottomLevel")
    bottomLevel = value;
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
    cout << endl << endl << "Pool::setProperty( DOUBLE ) wrong argument:" << prop;
    cout << ", right values: head | massFlowRate | velocity | density | referenceCrossSection | user1 | user2" << endl << endl;
  }
}
