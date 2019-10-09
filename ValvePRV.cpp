#include "ValvePRV.h"

ValvePRV::ValvePRV(const string a_name, const string a_startNodeName, const string a_endNodeName, const double a_density, const double a_referenceCrossSection, const double a_setting, const double a_minorLossCoeff, const double a_volumeFlowRate) : Valve(a_name, a_startNodeName, a_endNodeName, a_density,a_referenceCrossSection, a_volumeFlowRate) {
  type = "ValvePRV";
  setting = a_setting;
  minorLossCoeff = a_minorLossCoeff;

  status = 1; // 2: open, 1: active, 0: closed
}

//--------------------------------------------------------------
ValvePRV::~ValvePRV() {}

//--------------------------------------------------------------
string ValvePRV::info() {
  ostringstream ss;
  ss << Edge::info();
  ss << "\n connection            : " << startNodeName << " (index:" << startNodeIndex << ") --> " << endNodeName << " (index:" << endNodeIndex << ")";
  return ss.str();
}

//--------------------------------------------------------------
double ValvePRV::function(vector<double> x){
  double out;
  if(status == 2) // ACTIVE
  {
    out = x[1] - setting;
  }
  else if(status == 1) // OPEN
  {
    //double v = x[2]/1000./referenceCrossSection; // velocity in m/s
    out = x[1] - x[0] + (endHeight-startHeight);// + minorLossCoeff*v*v/(2*gravity);
  }
  else // CLOSED
  {
    out = x[2];
  }

  return out;
}

//--------------------------------------------------------------
vector<double> ValvePRV::functionDerivative(vector<double> x) {
  vector<double> out;
  if(status == 2) // ACTIVE
  {
    out.push_back(0.0);
    out.push_back(1.0);
    out.push_back(0.0);
  }
  else if(status == 1) // OPEN
  {
    out.push_back(-1.0);
    out.push_back(1.0);
    out.push_back(0.0); 
  }
  else // CLOSED
  {
    out.push_back(0.0);
    out.push_back(0.0);
    out.push_back(1.0);
    //out.push_back(minorLossCoeff*x[2]/1000/referenceCrossSection/referenceCrossSection/gravity);
  }

  return out;
}

//--------------------------------------------------------------
void ValvePRV::initialization(int mode, double value) {
  if (mode == 0)
    volumeFlowRate = 1.;
  else
    volumeFlowRate = value;
}

//--------------------------------------------------------------
void ValvePRV::setDoubleProperty(string property, double value) {
  if(property == "startHeight")
      startHeight = value;
  else if(property == "endHeight")
      endHeight = value;
  else {
    cout << endl << endl << "ERROR! ValvePRV::setDoubleProperty(property), unkown property: property=" << property << endl << endl;
  }
}

//--------------------------------------------------------------
double ValvePRV::getDoubleProperty(string property) {
  double out = 0.0;
  if (property == "volumeFlowRate")
    out = volumeFlowRate;
  else if ((property == "length") || (property == "L"))
    out = 0.5;
  else if (property == "cross_section")
    out = referenceCrossSection;
  else if (property == "startHeight")
    out = startHeight;
  else if (property == "endHeight")
    out = endHeight;
  else
    cout << endl << "ERROR! ValvePRV::getDoubleProperty(property), unkown property: property=" << property << endl << endl;

  return out;
}

//--------------------------------------------------------------
string ValvePRV::getStringProperty(string property) {
  string out = "";
  if (property == "type")
    out = type;
  else
    cout << endl << "ERROR! ValvePRV::getStringProperty(property), unkown property: property=" << property << endl << endl;
  return out;
}
