#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <math.h>
#include "Edge.h"
#include "Pool.h"

Pool::Pool(const string a_name, const string a_startNodeName, const double a_density, const double a_referenceCrossSection, const double a_bottomLevel, const double a_waterLevel, const double a_massFlowRate) :
    Edge(a_name, a_referenceCrossSection, a_massFlowRate, a_density) {
    type = "Pool";
    numberNode = 1;
    startNodeName = a_startNodeName;
    endNodeName = "<nincs>";
    bottomLevel = a_bottomLevel;
    waterLevel = a_waterLevel;
    head = (bottomLevel + waterLevel) * density * gravity;
}

//--------------------------------------------------------------
Pool::~Pool() {
}

//--------------------------------------------------------------
string Pool::info() {
    ostringstream strstrm;
    strstrm << Edge::info();
    strstrm << "\n               type : " << type;
    strstrm << "\n         connection : " << startNodeName << "(index:" << startNodeIndex << ")\n";
    strstrm << "     bottom level [m] : " << bottomLevel << endl;
    strstrm << "      water level [m] : " << waterLevel << endl;
    return strstrm.str();
}

//--------------------------------------------------------------
double Pool::function(vector<double> x) { //TODO L,D,A = ????
    double L = 1.;
    double D = 0.5;
    double A = D * D * M_PI / 4.;
    double c = 0.02 * L / D / 2. / gravity / density / density / A / A;
    double result = x[0] + x[2] + 0. * c * massFlowRate * fabs(massFlowRate) - (bottomLevel + waterLevel);

    return result;
}

//--------------------------------------------------------------
vector<double> Pool::functionDerivative(vector<double> x) {
    vector<double> result;
    result.push_back(1.0);
    result.push_back(0.0);
    double L = 1;
    double D = 0.5;
    double A = D * D * M_PI / 4;
    double c = 0.02 * L / D / 2. / gravity / density / density / A / A;
    result.push_back(0 * 2 * c * fabs(massFlowRate));
    //result.push_back(-head / density / gravity);
    result.push_back(-(bottomLevel + waterLevel));

    return result;
}

//--------------------------------------------------------------
void Pool::initialization(int mode, double value) {
    if (mode == 0)
        massFlowRate = 1;
    else
        massFlowRate = value;
}

//--------------------------------------------------------------
void Pool::setProperty(string mit, double mire) {

    if (mit == "bottLevel")
        bottomLevel = mire;
    else if (mit == "waterLevel")
        waterLevel = mire;
    else
        cout << endl
             << "HIBA! Pool::setProperty(mit), ismeretlen bemenet: mit="
             << mit << endl << endl;
}

//--------------------------------------------------------------
double Pool::getProperty(string mit) {

    double out = 0.0;
    if (mit == "bottLevel")
        out = bottomLevel;
    else if (mit == "massFlowRate")
        out = massFlowRate;
    else if (mit == "waterLevel")
        out = waterLevel;
    else if (mit == "headLoss")
        out = 0.0;
    else if (mit == "referenceCrossSection")
        out = referenceCrossSection;
    else {
        cout << endl
             << "HIBA! Pool::getProperty(mit), ismeretlen bemenet: mit="
             << mit << endl << endl;
        out = 0.0;
    }
    return out;
}


