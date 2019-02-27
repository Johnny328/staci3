#include "Edge.h"

class PressurePoint: public Edge
{
private:
    double head;
public:
    PressurePoint(const string a_name, const double a_referenceCrossSection, const string a_startNodeName, const double a_density, const double a_head, const double a_massFlowRate);
    ~PressurePoint();
    string info();
    double function(vector<double>);
    vector<double> functionDerivative(vector<double>);
    void initialization(int mode, double value);
    void setProperty(string mit, double mire);
    string getType()
    {
        return "PressurePoint";
    }
    double getProperty(string mit);
    void setHead(double a)
    {
        head = a;
    }
};
