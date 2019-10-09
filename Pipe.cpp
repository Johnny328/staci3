#include "Pipe.h"

using namespace std;

Pipe::Pipe(const string a_name, const string a_startNodeName, const string a_endNodeName, const double a_density, const double a_length, const double a_diameter, const double a_roughness, const double a_volumeFlowRate, bool a_isCheckValve) : Edge(a_name, a_diameter * a_diameter * M_PI / 4., a_volumeFlowRate, a_density)
{
  type = "Pipe";
  numberNode = 2;
  startNodeName = a_startNodeName;
  endNodeName = a_endNodeName;
  length = a_length;
  diameter = a_diameter;
  roughness = a_roughness;
  lambda = 0.02;
  isCheckValve = a_isCheckValve;
  status = 1;
}

//--------------------------------------------------------------
//! Sets the friction model
/*!
DW (frictionModel = 0) -> Darcy Wiesenbach
HW (frictionModel = 1) -> Hazen-Williams
*/
void Pipe::setFrictionModel(string a_fric_type) {
  if (a_fric_type == "DW")
    frictionModel = 0;
  else if (a_fric_type == "HW")
    frictionModel = 1;
  else
    cout << endl << "!ERROR! Unknown friction model: " << a_fric_type << endl << "Available options: DW | HW " << endl << endl;
}

//--------------------------------------------------------------
Pipe::~Pipe() {}

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
    strstrm << "DW";
  else if(frictionModel==1)
    strstrm << "HW";
  else
    strstrm << "!ERROR!";
  strstrm << endl;

  return strstrm.str();
}

//--------------------------------------------------------------
double Pipe::function(vector<double> x)// x = [Pstart, Pend, VolumeFlowRate]
{ 
  if(status == 1) // OPEN
    return x[1] - x[0] + (endHeight-startHeight) + computeHeadloss(x[2]);
  else // CLOSED
    return x[2];
}

//--------------------------------------------------------------
vector<double> Pipe::functionDerivative(vector<double> x)
{ 
  vector<double> out;
  if(status == 1) // OPEN
  { 
    out.push_back(-1.0);
    out.push_back(1.0);
    out.push_back(computeHeadlossDerivative(x[2]));
  }
  else // CLOSED
  {
    out.push_back(0.0);
    out.push_back(0.0);
    out.push_back(1.0);
  }
  return out;
}

//--------------------------------------------------------------
void Pipe::initialization(int mode, double value)
{
  if(mode == 0)
    setDoubleProperty("volumeFlowRate", 1.);
  else
    setDoubleProperty("volumeFlowRate", value);
}

//--------------------------------------------------------------
double Pipe::functionParameterDerivative(string parameter)
{
  double out = 0.0;
  if (parameter == "diameter"){
    out = -5. * getLambda(volumeFlowRate) * length / pow(diameter, 6) * 8 / density / pow(M_PI, 2) * volumeFlowRate * abs(volumeFlowRate);  // Pa/m
  }
  else if (parameter == "friction_coeff")
  {
    double old = roughness;
    double delta = roughness * 0.001;
    roughness += delta;
    double f1 = computeHeadloss(volumeFlowRate);
    roughness = old;
    double f0 = computeHeadloss(volumeFlowRate);
    out = (f1 - f0) / delta;
  }else{
    cout << endl << "!!!ERROR!!! Pipe::functionParameterDerivative(parameter), unkown input: parameter=" << parameter << endl << "Available parameters: diameter | friction_coeff" << endl;
    cout << endl << "Name of pipe: " << name << endl;
    out = 0.0;
  }
  return out;
  //return out / density / gravity;
}

//--------------------------------------------------------------
double Pipe::computeHeadloss(double q)
{ // q is volumeFlowRate
  double velocity = q / 1000. / referenceCrossSection;
  lambda = getLambda(q);
  double headloss = lambda * length / diameter / gravity / 2. * velocity * abs(velocity);
  //double headloss = lambda * length / diameter * density / 2. * velocity * abs(velocity);

  return headloss;
}

//--------------------------------------------------------------
double Pipe::computeHeadlossDerivative(double q)
{ // q is volumeFlowRate
  double out;
  out = 2.* getLambda(q) * length/diameter / gravity / 2. / (referenceCrossSection*referenceCrossSection) * abs(q/1000.) / density;
  //out = getLambda(q) * length / pow(diameter, 5) * 8 / density / pow(M_PI, 2) * 2 * abs(q);
  return out;
}

//--------------------------------------------------------------
//! Sets the friction coefficient lambda
/*!
Based on the actual friction model (DW, HW) computes the friction coefficient.

For DW (frictionModel = 0) -> Darcy Wiesenbach model parameter 'roughness'
is pipe surface roughness in mm

For HW (frictionModel = 1) -> Hazen-Williams model parameter 'roughness' is
the Hazen-Williamd constant. If the user-supplied value is less tha 10, it is
overwritten to 10.

For any of these models, if parameter roughness is negative, it is assumed that
lambda=-roughness
*/
double Pipe::getLambda(double q) { // q is volumeFlowRate
  double v_min = 0.001;
  double velocity = q / 1000. / referenceCrossSection;
  if(fabs(velocity) < v_min)
    velocity = v_min;
  double nu = 1e-6;
  double lambda_min = 0.001;
  double lambda_max = 64./0.1;
  double dp;

  if(roughness <= 0)
  {
    lambda = -roughness;
  }
  else if(frictionModel == 0)  // Darcy-Wiesenbach
  {
    double Re = fabs(velocity) * diameter / nu;

    double error = 1.0e10, tmp = 0.0, lambda_new = 0.0;
    unsigned int i = 0;
    while ((error > 1e-6) && (i < 20)) {
      if (Re < 2300)
        lambda_new = 64. / Re;
      else {
        tmp = -2.0 * log10(roughness / 1000 / diameter / 3.71 + 2.51 / Re / sqrt(lambda));
        lambda_new = 1 / tmp / tmp;
      }

      if (lambda_new < lambda_min)
        lambda_new = lambda_min;
      if (lambda > lambda_max)
        lambda_new = lambda_max;

      error = fabs((lambda - lambda_new) / lambda);
      lambda = lambda_new;

      i++;
    }
  }
  else if (frictionModel == 1)  // Hazen-Williams, C factor around 100 // TODO: REDUCE THE MATH OPERATIONS
  {
    if (roughness <= 0)
      lambda = -roughness;
    else {
      double C_factor = roughness;
      double C_MIN = 1.;

      if (C_factor < C_MIN) {
        C_factor = C_MIN;
        roughness = C_MIN;
      }

      dp = length / pow(C_factor, 1.852) / pow(diameter, 4.871) * 7.88 / pow(0.85, 1.85) * pow(abs(velocity*referenceCrossSection), 0.852) * (velocity * referenceCrossSection) * density * gravity;

      lambda = abs(dp / (length / diameter * density / 2 * velocity * fabs(velocity)));      
    }
  }
  else
  {
    cout << endl << "!ERROR! Pipe name: " << name << ", friction model is not set, frictionModel: " << frictionModel << endl;
    exit(-1);
  }
  return lambda;
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
  else if (prop == "headLoss" || prop == "headloss")
    out = abs(computeHeadloss(volumeFlowRate) / density / gravity);
  else if (prop == "headLossPerUnitLength" || prop == "headloss_per_unit_length")
    out = abs(computeHeadloss(volumeFlowRate) / density / gravity / length);
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
