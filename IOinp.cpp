#include "IOinp.h"

//--------------------------------------------------------------------------------
IOinp::IOinp(string fileName){
	inpFileName = fileName;
}

//--------------------------------------------------------------------------------
void IOinp::loadSystem(vector<Node *> &nodes, vector<Edge *> &edges){
	
	bool do_print = false; // printing to terminal
	double dem_pat_scal = 1.0; // scaling the demands 
	int warning_counter=0;

	// FOR NODES
	vector<string> node_name, node_dem_pat;
	vector<double> elev, demand, xcoord, ycoord;

	// FOR PIPES
	vector<string> pipe_name, node_from, node_to;
	vector<double> l, D, roughness;

	// FOR PRES
	vector<string> pres_name, pres_node_from, pres_node_to;
	vector<double> pres_p;

	// FOR POOLS
	vector<string> pool_name, pool_node_from, pool_node_to;
	vector<double> pool_botlev, pool_watlev, pool_aref;

	// FOR PUMPS
	vector<string> pump_name, pump_node_from, pump_node_to, pump_par;
	vector<vector<double> > pump_cv_x, pump_cv_y;

	// FOR VALVES
	vector<string> valve_name, valve_node_from, valve_node_to, valve_type;
	vector<double> valve_d, valve_set;

	// FOR DEMAND PATTERNS
	vector<string> dem_pat_id;
	vector<vector<double> > dem_pat;

	// FOR CURVES
	vector<string> curve;
	vector<vector<double> > curve_x, curve_y;

	// SETTINGS
	string flow_unit;
	string friction_model;

// ################
// READING THE FILE
// ################
	ifstream file_in;
  file_in.open(inpFileName+".inp");
  string line;
  if(file_in.is_open()){
	  while(getline(file_in,line)){
			if(line.substr(0,11) == "[JUNCTIONS]"){
				while(true){
					getline(file_in,line);
					if(line.length()<=1 || line.substr(0,5) == "[END]")
						break;
					if(line[0] != ';'){
						vector<string> sv=line2sv(line);
						node_name.push_back("NODE_"+sv[0]);
						elev.push_back(stod(sv[1],0));
						demand.push_back(stod(sv[2],0));
						xcoord.push_back(0.);ycoord.push_back(0.);
						if(sv.size()==4)
							node_dem_pat.push_back("DEM_PAT"+sv[3]);
						else
							node_dem_pat.push_back("DEFAULT");
					}
				}
			}
			if(line.substr(0,12) == "[RESERVOIRS]"){
				while(true){
					getline(file_in,line);
					if(line.length()<=1 || line.substr(0,5) == "[END]")
						break;
					if(line[0] != ';'){
						vector<string> sv=line2sv(line);
						pres_name.push_back("PRES_"+sv[0]);
						pres_p.push_back(stod(sv[1],0));
						// Creating the node_name
						node_name.push_back("NODE_"+sv[0]);
						elev.push_back(-1.); // no data in inp, will be equal to other end's
						demand.push_back(0.);
						xcoord.push_back(0.);ycoord.push_back(0.);
						node_dem_pat.push_back("DEFAULT");
						pres_node_to.push_back(node_name.back()+"_END");
						pres_node_from.push_back(node_name.back());
						node_name.push_back("NODE_"+sv[0]+"_END");
						elev.push_back(-1.);
						demand.push_back(0.);
						xcoord.push_back(0.);ycoord.push_back(0.);
						node_dem_pat.push_back("DEFAULT");
					}
				}
			}

			if(line.substr(0,7) == "[TANKS]"){
				while(true){
					getline(file_in,line);
					if(line.length()<=1 || line.substr(0,5) == "[END]")
						break;
					if(line[0] != ';'){
						vector<string> sv=line2sv(line);
						pool_name.push_back("POOL_"+sv[0]);
						pool_botlev.push_back(stod(sv[1],0));
						pool_watlev.push_back(stod(sv[2],0));
						double d = stod(sv[5],0);
						pool_aref.push_back(d*d*M_PI/4);
						// TODO: minlevel, maxlevel etc.
						// Creating the node_name
						node_name.push_back("NODE_"+sv[0]);
						elev.push_back(stod(sv[1],0));
						demand.push_back(0.);
						xcoord.push_back(0.);ycoord.push_back(0.);
						node_dem_pat.push_back("DEFAULT");
						pool_node_to.push_back(node_name.back()+"_END");
						pool_node_from.push_back(node_name.back());
						node_name.push_back("NODE_"+sv[0]+"_END");
						elev.push_back(stod(sv[1],0));
						demand.push_back(0.);
						xcoord.push_back(0.);ycoord.push_back(0.);
						node_dem_pat.push_back("DEFAULT");
					}
				}
			}

			if(line.substr(0,7) == "[PIPES]"){
				while(true){
					getline(file_in,line);
					if(line.length()<=1 || line.substr(0,5) == "[END]")
						break;
					if(line[0] != ';'){
						vector<string> sv=line2sv(line);
						pipe_name.push_back("PIPE_"+sv[0]);
						node_from.push_back("NODE_"+sv[1]);
						node_to.push_back("NODE_"+sv[2]);
						l.push_back(stod(sv[3],0));
						D.push_back(stod(sv[4],0));
						roughness.push_back(stod(sv[5],0));
					}
				}
			}

			if(line.substr(0,7) == "[PUMPS]"){
				while(true){
					getline(file_in,line);
					if(line.length()<=1 || line.substr(0,5) == "[END]")
						break;
					if(line[0] != ';'){
						vector<string> sv=line2sv(line);
						pump_name.push_back("PUMP_"+sv[0]);
						pump_node_from.push_back("NODE_"+sv[1]);
						pump_node_to.push_back("NODE_"+sv[2]);
						vector<double> v;
						pump_cv_x.push_back(v);
						pump_cv_y.push_back(v);
						if(sv.size()<=4){
							cout << "\n !WARNING! No pump_name curve, or power is defined for " << "PUMP_"+sv[0] << endl;
							warning_counter++;
						}
						if(sv.size()==5){
							if(sv[3]=="HEAD")
								pump_par.push_back("CV_"+sv[4]);
						} // TODO later: add POWER, SPEED etc.
					}
				}
			}

			if(line.substr(0,8) == "[VALVES]"){
				while(true){
					getline(file_in,line);
					if(line.length()<=1 || line.substr(0,5) == "[END]")
						break;
					if(line[0] != ';'){
						vector<string> sv=line2sv(line);
						valve_name.push_back("VALVE_"+sv[0]);
						valve_node_from.push_back("NODE_"+sv[1]);
						valve_node_to.push_back("NODE_"+sv[2]);
						valve_d.push_back(stod(sv[3],0));
						valve_type.push_back(sv[4]);
						valve_set.push_back(stod(sv[5],0));
					}
				}
			}

			if(line.substr(0,10) == "[PATTERNS]"){
				vector<double> v;
				while(true){
					getline(file_in,line);
					if(line[0] != ';'){
						if(line.length()<=1 || line.substr(0,5) == "[END]"){
							dem_pat.push_back(v);
							break;
						}
						vector<string> sv=line2sv(line);
						if(dem_pat_id.size()==0){
							dem_pat_id.push_back("DP_"+sv[0]);
							for(vector<string>::iterator i=sv.begin()+1; i!=sv.end(); i++)
								v.push_back(stod(*i,0));
						}else{
							if("DP_"+sv[0] == dem_pat_id.back()){
								for(vector<string>::iterator i=sv.begin()+1; i!=sv.end(); i++)
									v.push_back(stod(*i,0));
							}else{
								dem_pat.push_back(v);
								v.clear();
								dem_pat_id.push_back("DP_"+sv[0]);
								for(vector<string>::iterator i=sv.begin()+1; i!=sv.end(); i++)
									v.push_back(stod(*i,0));
							}
						}
					}
				}
			}

			// pump_name CURVES
			if(line.substr(0,8) == "[CURVES]"){
				vector<double> x,y;
				while(true){
					getline(file_in,line);
					if(line[0] != ';'){
						if(line.length()<=1 || line.substr(0,5) == "[END]"){
							curve_x.push_back(x);
							curve_y.push_back(y);
							break;
						}
						vector<string> sv=line2sv(line);
						if(curve.size()==0){
							curve.push_back("CV_"+sv[0]);
							x.push_back(stod(sv[1],0));
							y.push_back(stod(sv[2],0));
						}else{
							if("CV_"+sv[0] == curve.back()){
								x.push_back(stod(sv[1],0));
								y.push_back(stod(sv[2],0));
							}else{
								curve_x.push_back(x);
								curve_y.push_back(y);
								x.clear(); y.clear();
								curve.push_back("CV_"+sv[0]);
								x.push_back(stod(sv[1],0));
								y.push_back(stod(sv[2],0));
							}
						}
					}
				}
			}

			// BASIC SETTINGS
			if(line.substr(0,9) == "[OPTIONS]"){
			//TODO There are more options that might be intresting later
				while(true){
					getline(file_in,line);
					if(line.length()<=1 || line.substr(0,5) == "[END]")
						break;
					if(line[0] != ';'){
						vector<string> sv=line2sv(line);
						if(sv.size()>=2){
							if(sv[0] == "Units"){
								flow_unit = sv[1];
							}
							if(sv[0] == "Headloss"){ //TODO D-W, C-W
								if(sv[1] == "H-W")
									friction_model = "HW";
								if(sv[1] == "D-W")
									friction_model = "DW";
							}
						}
					}
				}
			}

			if(line.substr(0,13) == "[COORDINATES]"){
				while(true){
					getline(file_in,line);
					if(line.length()<=1 || line.substr(0,5) == "[END]")
						break;
					if(line[0] != ';'){
						vector<string> sv=line2sv(line);
						int idx=-1;
						for(int i=0; i<node_name.size(); i++)
							if(node_name[i]=="NODE_"+sv[0]){
								idx = i;
								break;
							}
						if(idx==-1){
							cout << "\n !WARNING! node_name not found for coordinate " << "NODE_"+sv[0] << endl;
							warning_counter++;
						}
						else{
							xcoord[idx] = stod(sv[1],0);
							ycoord[idx] = stod(sv[2],0);
						}
					}
				}
			}
	  }
  }else{
		cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!ERROR!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl << "epanet2staci(), File is not open when calling epanet2staci() function!!! file: " << caseName << endl;
		exit(-1);
  }
  file_in.close();
  cout << "\n[*]Reading EPANET file: OK" << endl;

// #######################
// POSTPROCESSING THE DATA
// #######################	
  //Setting the elevation for PRES nodes
  for(int i=0; i<pres_name.size(); i++){
  	for(int j=0; j<pipe_name.size(); j++){
 			if(node_from[j].substr(5,node_from[j].length()-5)==pres_name[i].substr(5,pres_name[i].length()-5)){
 				int idx_to=-1, idx_from=-1;
 				for(int k=0; k<node_name.size(); k++){
 					if(node_to[j]==node_name[k])
 						idx_to = k;
 					if(node_from[j]==node_name[k])
 						idx_from = k;
 					if(idx_from != -1 && idx_to != -1)
 						break;
 				}
 				elev[idx_from] = elev[idx_to];
 				elev[idx_from+1] = elev[idx_to];
 				break;
 			}
 			if(node_to[j].substr(5,node_to[j].length()-5)==pres_name[i].substr(5,pres_name[i].length()-5)){
 				int idx_to=-1, idx_from=-1;
 				for(int k=0; k<node_name.size(); k++){
 					if(node_to[j]==node_name[k])
 						idx_to = k;
 					if(node_from[j]==node_name[k])
 						idx_from = k;
 					if(idx_from != -1 && idx_to != -1)
 						break;
 				}
 				elev[idx_to] = elev[idx_from];
 				elev[idx_to+1] = elev[idx_from];
 				break;
 			}
  	}
  	for(int j=0; j<pump_name.size(); j++){
 			if(pump_node_from[j].substr(5,pump_node_from[j].length()-5)==pres_name[i].substr(5,pres_name[i].length()-5)){
 				int idx_to=-1, idx_from=-1;
 				for(int k=0; k<node_name.size(); k++){
 					if(pump_node_to[j]==node_name[k])
 						idx_to = k;
 					if(pump_node_from[j]==node_name[k])
 						idx_from = k;
 					if(idx_from != -1 && idx_to != -1)
 						break;
 				}
 				elev[idx_from] = elev[idx_to];
 				elev[idx_from+1] = elev[idx_to];
 				break;
 			}
 			if(pump_node_to[j].substr(5,pump_node_to[j].length()-5)==pres_name[i].substr(5,pres_name[i].length()-5)){
 				int idx_to=-1, idx_from=-1;
 				for(int k=0; k<node_name.size(); k++){
 					if(pump_node_to[j]==node_name[k])
 						idx_to = k;
 					if(pump_node_from[j]==node_name[k])
 						idx_from = k;
 					if(idx_from != -1 && idx_to != -1)
 						break;
 				}
 				elev[idx_to] = elev[idx_from];
 				elev[idx_to+1] = elev[idx_from];
 				break;
 			}
 		}
  }

  // Setting x-y coordinates for dummy nodes of pools and press
  for(int i=0; i<node_name.size(); i++){
  	if(x[i]==0){
  		xcoord[i] = xcoord[i-1];
  		ycoord[i] = ycoord[i-1];
  	}
  }

  // Matching curves to pumps
  for(int i=0; i<pump_name.size(); i++){
  	for(int j=0; j<curve.size(); j++){
  		if(pump_par[i]==curve[j]){
  			for(int k=0; k<curve_x[j].size(); k++){
  				pump_cv_x[i].push_back(curve_x[j][k]);
  				pump_cv_y[i].push_back(curve_y[j][k]);
  			}
  		}
  	}
  }

  // Modifying pump_name curves, STACI must have at least 3 points
  for(int i=0; i<pump_name.size(); i++)
  	if(pump_cv_x[i].size()<=2){
  		cout << "\n !WARNING! pump_name " << pump_name[i] << "does not have enough points. STACI needs at least 3 points\n";
  		warning_counter++;
  	}

  cout << "\nflow_unit: " << flow_unit << endl;
  // Converting units from EPANET to STACI (m3/h)
  double dem_unit;
  string unit;
  // US units
  if(flow_unit == "CFS"){ // cubic feet seconds
  	dem_unit = 101.940648;
  	unit = "US";
  }
  if(flow_unit == "GPM"){ // gallon(US) per minute
  	dem_unit = 0.227125;
  	unit = "US";
  }
  if(flow_unit == "MGD"){ // million gallon(US) per day
  	dem_unit = 157.72549166667;
  	unit = "US";
  }
  if(flow_unit == "IMGD"){ // imperial million gallon per day
  	dem_unit = 189.42041666667;
  	unit = "US";
  }
  if(flow_unit == "AFD"){ // acre-feet / day
  	dem_unit = 51.4;
  	unit = "US";
  }
  // NORMAL units
  if(flow_unit == "LPS"){ // litre per sec
  	dem_unit = 3.6;
  	unit = "SI";
  }
  if(flow_unit == "LPM"){ // litre per minute
  	dem_unit = 0.06;
  	unit = "SI";
  }
  if(flow_unit == "MLD"){ // million litre per day
  	dem_unit = 1000./24.;
  	unit = "SI";
  }
  if(flow_unit == "CMH"){ // cubic meter per hour
  	dem_unit = 1.;
  	unit = "SI";
  }
  if(flow_unit == "CMD"){ // cubic meter per day
  	dem_unit = 1./24.;
  	unit = "SI";
  }

  for(int i=0; i<node_name.size(); i++)
  	demand[i] = demand[i]*dem_unit*dem_pat_scal;
  if(unit=="US"){
		for(int i=0; i<node_name.size(); i++)
	  		elev[i] = elev[i]*0.3048; // feet to meter
	  for(int i=0; i<pipe_name.size(); i++){
  		l[i] = l[i]*0.3048; // feet to meter
  		D[i] = D[i]*0.0254; // inches to meter
  		 // TODO Roughness in D-W
  	}
		for(int i=0; i<pres_name.size(); i++)
		  pres_p[i] = pres_p[i]*0.3048; // feet to meter
		for(int i=0; i<pool_name.size(); i++){
			pool_botlev[i] = pool_botlev[i]*0.3048; // feet to meter
			pool_watlev[i] = pool_watlev[i]*0.3048; // feet to meter
			pool_aref[i] = pool_aref[i]*0.3048*0.3048; // square feet to square meter
	  }
	  for(int i=0; i<valve_name.size(); i++)
  		valve_d[i] = valve_d[i]*0.0254; // inches to meter
	  for(int i=0; i<pump_name.size(); i++){
  		for(int j=0; j<pump_cv_x[i].size(); j++){
  			pump_cv_x[i][j] = pump_cv_x[i][j]*dem_unit; // feet to meter
  			pump_cv_y[i][j] = pump_cv_y[i][j]*0.3048; // feet to meter
  		}
	  }
  }

	if(unit=="SI"){
		for(int i=0; i<pipe_name.size(); i++)
	  	D[i] = D[i]/1000.; // millimeter to meter
	  for(int i=0; i<valve_name.size(); i++)
  		valve_d[i] = valve_d[i]/1000.; // millimeter to meter
	}

	for(int i=0; i<pres_name.size(); i++){
		int idx=-1;
		for(int j=0; j<node_name.size(); j++)
			if(node_name[j]==pres_node_from[i])
				idx = j;

		pres_p[i] = (pres_p[i]-elev[idx])*9.81*1000; // meter to Pascal minus elevation
	}

  cout << "\n[*]Processing data: OK" << endl;


	// ########################
	// CREATING THE NODES/PIPES
	// ########################

	for(int i=0; i<node_name.size(); i++)
	{
	  nodes.push_back(new Node(node_name[i], xcoord[i], ycoord[i], elev[i], demand[i], 0.0, 1000.));
	}
	for(int i=0; i<pres_name.size(); i++)
	{	
		int idx = nodes.size()+i;
	  nodes.push_back(new Node(node_name[idx], xcoord[idx], ycoord[idx], elev[idx], demand[idx], 0.0, 1000.));
	  edges.push_back(new PressurePoint(pres_name[i], 1.0, pres_node_from[idx], 1000., pres_p[i], 0.0));
	}
	for(int i=0; i<pool_name.size(); i++)
	{
		int idx = nodes.size() + pres_name.size() + i;
	  nodes.push_back(new Node(node_name[idx], xcoord[idx], ycoord[idx], elev[idx], demand[idx], 0.0, 1000.));
	  edges.push_back(new Pool(pool_name[i], pool_node_from[idx], 1000., 1.0, pool_botlev[i], pool_watlev[i], 0.0));
	}

}

//--------------------------------------------------------------------------------
vector<string> IOinp::line2sv(string line){
	string s="";
	vector<string> sv;
	for(string::iterator i=line.begin(); i!=line.end(); i++){
		if(*i!=' ' && *i!='\t' && *i!=';' && *i!='\n' && *i!='\r')
			s += *i;
		else if(s.length()>0){
			sv.push_back(s);
			s="";
		}
	}
	if(s!="")
		sv.push_back(s);
	return sv;
}
