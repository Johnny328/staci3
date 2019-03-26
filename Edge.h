/*===================================================================*\
                                 Edge
                            ---------------                            

  Abstract class, because it contains pure virtual functions i.e.
  it cannot be instantiated. Multiple classes are derived (e.g.
  Pipe, Pool, PressurePoint, ... ).

  This calss involves general variables like startNodeName,
  endNodeName, massFlowRate etc. that every Edge requires.

  staci3 is using Eigen, see http://eigen.tuxfamily.org

    staci3
    Cs. Hos, R. Weber, T. Huzsvar
    https://github.com/weberrichard/staci3
\*==================================================================*/

#ifndef EDGE_H
#define EDGE_H

#include <iostream>
#include <vector>

using namespace std;

class Edge
{
public:
  Edge(const string a_name, const double a_referenceCrossSection, const double a_massFlowRate, const double a_densitiy);
  virtual ~Edge();

  /// Get/set double/int/string property in general for EDGE
  double getEdgeDoubleProperty(string prop);
  int getEdgeIntProperty(string prop);
  string getEdgeStringProperty(string prop);
  void setEdgeDoubleProperty(string prop, double value);
  void setEdgeIntProperty(string prop, int value);
  void setEdgeStringProperty(string prop, string value);

  virtual double getDoubleProperty(string prop) {cout << "\nERROR, prop: " << prop << endl; exit(0); return 0.; }
  virtual int getIntProperty(string prop) {cout << "\nERROR, prop: " << prop << endl; exit(0);  return 0; }
  virtual string getStringProperty(string prop) {cout << "\nERROR, prop: " << prop << endl; exit(0);  return ""; };
  virtual void setDoubleProperty(string prop, double value) {cout << "\nERROR, prop: " << prop << endl; exit(0); };
  virtual void setIntProperty(string prop, int value) {cout << "\nERROR, prop: " << prop << endl; exit(0); };
  virtual void setStringProperty(string prop, string value) {cout << "\nERROR, prop: " << prop << endl; exit(0); };

  /// A line of F(x) = equation, rearranged to 0 in w.c.m.
  virtual double function(vector<double>) = 0;

  /// Jacobian: df/dhe, df/dhv, df/dmp
  virtual vector<double> functionDerivative(vector<double>) = 0;

  /// Initialization, mode: 0->automatic | 1-> using value
  virtual void initialization(int mode, double value) = 0;

  /// Printing basic information
  virtual string info();

  /// Get equation derivative w.r.t. parameter
  virtual double getFunctionDerivative(string prop) { return 0.0; }

  /// Setting the fricition model: DW (Darcy-Weisbach) | HW (Hazen-Williams)
  virtual void setFrictionModel(string friction_model) {};

protected:
  double massFlowRate; // [kg/s]
  double density; // [kg/m3]
  double referenceCrossSection; // [m2], used for calculating velocity
  double user1, user2;
  const double gravity = 9.81; // [m/s2]
  int startNodeIndex, endNodeIndex;
  int numberNode;
  string startNodeName, endNodeName;
  string name;
  string type; // Pipe, pool, pump, etc.
};
#endif
