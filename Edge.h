/*===================================================================*\
                                 Edge
                            ---------------                            

  Abstract class, because it contains pure virtual functions i.e.
  it cannot be instantiated. Multiple classes are derived (e.g.
  Pipe, Pool, PressurePoint, ... ).

  This calss involves general variables like startNodeName,
  endNodeName, volumeFlowRate etc. that every Edge requires.

  staci3 is using Eigen, see http://eigen.tuxfamily.org

    staci3
    Cs. Hos, R. Weber, T. Huzsvar
    https://github.com/weberrichard/staci3
\*==================================================================*/

#ifndef EDGE_H
#define EDGE_H

#include "Statistic.h"

#include "Eigen/Eigen/Eigen"

#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <string>
#include <ctime>
#include <stdlib.h>

using namespace std;

class Edge
{
public:
  Edge(const string a_name, const double a_referenceCrossSection, const double a_volumeFlowRate, const double a_densitiy);
  virtual ~Edge();

  /// Get/set double/int/string property in general for EDGE
  double getEdgeDoubleProperty(string prop);
  int getEdgeIntProperty(string prop);
  string getEdgeStringProperty(string prop);
  void setEdgeDoubleProperty(string prop, double value);
  void setEdgeIntProperty(string prop, int value);
  void setEdgeStringProperty(string prop, string value);

  virtual double getDoubleProperty(string prop) {cout << "\nERROR, prop: " << prop << endl; exit(0); return 0.; }
  virtual vector<double> getVectorProperty(string prop) {cout << "\nERROR, prop: " << prop << endl; exit(0); vector<double> x; return x; }
  virtual int getIntProperty(string prop) {cout << "\nERROR, prop: " << prop << endl; exit(0);  return 0; }
  virtual string getStringProperty(string prop) {cout << "\nERROR, prop: " << prop << endl; exit(0);  return ""; };
  virtual void setDoubleProperty(string prop, double value) {cout << "\nERROR, prop: " << prop << endl; exit(0); };
  virtual void setIntProperty(string prop, int value) {cout << "\nERROR, prop: " << prop << endl; exit(0); };
  virtual void setStringProperty(string prop, string value) {cout << "\nERROR, prop: " << prop << endl; exit(0); };

  /// A line of F(x) = equation, rearranged to 0 in l/s.
  virtual double function(vector<double> x) = 0;

  /// Jacobian: df/dhe, df/dhv, df/dmp
  virtual vector<double> functionDerivative(vector<double>) = 0;

  /// Initialization, mode: 0->automatic | 1-> using value
  virtual void initialization(int mode, double value) = 0;

  /// Printing basic information
  virtual string info();

  /// Get equation derivative w.r.t. parameter
  virtual double functionParameterDerivative(string prop) { return 0.0; }

  /// Checking the pump whether the operating point is still between the min and max of the characteristic curve
  virtual void checkPump(){}

  /// Setting the fricition model: DW (Darcy-Weisbach) | HW (Hazen-Williams)
  virtual void setFrictionModel(string friction_model){};

  /// If this equals to true, it counts as closed, thus no equation will be solved, like it is not part of the system
  //bool isClosed = false;
  int status; // used in Valve classes

  // Used only mainly in active Valves(PRV,FCV...)
  double setting;

  // Used in Pipe
  bool isCheckValve; // if the pipe contains check valve it is true, else false

  // [l/s] for series calculations
  vector<double> vectorVolumeFlowRate;

  double volumeFlowRate; // [l/s]
  double density; // [kg/m3]
  double referenceCrossSection; // [m2], used for calculating velocity
  double startHeight, endHeight; // [m] height of the connecting nodes
  const double gravity = 9.81; // [m/s2]
  int startNodeIndex, endNodeIndex;
  int numberNode;
  int segment=-1; // the edge takes place in which segment
  string startNodeName, endNodeName;
  string name;
  string type; // Pipe, pool, pump, etc.
};
#endif
