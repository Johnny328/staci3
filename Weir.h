#include "Edge.h"

class Weir: public Edge
{
private:
    double bottomLevel;
    bool isOpen;
    double width;
    double overFlowHeight;
    double dischargeCoeff;
    double valveCoeff;

public:
    Weir(const string a_name, const string a_startNodeName, const string a_endNodeName, const double a_density, const double a_referenceCrossSection, const double a_bottomLevel, const bool a_isOpen, const double a_width, const double a_overFlowHeight, const double a_dischargeCoeff, const double a_valveCoeff, const double a_massFlowRate);
    ~Weir();
    string info();
    double function(vector<double>);
    vector<double> functionDerivative(vector<double>);
    void initialization(int mode, double value);
    string getType() {
        return "Weir";
    }
    void setProperty(string mit, double mire);
    double getProperty(string mit);
};
