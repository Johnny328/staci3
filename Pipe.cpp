#include "Pipe.h"

using namespace std;

Pipe::Pipe(const string a_name, const string a_startNodeName, const string a_endNodeName, const double a_density, const double a_length, const double a_diameter, const double a_roughness, const double a_volumeFlowRate, bool a_isCheckValve, int a_frictionModel) : Edge(a_name, a_diameter * a_diameter * M_PI / 4., a_volumeFlowRate, a_density)
{
  numberNode = 2;
  startNodeName = a_startNodeName;
  endNodeName = a_endNodeName;
  length = a_length;
  diameter = a_diameter;
  roughness = a_roughness;

  frictionModel = a_frictionModel;
  setPipeConst();

  isCheckValve = a_isCheckValve;
  if(isCheckValve)
  {
    typeCode = 0;
    type = "PipeCV";
  }
  else
  {
    typeCode = 1;
    type = "Pipe";
  }

  status = 1; // i.e. open
}

//--------------------------------------------------------------
Pipe::~Pipe(){}

//--------------------------------------------------------------
string Pipe::info()
{
  ostringstream strstrm;
  strstrm << Edge::info();
  strstrm << "\n type                  : " << type;
  strstrm << "\n connection            : " << startNodeName << "(index:" << startNodeIndex << ") --> " << endNodeName << "(index:" << endNodeIndex << ")";
  strstrm << "\n length                : " << length << " [m]";
  strstrm << "\n diameter              : " << diameter << " [m]";
  strstrm << "\n roughness             : " << roughness << " [mm]";
  strstrm << "\n lambda                : " << lambda << " [-]";
  strstrm << "\n headloss              : " << getDoubleProperty("headLoss") << " [m]";
  strstrm << "\n velocity              : " << getDoubleProperty("velocity") << " [m/s]";
  strstrm << "\n friction model        : ";
  if(frictionModel==0)
    strstrm << "H-W";
  else if(frictionModel==2)
    strstrm << "C-F";
  else
    strstrm << "!ERROR!";
  strstrm << endl;

  return strstrm.str();
}

//--------------------------------------------------------------
double Pipe::function(const VectorXd &ppq, VectorXd &fDer)// ppq = [Pstart, Pend, VolumeFlowRate]
{ 
  double out = 0.0;
  if(status == 1) // OPEN
  {
    fDer(0) = -1.0;
    fDer(1) = 1.0;
    if(frictionModel == 0) // H-W: HAZEN - WILLIAMS
    {
      double tmp = pipeConst * pow(abs(ppq(2)),0.85185);
      out = ppq(1) - ppq(0) + (endHeight-startHeight) +  ppq(2) * tmp;
      fDer(2) = 1.85185 * tmp;
    }
    else if(frictionModel == 2) // C-F: CONSTANT FRICTION COEFFICIENT
    {
      out = ppq(1) - ppq(0) + (endHeight-startHeight) + pipeConst * ppq(2) * abs(ppq(2));
      fDer(2) = 2 * pipeConst * abs(ppq(2));
    }
  }
  else // CLOSED, status is 0 or -1
  {
    //out = ppq(2);
    //fDer(2) = 1.0;

    out = ppq(1) - ppq(0) + (endHeight-startHeight) + 1e8 * ppq(2) * abs(ppq(2));
    out = 0.0;
    fDer(0) = -1.0;
    fDer(1) =  1.0;
    fDer(2) = 2 * 1e8;
  }
  
  return out;
  
}

//--------------------------------------------------------------
void Pipe::setPipeConst()
{
  if(frictionModel == 0)
    pipeConst = 10.654 * pow(roughness,-1.85185) * pow(diameter,-4.87) * length;
  else if(frictionModel == 2)
    pipeConst = roughness * length / (diameter * 2. * gravity * (diameter*diameter*M_PI/4.));
}

//--------------------------------------------------------------
void Pipe::initialization(int mode, double value)
{
  if(status == 1) // open
  {
    if(mode == 0)
      volumeFlowRate = 1.;
    else
      volumeFlowRate = value;
  }
  else // closed
  {
    volumeFlowRate = 0.0;
  }
}

//--------------------------------------------------------------
double Pipe::functionParameterDerivative(int parameter)
// 0: roughness coefficient, 1: diameter
{
  double out = 0.0;
  if(status == 1) // OPEN
  {
    if(parameter == 0) // roghness
    {
      if(frictionModel == 0) // H-W: HAZEN - WILLIAMS
      {
        out = pipeConst * (-1.85185) / roughness * volumeFlowRate * pow(abs(volumeFlowRate),0.85185);
      }
      else if(frictionModel == 2)// C-F: CONSTANT FRICTION COEFFICIENT
      {
        out = pipeConst / roughness * volumeFlowRate * abs(volumeFlowRate);
      }
    }
    else if(parameter == 1) // diameter
    {
      if(frictionModel == 0) // H-W: HAZEN - WILLIAMS
      {
        out = pipeConst * (-4.87) / diameter * volumeFlowRate * pow(abs(volumeFlowRate),0.85185);
      }
      else if(frictionModel == 2)// C-F: CONSTANT FRICTION COEFFICIENT
      {
        out = pipeConst * (-5) / diameter * volumeFlowRate * abs(volumeFlowRate);
      }
    }
    else
    {
      cout << endl << "!WARNING! Pipe::functionParameterDerivative(parameter), unkown input: parameter = " << parameter << endl << "Available parameters: 0: roughness, 1: diameter" << endl;
      cout << endl << "Name of pipe: " << name << endl;
    }
  }
  else // CLOSED
  {
    out = 0.0;
  }
  return out;
}

//--------------------------------------------------------------
double Pipe::getDoubleProperty(string prop)
{
  double out = 0.;
  if(prop == "diameter")
    out = diameter;
  else if(prop == "length")
    out = length;
  else if (prop == "lambda")
    out = lambda;
  else if (prop == "segment")
    out = (double)segment;
  else if (prop == "Rh")
    out = diameter / 2.;
  else if (prop == "roughness")
    out = roughness;
  else if(prop == "massFlowRate" || prop == "mass_flow_rate")
    out = volumeFlowRate * density/1000.;
  else if(prop == "volumeFlowRate" || prop == "volume_flow_rate")
    out = volumeFlowRate;
  else if(prop == "volumeFlowRateAbs")
    out = abs(volumeFlowRate);
  else if(prop == "velocity")
    out = volumeFlowRate / 1000. / referenceCrossSection;
  else if(prop == "density")
    out = density;
  else if(prop == "referenceCrossSection" || prop == "reference_cross_section" || prop == "Aref")
    out = referenceCrossSection;
  else if(prop == "startHeight")
    out = startHeight;
  else if(prop == "endHeight")
    out = endHeight;
  else if(prop == "volume")
    out = referenceCrossSection*length;
  else
  {
    cout << endl << endl << "DOUBLE Pipe::getDoubleProperty() wrong argument:" << prop;
    cout << ", right values: diamater | roughness | length | massFlowRate | velocity | density | referenceCrossSection | user1 | user2 | startHeight | endHeight | volume" << endl << endl;
  }
  return out;
}

//--------------------------------------------------------------
int Pipe::getIntProperty(string prop)
{
  int out = 0;
  if(prop == "frictionModel" || prop == "friction_model")
    out = frictionModel;
  else
  {
    cout << endl << endl << "INT Pipe::getIntProperty() wrong argument:" << prop;
    cout << ", right values: frictionModel" << endl << endl;
  }
  return out;
}

//--------------------------------------------------------------
void Pipe::setDoubleProperty(string prop, double value)
{
  if(prop == "diameter")
    diameter = value;
  else if(prop == "roughness")
    roughness = value;
  else if(prop == "length")
    length = value;
  else if(prop == "volumeFlowRate")
    volumeFlowRate = value;
  else if(prop == "density")
    density = value;
  else if(prop == "referenceCrossSection" || prop == "reference_cross_section")
    referenceCrossSection = value;
  else if(prop == "startHeight")
    startHeight = value;
  else if(prop == "endHeight")
    endHeight = value;
  else
  {  
    cout << endl << endl << "Pipe::setDoubleProperty( DOUBLE ) wrong argument:" << prop;
    cout << ", right values: diameter | roughness | length | volumeFlowRate | velocity | density | referenceCrossSection | userOutput | startHeight | endHeight" << endl << endl;
  }
}

//--------------------------------------------------------------
void Pipe::setIntProperty(string prop, int value)
{
  if(prop == "frictionModel" || prop == "friction_model")
    frictionModel = value;
  else
  {  
    cout << endl << endl << "Pipe::setIntProperty( DOUBLE ) wrong argument:" << prop;
    cout << ", right values: startNodeIndex | endNodeIndex | numberNode" << endl << endl;
  }
}
