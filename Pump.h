#include "Edge.h"
#include <iomanip>
#include <string>
#include <vector>
#include "Edge.h"
#include <math.h>
#include <iostream>

class Pump: public Edge {
private:
	vector<double> q, H, p;
	//double a, b, c;
	int order;
	double mer_szorzo;
public:
	Pump(const string nev, const string a_cspe_nev,
			const string a_cspv_nev, const double a_ro, const double Aref, const vector<double> q,
			const vector<double> H, const double a_mp);
	~Pump();
	string info();
	double function(vector<double>);
	vector<double> functionDerivative(vector<double>);
	void initialization(int mode, double value);
	void setProperty(string mit, double mire);
	double getProperty(string mit);
	string getType() {
		return "Pump";
	}
	double pumpCharCurve(double q);
	double getPumpHeadAt(double q) {
		return pumpCharCurve(q);
	}
};
