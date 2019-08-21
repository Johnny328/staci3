#include "Staci.h"

//--------------------------------------------------------------------------------
void Staci::loadSystem(){
	
	double dem_pat_scal = 1.0; // scaling the demands 
	int warning_counter=0;
	double density = 1000.;

	// FOR NODES
	vector<string> node_name;
	vector<double> elev, demand, xcoord, ycoord;

	// FOR PIPES
	vector<string> pipe_name, node_from, node_to;
	vector<double> l, D, roughness;

	// FOR PRES
	vector<string> pres_name, pres_node_from, pres_node_to;
	vector<double> pres_p;

	// FOR POOLS
	vector<string> pool_name, pool_node_from, pool_node_to;
	vector<double> pool_botlev, pool_watlev, pool_aref, pool_minlev, pool_maxlev;

	// FOR PUMPS
	vector<string> pump_name, pump_node_from, pump_node_to, pump_par;
	vector<vector<double> > pump_cv_x, pump_cv_y;

	// FOR VALVES
	vector<string> valve_name, valve_node_from, valve_node_to, valve_type;
	vector<double> valve_d, valve_set;

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
  file_in.open(definitionFile);
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
						node_name.push_back(sv[0]);
						elev.push_back(stod(sv[1],0));
						if(sv.size()>2)
							demand.push_back(stod(sv[2],0));
						else
							demand.push_back(0.);
						xcoord.push_back(-1.);ycoord.push_back(-1.);
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
						pres_name.push_back(sv[0]);
						pres_p.push_back(stod(sv[1],0));
						// Creating the node_name
						node_name.push_back(sv[0]);
						elev.push_back(-1.); // no data in inp, will be equal to other end's
						demand.push_back(0.);
						xcoord.push_back(-1.);ycoord.push_back(-1.);
						pres_node_to.push_back(node_name.back()+"_END");
						pres_node_from.push_back(node_name.back());
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
						pool_name.push_back(sv[0]);
						pool_botlev.push_back(stod(sv[1],0));
						pool_watlev.push_back(stod(sv[2],0));
						pool_minlev.push_back(stod(sv[3],0));
						pool_maxlev.push_back(stod(sv[4],0));
						double d = stod(sv[5],0);
						pool_aref.push_back(d*d*M_PI/4);
						// Creating the node_name
						node_name.push_back(sv[0]);
						elev.push_back(stod(sv[1],0));
						demand.push_back(0.);
						xcoord.push_back(-1.);ycoord.push_back(-1.);
						pool_node_to.push_back(node_name.back()+"_END");
						pool_node_from.push_back(node_name.back());
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
						pipe_name.push_back(sv[0]);
						node_from.push_back(sv[1]);
						node_to.push_back(sv[2]);
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
						pump_name.push_back(sv[0]);
						pump_node_from.push_back(sv[1]);
						pump_node_to.push_back(sv[2]);
						vector<double> v;
						pump_cv_x.push_back(v);
						pump_cv_y.push_back(v);
						if(sv.size()<=4){
							cout << "\n !WARNING! No pump_name curve, or power is defined for " << sv[0] << endl;
							warning_counter++;
						}
						if(sv.size()==5){
							if(sv[3]=="HEAD")
								pump_par.push_back(sv[4]);
						} // TODO later: add POWER, SPEED etc.
					}
				}
			}

			// LOADING THE DEMANDS FOR THE NODES
			// Here only the sum is considered, the patterns are dealt in SeriesHydraulics
	  	if(line.substr(0,9) == "[DEMANDS]"){
				while(true){
					getline(file_in,line);
					if(line.length()<=1 || line.substr(0,5) == "[END]")
						break;
					if(line[0] != ';'){
						vector<string> sv=line2sv(line);
						if(sv.size()>=2)
						{
							string id = sv[0];
							for(int i=0; i<node_name.size(); i++)
							{
								if(id == node_name[i])
								{
									demand[i] += stod(sv[1],0);
									break;
								}
								if(i==node_name.size()-1)
									cout << "\n!WARNING! demand ID " << id << " was not found" << endl;
							}
						}
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
						valve_name.push_back(sv[0]);
						valve_node_from.push_back(sv[1]);
						valve_node_to.push_back(sv[2]);
						valve_d.push_back(stod(sv[3],0));
						valve_type.push_back(sv[4]);
						valve_set.push_back(stod(sv[5],0));
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
							curve.push_back(sv[0]);
							x.push_back(stod(sv[1],0));
							y.push_back(stod(sv[2],0));
						}else{
							if(sv[0] == curve.back()){
								x.push_back(stod(sv[1],0));
								y.push_back(stod(sv[2],0));
							}else{
								curve_x.push_back(x);
								curve_y.push_back(y);
								x.clear(); y.clear();
								curve.push_back(sv[0]);
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
							if(sv[0] == "Units" || sv[0] == "UNITS"){
								flow_unit = sv[1];
							}
							if(sv[0] == "Headloss" || sv[0] == "HEADLOSS"){ //TODO D-W, C-W
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
							if(node_name[i]==sv[0]){
								idx = i;
								break;
							}
						if(idx==-1){
							cout << "\n !WARNING! node_name not found for coordinate " << sv[0] << endl;
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
  }
  else
  {
		cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!ERROR!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl << "epanet2staci(), File is not open when calling epanet2staci() function!!! file: " << definitionFile << endl;
		exit(-1);
  }
  file_in.close();

// #######################
// POSTPROCESSING THE DATA
// #######################	

  //Setting the elevation for PRES nodes
  for(int i=0; i<pres_name.size(); i++){
  	// if it is connected with pipe
  	for(int j=0; j<pipe_name.size(); j++){
 			if(node_from[j]==pres_name[i]){
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
 				//elev[idx_from+1] = elev[idx_to];
 				break;
 			}
 			if(node_to[j]==pres_name[i]){
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
 				//elev[idx_to+1] = elev[idx_from];
 				break;
 			}
  	}
  	// if it is connected with pump
  	for(int j=0; j<pump_name.size(); j++){
 			if(pump_node_from[j]==pres_name[i]){
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
 				//elev[idx_from+1] = elev[idx_to];
 				break;
 			}
 			if(pump_node_to[j]==pres_name[i]){
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
 				//elev[idx_to+1] = elev[idx_from];
 				break;
 			}
 		}
  }


  // Setting x-y coordinates for dummy nodes of pools and press
  for(int i=0; i<node_name.size(); i++){
  	if(xcoord[i]==-1.){
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


  // Converting units from EPANET to STACI (litre/sec)
  double dem_unit;
  string unit;
  // US units
  if(flow_unit == "CFS"){ // cubic feet seconds
  	dem_unit = 28.316846592;
  	unit = "US";
  }
  if(flow_unit == "GPM"){ // gallon(US) per minute
  	dem_unit = 0.06309;
  	unit = "US";
  }
  if(flow_unit == "MGD"){ // million gallon(US) per day
  	dem_unit = 43.8126364;
  	unit = "US";
  }
  if(flow_unit == "IMGD"){ // imperial million gallon per day
  	dem_unit = 52.6168042;
  	unit = "US";
  }
  if(flow_unit == "AFD"){ // acre-feet / day
  	dem_unit = 14.2764102;
  	unit = "US";
  }
  // NORMAL units
  if(flow_unit == "LPS"){ // litre per sec
  	dem_unit = 1.0;
  	unit = "SI";
  }
  if(flow_unit == "LPM"){ // litre per minute
  	dem_unit = 1./60.;
  	unit = "SI";
  }
  if(flow_unit == "MLD"){ // million litre per day
  	dem_unit = 11.5740741;
  	unit = "SI";
  }
  if(flow_unit == "CMH"){ // cubic meter per hour
  	dem_unit = 1./3.6;
  	unit = "SI";
  }
  if(flow_unit == "CMD"){ // cubic meter per day
  	dem_unit = 0.0115740741;
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
			pool_minlev[i] = pool_minlev[i]*0.3048; // feet to meter
			pool_maxlev[i] = pool_maxlev[i]*0.3048; // feet to meter
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
  else if(unit == "SI"){
  	for(int i=0; i<pipe_name.size(); i++)
	  	D[i] /= 1000.;
  }

	for(int i=0; i<pres_name.size(); i++){
		int idx=-1;
		for(int j=0; j<node_name.size(); j++)
			if(node_name[j]==pres_node_from[i])
				idx = j;

		pres_p[i] = (pres_p[i]-elev[idx])*9.81*1000.; // meter to Pascal minus elevation
	}

	// ########################
	// CREATING THE NODES/PIPES
	// ########################
	for(int i=0; i<node_name.size(); i++)
	{
	  nodes.push_back(new Node(node_name[i], xcoord[i], ycoord[i], elev[i], demand[i], 0.0, density));
	}
	for(int i=0; i<pres_name.size(); i++)
	{	
	  edges.push_back(new PressurePoint(pres_name[i], 1.0, pres_node_from[i], density, pres_p[i], 0.0));
	}
	for(int i=0; i<pool_name.size(); i++)
	{
	  edges.push_back(new Pool(pool_name[i], pool_node_from[i], density, pool_aref[i], pool_botlev[i], pool_watlev[i], 0.0));
	  edges[edges.size()-1]->setDoubleProperty("minLevel",pool_minlev[i]);
	  edges[edges.size()-1]->setDoubleProperty("maxLevel",pool_maxlev[i]);
	}
	for(int i=0; i<pipe_name.size(); i++)
	{
    	edges.push_back(new Pipe(pipe_name[i], node_from[i], node_to[i], density, l[i], D[i], roughness[i], 0.0));
    	edges[edges.size()-1]->setFrictionModel(friction_model);
	}
	for(int i=0; i<pump_name.size(); i++)
	{
    	edges.push_back(new Pump(pump_name[i], pump_node_from[i], pump_node_to[i], density, 1.0, pump_cv_x[i], pump_cv_y[i], 0.0));
	}
	for(int i=0; i<valve_name.size(); i++)
	{
		vector<double> valve_cv_x,valve_cv_y;
		valve_cv_x.push_back(0); valve_cv_y.push_back(0);
		valve_cv_x.push_back(100); valve_cv_y.push_back(1e10);
    	edges.push_back(new Valve(valve_name[i], valve_node_from[i], valve_node_to[i], density, valve_d[i]*valve_d[i]*M_PI/4., valve_cv_x, valve_cv_y, 0.0, 0.0));
	}
}

//-------------------------------------------------------------------
void Staci::saveSystem(vector<Node *> &nodes, vector<Edge *> &edges, string frictionModel){
  // Adding the elevation to pressure points
  for(int i=0; i<edges.size(); i++){
    if(edges[i]->getEdgeStringProperty("type") == "PressurePoint"){
      int idx=-1;
      for(int j=0; j<nodes.size(); j++)
        if(nodes[j]->getName()==edges[i]->getEdgeStringProperty("startNodeName"))
          idx = j;

    edges[i]->setDoubleProperty("head", (edges[i]->getDoubleProperty("head") + nodes[idx]->getProperty("height"))); // meter to Pascal minus elevation
    }
  }

  // Start of writing the file
  FILE *wfile;
  wfile = fopen(definitionFile.c_str(),"w");

  fprintf(wfile, "[TITLE]\n");
  fprintf(wfile, "%s\n", (definitionFile + "\n").c_str());

  // finding the nodes with pressurepoint or pool
  vector<bool> isRealNode(nodes.size(),true);
  for(int i=0; i<nodes.size(); i++)
  {
    for(int j=0; j<edges.size(); j++)
    {
      string type = edges[j]->getEdgeStringProperty("type");
      if(type == "PressurePoint" || type == "Pool")
      {
        if(nodes[i]->getName() == edges[j]->getEdgeStringProperty("startNodeName"))
        {
          isRealNode[i] = false;
          if(nodes[i]->getProperty("demand") != 0)
            cout << "!WARNING! Pressure point " << nodes[i]->getName() << " DOES have non-zero demand " << nodes[i]->getProperty("demand") << endl;
          string node_name = nodes[i]->getName();
          nodes[i]->setName(edges[j]->getEdgeStringProperty("name"));
          for(int k=0; k<edges.size(); k++)
          {
            if(node_name == edges[k]->getEdgeStringProperty("startNodeName"))
              edges[k]->setEdgeStringProperty("startNodeName",nodes[i]->getName());
            if(node_name == edges[k]->getEdgeStringProperty("endNodeName"))
              edges[k]->setEdgeStringProperty("endNodeName",nodes[i]->getName());
          }
        }
      }
    }
  }

  // TODO: Demand pattern
  fprintf(wfile, "\n[JUNCTIONS]\n");
  fprintf(wfile, ";ID              \tElev        \tDemand      \tPattern         \n");
  for(int i=0; i<nodes.size(); i++){
    if(isRealNode[i])
    	fprintf(wfile, " %-16s\t%-12.4f\t%-12.6f\t%-16s\t;\n", nodes[i]->getName().c_str(),nodes[i]->getProperty("geodeticHeight"),nodes[i]->getProperty("demand"),"");
  }

  fprintf(wfile, "\n[RESERVOIRS]\n");
  fprintf(wfile, ";ID              \tHead        \tPattern         \n");
  for(int i=0; i<edges.size(); i++){
    if(edges[i]->getEdgeStringProperty("type")=="PressurePoint"){
      fprintf(wfile," %-16s\t%-12.4f\t%-16s\n",edges[i]->getEdgeStringProperty("name").c_str(),edges[i]->getDoubleProperty("height") + edges[i]->getDoubleProperty("head"),"");
    }
  }

  // TODO: min/max level
  fprintf(wfile, "\n[TANKS]\n");
  fprintf(wfile, ";ID             \tElevation   \tInitLevel   \tMinLevel    \tMaxLevel    \tDiameter    \tMinVol      \tVolCurve\n");
  for(int i=0; i<edges.size(); i++){
    if(edges[i]->getEdgeStringProperty("type")=="Pool")
      fprintf(wfile, " %-16s\t%-12.4f\t%-12.4f\t%-12.4f\t%-12.4f\t%-12.4f\t%-12.4f\t%-16s\t\n", edges[i]->getEdgeStringProperty("name").c_str(),edges[i]->getDoubleProperty("bottomLevel"),edges[i]->getDoubleProperty("waterLevel"),0.0,1000.0,pow(edges[i]->getDoubleProperty("referenceCrossSection"),.5)*4./M_PI,0.,"");
  }

  fprintf(wfile, "\n[PIPES]\n"); // diameter is in mm in INP, while it is in m in SPR
  fprintf(wfile, ";ID             \tNode1           \tNode2           \tLength      \tDiameter    \tRoughness   \tMinorLoss   \tStatus\n");
  for(int i=0; i<edges.size(); i++){
    if(edges[i]->getEdgeStringProperty("type") == "Pipe")
      fprintf(wfile, " %-16s\t%-16s\t%-16s\t%-12.4f\t%-12.4f\t%-12.4f\t%-12.4f\t%-6s;\n",edges[i]->getEdgeStringProperty("name").c_str(),edges[i]->getEdgeStringProperty("startNodeName").c_str(),edges[i]->getEdgeStringProperty("endNodeName").c_str(),edges[i]->getDoubleProperty("length"),edges[i]->getDoubleProperty("diameter")*1000.,edges[i]->getDoubleProperty("roughness"),0.0,"Open");
    //if(edges[i]->getEdgeStringProperty("type") == "PressurePoint")
    //  fprintf(wfile, " %-16s\t%-16s\t%-16s\t%-12.4f\t%-12.4f\t%-12.4f\t%-12.4f\t%-6s;\n",("PIPE_" + edges[i]->getEdgeStringProperty("name")).c_str(),edges[i]->getEdgeStringProperty("name").c_str(),edges[i]->getEdgeStringProperty("startNodeName").c_str(),0.0,1.0,0.0,0.0,"Open");
  }
  // TODO: Pump curves

  fprintf(wfile, "\n[PUMPS]\n");
  fprintf(wfile, ";ID             \tNode1           \tNode2           \tParameters\n");
  int pump_counter=0;
  for(int i=0; i<edges.size(); i++){
    if(edges[i]->getEdgeStringProperty("type") == "Pump"){
      fprintf(wfile, " %-16s\t%-16s\t%-16s\t%-6s;\n", edges[i]->getEdgeStringProperty("name").c_str(), edges[i]->getEdgeStringProperty("startNodeName").c_str(), edges[i]->getEdgeStringProperty("endNodeName").c_str(), ("HEAD "+to_string(pump_counter)).c_str());
      pump_counter++;
    }
  }

  // TODO: Different type of valves
  fprintf(wfile, "\n[VALVES]\n");
  fprintf(wfile, ";ID             \tNode1           \tNode2           \tDiameter    \tType\tSetting     \tMinorLoss   \n");
  for(int i=0; i<edges.size(); i++){
    if(edges[i]->getEdgeStringProperty("type") == "Valve")
      fprintf(wfile, " %-16s\t%-16s\t%-16s\t%-12.4f\t%-4s\t%-12.4f\t%-12.4f\n", edges[i]->getEdgeStringProperty("name").c_str(), edges[i]->getEdgeStringProperty("startNodeName").c_str(), edges[i]->getEdgeStringProperty("endNodeName").c_str(), pow(edges[i]->getEdgeDoubleProperty("referenceCrossSection"),.5)*4/M_PI, "TCV",0.0,0.0);
  }

  fprintf(wfile, "\n[TAGS]\n");

  fprintf(wfile, "\n[DEMANDS]\n");
  fprintf(wfile, ";Junction       \tDemand      \tPattern         \tCategory\n");

  // TODO: ADD STATUS i.e. pumps/valves are closed or open
  fprintf(wfile, "\n[STATUS]\n");
  fprintf(wfile, ";ID              \tStatus/Setting\n");

  // TODO: ADD PATTERNS
  fprintf(wfile, "\n[PATTERNS]\n");
  fprintf(wfile, ";ID             \tMultipliers\n");

  // TODO: PUMP CURVES
  fprintf(wfile, "\n[CURVES]\n");
  fprintf(wfile, ";ID             \tX-Value     \tY-Value\n");
  pump_counter = 0;
  for(int i=0; i<edges.size(); i++){
    if(edges[i]->getEdgeStringProperty("type") == "Pump"){
      vector <double> pumpCurveVolumeFlowRate = edges[i]->getVectorProperty("volumeFlowRate");
      vector <double> pumpCurveHead = edges[i]->getVectorProperty("head");
      for(int j=0; j<pumpCurveVolumeFlowRate.size(); j++)
        fprintf(wfile, " %-16i\t%-12.4f\t%-12.4f\n", pump_counter, pumpCurveVolumeFlowRate[j], pumpCurveHead[j]);
      pump_counter++;
    }
  }

  fprintf(wfile, "\n[CONTROLS]\n");

  fprintf(wfile, "\n[RULES]\n");

  fprintf(wfile, "\n[ENERGY]\n");

  fprintf(wfile, "\n[EMITTERS]\n");

  fprintf(wfile, "\n[QUALITY]\n");

  fprintf(wfile, "\n[SOURCES]\n");

  fprintf(wfile, "\n[REACTIONS]\n");

  fprintf(wfile, "\n[MIXING]\n");

  // TODO: for extended calculations
  fprintf(wfile, "\n[TIMES]\n");

  fprintf(wfile, "\n[REPORT]\n");

  fprintf(wfile, "\n[OPTIONS]\n");
  fprintf(wfile, " Units              \tLPS\n");
  if(frictionModel == "HW")
    fprintf(wfile, " Headloss           \tH-W\n");
  else if(frictionModel == "DW")
    fprintf(wfile, " Headloss           \tD-W\n");
  else
    cout << endl << "WARNING, unkown friction model: " << frictionModel << endl;
  fprintf(wfile, " Specific Gravity   \t1.0\n");
  fprintf(wfile, " Viscosity          \t1.0\n");
  fprintf(wfile, " Trials             \t40\n");
  fprintf(wfile, " Accuracy           \t0.001\n");
  fprintf(wfile, " CHECKFREQ          \t2\n");
  fprintf(wfile, " MAXCHECK           \t10\n");
  fprintf(wfile, " DAMPLIMIT          \t0\n");
  fprintf(wfile, " Unbalanced         \tContinue 10\n");
  fprintf(wfile, " Pattern            \t1\n");
  fprintf(wfile, " Demand Multiplier  \t1.0\n");
  fprintf(wfile, " Emitter Exponent   \t0.5\n");
  fprintf(wfile, " Quality            \tChlorine mg/L\n");
  fprintf(wfile, " Diffusivity        \t1.0\n");
  fprintf(wfile, " Tolerance          \t0.01\n");

  // TODO: xpos or xcoord??
  fprintf(wfile, "\n[COORDINATES]\n");
  fprintf(wfile, ";Node           \tX-Coord         \tY-Coord\n");
  for(int i=0; i<nodes.size(); i++){
    fprintf(wfile, " %-16s\t%-16.4f\t%-16.4f\n", nodes[i]->getName().c_str(), nodes[i]->getProperty("xPosition"), nodes[i]->getProperty("yPosition"));
  }

  fprintf(wfile, "\n[VERTICES]\n");
  fprintf(wfile, "\n[LABELS]\n");
  fprintf(wfile, "\n[BACKDROP]\n");

  // End of writing the file
  fprintf(wfile, "\n[END]\n");
  fclose(wfile);

  cout << endl << "[*] File written successfully: " << definitionFile << endl;
}

//--------------------------------------------------
vector<string> Staci::line2sv(string line){
	string s="";
	vector<string> sv;
	for(string::iterator i=line.begin(); i!=line.end(); i++){
		if(*i != ';'){
			if(*i!=' ' && *i!='\t' && *i!='\n' && *i!='\r')
				s += *i;
			else if(s.length()>0){
				sv.push_back(s);
				s="";
			}
		}
		else
		{
			break;
		}
	}
	if(s!="")
		sv.push_back(s);
	return sv;
}
