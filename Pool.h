#include "Edge.h"

class Pool:public Edge{
  private:
    double bottomLevel;
    double waterLevel;
    double head;/// (density*gravity*(bottomLevel+waterLevel))
  public:
    Pool(const string a_name, const string a_startNodeName, const double a_density, const double a_referenceCrossSection, const double a_bottomLevel, const double a_waterLevel, const double a_massFlowRate);
    ~Pool();
    string info();
    double function(vector<double>);
    vector<double> functionDerivative(vector<double>);
    void initialization(int mode, double value);
    void setProperty(string mit, double mire);
    string getType() {
        return "Pool";
    }
    double getProperty(string mit);
  };
