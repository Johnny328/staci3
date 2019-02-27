#include <cmath>
#include <vector>
#include <iostream>
#include <sstream>
#include "Edge.h"
#include "PressurePoint.h"

PressurePoint::PressurePoint(const string a_name, const double a_referenceCrossSection, const string a_startNodeName, const double a_density, const double a_head, const double a_massFlowRate) :

    Edge(a_name, a_referenceCrossSection, a_massFlowRate, a_density) {
    type = "PressurePoint";
    numberNode = 1;

    endNodeName = "<nincs>";
    startNodeName = a_startNodeName;
    head = a_head;
}

//--------------------------------------------------------------
PressurePoint::~PressurePoint() {
}

//--------------------------------------------------------------
string PressurePoint::info() {
    ostringstream strstrm;
    strstrm << Edge::info();
    strstrm << endl << "  kapcsolodas : " << startNodeName << "(index:" << startNodeIndex << ")\n";
    return strstrm.str();
}

//--------------------------------------------------------------
double PressurePoint::function(vector<double> x) {
    double result = x[0] - head / density / gravity;
    return result;
}

//--------------------------------------------------------------
vector<double> PressurePoint::functionDerivative(vector<double> x) {
    vector<double> result;
    result.push_back(1);
    result.push_back(0);
    result.push_back(0);
    result.push_back(-head / density / gravity);

    return result;
}

//--------------------------------------------------------------
void PressurePoint::initialization(int mode, double value) {
    if (mode == 0)
        massFlowRate = 0.01;
    else
        massFlowRate = value;
}

//--------------------------------------------------------------
void PressurePoint::setProperty(string property, double value) {
    if(property == "head")
        setHead(value);
    else
        cout << endl << "ERROR! PressurePoint::setProperty(property), unkown input: property=" << property << endl << endl;
}

double PressurePoint::getProperty(string property) {
    double out = 0.0;
    if (property == "pres")
        out = head;
    else {
        cout << endl << "ERROR! Cso::getProperty(property), unkown input: property=" << property << endl << endl;
        cout << endl << "Name of PressurePoint: " << name << endl;
    }
    return out;
}