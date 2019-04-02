/*===================================================================*\
                               buildStaci  
                            ----------------

  This code is capable of building the network file (SPR) for 
  hydraulic calculations in STACI. This project does not connected
  directly to STACI i.e. it works independently from that.
  
  HOW TO USE: 

      - CASE_NAME is a folder containing two CSV files, one for the
        pipelines (pipelines.csv) one for the gate valves 
        (gate_valves.csv)

      - SET:  tol [m]           minimal distance between end of pipelines
              filter [m]        deleting pipelines shorter that "filter" and having only one connected end i.e. endpipelines
              filter2 [m]       deleting pipelines shorter than "filter2" and having rank of 2 at both ends
              gv_dist_min [m]   minimal distance between gate valve and a node (connecting pipelines)

      - OUTPUT: CASE_NAME.spr            File for STACI
                missing_extra.log        If some extra information is missing (e.g. year, material), it is listed in this file
                missed_gate_valves.log   If there was no node within "gv_dist_min" distance, it is listed in this file

      - RUN: ./buildStaci CASE_NAME

      - COMPILE: clang++ buildStaci.cpp -o buildStaci

    staci3
    Cs. Hos, R. Weber, T. Huzsvar
    https://github.com/weberrichard/staci3
\*==================================================================*/

// SET
double tol = 0.0; 
double filter = 20.; 
double filter2 = 50.; 
double gv_dist_min = 1.;

#include <iostream>
#include <fstream>
#include <random>
#include <iomanip>
#include <string>
#include <algorithm>

using namespace std;

int VectorAbsMax(const vector<int> &v);
double VectorMax(const vector<double> &v);
double VectorMin(const vector<double> &v);
vector<int> VectorAbsUnique(vector<int> &v);
double VectorAverage(const vector<double> &v);

int main(int argc, char* argv[]){

  time_t ido = time(0);
  string case_name;
  if(argc == 1){
  	case_name = "test";
  }else if(argc == 2){
    case_name = argv[1];
  }
  string case_id = case_name;
  string folder_name = case_name + "/";
  string pipeline_file = folder_name + "pipelines.csv";
  string gate_valve_file = folder_name + "gate_valves.csv";

  char separator = ';';
  cout << setprecision(10);

  vector<string> id; // ID of the pipe
  vector<double> xe; // x coordinate of the start of the pipe [m]
  vector<double> ye; // y coordinate of the start of the pipe [m]
  vector<double> xv; // x coordinate of the end of the pipe [m]
  vector<double> yv; // y coordinate of the end of the pipe [m]
  vector<double> l; // length of the pipe [m]
  vector<double> d; // diameter of the pipe [mm]
  vector<string> mat; // material of the pipe
  vector<string> year; // year of installation

  // IMPORTANTING THE DATA
  vector<int> missing_impdata; // number of the rows where important data is missing
  vector<int> missing_extdata; // number of the rows where extra data is missing
  vector<int> missing_length; // number of the rows where extra data is missing
  string line;
  ifstream file_in(pipeline_file.c_str());
  if(!file_in.is_open()){
  	cout << "\nERROR! File " << pipeline_file << " does not exist!" << endl << "Exitiing..." << endl;
  	exit(-1);
  }
  int i=0;
  while(getline(file_in,line)){
    string temp;
    vector<string> segments;
    for (string::iterator j=line.begin(); j!=line.end(); j++){
      if(*j!=separator && j!=line.end()-1)
        temp += *j;
      else{
        if(temp.size()==0)
          segments.push_back("-1");
        else{ 
          segments.push_back(temp);
          temp = "";
        }
      }
    }

    bool missing_coord=false;
    for(int j=0; j<=4; j++)
    	if(segments[j]=="-1")
    		missing_coord=true;
    if(missing_coord){
    	cout << "\nERROR! Missing coordinate or ID in line " << i << " in file " << folder_name + pipeline_file << endl << "Exiting..." << endl;
    	exit(-1);
    }
    id.push_back(segments[0]);
    xe.push_back(stod(segments[1],0));
    ye.push_back(stod(segments[2],0));
    xv.push_back(stod(segments[3],0));
    yv.push_back(stod(segments[4],0));
    if(segments[5]=="-1"){
      l.push_back(pow(pow(xe[i]-xv[i],2.)+pow(ye[i]-yv[i],2.),.5));
      missing_length.push_back(i);
    }
    else
      l.push_back(stod(segments[5],0));
    d.push_back(stod(segments[6],0));
    mat.push_back(segments[7]);
    year.push_back(segments[8]);

    for(int j=0; j<segments.size(); j++){
      if(segments[j]=="-1"){
        if(j<=4 || j==6){ 
          if(missing_impdata.size()==0)
            missing_impdata.push_back(i);
          else if(missing_impdata.back()!=i)
            missing_impdata.push_back(i);
        }
        if(j>=7)
          missing_extdata.push_back(i);
      }
    }
    segments.clear();
    i++;
  }
  file_in.close();

  if(missing_extdata.size()>0){
    string file_name = folder_name + "missing_extra.log";
    ofstream file(file_name);
    cout << endl << "There are missing extra data, see: " << file_name << ", or check in STACI";
    file << "There are missing extra information in the following rows:" << endl;
    for(vector<int>::iterator i = missing_extdata.begin() ; i != missing_extdata.end(); i++)
      file << *i+1 << endl;
    file.close();
    cout << endl << "Continouing..." << endl;
  }
  if(missing_length.size()>0){
    string file_name = folder_name + "missing_length.log";
    ofstream file(file_name);
    cout << endl << "There are missing length data, replaced with distance of coordinates, see the imperfect rows: " << file_name << ", or check in STACI";
    file << "There are missing length information in the following rows:" << endl;
    for(vector<int>::iterator i = missing_length.begin() ; i != missing_length.end(); i++)
      file << *i+1 << endl;
    file.close();
    cout << endl << "Continouing..." << endl;
  }
  if(missing_impdata.size()>0){
    string file_name = folder_name + "missing_IMP.log";
    ofstream file(file_name);
    cout << endl << "There are missing IMPORTANT data, see: " << file_name << ", or check in STACI";
    file << "There are missing extra information in the following rows:" << endl;
    for(vector<int>::iterator i = missing_impdata.begin() ; i != missing_impdata.end(); i++)
      file << *i+1 << endl;
    cout << endl << "Continouing..." << endl;
    file.close();
  }
  cout << "\n[*]Importing pipelines: OK" << endl;

  // BUILDING THE TOPOLOGY
  // Finding the close pipes
  vector<int> edge1, edge2;
  for(int i=0; i<id.size(); i++){
    for(int j=i+1; j<id.size(); j++){
      double d1=(xe[i]-xe[j])*(xe[i]-xe[j]) + (ye[i]-ye[j])*(ye[i]-ye[j]);
      double d2=(xe[i]-xv[j])*(xe[i]-xv[j]) + (ye[i]-yv[j])*(ye[i]-yv[j]);
      double d3=(xv[i]-xe[j])*(xv[i]-xe[j]) + (yv[i]-ye[j])*(yv[i]-ye[j]);
      double d4=(xv[i]-xv[j])*(xv[i]-xv[j]) + (yv[i]-yv[j])*(yv[i]-yv[j]);
      vector<double> d_tmp = {d1,d2,d3,d4};

      int idx = 0;
      double dmin=d_tmp[idx];
      for(int i=1;i<d_tmp.size();i++)
        if(d_tmp[i]<=dmin){
          dmin = d_tmp[i];
          idx = i;
        }

      if(dmin<=tol*tol){
        if(idx==0){ 
          edge1.push_back(i+1);
          edge2.push_back(j+1);
        }
        if(idx==1){ 
          edge1.push_back(i+1);
          edge2.push_back(-j-1);
        }
        if(idx==2){ 
          edge1.push_back(-i-1);
          edge2.push_back(j+1);
        }
        if(idx==3){ 
          edge1.push_back(-i-1);
          edge2.push_back(-j-1);
        }
      }
    }
  }
  cout << "\n[*]Finding close pipes: OK" << endl;

  // Creating nodes and recording the close pipes
  vector<string> node, node_from(id.size()), node_to(id.size());
  vector<int> connected_pipes;
  vector<double> x,y;
  int node_counter=0;
  for(int i=0; i<edge1.size(); i++){
    bool connected=false;
    for(int j=0; j<connected_pipes.size(); j++)
      if(edge1[i]==connected_pipes[j] || edge2[i]==connected_pipes[j])
        connected=true;
    if(!connected){
      vector<int> pipes;
      vector<double> x_tmp, y_tmp;
      pipes.push_back(edge1[i]);
      pipes.push_back(edge2[i]);
      node.push_back("N_"+to_string(node_counter));
      node_counter++;
      double max_pipe = VectorAbsMax(pipes);
      int j=i+1;
      while(edge1[j]<=max_pipe){
        if(edge1[j]==edge1[i]){
          pipes.push_back(edge2[j]);
        }else if(edge1[j]==edge2[i]){
          pipes.push_back(edge2[j]);
        }
        max_pipe = VectorAbsMax(pipes);
        j++;
      }
      pipes = VectorAbsUnique(pipes);
      for(int j=0; j<pipes.size(); j++){ 
        connected_pipes.push_back(pipes[j]);
        if(pipes[j]>0){
          node_from[pipes[j]-1] = node[node_counter-1];
          x_tmp.push_back(xe[pipes[j]-1]);
          y_tmp.push_back(ye[pipes[j]-1]);
        }
        if(pipes[j]<0){ 
          node_to[-pipes[j]-1] = node[node_counter-1];
          x_tmp.push_back(xv[-pipes[j]-1]);
          y_tmp.push_back(yv[-pipes[j]-1]);
        }
      }
      x.push_back(VectorAverage(x_tmp));
      y.push_back(VectorAverage(y_tmp));
    }
  }
  cout << "\n[*]Creating nodes: OK" << endl;

  // handling the pipeeindings where no connection was found
  vector<bool> pipe_erase(id.size(),false);
  for(int i=0; i<id.size(); i++){
    if(node_from[i]==""){
    	if(l[i]>filter){
	      node.push_back("N_"+to_string(node_counter));
	      node_from[i] = node[node_counter];
	      x.push_back(xe[i]);
	      y.push_back(ye[i]);
	      node_counter++;
    	}else
    		pipe_erase[i]=true;
    }
    if(node_to[i]==""){
    	if(l[i]>filter){
	      node.push_back("N_"+to_string(node_counter));
	      node_to[i] = node[node_counter];
	      x.push_back(xv[i]);
	      y.push_back(yv[i]);
	      node_counter++;
	    }else
    		pipe_erase[i]=true;
    }
  }
  // handling if the node_from and node_to is equal
  for(int i=0; i<id.size();i++){
  	if(node_from[i] == node_to[i]){
      node.push_back("N_"+to_string(node_counter));
  		node_to[i] = node[node_counter];
  		x.push_back(xv[i]);
      y.push_back(yv[i]);
      node_counter++;
  	}
  }

  // counting the rank
  vector<int> rank(node.size(),0);
  for(int i=0; i<node.size(); i++){
  	for(int j=0; j<id.size(); j++){
  		if(pipe_erase[j]==false)
  			if(node[i]==node_from[j] || node[i]==node_to[j])
  				rank[i]++;
  	}
  }
  int pipe_erased=0;
  for(vector<bool>::iterator i=pipe_erase.begin(); i!=pipe_erase.end(); i++)
  	pipe_erased += *i;
  cout << "\n[*]Filtering short pipes: OK" << endl;

  // GATE VALVES
  vector<string> id_gv,state;
	vector<double> x_gv,y_gv;
  file_in.open(gate_valve_file.c_str());
  i=0;
  while(getline(file_in,line)){
    string temp;
    vector<string> segments;
    for (string::iterator j=line.begin(); j!=line.end(); j++){
      if(*j!=separator && j!=line.end()-1)
        temp += *j;
      else{
        if(temp.size()==0)
          segments.push_back("-1");
        else{ 
          segments.push_back(temp);
          temp = "";
        }
      }
    }
   	id_gv.push_back(segments[0]);
   	x_gv.push_back(stod(segments[1],0));
   	y_gv.push_back(stod(segments[2],0));
   	state.push_back(segments[3]);

    segments.clear();
    i++;
  }
  file_in.close();
  cout << "\n[*]Importing gate valves: OK" << endl;

  // Finding closest node
  vector<bool> gv_erase(id_gv.size(),false);
  ofstream file_out;
  file_out.open(folder_name + "missed_gate_valves.log");
  file_out << "There could not be found any node with rank of two for the following gate valves:\n";
  vector<string> node_from_gv(id_gv.size()), node_to_gv(id_gv.size());
  vector<double> e(id_gv.size());
  for(int i=0; i<id_gv.size(); i++){
  	int node_min = -1;
  	double dist_min = gv_dist_min*gv_dist_min;
  	for(int j=0; j<node.size(); j++){
  		if(rank[j]==2){
  			double dist = (x[j]-x_gv[i])*(x[j]-x_gv[i]) + (y[j]-y_gv[i])*(y[j]-y_gv[i]);
  			if(dist<dist_min){
	  			dist_min = dist;
	  			node_min = j;
  			}
  		}
  	}
  	//cout << "i: " << i << "  node_min: " << node_min << "  dist_min: " << dist_min << endl;
  	if(node_min == -1){
  		file_out << "id: " << id_gv[i] << "  in line: " << i << endl;
  		e[i]=0.;
	    node.push_back("N_"+to_string(node_counter));
  		node_from_gv[i] = node[node_counter];
			x.push_back(x_gv[i]);
		  y.push_back(y_gv[i]);
		  rank.push_back(1);
		  node_counter++;
		  node.push_back("N_"+to_string(node_counter));
  		node_to_gv[i] = node[node_counter];
			x.push_back(x_gv[i]);
		  y.push_back(y_gv[i]-1.);
		  rank.push_back(1);
		  node_counter++;
  		gv_erase[i]=true;
  	}else{
	  	if(state[i]!="zart"){
	  		e[i] = 0.;
		  	node_from_gv[i] = node[node_min];
		  	rank[node_min]++;
			  node.push_back((node[node_min]+"_1"));
				node_to_gv[i] = node[node_counter];
				x.push_back(x[node_min]+1.);
		    y.push_back(y[node_min]+1.);
		    rank.push_back(1);
		    node_counter++;
	  	}else{
	  		e[i] = 100.;
	  		node_from_gv[i] = node[node_min];
			  node.push_back(node[node_min]+"_1");
				node_to_gv[i] = node[node_counter];
		    rank.push_back(2);
				for(int j=0; j<id.size(); j++){
					if(node[node_min]==node_from[j]){
						node_from[j] = node[node_counter];
						double norm = (xe[j]-xv[j])*(xe[j]-xv[j]) + (ye[j]-yv[j])*(ye[j]-yv[j]);
						x.push_back(xe[j]+(xv[j]-xe[j])/norm*0.2);
						y.push_back(yv[j]+(yv[j]-ye[j])/norm*0.2);
						break;
					}
					if(node[node_min]==node_to[j]){
						node_to[j] = node[node_counter];
						double norm = (xe[j]-xv[j])*(xe[j]-xv[j]) + (ye[j]-yv[j])*(ye[j]-yv[j]);
						x.push_back(xv[j]+(xe[j]-xv[j])/norm*0.2);
						y.push_back(yv[j]+(ye[j]-yv[j])/norm*0.2);
						break;
					}
				}
		    node_counter++;
	  	}
  	}
  }
  int gv_erased=0;
  for(vector<bool>::iterator i=gv_erase.begin(); i!=gv_erase.end(); i++)
  	gv_erased += *i;
  if(gv_erased>0)
  	cout << "\nWARNING! There was " << gv_erased << " number of gate valves without any connection.\nFor details see missed_gate_valves.log\nContinouing..." << endl; 
  file_out.close();
  cout << "\n[*]Placing gate valves: OK" << endl;

  // FILTER2: removing pipes with rank 2 at both end and adding to neighboring pipe
  vector<bool> node_erase(node.size(),false);
  int filter2_no=0;
  for(int i=0; i<id.size(); i++){
  	if(!pipe_erase[i]){
  		int r_from=0,r_to=0, node_j;
  		for(int j=0; j<node.size(); j++){	
  			if(node_from[i]==node[j])
  				r_from = rank[j];
  			if(node_to[i]==node[j]){
  				r_to = rank[j];
  				node_j=j;
  			}
  			if(r_to>0 && r_from>0)
  				break;
  		}
  		if(r_from==2 && r_to==2 && l[i]<filter2){
        
  			for(int j=0; j<id.size(); j++){
  				if(i!=j && d[i] == d[j] && !pipe_erase[j]){
  					if(node_to[i] == node_from[j]){
  						node_from[j]=node_from[i];
  						l[j] += l[i];
              filter2_no++;
              pipe_erase[i] = true;
              node_erase[node_j] = true;
  						break;
  					}
  					if(node_to[i] == node_to[j]){
  						node_to[j]=node_from[i];
  						l[j] += l[i];
              filter2_no++;
              pipe_erase[i] = true;
              node_erase[node_j] = true;
  						break;
  					}
  				}
  			}
  		}
  	}
  }
  for(int i=0; i<id.size(); i++){
  	if(!pipe_erase[i]){
  		bool megvan_from=false, megvan_to=false;
  		for(int j=0; j<node.size(); j++){
  			if(!node_erase[j]){
  				if(node_from[i]==node[j])
  					megvan_from = true;
  				if(node_to[i]==node[j])
  					megvan_to = true;
  				if(megvan_to && megvan_from)
  					break;
  			}
  		}
  		if(!(megvan_from && megvan_to))
  			cout << "\nnincs meg:" << id[i];
  	}
  }

  // WRITING TO FILE
  int case_width = 800, case_height = 800;
  file_out.open(folder_name + case_name + ".spr");
  file_out << "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n<staci>\r\n";
  file_out << "\t<settings>\r\n\t\t<id>\r\n\t\t\t"+case_id+"\r\n\t\t</id>\r\n\t\t<name>\r\n\t\t\t"+case_name+"\r\n\t\t</name>\r\n\t\t<color/>\r\n\t\t<counter>\r\n\t\t\t1\r\n\t\t</counter>\r\n\t\t<user1>\r\n\t\t\t0\r\n\t\t</user1>\r\n\t\t<user2>\r\n\t\t\t0\r\n\t\t</user2>\r\n\t\t<zone>\r\n\t\t\t0\r\n\t\t</zone>\r\n\t\t<cpp_xml_debug>\r\n\t\t\tfalse\r\n\t\t</cpp_xml_debug>\r\n\t\t<solution_exists>\r\n\t\t\ttrue\r\n\t\t</solution_exists>\r\n\t\t<tsolution_exists>\r\n\t\t\tfalse\r\n\t\t</tsolution_exists>\r\n\t\t<csolution_exists>\r\n\t\t\tfalse\r\n\t\t</csolution_exists>\r\n\t\t<serial_exists>\r\n\t\t\tfalse\r\n\t\t</serial_exists>\r\n\t\t<out_file>\r\n\t\t\tstaci.out\r\n\t\t</out_file>\r\n\t\t<debug_level>\r\n\t\t\t2\r\n\t\t</debug_level>\r\n\t\t<friction_model>\r\n\t\t\tHW\r\n\t\t</friction_model>\r\n\t\t<version>\r\n\t\t\t1\r\n\t\t</version>\r\n\t\t<xstart>\r\n\t\t\t0\r\n\t\t</xstart>\r\n\t\t<ystart>\r\n\t\t\t0\r\n\t\t</ystart>\r\n\t\t<xend>\r\n\t\t\t100\r\n\t\t</xend>\r\n\t\t<yend>\r\n\t\t\t100\r\n\t\t</yend>\r\n\t\t<owner>\r\n\t\t\trweber\r\n\t\t</owner>\r\n\t\t<width>\r\n\t\t\t"+to_string(case_width)+"\r\n\t\t</width>\r\n\t\t<height>\r\n\t\t\t"+to_string(case_height)+"\r\n\t\t</height>\r\n\t\t<hasmap>\r\n\t\t\tfalse\r\n\t\t</hasmap>\r\n\t\t<image/>\r\n\t\t<locked>\r\n\t\t\tfalse\r\n\t\t</locked>\r\n\t\t<iter_max>\r\n\t\t\t100\r\n\t\t</iter_max>\r\n\t\t<e_p_max>\r\n\t\t\t0.0001\r\n\t\t</e_p_max>\r\n\t\t<e_mp_max>\r\n\t\t\t0.01\r\n\t\t</e_mp_max>\r\n\t\t<relax>\r\n\t\t\t1\r\n\t\t</relax>\r\n\t\t<relax_mul>\r\n\t\t\t1.2\r\n\t\t</relax_mul>\r\n\t\t<tt_length>\r\n\t\t\t1.0\r\n\t\t</tt_length>\r\n\t\t<cl_length>\r\n\t\t\t1.0\r\n\t\t</cl_length>\r\n\t\t<mp_init>\r\n\t\t\t100.0\r\n\t\t</mp_init>\r\n\t\t<p_init>\r\n\t\t\t5.0\r\n\t\t</p_init>\r\n\t\t<sum_of_inflow>\r\n\t\t\t0.0\r\n\t\t</sum_of_inflow>\r\n\t\t<sum_of_demand>\r\n\t\t\t0.0\r\n\t\t</sum_of_demand>\r\n\t\t<fluid_volume>\r\n\t\t\t1.41372e+02\r\n\t\t</fluid_volume>\r\n\t\t<reservoir_capacity>\r\n\t\t\t0.0\r\n\t\t</reservoir_capacity>\r\n\t\t<start_date>\r\n\t\t\t2007-01-01\r\n\t\t</start_date>\r\n\t\t<start_time>\r\n\t\t\t0.0\r\n\t\t</start_time>\r\n\t\t<time_step>\r\n\t\t\t1.0\r\n\t\t</time_step>\r\n\t\t<step_count>\r\n\t\t\t24\r\n\t\t</step_count>\r\n\t\t<play_step>\r\n\t\t\t10.0\r\n\t\t</play_step>\r\n\t\t<muls>\r\n\t\t\t<mul>\r\n\t\t\t\t0.0\r\n\t\t\t</mul>\r\n\t\t</muls>\r\n\t</settings>\r\n";
  file_out << "\t<texts/>\r\n";

  // Printing nodes
  file_out << "\t<nodes>\r\n";
  double xmin = VectorMin(x), xmax = VectorMax(x), ymin = VectorMin(y), ymax = VectorMax(y);
  for(int i=0;i<node.size();i++){
  	if(!node_erase[i]){
	    double xpos,ypos,demand;
	    double delta = max(ymax-ymin,xmax-xmin);
	    xpos = (x[i]-xmin)/delta*(case_width-20)+10;
	    ypos = -((y[i]-ymin)/delta*(case_height-20)+10)+case_height;
	    demand = 0.;
	    file_out << "\t\t<node>\r\n\t\t\t<id>\r\n\t\t\t\t"+node[i]+"\r\n\t\t\t</id>\r\n\t\t\t<name>\r\n\t\t\t\t"+node[i]+"\r\n\t\t\t</name>\r\n\t\t\t<color>\r\n\t\t\t\t-8684677\r\n\t\t\t</color>\r\n\t\t\t<counter>\r\n\t\t\t\t14\r\n\t\t\t</counter>\r\n\t\t\t<user1>\r\n\t\t\t\t"+to_string(rank[i])+"\r\n\t\t\t</user1>\r\n\t\t\t<user2>\r\n\t\t\t\t0\r\n\t\t\t</user2>\r\n\t\t\t<zone>\r\n\t\t\t\t0\r\n\t\t\t</zone>\r\n\t\t\t<xcoord>\r\n\t\t\t\t"+to_string(x[i])+"\r\n\t\t\t</xcoord>\r\n\t\t\t<ycoord>\r\n\t\t\t\t"+to_string(y[i])+"\r\n\t\t\t</ycoord>\r\n\t\t\t<xpos>\r\n\t\t\t\t"+to_string(xpos)+"\r\n\t\t\t</xpos>\r\n\t\t\t<ypos>\r\n\t\t\t\t"+to_string(ypos)+"\r\n\t\t\t</ypos>\r\n\t\t\t<height>\r\n\t\t\t\t1\r\n\t\t\t</height>\r\n\t\t\t<demand>\r\n\t\t\t\t"+to_string(demand)+"\r\n\t\t\t</demand>\r\n\t\t\t<density>\r\n\t\t\t\t1000.0\r\n\t\t\t</density>\r\n\t\t\t<cl_input>\r\n\t\t\t\t0.0\r\n\t\t\t</cl_input>\r\n\t\t\t<pressure>\r\n\t\t\t\t0.00000\r\n\t\t\t</pressure>\r\n\t\t\t<head>\r\n\t\t\t\t0.00000\r\n\t\t\t</head>\r\n\t\t\t<endnode>\r\n\t\t\t\tfalse\r\n\t\t\t</endnode>\r\n\t\t\t<travel_time>\r\n\t\t\t\t0.00000e+00\r\n\t\t\t</travel_time>\r\n\t\t\t<concentration>\r\n\t\t\t\t0.0000\r\n\t\t\t</concentration>\r\n\t\t\t<curve>\r\n\t\t\t\t<id>\r\n\t\t\t\t\tmuls\r\n\t\t\t\t</id>\r\n\t\t\t\t<x_val/>\r\n\t\t\t\t<x_dim/>\r\n\t\t\t\t<y_val/>\r\n\t\t\t\t<y_dim/>\r\n\t\t\t\t<points>\r\n\t\t\t\t\t<point_x>\r\n\t\t\t\t\t\t0,0000\r\n\t\t\t\t\t</point_x>\r\n\t\t\t\t\t<point_y>\r\n\t\t\t\t\t\t0,0000\r\n\t\t\t\t\t</point_y>\r\n\t\t\t\t</points>\r\n\t\t\t</curve>\r\n\t\t</node>\r\n";
  	}
  	
  }
  file_out << "\t</nodes>\r\n";

  // Printing Edges
  file_out << "\t<edges>\r\n";
  for(int i=0; i<id.size(); i++){
  	if(!pipe_erase[i]){
	    double friction_coeff = -0.02;
	    string edge_id = "PIPE_" + node_from[i] + "_to_" + node_to[i];
	    file_out << "\t\t<edge>\r\n\t\t\t<id>\r\n\t\t\t\t"+id[i]+"\r\n\t\t\t</id>\r\n\t\t\t<name>\r\n\t\t\t\t"+edge_id+"\r\n\t\t\t</name>\r\n\t\t\t<color>\r\n\t\t\t\t0\r\n\t\t\t</color>\r\n\t\t\t<counter>\r\n\t\t\t\t19\r\n\t\t\t</counter>\r\n\t\t\t<user1>\r\n\t\t\t\t0\r\n\t\t\t</user1>\r\n\t\t\t<user2>\r\n\t\t\t\t0\r\n\t\t\t</user2>\r\n\t\t\t<zone>\r\n\t\t\t\t0\r\n\t\t\t</zone>\r\n\t\t\t<pipe_type>\r\n\t\t\t\tpipe\r\n\t\t\t</pipe_type>\r\n\t\t\t<node_from>\r\n\t\t\t\t"+node_from[i]+"\r\n\t\t\t</node_from>\r\n\t\t\t<node_to>\r\n\t\t\t\t"+node_to[i]+"\r\n\t\t\t</node_to>\r\n\t\t\t<density>\r\n\t\t\t\t1000\r\n\t\t\t</density>\r\n\t\t\t<aref>\r\n\t\t\t\t7.85398e-03\r\n\t\t\t</aref>\r\n\t\t\t<mass_flow_rate>\r\n\t\t\t\t0.00\r\n\t\t\t</mass_flow_rate>\r\n\t\t\t<volume_flow_rate>\r\n\t\t\t\t0.00\r\n\t\t\t</volume_flow_rate>\r\n\t\t\t<velocity>\r\n\t\t\t\t0.00\r\n\t\t\t</velocity>\r\n\t\t\t<headloss>\r\n\t\t\t\t0.00\r\n\t\t\t</headloss>\r\n\t\t\t<head_loss_per_unit_length>\r\n\t\t\t\t0.00\r\n\t\t\t</head_loss_per_unit_length>\r\n\t\t\t<travel_time>\r\n\t\t\t\t0.00000e+00\r\n\t\t\t</travel_time>\r\n\t\t\t<concentration>\r\n\t\t\t\t0.0000\r\n\t\t\t</concentration>\r\n\t\t\t<edge_spec>\r\n\t\t\t\t<pipe>\r\n\t\t\t\t\t<length>\r\n\t\t\t\t\t\t"+to_string(l[i])+"\r\n\t\t\t\t\t</length>\r\n\t\t\t\t\t<diameter>\r\n\t\t\t\t\t\t"+to_string(d[i])+"\r\n\t\t\t\t\t</diameter>\r\n\t\t\t\t\t<roughness>\r\n\t\t\t\t\t\t"+to_string(friction_coeff)+"\r\n\t\t\t\t\t</roughness>\r\n\t\t\t\t\t<friction_coeff>\r\n\t\t\t\t\t\t"+to_string(friction_coeff)+"\r\n\t\t\t\t\t</friction_coeff>\r\n\t\t\t\t\t<cl_k>\r\n\t\t\t\t\t\t0.016\r\n\t\t\t\t\t</cl_k>\r\n\t\t\t\t\t<cl_w>\r\n\t\t\t\t\t\t0.0\r\n\t\t\t\t\t</cl_w>\r\n\t\t\t\t</pipe>\r\n\t\t\t</edge_spec>\r\n\t\t</edge>\r\n";
  	}
  }
  // Printing gate valves
  for(int i=0; i<id_gv.size(); i++){
	  int color;
  	if(!gv_erase[i]){
	  	if(e[i]==0)
	  		color = -16776961;
	  	else
	  		color = -65536;
	  }else
	  	color = -16711936;
		string edge_id = "VALVE_" + node_from_gv[i] + "_to_" + node_to_gv[i];
	  file_out << "\t\t<edge>\r\n\t\t\t<id>\r\n\t\t\t\t"+id_gv[i]+"\r\n\t\t\t</id>\r\n\t\t\t<name>\r\n\t\t\t\t"+id_gv[i]+"\r\n\t\t\t</name>\r\n\t\t\t<color>\r\n\t\t\t\t"+to_string(color)+"\r\n\t\t\t</color>\r\n\t\t\t<counter>\r\n\t\t\t\t543\r\n\t\t\t</counter>\r\n\t\t\t<pipe_type>\r\n\t\t\t\tvalve\r\n\t\t\t</pipe_type>\r\n\t\t\t<node_from>\r\n\t\t\t\t"+node_from_gv[i]+"\r\n\t\t\t</node_from>\r\n\t\t\t<node_to>\r\n\t\t\t\t"+node_to_gv[i]+"\r\n\t\t\t</node_to>\r\n\t\t\t<density>\r\n\t\t\t\t1000\r\n\t\t\t</density>\r\n\t\t\t<aref>\r\n\t\t\t\t3,1416e-04\r\n\t\t\t</aref>\r\n\t\t\t<mass_flow_rate>\r\n\t\t\t\t0.00000e+00\r\n\t\t\t</mass_flow_rate>\r\n\t\t\t<volume_flow_rate>\r\n\t\t\t\t0.00000e+00\r\n\t\t\t</volume_flow_rate>\r\n\t\t\t<velocity>\r\n\t\t\t\t0.00000e+00\r\n\t\t\t</velocity>\r\n\t\t\t<headloss>\r\n\t\t\t\t0.00\r\n\t\t\t</headloss>\r\n\t\t\t<head_loss_per_unit_length>\r\n\t\t\t\t0.00\r\n\t\t\t</head_loss_per_unit_length>\r\n\t\t\t<travel_time>\r\n\t\t\t\t0.00000e+00\r\n\t\t\t</travel_time>\r\n\t\t\t<concentration>\r\n\t\t\t\t0\r\n\t\t\t</concentration>\r\n\t\t\t<edge_spec>\r\n\t\t\t\t<valve>\r\n\t\t\t\t\t<position>\r\n\t\t\t\t\t\t"+to_string(e[i])+"\r\n\t\t\t\t\t</position>\r\n\t\t\t\t\t<adzeta>\r\n\t\t\t\t\t\t"+to_string(e[i]*1e8)+"\r\n\t\t\t\t\t</adzeta>\r\n\t\t\t\t\t<curve>\r\n\t\t\t\t\t\t<id>\r\n\t\t\t\t\t\t\tcurve\r\n\t\t\t\t\t\t</id>\r\n\t\t\t\t\t\t<x_val>\r\n\t\t\t\t\t\t\te\r\n\t\t\t\t\t\t</x_val>\r\n\t\t\t\t\t\t<x_dim>\r\n\t\t\t\t\t\t\tpercent\r\n\t\t\t\t\t\t</x_dim>\r\n\t\t\t\t\t\t<y_val>\r\n\t\t\t\t\t\t\tdzeta\r\n\t\t\t\t\t\t</y_val>\r\n\t\t\t\t\t\t<y_dim>\r\n\t\t\t\t\t\t\tPa*s2/kg2\r\n\t\t\t\t\t\t</y_dim>\r\n\t\t\t\t\t\t<points>\r\n\t\t\t\t\t\t\t<point_x>\r\n\t\t\t\t\t\t\t\t0,0000\r\n\t\t\t\t\t\t\t</point_x>\r\n\t\t\t\t\t\t\t<point_y>\r\n\t\t\t\t\t\t\t\t0,0000\r\n\t\t\t\t\t\t\t</point_y>\r\n\t\t\t\t\t\t\t<point_x>\r\n\t\t\t\t\t\t\t\t100,0000\r\n\t\t\t\t\t\t\t</point_x>\r\n\t\t\t\t\t\t\t<point_y>\r\n\t\t\t\t\t\t\t\t1e10\r\n\t\t\t\t\t\t\t</point_y>\r\n\t\t\t\t\t\t</points>\r\n\t\t\t\t\t</curve>\r\n\t\t\t\t</valve>\r\n\t\t\t</edge_spec>\r\n\t\t</edge>\r\n";
  }
  file_out << "\t</edges>\r\n";

  file_out << "</staci>\r\n";
  file_out.close();
  cout << "\n[*]Writing to file: OK" << endl;
  cout << endl << "[*]File has been created successfully, name: " << case_name + ".spr" << endl;

  cout <<"\n    =========\n    |SUMMARY|\n    =========\n";
  cout << "\nLoaded pipes:         " << id.size(); 
  cout << "\nFiltered1 pipes:      " << pipe_erased;
  cout << "\nFiltered2 pipes:      " << filter2_no;
  cout << "\nLoaded gate valves:   " << id_gv.size() << endl;
  cout << "\nNumber of pipes:      " << id.size()-pipe_erased-filter2_no;
  cout << "\nNumber of nodes:      " << node.size();
  cout << "\nNumber of placed gv:  " << id_gv.size() - gv_erased;

  cout << "\n\nOverall CPU time:     " << time(0)-ido << " s\n" << endl;

  return 0;
}

int VectorAbsMax(const vector<int> &v){
  int max;
  if(v.size()>0){
    max=abs(v[0]);
    for(int i=0; i<v.size(); i++)
      if(abs(v[i])>max)
        max = abs(v[i]);
  }else{
    cout << "\nERROR! VectorAbsMax: vector is empty! Exiting..." << endl;
    exit(-1);
  }
  return max;
}

double VectorMax(const vector<double> &v){
  double max;
  if(v.size()>0){
    max=v[0];
    for(int i=0; i<v.size(); i++)
      if(v[i]>max)
        max = v[i];
  }else{
    cout << "\nERROR! VectorMax: vector is empty! Exiting..." << endl;
    exit(-1);
  }
  return max;
}

double VectorMin(const vector<double> &v){
  double max;
  if(v.size()>0){
    max=v[0];
    for(int i=0; i<v.size(); i++)
      if(v[i]<max)
        max = v[i];
  }else{
    cout << "\nERROR! VectorMax: vector is empty! Exiting..." << endl;
    exit(-1);
  }
  return max;
}

vector<int> VectorAbsUnique(vector<int> &v){
  vector<int> w;
  if(v.size()>0){
    w.push_back(v[0]);
    for(int i=1; i<v.size(); i++){
      bool unique=true;
      for(int j=0; j<w.size(); j++)
        if(abs(w[j])==abs(v[i]))
          unique=false;
      if(unique)
        w.push_back(v[i]);
    }
  }else{
    cout << "\nERROR! VectorUnique: vector is empty! Exiting..." << endl;
    exit(-1);
  }
  return w;
}

double VectorAverage(const vector<double> &v){
  double m=0.;
  if(v.size()>0){
    for(int i=0; i<v.size(); i++)
      m+=v[i];
  }else{
    cout << "\nERROR! VectorAverage: vector is empty! Exiting..." << endl;
    exit(-1);
  }
  return m/(double)v.size();
}
