#include <iostream>
#include "../../SVDCalibration.h"
#include <fstream>
#include <random>
#include <iomanip>
#include <string>
#include <algorithm>

using namespace std;

SVDCalibration *wds;

vector<string> LoadMeasIds(string file_name);

int main(int argc, char* argv[]){

  double tol = 1.0;
  // Name of containing folder of staci file
  string case_name;
  if(argc == 1){
      //case_name = "VIZ-SOPTVR-B-input_mod";
      //case_name = "Anytown_DC1";
      //case_name = "C-town_mod_10";
      //case_name = "VIZ-SOPTVR-M-input_mod";
      //case_name = "VIZ-SOPTVR-Q-input_mod";
      //case_name = "lin_tank_10";
      case_name = "lin_alap_9";
      //case_name = "racs_alap_9";
      //case_name = "kor_alap_6";
  }else if(argc == 2){
      case_name = argv[1];
  }
  cout << endl << "Case: " << case_name << endl;
  cout << setprecision(6) << fixed;
  srand((unsigned int) time(0));
  //string case_folder = "/home/rweber/0_PhD/Halozatok/sopron_halozatok/";
  //string case_folder = "/home/rweber/0_PhD/Halozatok/general_alap/";
  //string case_folder = "/home/rweber/0_PhD/Halozatok/anytown/";
  //string case_folder = "/home/rweber/0_PhD/Halozatok/";
  string case_folder = "";

  string sens_par_name = "friction_coeff";
  // Creating the case
  //wds = new Staci(case_folder + case_name + ".spr");
  wds = new SVDCalibration(case_folder + case_name + ".spr");
  wds->setDebugLevel(1);
  wds->buildSystem();
  wds->initialization();
  int n_nodes = wds->nodes.size();
  int n_edges = wds->edges.size();
  int n_pipe=0;
  for(int i=0; i<n_edges; i++)
    if(wds->edges.at(i)->getEdgeStringProperty("type") == "Pipe")
      n_pipe++;
  
  vector<double> fric_real, p_real;
  cout << "\nfric_real:\n";
  for(int i=0; i<n_edges; i++)
    if(wds->edges.at(i)->getEdgeStringProperty("type") == "Pipe"){
      //wds->edges.at(i)->setProperty("roughness",-0.03);
      //wds->edges.at(i)->Set_friction_model("HW");
      //wds->edges.at(i)->setProperty("roughness",100.*(.1*sin(i)+1));
      //wds->edges.at(i)->setProperty("erdesseg",-0.04*(.1*sin(i)+1));
      fric_real.push_back(wds->edges.at(i)->getDoubleProperty("roughness"));
      cout << "f at " << i << "\t" << fric_real.at(i) << endl;
    }

  cout << "\ncons:";
  double cons_nom = 0.0;
  for(int i=0; i<wds->nodes.size(); i++)
    if(wds->nodes[i]->getProperty("demand") > 0.)
      cons_nom += wds->nodes[i]->getProperty("demand");
  if(case_folder == "/home/rweber/0_PhD/Halozatok/general_alap/" || case_folder == ""){
    wds->nodes.at(n_nodes-1)->setProperty("demand",-cons_nom);
    //wds->nodes.at(5)->setProperty("demand",cons_nom/2.);
    for(int i=0; i<n_nodes; i++){
      if(wds->nodes.at(i)->getProperty("demand")>0.)
        wds->nodes.at(i)->setProperty("demand",cons_nom/(double)(n_nodes-1.0));
      cout << endl << wds->nodes.at(i)->getProperty("demand");
    }
  }

  wds->solveSystem();
  cout << "\nPressure dist:";
  for(int i=0; i<n_nodes; i++){
    cout << endl << wds->nodes.at(i)->getProperty("head");
    p_real.push_back(wds->nodes.at(i)->getProperty("head"));
  }
  cout << "\nFlow meters:";
  for(int i=0; i<n_edges;i++)
    if(wds->edges.at(i)->getEdgeStringProperty("type") == "Pipe")
      cout << endl << wds->edges.at(i)->getDoubleProperty("velocity");

  vector<int> fogy_meas_idx;
  //fogy_meas_idx.push_back(5);
  //fogy_meas_idx.push_back(8);
  vector<string> fogy_meas_id;
  for(int i=0; i<fogy_meas_idx.size(); i++)
    fogy_meas_id.push_back(wds->nodes.at(fogy_meas_idx.at(i))->getName());
  cout << "\nfogy_meas_id.size: " << fogy_meas_idx.size() << endl << endl << endl;

  vector<double> fogy_pert; fogy_pert.push_back(1.0); fogy_pert.push_back(0.1);
  string fogy_pert_mode = "None";

  wds->generateMeasurement(fric_real,fogy_pert,fogy_pert_mode);
  MatrixXd p_meas = wds->getMeasuredPressure();
  MatrixXd p_meas_all = wds->getEveryPressure();
  cout << "\np_meas: " << p_meas.rows() << endl;
  cout << "\np_meas_all: " << p_meas_all.rows() << 'x' << p_meas_all.cols() << endl;

  VectorXd fogy_sum = wds->getMeasuredDemandSum();
  cout << "\nMeasured values:";
  printf("\n a_fogy | p ");
  for(int i=0; i<fogy_sum.rows(); i++){
    printf("\n %6.3f | ",fogy_sum(i));
    for(int j=0; j<p_meas.cols(); j++)
      printf(" %6.3f",p_meas(i,j));
  }

  //cout << "\nfogy_meas size: " << fogy_meas.size() << "x" << fogy_meas.at(0).size();
  cout << "\nfogy_sum size: " << fogy_sum.rows();
  cout << "\nfogy_meas_id size: " << fogy_meas_id.size() << endl;

  //double tol = -1e3;
  vector<double> fric_est;
  for(int i=0; i<n_edges; i++)
    if(wds->edges.at(i)->getEdgeStringProperty("type") == "Pipe"){ 
      //fric_est.push_back(-0.02);
      //fric_est.push_back(fric_real.at(i)*wds->stat_tools->NormalDist(1.0,0.2));
      fric_est.push_back(fric_real.at(i)*1.5);
      wds->edges.at(i)->setDoubleProperty("roughness",fric_est.at(i));
    }
  wds->calculateSensitivity(sens_par_name,0.);

  int konv = wds->calibrate(fric_est, tol);

  wds->generateMeasurement(fric_est,fogy_pert,fogy_pert_mode);
  MatrixXd p_est = wds->getMeasuredPressure();
  MatrixXd p_est_all = wds->getEveryPressure();

  //cout << "\nfrics after Calibration: \n";
  //printf(" # |    calib   |    real    |   estima   |\n");
  //printf("---+------------+------------+------------|\n");
  //for(int i=0; i<n_edges; i++)
  //  if(wds->edges.at(i)->getEdgeStringProperty("type") == "Pipe")
  //    printf("%2i | %10.3f | %10.3f | %10.3f |\n", i, wds->edges.at(i)->getDoubleProperty("roughness"), fric_real.at(i),fric_est.at(i));

  //cout << "\npres after Calibration: \n";
  //printf(" # |    calib   |    real    |   estima   |\n");
  //printf("---+------------+------------+------------|\n");
  //for(int i=0; i<n_nodes; i++)
  //  printf("%2i | %10.6f | %10.6f | %10.6f |\n", i, wds->nodes[i]->getProperty("head"), p_real.at(i),p_est_all.at(i).at(0));

  double e_fric=0.;
  for(int i=0; i<fric_real.size(); i++)
    e_fric += pow(fric_real[i] - fric_est.at(i), 2.);
  e_fric = pow(e_fric/n_pipe,.5);
  cout << "\n\n Fric difference at estimation: " << e_fric;

  e_fric=0.;
  for(int i=0; i<fric_real.size(); i++)
    e_fric += pow(wds->edges.at(i)->getDoubleProperty("roughness") - fric_real.at(i), 2.);
  e_fric = pow(e_fric/n_pipe,.5);
  cout << "\n Fric difference after calib:   " << e_fric;

  vector<double> fric_calib;
  for(int i=0; i<n_edges; i++)
    if(wds->edges[i]->getEdgeStringProperty("type") == "Pipe")
      fric_calib.push_back(wds->edges[i]->getDoubleProperty("roughness"));

  wds->generateMeasurement(fric_calib,fogy_pert,fogy_pert_mode);
  MatrixXd p_calib = wds->getMeasuredPressure();
  MatrixXd p_calib_all = wds->getEveryPressure();


  double e_p=0.;
  for(int i=0; i<fogy_sum.rows(); i++)
    for(int j=0; j<n_nodes; j++)
      e_p += pow(p_est_all(i,j) - p_meas_all(i,j),2.);
  e_p = pow(e_p/(double)n_nodes,.5);
  printf("\n\n Pres difference at estimation: %8.5e",e_p);

  e_p=0.;
  for(int i=0; i<fogy_sum.rows(); i++)
    for(int j=0; j<n_nodes; j++)
      e_p += pow(p_calib_all(i,j) - p_meas_all(i,j),2.);
  e_p = pow(e_p/(double)n_nodes,.5);
  printf("\n Pres difference after calib:   %8.5e",e_p); 

  printf("\n\n Konv: ");
  switch(konv) {
    case 0 : printf("NO :(\n"); break;
    case 1 : printf("FULL :)\n"); break;
    case 2 : printf("LOCAL :|\n"); break;
}

  //string new_case_name = case_name +"_c";
  //wds->new_def_file = case_folder + new_case_name + ".spr";
  //wds->copy_file(wds->get_def_file(), wds->new_def_file);
  //wds->set_res_file(wds->new_def_file);
  //wds->save_results(true);

  cout << endl;
  return 0;
}

vector<string> LoadMeasIds(string file_name){
  ifstream file;
  file.open(file_name);
  string line;
  vector<string> names;
  if(file.is_open()){
  while(getline(file,line)){
    if(line[0] != '/'){
      string s;
      for(string::iterator j=line.begin(); j!=line.end(); j++){
        if(*j!=' ')
          s +=*j;
      }
    names.push_back(s);
    }
  }
  }else
  cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!ERROR!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl << "LoadMeasIds(), File is not open when calling LoadMeasIds() function!!!" << endl;
  file.close();

  return names;
}