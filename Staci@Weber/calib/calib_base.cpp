#include <iostream>
#include "../../SVDCalibration.h"
#include <fstream>
#include <random>
#include <iomanip>
#include <string>
#include <algorithm>

using namespace std;

SVDCalibration *wds;

int main(int argc, char* argv[]){

  // Name of containing folder of staci file
  string case_name = "lin_alap_9";
  cout << endl << "Case: " << case_name << endl;
  string sens_par_name = "friction_coeff";

  // Creating the case
  wds = new SVDCalibration(case_name + ".spr");
  wds->setDebugLevel(2);
  wds->buildSystem();
  wds->initialization();
  wds->listSystem();
  int numberEdges = wds->edges.size();

  cout << endl << "numberEdges: " << numberEdges << endl;
  
  // Setting the real friction and the estimations
  vector<double> fric_real, fric_est;
  cout << "\nfric_real:\n";
  for(int i=0; i<numberEdges; i++)
    if(wds->edges.at(i)->getType() == "Pipe"){
      fric_real.push_back(wds->edges.at(i)->getProperty("roughness"));
      fric_est.push_back(fric_real.at(i)*1.5);
      cout << "f at " << i << "\t" << fric_real.at(i) << endl;
    }

  // Setting the demand perturbation values and mode
  vector<double> d_noise; d_noise.push_back(1.0); d_noise.push_back(0.1);
  string d_noise_mode = "None";

  // Evaluating the "measurements"
  wds->generateMeasurement(fric_real,d_noise,d_noise_mode);

  // Printing the loaded measurement data to consol
  VectorXd d_meas_sum = wds->getMeasuredDemandSum();
  MatrixXd p_meas = wds->getMeasuredPressure();
  cout << "\nMeasured values:";
  printf("\n a_fogy | p ");
  for(int i=0; i<d_meas_sum.rows(); i++){
    printf("\n %6.3f | ",d_meas_sum(i));
    for(int j=0; j<p_meas.cols(); j++)
      printf(" %6.3f",p_meas(i,j));
  }

  // Perfroming the calibration
  double tol = 1.0;
  int konv = wds->Calibrate(fric_est, tol);

  // Printing the frictions after, before the calibration and the "real"
  cout << "\nfrics after Calibration: \n";
  printf(" # |    calib   |    real    |   estima   |\n");
  printf("---+------------+------------+------------|\n");
  for(int i=0; i<numberEdges; i++)
    if(wds->edges.at(i)->getType() == "Pipe")
      printf("%2i | %10.3f | %10.3f | %10.3f |\n", i, wds->edges.at(i)->getProperty("roughness"), fric_real.at(i),fric_est.at(i));

  // Calculating the norm of the differences
  double e_fric=0.;
  for(int i=0; i<fric_real.size(); i++)
    e_fric += pow(fric_real[i] - fric_est.at(i), 2.);
  e_fric = pow(e_fric/(numberEdges-1),.5);
  cout << "\n\n Fric difference at estimation: " << e_fric;

  e_fric=0.;
  for(int i=0; i<fric_real.size(); i++)
    e_fric += pow(wds->edges.at(i)->getProperty("roughness") - fric_real.at(i), 2.);
  e_fric = pow(e_fric/(numberEdges-1),.5);
  cout << "\n Fric difference after calib:   " << e_fric;

  // Printing whether Calibration converged
  printf("\n\n Konv: ");
  switch(konv) {
    case 0 : printf("NO :(\n"); break;
    case 1 : printf("FULL :)\n"); break;
    case 2 : printf("LOCAL :|\n"); break;
  }

  cout << endl;
  return 0;
}