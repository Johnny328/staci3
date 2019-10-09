/*===================================================================*\
                                  Pool
                            ---------------

  Derived from Edge class and can be instantiated. Simply pool
  element. Prescribes a certain pressure at a Node.
 
  staci3 is using Eigen, see http://eigen.tuxfamily.org

    staci3
    Cs. Hos, R. Weber, T. Huzsvar
    https://github.com/weberrichard/staci3
\*==================================================================*/

#ifndef POOl_H
#define POOl_H

#include "Edge.h"

class Pool:public Edge{

//private:
  double bottomLevel;
  double waterLevel;
  double minLevel;
  double maxLevel;

public:
  Pool(const string a_name, const string a_startNodeName, const double a_density, const double a_referenceCrossSection, const double a_bottomLevel, const double a_waterLevel, const double a_massFlowRate);
  ~Pool();

  /// Provides basic informations
  string info();

  /// A line of F(x) = equation, rearranged to 0 in w.c.m.
  double function(vector<double> x);

  /// Jacobian: df/dhe, df/dhv, df/dmp
  vector<double> functionDerivative(vector<double>);

  /// Initialization, mode: 0->automatic | 1-> using value
  void initialization(int mode, double value);
  
  //========================
  //GETSETGETSETGETSETGETSET
  //========================
  double getDoubleProperty(string prop);
  void setDoubleProperty(string prop, double value);
};

#endif