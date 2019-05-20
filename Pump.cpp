#include "Pump.h"

using namespace std;
using namespace Eigen;

Pump::Pump(const string a_name, const string a_startNodeName, const string a_endNodeName, const double a_density, const double a_referenceCrossSection, vector<double> a_volumeFlowRate, vector<double> a_head, const double a_massFlowRate) : Edge(a_name, a_referenceCrossSection, a_massFlowRate, a_density){

  type = "Pump";
  numberNode = 2;
  startNodeName = a_startNodeName;
  endNodeName = a_endNodeName;
  volumeFlowRate = a_volumeFlowRate;
  head = a_head;
  int numberPoint = volumeFlowRate.size();
  
  if(numberPoint != head.size()){
    cout << endl << "!!!ERROR!!! Pump(" << name << ") characteristic curve: number of Q (" << numberPoint << ") is NOT equal to number of H (" << volumeFlowRate.size() << ") !" << endl << "Pump ID: " << a_name << endl << "Exiting..." << endl;
    exit(0);
  }

  // Converting the volume flow rate from m3/h to l/s
  for(int i=0; i<numberPoint; i++) 
    volumeFlowRate.at(i) /= 3.6;

  // Copying vector<double> to Eigen::VectorXd
  VectorXd Q(numberPoint), H(numberPoint);
  for(int i=0; i<numberPoint; i++){
    Q(i) = volumeFlowRate[i];
    H(i) = head[i];
  }

  // Fitting second order polynomial to Q-H points with least squares method
  curveOrder = 2;
  VectorXd C = leastSquaresPolynomial(Q,H,curveOrder);

  // Copying back to vector<double>
  for(int i=0; i<C.rows(); i++)
    coefficients.push_back(C(i));
}

//--------------------------------------------------------------
Pump::~Pump() {}

//--------------------------------------------------------------
string Pump::info(){
  ostringstream strstrm;
  strstrm << Edge::info();
  strstrm << "\n head                  : " << getDoubleProperty("head") << " [m]";
  strstrm << "\n type                  : " << type;
  strstrm << "\n connection            : " << startNodeName << "(index:" << startNodeIndex << ") --> " << endNodeName << "(index:" << endNodeIndex << ")\n";

  strstrm << scientific << setprecision(3);
  strstrm << "\n characteristic curve\n----------------------\n Q [m3/h] |  H [m]   ";
  for(int i = 0; i < volumeFlowRate.size(); i++)
    strstrm << endl << volumeFlowRate[i] * 3600. << " | " << head[i];
  strstrm << "\n\n fitted polynomial (order: " << curveOrder << ")\n--------------------------------\n H(Q) = " << coefficients[0];
  for(int i = 1; i < coefficients.size(); i++){
    strstrm << " + " << coefficients[i] << "*Q";
    if(i>1)
      strstrm << "^" << i;
  } 
  strstrm << endl;
  return strstrm.str();
}

//--------------------------------------------------------------
double Pump::function(vector<double> x){ // x = [Pstart, Pend, MassFlowRate]
  return (x[1] - x[0]) - characteristicCurve(x[2]) + (endHeight - startHeight);
}

//--------------------------------------------------------------
vector<double> Pump::functionDerivative(vector<double> x){
  vector<double> out;
  out.push_back(-1.0);
  out.push_back(+1.0);

  if(x[2]/density<min(volumeFlowRate))
    cout << endl << "!!!WARNING!!! Pump (" << name << ") characteristic curve: volume flow rate (" << x[2]/density << ") is lower than curve minimum (" << min(volumeFlowRate) << ")" << endl << "Extrapolating..." << endl;

  if(x[2]/density>max(volumeFlowRate))
    cout << endl << "!!!WARNING!!! Pump (" << name << ") characteristic curve: volume flow rate (" << x[2]/density << ") is larger than curve minimum (" << max(volumeFlowRate) << ")" << endl << "Extrapolating..." << endl;

  // Calculating the derivative analytically from characteristic curve
  double derivative = 0.0;
  for (int i = 1; i < curveOrder; i++) 
    derivative += coefficients[i] * i * pow(x[2] / density, i - 1);
  derivative /= -density;

  out.push_back(derivative);

  return out;
}

//--------------------------------------------------------------
void Pump::initialization(int mode, double value){
  if(mode == 0)
    setDoubleProperty("massFlowRate", 1.);
  else
    setDoubleProperty("massFlowRate", value);
}

//--------------------------------------------------------------
double Pump::characteristicCurve(double mp){

  double H = 0.;
  double Q = mp/density;
  
  if(Q<min(volumeFlowRate))
    cout << endl << "!!!WARNING!!! Pump (" << name << ") characteristic curve: volume flow rate (" << Q << ") is lower than curve minimum (" << min(volumeFlowRate) << ")" << endl << "Extrapolating..." << endl;

  if(Q>max(volumeFlowRate))
    cout << endl << "!!!WARNING!!! Pump (" << name << ") characteristic curve: volume flow rate (" << Q << ") is larger than curve minimum (" << max(volumeFlowRate) << ")" << endl << "Extrapolating..." << endl;

  for(int i=0; i<coefficients.size(); i++)
    H += coefficients[i] * pow(revolutionNumber,2-i) * pow(Q,i);

  return H;
}

//--------------------------------------------------------------
double Pump::getDoubleProperty(string prop){
  double out = 0.0;
  if(prop == "referenceCrossSection" || prop == "reference_cross_section")
    out = referenceCrossSection;
  else if(prop == "head")
    out = characteristicCurve(massFlowRate);
  else if(prop == "revolutionNumber")
    out = revolutionNumber;
  else if(prop == "massFlowRate" || prop == "mass_flow_rate")
    out = massFlowRate;
  else if(prop == "volumeFlowRate" || prop == "volume_flow_rate")
    out = massFlowRate / density;
  else if(prop == "velocity")
    out = massFlowRate / density / referenceCrossSection;
  else if(prop == "headLoss" || prop == "headloss")
    out = abs(characteristicCurve(massFlowRate));
  else if(prop == "user1")
    out = user1;
  else if(prop == "user2")
    out = user2;
  else{
    cout << endl << endl << "DOUBLE Pump::getDoubleProperty() wrong argument:" << prop;
    cout << ", right values: massFlowRate | head | revolutionNumber | velocity | density | referenceCrossSection | user1 | user2" << endl << endl;
    out = 0.0;
  }
  return out;
}

//--------------------------------------------------------------
void Pump::setDoubleProperty(string prop, double value){
  if(prop == "massFlowRate" || prop == "mass_flow_rate")
    massFlowRate = value;
  else if(prop == "density")
    density = value;
  else if(prop == "revolutionNumber")
    revolutionNumber = value;
  else if(prop == "referenceCrossSection" || prop == "reference_cross_section")
    referenceCrossSection = value;
  else if(prop == "user1")
    user1 = value;
  else if(prop == "user2")
    user2 = value;
  else if(prop == "startHeight")
    startHeight = value;
  else if(prop == "endHeight")
    endHeight = value;
  else
  {  
    cout << endl << endl << "Pump::setDoubleProperty( DOUBLE ) wrong argument:" << prop;
    cout << ", right values: massFlowRate | density | revolutionNumber | referenceCrossSection | user1 | user2" << endl << endl;
  }
}

//--------------------------------------------------------------
vector<double> Pump::getVectorProperty(string prop){
  vector<double> out;
  if(prop == "volumeFlowRate")
    out = volumeFlowRate;
  else if(prop == "head")
    out = head;
  else
  {  
    cout << endl << endl << "vector <double> Pump::getVectorProperty( string prop ) wrong argument:" << prop;
    cout << ", right values: volumeFlowRate | head " << endl << endl;
  }
  return out;
}