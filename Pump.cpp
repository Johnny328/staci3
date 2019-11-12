#include "Pump.h"

using namespace std;
using namespace Eigen;

Pump::Pump(const string a_name, const string a_startNodeName, const string a_endNodeName, const double a_density, const double a_referenceCrossSection, vector<double> a_qCurve, vector<double> a_hCurve, const double a_volumeFlowRate, string a_curveType) : Edge(a_name, a_referenceCrossSection, a_volumeFlowRate, a_density){

  type = "Pump";
  numberNode = 2;
  startNodeName = a_startNodeName;
  endNodeName = a_endNodeName;
  qCurve = a_qCurve;
  hCurve = a_hCurve;
  curveType = a_curveType;
  status = 1; // OPEN
  typeCode = 2;

  int numberPoint = qCurve.size();
  if(numberPoint != hCurve.size()){
    cout << endl << "!!!ERROR!!! Pump(" << name << ") characteristic curve: number of Q (" << numberPoint << ") is NOT equal to number of H (" << qCurve.size() << ") !" << endl << "Pump ID: " << a_name << endl << "Exiting..." << endl;
    exit(0);
  }

  if(curveType == "parabolic")
  {
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
  else
  {
    cout << endl << "!ERROR! curve type: " << curveType << " unkown at pump " << name << endl << "Exiting..." << endl;
    exit(-1);
  }

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
    strstrm << endl << qCurve[i] << " | " << hCurve[i];
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
double Pump::function(const VectorXd &ppq, VectorXd &fDer)// ppq = [Pstart, Pend, VolumeFlowRate]
{
  double out;
  if(status == 1) // OPEN
  {
    double dp = characteristicCurve(ppq(2));
    out = (ppq(1) - ppq(0)) - dp + (endHeight - startHeight);

    double dx;
    if(abs(ppq(2))>0.1)
      dx = ppq(2)*0.001;
    else
      dx = 0.01;

    double dp2 = characteristicCurve(ppq(2)+dx);
    double derivative = (dp-dp2)/dx;

    fDer(0) = -1.0;
    fDer(1) = 1.0;
    fDer(2) = derivative;
  }
  else // CLOSED, status is 0 or -1
  {
    out = ppq(2); 
    fDer(2) = 1.0;

    //alternative solution, somehow converges slowly
    //out = ppq(1) - ppq(0) + (endHeight-startHeight) + 1e15 * ppq(2) * abs(ppq(2));
    //fDer(2) = 2 * 1e15 * abs(ppq(2));
  }
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
    
  double H = 0.;
  if(curveType == "parabolic")
  {
    for(int i=0; i<coeffCurve.size(); i++)
      H += coeffCurve[i] * pow(revolutionNumber,2-i) * pow(Q,i);
  }
  else if (curveType == "linear")
  {
    H = linearInterpolation(qCurve, hCurve, Q);
  }

  return H;
}

//--------------------------------------------------------------
void Pump::checkPump()
{
  if(curveOrder >= 1)
  {
    if(volumeFlowRate < min(qCurve))
      cout << endl << "!WARNING! Volume flow rate (" << volumeFlowRate << ") is smaller than the minimum (" << min(qCurve) << ") at Pump: " << name << endl;
    if(volumeFlowRate > max(qCurve))
      cout << endl << "!WARNING! Volume flow rate (" << volumeFlowRate << ") is larger than the maximum (" << max(qCurve) << ") at Pump: " << name << endl;
  }
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
void Pump::setStringProperty(string prop, string value){
  if(prop == "curveType")
    curveType = value;
  else
  {  
    cout << endl << endl << "void Pump::setStringProperty( STRING ) wrong argument:" << prop;
    cout << ", right values: curveType " << endl << endl;
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
    cout << ", right values: qCurve | hCurve " << endl << endl;
  }
  return out;
}