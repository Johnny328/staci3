#include "Sensitivity.h"

#include <stdio.h>
#include <fstream>
#include <string>

using namespace std;

class Calibration : public Sensitivity
{
public:
	Calibration(string spr_filename);
	~Calibration();

	// Creating virtual measurent data for testing calibration algorithms
	// IMPORTANT: demands must be at nominal values!!!
  void generateMeasurement(const vector<double> &fric, vector<double> demandNoiseValue, string demandNoiseMode);
  void loadMeasurement(string spr_filename);
  void loadDemandSum(string spr_filename);
  void loadPressureID(string spr_filename);
  void loadDemand(string spr_filename);

  //========================
  //GETSETGETSETGETSETGETSET
  //========================
  MatrixXd getMeasuredPressure(){
  	return measuredPressure;
  }
  MatrixXd getEveryPressure(){
  	return everyPressure;
  }
  VectorXd getMeasuredDemandSum(){
    return measuredDemandSum;
  }
  void setMeasuredPressureIndex(vector<string> a_p_meas_ids){
  	measuredPressureID = a_p_meas_ids;
    measuredPressureIndex = ID2Index(measuredPressureID);
  	numberPressure = measuredPressureID.size();
  }
  void setMeasuredDemandIndex(vector<string> a_d_meas_ids){
  	measuredDemandID = a_d_meas_ids;
    measuredDemandIndex = ID2Index(measuredDemandID);
  	numberDemand = measuredDemandID.size();
  }
  void setMeasuredDemandSum(VectorXd a_d_meas_sum){
  	measuredDemandSum = a_d_meas_sum;
  	numberOperationalPoint = measuredDemandSum.size();
  }

protected:
	MatrixXd measuredPressure; // Pressure values at actual measure nodes
	MatrixXd everyPressure; // Every pressure value
	vector<string> measuredPressureID; // Node IDs for pressure
	vector<int> measuredPressureIndex; // Node indicies for pressure

	VectorXd measuredDemandSum; // Overall demands
	MatrixXd measuredDemand; // Demand measurement values (can be empty)
	MatrixXd everyDemand; // Every demand measurement value (can be empty)
	vector<string> measuredDemandID; // Node IDs for demands (can be empty)
	vector<int> measuredDemandIndex; // Node indicies for demands (can be empty)

	int numberPressure;  // Number of measured pressure nodes
	int numberDemand;  // Number of measured demand nodes
	int numberOperationalPoint; // Number of measured operational points

	vector<double> demandNoiseValue;
	string demandNoiseMode;
  string measurementFolder;
};