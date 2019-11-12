/*===================================================================*\
                                  Valve
                            ---------------

  Valve class derived from Edge class. Virtual class for every Valve
  type: TCV, PRV, PSV, ISO, FCV
 
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
  Valve(const string a_name, const string a_startNodeName, const string a_endNodeName, const double a_density, const double a_referenceCrossSection, const double a_massFlowRate);
  ~Valve();

  /// Provides basic informations
  virtual string info();
  
  /// A line of F(x) = equation, rearranged to 0 in w.c.m.
  virtual double function(const VectorXd &ppq, VectorXd &fDer)=0;
  
  /// Jacobian: df/dhe, df/dhv, df/dmp
  //virtual vector<double> functionDerivative(vector<double>)=0;

  /// Initialization, mode: 0->automatic | 1-> using value
  virtual void initialization(int mode, double value)=0;
};

#endif