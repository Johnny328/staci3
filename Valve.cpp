#include "Valve.h"

Valve::Valve(const string a_name, const string a_startNodeName, const string a_endNodeName, const double a_density, const double a_referenceCrossSection, const double a_volumeFlowRate) : Edge(a_name, a_referenceCrossSection, a_volumeFlowRate, a_density) {
  numberNode = 2;
  startNodeName = a_startNodeName;
  endNodeName = a_endNodeName;
}

//--------------------------------------------------------------
Valve::~Valve(){}

//--------------------------------------------------------------
string Valve::info(){
  ostringstream ss;
  ss << Edge::info();
  ss << "\n connection            : " << startNodeName << " (index:" << startNodeIndex << ") --> " << endNodeName << " (index:" << endNodeIndex << ")";

  return ss.str();
}