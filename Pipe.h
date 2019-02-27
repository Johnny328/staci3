#include "Edge.h"

class Pipe : public Edge
{
private:
    double length, diameter, lambda;
    double roughness; // Pipefal roughness (HW: C factor, DW: relative roughness)
    int frictionModelType; // 0 - Darcy-Weisbach (DW), 1 - Hazen-Williams (HW)
public:
    Pipe(const string a_name, const string a_startNodeName, const string a_endNodeName, const double a_density, const double a_length, const double a_diameter, const double a_rougness, const double a_massFlowRate);
    ~Pipe();
    string info();
    double function(vector<double>);
    vector<double> functionDerivative(vector<double>);
    void initialization(int mode, double value);
    double getLambda();
    double getProperty(string mit);
    double getFunctionDerivative(string mit);
    void setProperty(string mit, double mire);
    string getType()
    {
        return "Pipe";
    }    
    double computeHeadloss();
    double computeHeadlossDerivative();
    void setFrictionModel(string friction_model);
};
