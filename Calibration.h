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
  void GenMeas(const vector<double> &fric, vector<double> d_noise_value, string d_noise_mode);
  void LoadMeas(string spr_filename);
  void LoadDemSum(string spr_filename);
  void LoadPresIDs(string spr_filename);
  void LoadDem(string spr_filename);


  //========================
  //GETSETGETSETGETSETGETSET
  //========================
  vector<vector<double> > Get_p_meas(){
  	return p_meas;
  }
  vector<vector<double> > Get_p_meas_all(){
  	return p_meas_all;
  }
  vector<double> Get_d_meas_sum(){
    return d_meas_sum;
  }
  void Set_p_meas_idx(vector<string> a_p_meas_ids){
  	p_meas_ids = a_p_meas_ids;
    p_meas_idx = Id2Idx(p_meas_ids);
  	n_p = p_meas_ids.size();
  }
  void Set_d_meas_idx(vector<string> a_d_meas_ids){
  	d_meas_ids = a_d_meas_ids;
    d_meas_idx = Id2Idx(d_meas_ids);
  	n_d = d_meas_ids.size();
  }
  void Set_d_meas_sum(vector<double> a_d_meas_sum){
  	d_meas_sum = a_d_meas_sum;
  	n_op = d_meas_sum.size();
  }

protected:
	vector<vector<double> > p_meas; // Pressure values at actual measure nodes
	vector<vector<double> > p_meas_all; // Every pressure value
	vector<string> p_meas_ids; // Node IDs for pressure
	vector<int> p_meas_idx; // Node indicies for pressure

	vector<double> d_meas_sum; // Overall demands
	vector<vector<double> > d_meas; // Demand measurement values (can be empty)
	vector<vector<double> > d_meas_all; // Every demand measurement value (can be empty)
	vector<string> d_meas_ids; // Node IDs for demands (can be empty)
	vector<int> d_meas_idx; // Node indicies for demands (can be empty)

	int n_p;  // Number of measured pressure nodes
	int n_d;  // Number of measured demand nodes
	int n_op; // Number of measured operational points

	vector<double> d_noise_value;
	string d_noise_mode;
  string folder;
};