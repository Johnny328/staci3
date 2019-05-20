/*===================================================================*\
                                  Pipe
                            ---------------

  Derived from Edge class and can be instantiated. Simply pipeline
  element. Equation is a Bernoulli equation with friction losses
  and geodetic height.
 
  staci3 is using Eigen, see http://eigen.tuxfamily.org

    staci3
    Cs. Hos, R. Weber, T. Huzsvar
    https://github.com/weberrichard/staci3
\*==================================================================*/

#ifndef PIPE_H
#define PIPE_H

#include "Edge.h"

class Pipe : public Edge
{

public:
    Pipe(const string a_name, const string a_startNodeName, const string a_endNodeName, const double a_density, const double a_length, const double a_diameter, const double a_rougness, const double a_massFlowRate);
    ~Pipe();

    /// Provides basic informations
    string info();

    /// A line of F(x) = equation, rearranged to 0 in w.c.m.
    double function(vector<double>);

    /// Function ferivative w.r.t. variable (head / mass flow rate)
    vector<double> functionDerivative(vector<double>);

    /// Function ferivative w.r.t. parameter (diameter / roughness)
    double functionParameterDerivative(string parameter);

    /// Initialization, mode: 0->automatic | 1-> using value
    void initialization(int mode, double value);
    
    //========================
    //GETSETGETSETGETSETGETSET
    //========================
    double getDoubleProperty(string prop);
    int getIntProperty(string prop);
    void setDoubleProperty(string prop, double value);
    void setIntProperty(string prop, int value);
    
    void setFrictionModel(string friction_model);

private:
    double length, diameter, lambda;
    double roughness; // Pipefal roughness (HW: C factor, DW: relative roughness)
    double startHeight, endHeight; // Height of the starting and ending nodes
    int frictionModel; // 0 - Darcy-Weisbach (DW), 1 - Hazen-Williams (HW)

    //! Computes the head loss in Pa
    /*! dp'=lambda*length/diameter*density/2*velocity*fabs(velocity)*/
    double computeHeadloss(double mp);

    //! Computes the head loss derivative w.r.t. mass flow rate
    /*! dp'=lambda*length/diameter*density/2*velocity*fabs(velocity)
    diameter dp'/dmp=lambda*length/diameter*density/2*1/(density*A)^2*abs(velocity)*/
    double computeHeadlossDerivative(double mp);

    /// Calculating lambda value from roughness based on friction model (DW / HW)
    double getLambda(double mp);

};

#endif