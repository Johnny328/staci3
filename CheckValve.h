#include "Edge.h"

class CheckValve : public Edge {
private:
	double normalLossCoeff, backflowLossCoeff; // loss coeffitients
public:
	CheckValve(const string a_name, const string a_startNodeName,const string a_endNodeName, const double a_density, const double a_referenceCrossSection, const double a_normalLossCoeff,const double a_backflowLossCoeff, const double a_massFlowRate);
	~CheckValve();
	string info();
	double function(vector<double>);
	vector<double> functionDerivative(vector<double>);
	void initialization(int mode, double value);
	void setProperty(string mit, double mire);
	string getType() {
		return "CheckValve";
	}
	double getProperty(string mit);
};
