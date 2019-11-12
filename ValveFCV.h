/*===================================================================*\
                                  ValveFCV
                            ---------------

  ValveFCV class derived from Valve class. Flow Control Valve.
 
  staci3 is using Eigen, see http://eigen.tuxfamily.org

    staci3
    Cs. Hos, R. Weber, T. Huzsvar
    https://github.com/weberrichard/staci3
\*==================================================================*/

#ifndef VALVEFCV_H
#define VALVEFCV_H

#include "Valve.h"

class ValveFCV : public Valve
{
public:
  ValveFCV(const string a_name, const string a_startNodeName, const string a_endNodeName, const double a_density, const double a_referenceCrossSection, const double a_setting, const double a_volumeFlowRate);
  ~ValveFCV();

  /// Provides basic informations
  string info();
  
  /// A line of F(x) = equation, rearranged to 0 in w.c.m.
  double function(const VectorXd &ppq, VectorXd &fDer);
  
  /// Jacobian: df/dhe, df/dhv, df/dmp
  //vector<double> functionDerivative(vector<double>);

  /// Initialization, mode: 0->automatic | 1-> using value
  void initialization(int mode, double value);

  // Prescribed maximum volume flow rate
  //double setting;

  //========================
  //GETSETGETSETGETSETGETSET
  //========================
  void setDoubleProperty(string prop, double value);
  double getDoubleProperty(string prop);
  string getStringProperty(string prop);
};

#endif