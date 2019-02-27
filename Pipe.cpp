#include <cmath>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "Edge.h"
#include "Pipe.h"

using namespace std;

Pipe::Pipe(const string a_name, const string a_startNodeName, const string a_endNodeName, const double a_density, const double a_length, const double a_diameter, const double a_roughness, const double a_massFlowRate) : Edge(a_name, a_diameter * a_diameter * M_PI / 4., a_massFlowRate, a_density) {
  type = "Pipe";
  numberNode = 2;
  startNodeName = a_startNodeName;
  endNodeName = a_endNodeName;
  length = a_length;
  diameter = a_diameter;
  roughness = a_roughness;
  fluidVolume = diameter * diameter * M_PI / 4. * length;
  lambda = 0.02;
}

//--------------------------------------------------------------
//! Sets the friction model
/*!
DW (frictionModelType = 0) -> Darcy Wiesenbach
HW (frictionModelType = 1) -> Hazen-Williams
*/
void Pipe::setFrictionModel(string a_fric_type) {
  if (a_fric_type == "DW")
    frictionModelType = 0;
  else {
    if (a_fric_type == "HW")
      frictionModelType = 1;
    else {
      cout << endl
           << "HIBA! Pipe::computeHeadloss, ismeretlen surlodasi modell (DW|HW) "
           ": "
           << a_fric_type << endl
           << endl;
    }
  }
}

//--------------------------------------------------------------
Pipe::~Pipe() {}

//--------------------------------------------------------------
string Pipe::info() {
  ostringstream strstrm;
  strstrm << Edge::info();
  strstrm << "\n       type  : " << type;
  strstrm << "\n  connection : " << startNodeName << "(index:" << startNodeIndex << ") --> " << endNodeName << "(index:" << endNodeIndex << ")\n";
  strstrm << "       data    : length=" << length << "[m], diameter=" << diameter << "[m], roughness=" << roughness << "[mm], lambda=" << lambda << "[-]" << endl;

  return strstrm.str();
}

//--------------------------------------------------------------
double Pipe::function(vector<double> x) {
  double result;
  double startPressure = x[0] * density * gravity;
  double endPressure = x[1] * density * gravity;
  double startHeight = x[2];
  double endHeight = x[3];

  result = endPressure - startPressure + density*gravity*(endHeight-startHeight) + computeHeadloss();
  lambda = getLambda();

  return result / density / gravity;
}

//--------------------------------------------------------------
vector<double> Pipe::functionDerivative(vector<double> x) {
  double startHeight = x[2];
  double endHeight = x[3];
  vector<double> result;
  result.push_back(-density * gravity);
  result.push_back(+density * gravity);
  result.push_back(computeHeadlossDerivative());
  result.push_back(-density * gravity * (endHeight - startHeight));

  for(int i=0; i<result.size(); i++)
    result.at(i) /= density * gravity;
  return result;
}

//--------------------------------------------------------------
void Pipe::initialization(int mode, double value) {
  if (mode == 0)
    setMassFlowRate(1.);
  else
    setMassFlowRate(value);
}

//--------------------------------------------------------------
//! Sets the friction coefficient lambda
/*!
Based on the actual friction model (DW, HW) computes the friction coefficient.

For DW (frictionModelType = 0) -> Darcy Wiesenbach model parameter 'roughness'
is pipe surface roughness in mm

For HW (frictionModelType = 1) -> Hazen-Williams model parameter 'roughness' is
the Hazen-Williamd constant. If the user-supplied value is less tha 10, it is
overwritten to 10.

For any of these models, if parameter roughness is negative, it is assumed that
lambda=-roughness
*/
double Pipe::getLambda() {
  double v_min = 0.001;
  double velocity = massFlowRate / density / (diameter * diameter * M_PI / 4);
  if(fabs(velocity) < v_min)
    velocity = v_min;
  double nu = 1e-6;
  double lambda_min = 0.001;
  double lambda_max = 64./0.1;
  double dp;

  if (frictionModelType == 0)  // Darcy-Wiesenbach
  {
    if (roughness <= 0)
      lambda = -roughness;
    else {
      double Re = fabs(velocity) * diameter / nu;

      double hiba = 1.0e10, ize = 0.0, lambda_uj = 0.0;
      unsigned int i = 0;
      while ((hiba > 1e-6) && (i < 20)) {
        if (Re < 2300)
          lambda_uj = 64. / Re;
        else {
          ize = -2.0 *
                log10(roughness / 1000 / diameter / 3.71 + 2.51 / Re / sqrt(lambda));
          lambda_uj = 1 / ize / ize;
        }

        if (lambda_uj < lambda_min)
          lambda_uj = lambda_min;
        if (lambda > lambda_max)
          lambda_uj = lambda_max;

        hiba = fabs((lambda - lambda_uj) / lambda);
        lambda = lambda_uj;

        i++;
      }
    }
  }

  if (frictionModelType == 1)  // Hazen-Williams, C_factor around 100
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

      dp = length / pow(C_factor, 1.85) / pow(diameter, 4.87) * 7.88 / pow(0.85, 1.85) *
           pow(abs(velocity*referenceCrossSection), 0.85) * (velocity * referenceCrossSection) * density * gravity;

      lambda = abs(dp / (length / diameter * density / 2 * velocity * fabs(velocity)));      
    }
  }
  return lambda;
}

//--------------------------------------------------------------
double Pipe::getProperty(string str) {
  double out = 0.0;
  if (str == "referenceCrossSection")
    out = referenceCrossSection;
  else if (str == "lambda")
    out = lambda;
  else if (str == "diameter")
    out = diameter;
  else if (str == "length")
    out = length;
  else if (str == "Rh")
    out = diameter / 2.;
  else if (str == "roughness")
    out = roughness;
  else if (str == "headLoss")
    out = abs(computeHeadloss() / density / gravity);
  else if (str == "headloss_per_unit_length")
    out = abs(computeHeadloss() / density / gravity / length);
  else if (str == "massFlowRate")
    out = massFlowRate;
  else {
    cout << endl << "ERROR! Pipe::getProperty(str), unkown input: str=" << str << endl << endl;
    cout << endl << "Name of pipe: " << name << endl;
    cin.get();
  }
  return out;
}

//--------------------------------------------------------------
double Pipe::getFunctionDerivative(string str) {
  double out = 0.0;
  if (str == "diameter")
    out = -5. * getLambda() * length / pow(diameter, 6) * 8 / density / pow(M_PI, 2) * massFlowRate * abs(massFlowRate);  // Pa/m
  else if (str == "friction_coeff") {
    double old = roughness;
    double delta = roughness * 0.01;
    roughness += delta;
    double f1 = computeHeadloss();
    roughness = old;
    double f0 = computeHeadloss();
    out = (f1 - f0) / delta;
  } else {
    cout << endl << "HIBA! Pipe::getFunctionDerivative(str), unkown input: str=" << str << endl << endl;
    cout << endl << "Name of pipe: " << name << endl;
    out = 0.0;
  }
  return out / density / gravity;
}

//--------------------------------------------------------------
void Pipe::setProperty(string str, double var) {
  if (str == "diameter") {
    diameter = var;
    referenceCrossSection = diameter * diameter * M_PI / 4.;
    fluidVolume = referenceCrossSection * length;
  }
  else if (str == "roughness") {
    roughness = var;
  }
  else if (str == "length") {
    length = var;
  } else {
    cout << endl << "HIBA! Pipe::setProperty(str), unkown input: str=" << str << endl << endl;
  }
}

//--------------------------------------------------------------
//! Computes the head loss in Pa
/*!
dp'=lambda*length/diameter*density/2*velocity*fabs(velocity)
*/
double Pipe::computeHeadloss() {
  double velocity = massFlowRate / density / referenceCrossSection;
  double lambda = getLambda();
  double headloss = lambda * length / diameter * density / 2. * velocity * fabs(velocity);

  return headloss;
}

//--------------------------------------------------------------
//! Computes the head loss derivative w.r.t. mass flow rate
/*!
dp'=lambda*length/diameter*density/2*velocity*fabs(velocity)
diameter dp'/dmp=lambda*length/diameter*density/2*1/(density*A)^2*abs(velocity)
*/

double Pipe::computeHeadlossDerivative() {
  double der;
  der = getLambda() * length / pow(diameter, 5) * 8 / density / pow(M_PI, 2) * 2 *
        abs(massFlowRate);  // Pa/(kg/s)
  return der;
}
