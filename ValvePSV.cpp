#include "ValvePSV.h"

ValvePSV::ValvePSV(const string a_name, const string a_startNodeName, const string a_endNodeName, const double a_density, const double a_referenceCrossSection, const double a_setting, const double a_minorLossCoeff, const double a_volumeFlowRate) : Valve(a_name, a_startNodeName, a_endNodeName, a_density,a_referenceCrossSection, a_volumeFlowRate) {
  type = "ValvePSV";
  setting = a_setting;
  minorLossCoeff = a_minorLossCoeff;

  status = 1; // 2: open, 1: active, 0: closed
  typeCode = 4;
}

//--------------------------------------------------------------
ValvePSV::~ValvePSV(){}

//--------------------------------------------------------------
string ValvePSV::info()
{
  ostringstream ss;
  ss << Edge::info();
  ss << "\n connection            : " << startNodeName << " (index:" << startNodeIndex << ") --> " << endNodeName << " (index:" << endNodeIndex << ")";
  return ss.str();
}

//--------------------------------------------------------------
double ValvePSV::function(const VectorXd &ppq, VectorXd &fDer)
{
  double out;
  if(status == 2) // ACTIVE
  {
    //out = ppq(0) - setting;
    //fDer(0) = 1.0;

    double k = 10.;
    out = ppq(1) - ppq(0) + (endHeight-startHeight) + k * ppq(2) * abs(ppq(2)) + 1e8*(ppq(0) - setting);
    fDer(0) = 1e8;
    fDer(1) = 1.0;
    fDer(2) = 2 * k * abs(ppq(2));
  }
  else if(status == 1) // OPEN
  {
    out = ppq(1) - ppq(0) + (endHeight-startHeight);// + minorLossCoeff*v*v/(2*gravity);
    fDer(0) = -1.0;
    fDer(1) =  1.0;
  }
  else // CLOSED, status is 0 or -1
  {
    //out = ppq(2);
    //fDer(2) = 1.0;

    double k = 10.;
    out = ppq(1) - ppq(0) + (endHeight-startHeight) + k * ppq(2) * abs(ppq(2)) + 1e8 * ppq(2);
    fDer(0) = -1.0;
    fDer(1) =  1.0;
    fDer(2) = 2 * k * abs(ppq(2)) + 1e8;
  }
  return out;
}

//--------------------------------------------------------------
void ValvePSV::initialization(int mode, double value) {
  if (mode == 0)
    volumeFlowRate = 1.;
  else
    volumeFlowRate = value;
}

//--------------------------------------------------------------
void ValvePSV::setDoubleProperty(string property, double value) {
  if(property == "startHeight")
      startHeight = value;
  else if(property == "endHeight")
      endHeight = value;
  else {
    cout << endl << endl << "ERROR! ValvePSV::setDoubleProperty(property), unkown property: property=" << property << endl << endl;
  }
}

//--------------------------------------------------------------
double ValvePSV::getDoubleProperty(string property) {
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
    cout << endl << "ERROR! ValvePSV::getDoubleProperty(property), unkown property: property=" << property << endl << endl;

  return out;
}

//--------------------------------------------------------------
string ValvePSV::getStringProperty(string property) {
  string out = "";
  if (property == "type")
    out = type;
  else
    cout << endl << "ERROR! ValvePSV::getStringProperty(property), unkown property: property=" << property << endl << endl;
  return out;
}
