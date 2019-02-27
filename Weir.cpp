#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <cmath>
#include <ctime>
#include "Weir.h"

Weir::Weir(const string a_name, const string a_startNodeName, const string a_endNodeName, const double a_density, const double a_referenceCrossSection, const double a_bottomLevel, const bool a_isOpen, const double a_width, const double a_overFlowHeight, const double a_dischargeCoeff, const double a_valveCoeff, const double a_massFlowRate) :
  Edge(a_name, a_referenceCrossSection, a_massFlowRate, a_density)
{
  type = "Weir";
  numberNode = 2;
  startNodeName = a_startNodeName;
  endNodeName = a_endNodeName;
  isOpen = a_isOpen;
  bottomLevel = a_bottomLevel;
  width = a_width;
  overFlowHeight = a_overFlowHeight;
  dischargeCoeff = a_dischargeCoeff;
  valveCoeff = a_valveCoeff;
}

//--------------------------------------------------------------
Weir::~Weir()
{
}

//--------------------------------------------------------------
string Weir::info()
{
  ostringstream strstrm;
  strstrm << Edge::info();
  strstrm << endl << "         type : " << type;
  strstrm << endl << "   connection : " << startNodeName << "(index:" << startNodeIndex
          << ") ==> " << endNodeName << "(index:" << endNodeIndex << ")";
  if (isOpen)
    strstrm << " igen";
  else
    strstrm << " nem";
  strstrm << "\n                bottom level            : " << bottomLevel;
  strstrm << "\n                width                   : " << width;
  strstrm << "\n                over flow height        : " << overFlowHeight;
  strstrm << "\n                discharge coeffitient   : " << dischargeCoeff;
  strstrm << "\n                valve coefficient       : " << valveCoeff << endl;
  return strstrm.str();
}

//--------------------------------------------------------------
double Weir::function(vector<double> x)
{
  double result;
  double pe = x[0] * density * gravity;
  double pv = x[1] * density * gravity;
  double he = x[2];
  double hv = x[3];

  if (isOpen)
  {
    result = (pv - pe) / density / gravity + (hv - he) + valveCoeff * massFlowRate * fabs(massFlowRate);
  }
  else
  {
    double hve = pe / density / gravity + he - bottomLevel;
    double hvu = pv / density / gravity + hv - bottomLevel;
    double Q = massFlowRate / density;
    double ve = Q / width / hve;
    double vu = Q / width / hvu;

    if (hve > overFlowHeight)
    {
      // normal flow WITHOUT over flow
      if (hvu > overFlowHeight)
      {
        result = (hvu + vu * vu / 2 / gravity) - (hve + ve * ve / 2 / gravity);
      }
      else
      {
        // normal flow WITH over flow
        double H = hve - overFlowHeight;
        result = massFlowRate / density - width * H * dischargeCoeff * sqrt(2 * gravity * H);
      }
    }
    else
    {
      // backflow WITH over flow
      if (hvu > overFlowHeight)
      {
        double H = hvu - overFlowHeight;
        result = -massFlowRate / density - width * H * dischargeCoeff * sqrt(2 * gravity * H);
      }
      // both side is under over flow level i.e. "beszoptuk, mindket oldalon a bukasi szint alatt vagyunk" by HCs
      else
      {
        result = massFlowRate / density;
      }
    }
  }

  return result;
}

//--------------------------------------------------------------
vector<double> Weir::functionDerivative(vector<double> x)
{
  vector<double> result, temp = x;
  double f1 = function(x), f2, dtemp;

  // df/dhe
  x.at(0) *= 1.01;
  f2 = function(x);
  result.push_back((f2 - f1) / 0.01 / x.at(0));

  // df/dhv
  x = temp;
  x.at(1) *= 1.01;
  f2 = function(x);
  result.push_back((f2 - f1) / 0.01 / x.at(1));

  // df/dmp
  dtemp = massFlowRate;
  massFlowRate *= 1.01;
  f2 = function(x);
  result.push_back((f2 - f1) / 0.01 / massFlowRate);
  massFlowRate = dtemp;

  // Konstans tag:
  result.push_back(0.0);

  double pe = x[0] * density * gravity;
  double pv = x[1] * density * gravity;
  double he = x[2];
  double hv = x[3];
  double hve = pe / density / gravity + he - bottomLevel;
  double hvu = pv / density / gravity + hv - bottomLevel;

  if ((hve > overFlowHeight) && (hvu < overFlowHeight))
    result.at(1) = 0.0;
  if ((hvu > overFlowHeight) && (hve < overFlowHeight))
    result.at(0) = 0.0;

  return result;
}

//--------------------------------------------------------------
void Weir::initialization(int mode, double value)
{
  if (mode == 0)
    massFlowRate = 1;
  else
    massFlowRate = value;
}

//--------------------------------------------------------------
void Weir::setProperty(string property, double mire) {
//    if (property=="diameter")
//      D=mire;
//    else
//      {
  cout << endl << "ERROR! Weir::setProperty(property), unknown input: property="
       << property << endl << endl;
//      }
}

double Weir::getProperty(string property) {
  double out = 0.0;
  // if (property == "Aref")
  //   out = Aref;
  // else if (property == "lambda")
  //   out = lambda;
  // else {
  cout << endl << "ERROR! Cso::getProperty(property), unknown input: property=" << property << endl << endl;
  cout << endl << "Name of Weir: " << name << endl;
  cin.get();
  // }
  return out;
}