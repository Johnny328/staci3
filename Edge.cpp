#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <cmath>
#include <ctime>
#include <stdlib.h>
#include "Edge.h"

Edge::Edge(const string a_name, const double a_referenceCrossSection, const double a_massFlowRate, const double a_density){
  massFlowRate = a_massFlowRate;
  density = a_density;
  name = a_name;
  referenceCrossSection = a_referenceCrossSection;
  startNodeIndex = -1;
  endNodeIndex = -1;
  string startNodeName = "startNothingYet";
  endNodeName = "endNothingYet";

  if(abs(a_density) < 1.0e-3){
    ostringstream strstrm;
    strstrm.str("");
    strstrm << "\n\n******** ERROR *********";
    strstrm << "\n\telement name: " << name;
    strstrm << "\n\tmethod      : " << "Edge constructor";
    strstrm << "\n\tmessage     : " << "Density is to low." << "\n\n";
    cout << strstrm.str();
  exit(0);
  }

  user1=0.;
  user2=0.;
}

//--------------------------------------------------------------
Edge::~Edge(){}

//--------------------------------------------------------------
string Edge::info(){
  ostringstream strstrm;
  strstrm << "\n Edge name             : " << name;
  strstrm << "\n type                  : " << type;
  strstrm << "\n density               : " << density << " [kg/m^3]";
  strstrm << "\n referenceCrossSection : " << referenceCrossSection << " [m^2]";
  strstrm << "\n mass flow rate        : " << massFlowRate / density * 3600 << " [m3/h]";
  return strstrm.str();
}

//--------------------------------------------------------------
double Edge::getEdgeDoubleProperty(string prop){
  double out = 0.;
  if(prop == "massFlowRate" || prop == "mass_flow_rate")
    out = massFlowRate;
  else if(prop == "volumeFlowRate" || prop == "volume_flow_rate")
    out = massFlowRate / density;
  else if(prop == "velicoty")
    out = massFlowRate / density / referenceCrossSection;
  else if(prop == "density")
    out = density;
  else if(prop == "referenceCrossSection" || prop == "reference_cross_section" || prop == "Aref")
    out = referenceCrossSection;
  else if(prop == "user1")
    out = user1;
  else if(prop == "user2")
    out = user2;
  else
  {
    cout << endl << endl << "DOUBLE Edge::getEdgeDoubleProperty() wrong argument:" << prop;
    cout << ", right values: massFlowRate | velocity | density | referenceCrossSection | user1 | user2" << endl << endl;
  }
  return out;
}

//--------------------------------------------------------------
int Edge::getEdgeIntProperty(string prop){
  int out = 0;
  if(prop == "startNodeIndex" || prop == "start_node_index")
    out = startNodeIndex;
  else if(prop == "endNodeIndex" || prop == "end_node_index")
    out = endNodeIndex;
  else if(prop == "numberNode")
    out = numberNode;
  else
  {
    cout << endl << endl << "INT Edge::getEdgeIntProperty() wrong argument:" << prop;
    cout << ", right values: startNodeIndex | endNodeIndex | numberNode" << endl << endl;
  }
  return out;
}

//--------------------------------------------------------------
string Edge::getEdgeStringProperty(string prop){
  string out = "";
  if(prop == "startNodeName" || prop == "start_node_name")
    out = startNodeName;
  else if(prop == "endNodeName" || prop == "end_node_name")
    out = endNodeName;
  else if(prop == "name")
    out = name;
  else if(prop == "type")
    out = type;
  else
  {
    cout << endl << endl << "STRING Edge::getEdgeStringProperty() wrong argument:" << prop;
    cout << ", right values: startNodeName | endNodeName | name | type" << endl << endl;
  }
  return out;
}

//--------------------------------------------------------------
void Edge::setEdgeDoubleProperty(string prop, double value){
  if(prop == "massFlowRate" || prop == "mass_flow_rate")
    massFlowRate = value;
  else if(prop == "density")
    density = value;
  else if(prop == "referenceCrossSection" || prop == "reference_cross_section")
    referenceCrossSection = value;
  else if(prop == "user1")
    user1 = value;
  else if(prop == "user2")
    user2 = value;
  else
  {  
    cout << endl << endl << "Edge::setEdgeProperty( DOUBLE ) wrong argument:" << prop;
    cout << ", right values: massFlowRate | density | referenceCrossSection | user1 | user2" << endl << endl;
  }
}

//--------------------------------------------------------------
void Edge::setEdgeIntProperty(string prop, int value){
  if(prop == "startNodeIndex" || prop == "start_node_index")
    startNodeIndex = value;
  else if(prop == "endNodeIndex" || prop == "end_node_index")
    endNodeIndex = value;
  else if(prop == "numberNode")
    numberNode = value;
  else
  {  
    cout << endl << endl << "Edge::setEdgeProperty( DOUBLE ) wrong argument:" << prop;
    cout << ", right values: startNodeIndex | endNodeIndex | numberNode" << endl << endl;
  }
}

//--------------------------------------------------------------
void Edge::setEdgeStringProperty(string prop, string value){
  if(prop == "startNodeName" || prop == "start_node_name")
    startNodeName = value;
  else if(prop == "endNodeName" || prop == "end_node_name")
    endNodeName = value;
  else if(prop == "name")
    name = value;
  else if(prop == "type")
    type = value;
  else
  {  
    cout << endl << endl << "Edge::setEdgeProperty( DOUBLE ) wrong argument:" << prop;
    cout << ", right values: startNodeName | endNodeName | name | type" << endl << endl;
  }
}
