#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <cmath>
#include "Edge.h"
#include "CheckValve.h"

CheckValve::CheckValve(const string a_name, const string a_startNodeName,const string a_endNodeName, const double a_density, const double a_referenceCrossSection, const double a_normalLossCoeff,const double a_backflowLossCoeff, const double a_massFlowRate) :
    Edge(a_name, a_referenceCrossSection, a_massFlowRate, a_density)
{
    type = "CheckValve";
    numberNode = 2;
    startNodeName = a_startNodeName;
    endNodeName = a_endNodeName;
    normalLossCoeff = a_normalLossCoeff;
    backflowLossCoeff = a_backflowLossCoeff;
}

//--------------------------------------------------------------
CheckValve::~CheckValve()
{
}

//--------------------------------------------------------------
string CheckValve::info()
{
    ostringstream strstrm;
    strstrm << Edge::info();
    strstrm << "\n      type  : " << type;
    strstrm << "\nconnection  : " << startNodeName << " (index: " << startNodeIndex << "  " << endNodeName << "(index:" << endNodeIndex << ")\n";
    strstrm << "\n\t normal loss coefficient   : " << normalLossCoeff;
    strstrm << "\n\t backflow loss coefficient : " << backflowLossCoeff << "\n";
    return strstrm.str();
}

//--------------------------------------------------------------
double CheckValve::function(vector<double> x)
{
    double result, lossCoeff;
    double startPressure = x[0] * density * gravity;
    double endPressure = x[1] * density * gravity;
    double startGeodeticHeight = x[2];
    double endGeodeticHeight = x[3];

    if (massFlowRate > 0)
        lossCoeff = normalLossCoeff;
    else
        lossCoeff = backflowLossCoeff;

    result = (endPressure - startPressure) / density / gravity + (endGeodeticHeight - startGeodeticHeight) + lossCoeff * massFlowRate * fabs(massFlowRate);

    return result;
}

//--------------------------------------------------------------
vector<double> CheckValve::functionDerivative(vector<double> x)
{
    vector<double> result;
    if (massFlowRate > 0)
    {
        result.push_back(-1.0);
        result.push_back(1.0);
        result.push_back(normalLossCoeff * 2 * massFlowRate);
        result.push_back(0);
    }
    else
    {
        result.push_back(-1.0);
        result.push_back(1.0);
        result.push_back(-backflowLossCoeff * 2 * massFlowRate);
        result.push_back(0);
    }

    return result;
}

//--------------------------------------------------------------
void CheckValve::initialization(int mode, double value)
{
    if (mode == 0)
        massFlowRate = 1;
    else
        massFlowRate = value;
}

//--------------------------------------------------------------
void CheckValve::setProperty(string property, double value)
{
    cout << endl << "HIBA! CheckValve::setProperty(property), ismeretlen bemenet: property=" << property << endl << endl;
}

double CheckValve::getProperty(string property) {
    double out = 0.0;
    cout << endl << "HIBA! Cso::getProperty(property), ismeretlen bemenet: property=" << property << endl << endl;
    cout << endl << "Name of CheckValve: " << name << endl;
    cin.get();
    return out;
}