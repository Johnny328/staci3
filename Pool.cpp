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
  strstrm << "\n type                  : " << type;
  strstrm << "\n connection            : " << startNodeName << "(index:" << startNodeIndex << ")";
  strstrm << "\n bottom level          : " << bottomLevel << " [m]";
  strstrm << "\n water level           : " << waterLevel << " [m]" << endl;
  return strstrm.str();
}

//--------------------------------------------------------------
double Pool::function(vector<double> x){ // x = [Pstart, Pend, MassFlowRate]
  return x[0] + height - (bottomLevel + waterLevel);
}

//--------------------------------------------------------------
vector<double> Pool::functionDerivative(vector<double> x){
    vector<double> result;
    result.push_back(1.0);
    result.push_back(0.0);
    result.push_back(0.0);
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
  else if(prop == "height")
    height = value;
  else
  {  
    cout << endl << endl << "Pool::setProperty( DOUBLE ) wrong argument:" << prop;
    cout << ", right values: head | massFlowRate | velocity | density | referenceCrossSection | user1 | user2" << endl << endl;
  }
}
