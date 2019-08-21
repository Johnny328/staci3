/*===================================================================*\
                              Calibration
                            ---------------

  Main class for calibration techniques, derived from Sensitivity.
  Contains the loading of measurement data and generating artificial
  measurment data for debugging and comparison.
 
  staci3 is using Eigen, see http://eigen.tuxfamily.org

    staci3
    Cs. Hos, R. Weber, T. Huzsvar
    https://github.com/weberrichard/staci3
\*==================================================================*/

#ifndef CALIBRATION_H
#define CALIBRATION_H

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

  /*! WR: creating virtual measurement data from pressure measured node IDs and sum of demands
  *  fric:              friction values for each pipeline
  *  demandNoiseValue:  *    | mean and stdev |  min and max
  *  demandNoiseMode:   None |     Normal     |   Uniform 
	*  IMPORTANT: demands must be at nominal values
  *  measuredPressureID, measuredDemandSum, (measuredDemand*) ==>> measuredPressure, everyPressure*/
  void generateMeasurement(const vector<double> &fric, vector<double> demandNoiseValue, string demandNoiseMode);

  /*! WR: creating virtual measurement data from pressure measured node IDs and sum of demands
  *  fric:              friction values for each pipeline
  *  demandNoiseValue:  *    | mean and stdev |  min and max
  *  demandNoiseMode:   None |     Normal     |   Uniform 
  *  IMPORTANT: demands must be at nominal values*/
  void loadMeasurement(string spr_filename, bool isPressure);

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

protected:
	MatrixXd measuredPressure; // Pressure values at actual measure nodes
	MatrixXd everyPressure; // Every pressure value
	vector<int> measuredPressureIndex; // Node indicies for pressure

	VectorXd measuredDemandSum; // Overall demands
	MatrixXd measuredDemand; // Demand measurement values (can be empty)
	MatrixXd everyDemand; // Every demand measurement value (can be empty)
	vector<int> measuredDemandIndex; // Node indicies for demands (can be empty)

	int numberPressure;  // Number of measured pressure nodes
	int numberDemand;  // Number of measured demand nodes
	int numberOperationalPoint; // Number of measured operational points

private:
  vector<string> measuredPressureID; // Node IDs for pressure
  vector<string> measuredDemandID; // Node IDs for demands (can be empty)

	vector<double> demandNoiseValue;
	string demandNoiseMode;
  string measurementFolder;
};

#endif
