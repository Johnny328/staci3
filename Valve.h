#include "Edge.h"

class Valve : public Edge
{
private:
  vector<double> charX, charY; // X-Y coordinate of characteristic curve, that is position-dzeta
  double position, loss;
  //nt order;
  double headLoss;
public:
  Valve(const string a_name, const string a_startNodeName, const string a_endNodeName, const double a_density, const double a_referenceCrossSection, vector<double> a_charX, vector<double> a_charY, double a_position, const double a_massFlowRate);
  void updateLoss();
  ~Valve();
  string info();
  double function(vector<double>);
  vector<double> functionDerivative(vector<double>);
  void initialization(int mode, double value);
  void setProperty(string mit, double mire);
  double getProperty(string mit);
  string getType()
  {
    return "Valve";
  }
};

