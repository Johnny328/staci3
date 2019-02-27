#include "Calibration.h"

using namespace std;

Calibration::Calibration(string spr_filename) : Sensitivity(spr_filename){}
Calibration::~Calibration(){}

void Calibration::GenMeas(const vector<double> &fric, vector<double> d_noise_value, string d_noise_mode){

  string spr_filename = getDefinitionFile();
  string folder = spr_filename.substr(0,spr_filename.rfind('.')) + '/';

  LoadDemSum(spr_filename);
  if(d_meas_sum.size()==0){
    cout << endl << "ERROR!!! d_meas_sum is empty i.e. the sum of the overall consumptions must be given in " << folder + "/d_meas_sum.txt file" << endl;
    exit(-1);
  }

  LoadPresIDs(spr_filename);
  if(p_meas_ids.size()==0){
    cout << endl << "ERROR!!! p_meas_ids is empty i.e. the node IDs are not given in " << folder + "/d_meas_sum.txt file" << endl;
    exit(-1);
  }

  LoadDem(spr_filename);

  int numberNodes = nodes.size(), numberEdges = edges.size();

  // Saving original frics and demands
  vector<double> fric_orig(numberNodes,0.), d_orig(numberEdges,0.);
  for(int k=0;k<numberNodes;k++)
    d_orig[k] = nodes.at(k)->getDemand();
  for(int k=0;k<numberEdges;k++)
    if(edges[k]->getType() == "Pipe")
      fric_orig[k] = edges.at(k)->getProperty("roughness");

  //if((d_noise_mode == "Uniform" || d_noise_mode == "Normal" || d_noise_mode == "None") == false){
  if((d_noise_mode == "None") == false){
    cout << "\n!!!!! ERROR !!!!!\nStaci:GenMeas function\nPerturbation methods available: Uniform || Normal || None\n\"" << d_noise_mode << "\" is not existing.\nChanging to None!!! Continouing...\n\n";
    d_noise_mode = "None";
  }

  double d_sum_orig = get_sum_of_pos_consumption(); // Sum of every demands at nominal operational point
  double d_sum_meas_orig=0.; // Sum of measured demands at nominal operational point
  for(int i=0; i<n_d; i++)
    d_sum_meas_orig += nodes[d_meas_idx[i]]->getDemand();

  string save_folder = getDefinitionFile();
  save_folder = save_folder.substr(0,save_folder.rfind('.')) + '/';

  FILE *p_file, *pall_file;
  p_file = fopen((save_folder+"p_meas.txt").c_str(),"w");
  if(p_file==NULL)
    cout << endl << "Warning!!! Folder named: " << save_folder << " does not exist! No saving will be performed in GenMeas..." << endl;
  pall_file = fopen((save_folder+"p_meas_all.txt").c_str(),"w");
  if(pall_file==NULL)
    cout << endl << "Warning!!! Folder named: " << save_folder << " does not exist! No saving will be performed in GenMeas..." << endl;

  // Clearing vectors
  p_meas.clear();
  p_meas.resize(n_op, vector<double>(n_p));
  p_meas_all.clear();
  p_meas_all.resize(n_op, vector<double>(numberNodes));

  for(int k=0;k<numberEdges;k++)
    if(edges[k]->getType() == "Pipe")
      edges[k]->setProperty("roughness",fric[k]);
  for(int j=0;j<n_op;j++){
    double d_meas_real=0.;
    for(int k=0;k<n_d;k++){
      nodes.at(d_meas_idx[k])->setDemand(d_meas[j][k]);
      d_meas_real += d_meas[j][k];
    } 
    for(int k=0;k<numberNodes;k++){
      bool measured=false; int l=0;
      while(l<d_meas_ids.size() && !measured){
        if(d_meas_ids[l] == nodes[k]->getName())
          measured = true;
        l++;
      }
      if(d_noise_mode == "Uniform")
        if(nodes[k]->getDemand() > 0. && !measured)
          nodes[k]->setDemand((d_meas_sum[j]*d_sum_orig-d_meas_real)/(d_sum_orig-d_sum_meas_orig)*d_orig[k]*statTools->UniformDist(d_noise_value[0],d_noise_value[1]));
      if(d_noise_mode == "Normal")
        if(nodes[k]->getDemand() > 0. && !measured)
          nodes[k]->setDemand((d_meas_sum[j]*d_sum_orig-d_meas_real)/(d_sum_orig-d_sum_meas_orig)*d_orig[k]*statTools->NormalDist(d_noise_value[0],d_noise_value[1]));
      if(d_noise_mode == "None")
        if(nodes[k]->getDemand() > 0. && !measured)
          nodes[k]->setDemand((d_meas_sum[j]*d_sum_orig-d_meas_real)/(d_sum_orig-d_sum_meas_orig)*d_orig[k]); 
    }
    // Adjusting the sum of the consumptions in case of noise
    if(d_noise_mode == "Uniform" || d_noise_mode == "Normal"){
      d_meas_sum[j] = 0.;
      for(int k=0; k<numberNodes; k++)
        if(nodes[k]->getDemand()>0.)
          d_meas_sum[j] += nodes[k]->getDemand();
      d_meas_sum[j] = d_meas_sum[j]/d_sum_orig;
    }
    solveSystem();
    for(int k=0;k<n_p;k++){
      p_meas[j][k] = nodes[p_meas_idx[k]]->getHead();
      if(p_file!=NULL)
	      fprintf(p_file,"%8.5e;",p_meas[j][k]);
    }
    if(p_file!=NULL)
	    fprintf(p_file,"\n");
    for(int k=0; k<numberNodes; k++){
      p_meas_all[j][k] = nodes[k]->getHead();
      if(pall_file!=NULL)
	      fprintf(pall_file,"%8.5e;",p_meas_all[j][k]);
    }
    if(pall_file!=NULL)
	    fprintf(pall_file,"\n");
  }

  if(p_file!=NULL)
  	fclose(p_file);
  if(pall_file!=NULL)
  	fclose(pall_file);

  // setting back original frics and demands
  for(int k=0;k<numberNodes;k++)
    nodes[k]->setDemand(d_orig[k]);
  for(int k=0;k<numberEdges;k++)
    if(edges[k]->getType() == "Pipe")
      edges[k]->setProperty("roughness",fric_orig[k]);
  solveSystem();

  if(getDebugLevel()>1)
    cout << endl << "Generating measurement data: OK" << endl;
}

void Calibration::LoadMeas(string spr_filename){
  string folder = spr_filename.substr(0,spr_filename.rfind('.')) + '/';
  ifstream ifile;

  p_meas.clear();
  // Reading the measured pressure values
  ifile.open((folder+"p_meas.txt").c_str());
  if(ifile.is_open()){
    p_meas = CSVRead(ifile,';');
  }
  else{
    cout << endl << "ERROR!!! File " << folder+"p_meas.txt" << " does NOT exist!" << endl << "Exiting..." << endl;
    exit(-1);
  }
  ifile.close();

  // Reading the IDs of the measured nodes
  LoadPresIDs(spr_filename);

  // Reading the sum of the consumptions
  LoadDemSum(spr_filename);

  // Reading the individually measured consumptions
  LoadDem(spr_filename);

  // IMPORTANT: CHECKING THE SIZE OF THE VARIABLES
  if(p_meas.size() != n_op){
    cout << endl << "ERROR!!! p_meas row size doest not match n_op / d_meas_sum" << endl << "The measured pressure values does not match with the measured operational points" << endl;
    exit(-1);
    if(p_meas.size()>0){
      if(p_meas[0].size() != n_p){
        cout << endl << "ERROR!!! p_meas col size doest not match n_op / p_meas_ids" << endl << "The measured pressure values does not match with the measured node IDs" << endl;
        exit(-1);
      }
    }
  }
  if(n_d>0){
    if(d_meas.size() != n_op){
      cout << endl << "ERROR!!! d_meas size doest not match n_op / d_meas_sum" << endl << "The measured demand values does not match with the measured operational points" << endl;
      exit(-1);
      if(d_meas.size()>0){
        if(p_meas[0].size() != n_d){
          cout << endl << "ERROR!!! d_meas size doest not match n_d / d_meas_ids" << endl << "The measured pressure values does not match with the measured node IDs" << endl;
          exit(-1);
        }
      }
    }
  }
 if(getDebugLevel()>1)
  cout << endl << "Loading measurement data: OK" << endl; 
}

void Calibration::LoadDemSum(string spr_filename){
  string folder = spr_filename.substr(0,spr_filename.rfind('.')) + '/';
  ifstream ifile;

  // Reading the sum of the consumptions
  ifile.open((folder+"d_meas_sum.txt").c_str());
  if(ifile.is_open()){
    vector<string> temp = ReadStrings(ifile);    
    d_meas_sum.clear();
    for(int i=0; i<temp.size(); i++)
      d_meas_sum.push_back(stod(temp[i],0));
    n_op = d_meas_sum.size();
    if(n_op==0)
      cout << endl << "Warning! File " << folder+"d_meas_sum.txt" << " does exist, but does NOT contain anything!" << endl;
  }
  else{
    cout << endl << "ERROR!!! File " << folder+"d_meas_sum.txt" << " does NOT exist!" << endl << "Exiting..." << endl;
    exit(-1);
  }
  ifile.close();
}

void Calibration::LoadPresIDs(string spr_filename){
  string folder = spr_filename.substr(0,spr_filename.rfind('.')) + '/';
  ifstream ifile;

  // Reading the IDs of the measured nodes
  ifile.open((folder+"p_meas_ids.txt").c_str());
  if(ifile.is_open()){
    p_meas_ids.clear();
    p_meas_idx.clear();
    p_meas_ids = ReadStrings(ifile);
    p_meas_idx = Id2Idx(p_meas_ids);
    n_p = p_meas_idx.size();
    if(n_p==0)
      cout << endl << "Warning! File " << folder+"p_meas_ids.txt" << " does exist, but does NOT contain anything!" << endl;
  }
  else{
    cout << endl << "ERROR!!! File " << folder+"p_meas_ids.txt" << " does NOT exist!" << endl << "Exiting..." << endl;
    exit(-1);
  }
  ifile.close();
}

void Calibration::LoadDem(string spr_filename){
  ifstream ifile;

  // Reading the measured consumptions if they existed
  ifile.open((folder+"d_meas.txt").c_str());
  if(ifile.is_open()){
    d_meas.clear();
    d_meas = CSVRead(ifile,';');
  }
  else{
    if(getDebugLevel()>2){
      cout << endl << "Warning!!! File " << folder+"d_meas.txt" << " does NOT exist!" << endl << "Exiting..." << endl;
    }
  }
  ifile.close();

  // Reading the ids of the nodes where consumptions were measured if they existed
  ifile.open((folder+"d_meas_ids.txt").c_str());
  if(ifile.is_open()){
    d_meas_ids.clear();
    d_meas_idx.clear();
    d_meas_ids = ReadStrings(ifile);
    d_meas_idx = Id2Idx(d_meas_ids);
    n_d = d_meas_idx.size();
  }
  else{
    if(getDebugLevel()>2){
      cout << endl << "Warning!!! File " << folder+"d_meas_ids.txt" << " does NOT exist!" << endl << "Exiting..." << endl;
    }
  }
  ifile.close();
}