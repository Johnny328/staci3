#include "ValveTCV.h"

ValveTCV::ValveTCV(const string a_name, const string a_startNodeName, const string a_endNodeName, const double a_density, const double a_referenceCrossSection, const double a_setting, const double a_volumeFlowRate) : Valve(a_name, a_startNodeName, a_endNodeName, a_density, a_referenceCrossSection, a_volumeFlowRate)
{
  type = "ValveTCV";

  setting = a_setting / (2. * gravity * referenceCrossSection * referenceCrossSection);

  cout << endl << "k: " << a_setting << "  m: " << setting << endl;

  status = 2; // active
  typeCode = 7;  
}

//--------------------------------------------------------------
ValveTCV::~ValveTCV(){}

//--------------------------------------------------------------
string ValveTCV::info()
{
  ostringstream ss;
  ss << Edge::info();
  ss << "\n connection            : " << startNodeName << " (index:" << startNodeIndex << ") --> " << endNodeName << " (index:" << endNodeIndex << ")";
  ss << "\n setting              : " << setting;

  return ss.str();
}

//--------------------------------------------------------------
double ValveTCV::function(const VectorXd &ppq, VectorXd &fDer)
{
  double out;
  if(status == 2) // active, i.e. there is a loss coefficient
  {
    out = ppq(1) - ppq(0) + (endHeight-startHeight) + setting * ppq(2) * abs(ppq(2));
    fDer(0) = -1.0;
    fDer(1) =  1.0;
    fDer(2) =  setting * abs(ppq(2));
  }
  else if(status == 1) // open, i.e. without loss
  {
    out = ppq(1) - ppq(0) + (endHeight-startHeight);
    fDer(0) = -1.0;
    fDer(1) =  1.0;
  }
  else // Closed
  {
    out = ppq(1) - ppq(0) + (endHeight-startHeight) + 1e8 * ppq(2);
    fDer(0) = -1.0;
    fDer(1) =  1.0;
    fDer(2) =  1e8;
  }
  return out;
}

//--------------------------------------------------------------
void ValveTCV::initialization(int mode, double value)
{
  if (mode == 0)
    volumeFlowRate = 1.;
  else
    volumeFlowRate = value;
}

//--------------------------------------------------------------
void ValveTCV::setDoubleProperty(string property, double value)
{
  if (property == "position") {
    position = value;
    updateLoss();
  }else if(property == "startHeight")
      startHeight = value;
  else if(property == "endHeight")
      endHeight = value;
  else {
    cout << endl << endl << "ERROR! ValveTCV::setDoubleProperty(property), unkown property: property=" << property << endl << endl;
  }
}

//--------------------------------------------------------------
void ValveTCV::setIntProperty(string property, int value)
{
  if(property == "segment")
    segment = value;
  else
    cout << endl << "ERROR! ValveTCV::setIntProperty(property), unkown property: property=" << property << endl << endl;
}

//--------------------------------------------------------------
double ValveTCV::getDoubleProperty(string property)
{
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
    cout << endl << "ERROR! ValveTCV::getDoubleProperty(property), unkown property: property=" << property << endl << endl;

  return out;
}

//--------------------------------------------------------------
string ValveTCV::getStringProperty(string property)
{
  string out = "";
  if (property == "type")
    out = type;
  else
    cout << endl << "ERROR! ValveTCV::getStringProperty(property), unkown property: property=" << property << endl << endl;
  return out;
}

//--------------------------------------------------------------
int ValveTCV::getIntProperty(string property)
{
  int out = 0;
  if(property == "segment")
    out = segment;
  else
    cout << endl << "ERROR! ValveTCV::getIntProperty(property), unkown property: property=" << property << endl << endl;
  return out;
}

//--------------------------------------------------------------
void ValveTCV::updateLoss()
{
  loss = interpolate(charX, charY, position);
  if (loss < 0.){
    loss = 0.;
    cout << endl << " !!!WARNING!!! ValveTCV: loss coefficient is smaller than zero.\n Overriding with zero, and contionouing..." << endl;
  }
}