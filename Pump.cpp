#include "Pump.h"

using namespace std;
using namespace Eigen;

Pump::Pump(const string a_name, const string a_startNodeName, const string a_endNodeName, const double a_density, const double a_referenceCrossSection, vector<double> a_qCurve, vector<double> a_hCurve, const double a_volumeFlowRate) : Edge(a_name, a_referenceCrossSection, a_volumeFlowRate, a_density){

  type = "Pump";
  numberNode = 2;
  startNodeName = a_startNodeName;
  endNodeName = a_endNodeName;
  qCurve = a_qCurve;
  hCurve = a_hCurve;
  int numberPoint = qCurve.size();
  
  if(numberPoint != hCurve.size()){
    cout << endl << "!!!ERROR!!! Pump(" << name << ") characteristic curve: number of Q (" << numberPoint << ") is NOT equal to number of H (" << qCurve.size() << ") !" << endl << "Pump ID: " << a_name << endl << "Exiting..." << endl;
    exit(0);
  }

  // Converting the volume flow rate from m3/h to l/s
  //for(int i=0; i<numberPoint; i++) 
    //qCurve.at(i) /= 3.6;

  // Copying vector<double> to Eigen::VectorXd
  VectorXd Q(numberPoint), H(numberPoint);
  for(int i=0; i<numberPoint; i++){
    Q(i) = qCurve[i];
    H(i) = hCurve[i];
  }

  // Fitting second order polynomial to Q-H points with least squares method
  curveOrder = 2;
  VectorXd C = leastSquaresPolynomial(Q,H,curveOrder);

  // Copying back to vector<double>
  for(int i=0; i<C.rows(); i++)
    coeffCurve.push_back(C(i));
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
  strstrm << "\n characteristic curve\n----------------------\n Q [l/s] |  H [m]   ";
  for(int i = 0; i < qCurve.size(); i++)
    strstrm << endl << qCurve[i] * 3.6 << " | " << hCurve[i];
  strstrm << "\n\n fitted polynomial (order: " << curveOrder << ")\n--------------------------------\n H(Q) = " << coeffCurve[0];
  for(int i = 1; i < coeffCurve.size(); i++){
    strstrm << " + " << coeffCurve[i] << "*Q";
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

  //if(x[2]/1000.<min(qCurve))
    //cout << endl << "!!!WARNING!!! Pump (" << name << ") characteristic curve: volume flow rate (" << x[2]/density << ") is lower than curve minimum (" << min(qCurve) << ")" << endl << "Extrapolating..." << endl;

  //if(x[2]/1000.>max(qCurve))
    //cout << endl << "!!!WARNING!!! Pump (" << name << ") characteristic curve: volume flow rate (" << x[2]/density << ") is larger than curve minimum (" << max(qCurve) << ")" << endl << "Extrapolating..." << endl;

  // Calculating the derivative analytically from characteristic curve
  double derivative = 0.0;
  for(int i = 1; i < coeffCurve.size(); i++) 
    derivative += coeffCurve[i] * i * pow(x[2], i - 1);
  //derivative /= -1000.;

  out.push_back(derivative);

  return out;
}

//--------------------------------------------------------------
void Pump::initialization(int mode, double value){
  if(mode == 0)
    setDoubleProperty("volumeFlowRate", 1.);
  else
    setDoubleProperty("volumeFlowRate", value);
}

//--------------------------------------------------------------
double Pump::characteristicCurve(double Q){
  
  if(Q<min(qCurve))
    cout << endl << "!!!WARNING!!! Pump (" << name << ") characteristic curve: volume flow rate (" << Q << ") is lower than curve minimum (" << min(qCurve) << ")" << endl << "Extrapolating..." << endl;

  if(Q>max(qCurve))
    cout << endl << "!!!WARNING!!! Pump (" << name << ") characteristic curve: volume flow rate (" << Q << ") is larger than curve minimum (" << max(qCurve) << ")" << endl << "Extrapolating..." << endl;

  double H = 0.;
  for(int i=0; i<coeffCurve.size(); i++)
    H += coeffCurve[i] * pow(revolutionNumber,2-i) * pow(Q,i);

  return H;
}

//--------------------------------------------------------------
double Pump::getDoubleProperty(string prop){
  double out = 0.0;
  if(prop == "referenceCrossSection" || prop == "reference_cross_section")
    out = referenceCrossSection;
  else if(prop == "head")
    out = characteristicCurve(volumeFlowRate);
  else if(prop == "revolutionNumber")
    out = revolutionNumber;
  else if(prop == "startHeight")
    out = startHeight;
  else if(prop == "endHeight")
    out = endHeight;
  else if(prop == "massFlowRate")
    out = volumeFlowRate * density/1000.;
  else if(prop == "volumeFlowRate")
    out = volumeFlowRate;
  else if(prop == "velocity")
    out = volumeFlowRate / 1000. / referenceCrossSection;
  else if(prop == "headLoss" || prop == "headloss")
    out = abs(characteristicCurve(volumeFlowRate));
  else{
    cout << endl << endl << "DOUBLE Pump::getDoubleProperty() wrong argument:" << prop;
    cout << ", right values: massFlowRate | head | revolutionNumber | velocity | density | referenceCrossSection | user1 | user2" << endl << endl;
    out = 0.0;
  }
  return out;
}

//--------------------------------------------------------------
void Pump::setDoubleProperty(string prop, double value){
  if(prop == "volumeFlowRate")
    volumeFlowRate = value;
  else if(prop == "density")
    density = value;
  else if(prop == "revolutionNumber")
    revolutionNumber = value;
  else if(prop == "referenceCrossSection" || prop == "reference_cross_section")
    referenceCrossSection = value;
  else if(prop == "startHeight")
    startHeight = value;
  else if(prop == "endHeight")
    endHeight = value;
  else
  {  
    cout << endl << endl << "Pump::setDoubleProperty( DOUBLE ) wrong argument:" << prop;
    cout << ", right values: volumeFlowRate | density | revolutionNumber | referenceCrossSection | user1 | user2" << endl << endl;
  }
}

//--------------------------------------------------------------
vector<double> Pump::getVectorProperty(string prop){
  vector<double> out;
  if(prop == "qCurve")
    out = qCurve;
  else if(prop == "hCurve")
    out = hCurve;
  else
  {  
    cout << endl << endl << "vector <double> Pump::getVectorProperty( string prop ) wrong argument:" << prop;
    cout << ", right values: qCurve | head " << endl << endl;
  }
  return out;
}