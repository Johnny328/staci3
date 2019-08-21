#include "Valve.h"

Valve::Valve(const string a_name, const string a_startNodeName, const string a_endNodeName, const double a_density, const double a_referenceCrossSection, vector<double> a_charX, vector<double> a_charY, double a_position, const double a_volumeFlowRate) :
        Edge(a_name, a_referenceCrossSection, a_volumeFlowRate, a_density) {
    type = "Valve";
    numberNode = 2;
    startNodeName = a_startNodeName;
    endNodeName = a_endNodeName;
    charX = a_charX;
    charY = a_charY;
    position = a_position;
    updateLoss();

}

//--------------------------------------------------------------
Valve::~Valve() {
}

//--------------------------------------------------------------
string Valve::info() {
  ostringstream ss;
  ss << Edge::info();
  ss << "\n segment from          : " << startSegment;
  ss << "\n segment to            : " << endSegment;
  ss << "\n connection            : " << startNodeName << " (index:" << startNodeIndex << ") --> " << endNodeName << " (index:" << endNodeIndex << ")";
  vector<double>::iterator it;
  ss << "\n char curve X [%]      : ";
  for (it = charX.begin(); it != charX.end(); it++)
    ss << *(it) << "  ";
  ss << "\n char curve Y [-]      : ";
  for (it = charY.begin(); it != charY.end(); it++)
    ss << *(it) << "  ";
  ss << "\n position              : " << position;
  ss << "\n loss coefficient      : " << loss << endl;

  return ss.str();
}

//--------------------------------------------------------------
double Valve::function(vector<double> x) {
  return x[1] - x[0] + (endHeight-startHeight) + loss * x[2] * abs(x[2]);
}

//--------------------------------------------------------------
vector<double> Valve::functionDerivative(vector<double> x) {
  vector<double> result;
  result.push_back(-1.0);
  result.push_back(+1.0);
  result.push_back(+2 * loss * abs(x[2]));

  return result;
}

//--------------------------------------------------------------
void Valve::initialization(int mode, double value) {
  if (mode == 0)
    volumeFlowRate = 1.;
  else
    volumeFlowRate = value;
}

//--------------------------------------------------------------
void Valve::setDoubleProperty(string property, double value) {
  if (property == "position") {
    position = value;
    updateLoss();
  }else if(property == "startHeight")
      startHeight = value;
  else if(property == "endHeight")
      endHeight = value;
  else {
    cout << endl << endl << "ERROR! Valve::setDoubleProperty(property), unkown property: property=" << property << endl << endl;
  }
}

//--------------------------------------------------------------
void Valve::setIntProperty(string property, int value) {
  if(property == "startSegment")
    startSegment = value;
  else if(property == "endSegment")
    endSegment = value;
  else
    cout << endl << "ERROR! Valve::setIntProperty(property), unkown property: property=" << property << endl << endl;
}

//--------------------------------------------------------------
double Valve::getDoubleProperty(string property) {
  double out = 0.0;
  if (property == "position")
    out = position;
  else if (property == "loss")
    out = loss;
  else if (property == "volumeFlowRate")
    out = volumeFlowRate;
  else if (property == "headLoss")
    out = loss * volumeFlowRate * abs(volumeFlowRate);
  else if (property == "headLoss_per_unit_length")
    out = headLoss;
  else if ((property == "length") || (property == "L"))
    out = 0.5;
  else if (property == "cross_section")
    out = referenceCrossSection;
  else if (property == "startHeight")
    out = startHeight;
  else if (property == "endHeight")
    out = endHeight;
  else
    cout << endl << "ERROR! Valve::getDoubleProperty(property), unkown property: property=" << property << endl << endl;

  return out;
}

//--------------------------------------------------------------
string Valve::getStringProperty(string property) {
  string out = "";
  if (property == "type")
    out = type;
  else
    cout << endl << "ERROR! Valve::getStringProperty(property), unkown property: property=" << property << endl << endl;
  return out;
}

//--------------------------------------------------------------
int Valve::getIntProperty(string property) {
  int out = 0;
  if(property == "startSegment")
    out = startSegment;
  else if(property == "endSegment")
    out = endSegment;
  else
    cout << endl << "ERROR! Valve::getIntProperty(property), unkown property: property=" << property << endl << endl;
  return out;
}

//--------------------------------------------------------------
void Valve::updateLoss() {
  loss = interpolate(charX, charY, position);
  if (loss < 0.){
    loss = 0.;
    cout << endl << " !!!WARNING!!! Valve: loss coefficient is smaller than zero.\n Overriding with zero, and contionouing..." << endl;
  }
}