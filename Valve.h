/*===================================================================*\
                                  Valve
                            ---------------

  Valve class derived from Edge class. Basic valve with loss curve.
 
  staci3 is using Eigen, see http://eigen.tuxfamily.org

    staci3
    Cs. Hos, R. Weber, T. Huzsvar
    https://github.com/weberrichard/staci3
\*==================================================================*/

#ifndef VALVE_H
#define VALVE_H

#include "Edge.h"

class Valve : public Edge
{
public:
  Valve(const string a_name, const string a_startNodeName, const string a_endNodeName, const double a_density, const double a_referenceCrossSection, vector<double> a_charX, vector<double> a_charY, double a_position, const double a_massFlowRate);
  ~Valve();

  /// Provides basic informations
  string info();
  
  /// A line of F(x) = equation, rearranged to 0 in w.c.m.
  double function(vector<double>);
  
  /// Jacobian: df/dhe, df/dhv, df/dmp
  vector<double> functionDerivative(vector<double>);

  /// Initialization, mode: 0->automatic | 1-> using value
  void initialization(int mode, double value);

  //========================
  //GETSETGETSETGETSETGETSET
  //========================
  void setDoubleProperty(string prop, double value);
  void setIntProperty(string property, int value);
  double getDoubleProperty(string prop);
  string getStringProperty(string prop);
  int getIntProperty(string property);

private:
  vector<double> charX, charY; // X-Y coordinate of characteristic curve, that is position-dzeta
  double startHeight, endHeight; // Height of the starting and ending nodes
  double position, loss;
  double headLoss;
  int startSegment, endSegment; // containing which segments are connected through this valve
  void updateLoss();
};

#endif