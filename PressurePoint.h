/*===================================================================*\
                             PressurePoint
                            ---------------

  Derived from Edge class and can be instantiated. Simply pool
  element. Prescribes a certain pressure at a Node.
 
  staci3 is using Eigen, see http://eigen.tuxfamily.org

    staci3
    Cs. Hos, R. Weber, T. Huzsvar
    https://github.com/weberrichard/staci3
\*==================================================================*/

#ifndef PRESSUREPOINT_H
#define PRESSUREPOINT_H

#include "Edge.h"

class PressurePoint: public Edge
{
public:
    PressurePoint(const string a_name, const double a_referenceCrossSection, const string a_startNodeName, const double a_density, const double a_head, const double a_massFlowRate);
    ~PressurePoint();

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

//private:
    double head;
};

#endif