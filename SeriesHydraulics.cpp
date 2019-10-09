#include "SeriesHydraulics.h"

//-------------------------------------------------------------------
SeriesHydraulics::SeriesHydraulics(string fileName) : HydraulicSolver(fileName){
  loadTimeSettings();  
  patternIDtoIndex(); // FILLING THE PATTERNS TO THE NODES
}

//-------------------------------------------------------------------
SeriesHydraulics::~SeriesHydraulics(){}

//-------------------------------------------------------------------
void SeriesHydraulics::seriesSolve()
{
	initialization();

	bool convOk = true;
	time = startClock;
	clockTime = startClock;
	while(time<=duration && convOk)
	{
		updateDemand();
		convOk = solveSystem();

		// printing basic info to consol
		seriesInfo();	
		//cout << endl << edges[1]->name << "  wl: " << edges[1]->getDoubleProperty("waterLevel")/0.3048 << "  st: " << edges[1]->status << endl;
		//cout << edges[15]->name << "  vf: " << edges[15]->volumeFlowRate*15.8504 << " v: " << edges[15]->getEdgeDoubleProperty("velocity")/0.3048 << "  st: " << edges[15]->setting/0.7032 << "  status: " << edges[15]->status << endl;
		//cout << nodes[4]->name << "  p: " << nodes[4]->head/0.7032 << endl;
		//cout << nodes[4]->name << "  p: " << (nodes[4]->head+nodes[4]->geodeticHeight)/0.3048 << endl;
		//cout << nodes[8]->name << "  p: " << (nodes[8]->head+nodes[8]->geodeticHeight)/0.3048 << endl;
		//cout << edges[15]->name << "  vf:" << edges[15]->volumeFlowRate*15.8504 << "  st: " << edges[15]->status << endl;
		//cin.get();

		// saving head, consumptions for nodes and flows for edges
		saveOutput();

		hydraulicTimeStep = newHydraulicTimeStep();
		time += hydraulicTimeStep;

		// updateing the settings, pools etc.
		updatePool();
		updateControl();
		//updateRule();

		clockTime = time;
		while(clockTime>=86400.)
			clockTime -= 86400.;
	}

	if(!convOk)
		cout << endl << "!ERROR! Convergence was not achieved in solveSystem!" << endl;
}

//-------------------------------------------------------------------
void SeriesHydraulics::saveOutput()
{
	for(int i=0; i<numberNodes; i++)
	{
		nodes[i]->vectorHead.push_back(nodes[i]->getProperty("head"));
		nodes[i]->vectorConsumption.push_back(nodes[i]->getProperty("consumption"));
	}
	for(int i=0; i<numberEdges; i++)
	{
		edges[i]->vectorVolumeFlowRate.push_back(edges[i]->getDoubleProperty("volumeFlowRate"));
	}
}

//-------------------------------------------------------------------
void SeriesHydraulics::updateDemand()
{	
	for(int i=0; i<numberNodes; i++)
	{
		double demand=0.;
		for(int j=0; j<nodes[i]->vectorDemand.size(); j++)
		{
			if(nodes[i]->vectorPatternIndex[j] == -1)
				demand += nodes[i]->vectorDemand[j];
			else{
				int timeIndex = time/patternTimeStep;
				int ptIndex = nodes[i]->vectorPatternIndex[j];
				// making patternvalues periodic
				while(timeIndex>=patternValue[ptIndex].size())
					timeIndex -= patternValue[ptIndex].size();
				demand += nodes[i]->vectorDemand[j]*patternValue[ptIndex][timeIndex];
			}
		}
		//nodes[i]->setProperty("demand",demand);
		nodes[i]->demand = demand;
	}
}

//-------------------------------------------------------------------
double SeriesHydraulics::newHydraulicTimeStep()
{	
	double dt = hydraulicTimeStepOriginal, dt2 = hydraulicTimeStepOriginal;

	// Checking the controls
	for(int i=0; i<controlEdgeID.size(); i++)
	{
		if(controlType[i] == "Pool")
		{
			double wl = edges[controlNodeIndex[i]]->getDoubleProperty("waterLevel");
			double aref = edges[controlNodeIndex[i]]->referenceCrossSection;
			double set = controlValue[i];
			double vf = edges[controlNodeIndex[i]]->volumeFlowRate;

			if(edges[controlNodeIndex[i]]->volumeFlowRate > 0. && controlAbove[i]) // FILLING THE TANK
				dt2 = (set - wl) * aref * 1000./vf;
			else if(edges[controlNodeIndex[i]]->volumeFlowRate < 0. && !controlAbove[i])
				dt2 = (set - wl) * aref * 1000./vf;

			if(dt2<0.)
				cout << endl << "!WARNING! Something is wrong, dt is negative :(";
			else if(dt2<dt)
				dt = dt2;
		}
		else
			cout << endl << "!WARNING! [CONTROLS] control type unkown: " << controlType[i] << endl;
	}

	// Checking the tank fillups
	for(int i=0; i<numberEdges; i++)
	{
		if(edges[i]->type == "Pool" && edges[i]->status == 1)
		{
			if(edges[i]->volumeFlowRate > 0.) // VF > 0. -> FILLING UP
				dt2 = (edges[i]->getDoubleProperty("maxLevel")-edges[i]->getDoubleProperty("waterLevel")) * edges[i]->referenceCrossSection * 1000. / edges[i]->volumeFlowRate;
			else if(edges[i]->volumeFlowRate < 0.) // VF < 0. -> EMPTYING
				dt2 = (edges[i]->getDoubleProperty("minLevel")-edges[i]->getDoubleProperty("waterLevel")) * edges[i]->referenceCrossSection * 1000. / edges[i]->volumeFlowRate;
			if(dt2<0.)
				cout << endl << "!WARNING! Something is wrong, dt is negative :(";
			else if(dt2<dt)
				dt = dt2;
		}
	}

	// setting back timestep to hydraulicTimeStep*i where i is integer
	dt2 = 0.;
	while(dt2 < time)
		dt2 += hydraulicTimeStepOriginal;
	double res = dt2 - time;
	if(res > 1e-1 && dt == hydraulicTimeStepOriginal) // there was a change in timeStep before
		dt = res;

	// avoiding too small time steps
	if(dt < hydraulicTimeStepOriginal/100.)
		dt = hydraulicTimeStepOriginal/100.;

	return dt;
}

//-------------------------------------------------------------------
void SeriesHydraulics::updatePool()
{
	for(int i=0; i<numberEdges; i++)
	{	
		if(edges[i]->type == "Pool")
		{
			if(edges[i]->status == 0) // CLOSED i.e. FULL
			{
				//if(!nodes[edges[i]->getEdgeIntProperty("startNodeIndex")]->isClosed)
				//{
					if(nodes[edges[i]->startNodeIndex]->head < edges[i]->getDoubleProperty("waterLevel"))
						edges[i]->status = 1;
				//}
			}
			else // OPEN
			{
				double waterLevel = edges[i]->getDoubleProperty("waterLevel");
				waterLevel += edges[i]->volumeFlowRate/1000. * hydraulicTimeStep / edges[i]->referenceCrossSection;
				if(waterLevel <= edges[i]->getDoubleProperty("minLevel"))
					edges[i]->setDoubleProperty("waterLevel",edges[i]->getDoubleProperty("minLevel"));
				else if(waterLevel >= edges[i]->getDoubleProperty("maxLevel")){
					edges[i]->setDoubleProperty("waterLevel",edges[i]->getDoubleProperty("maxLevel"));
					edges[i]->status = 0;
				}
				else
					edges[i]->setDoubleProperty("waterLevel",waterLevel);
			}
		}
	}
}

//-------------------------------------------------------------------
void SeriesHydraulics::updateControl()
{
	for(int i=0; i<controlEdgeID.size(); i++)
	{
		if(controlType[i] == "Pool")
		{		
			if(controlAbove[i]){
				if(edges[controlNodeIndex[i]]->getDoubleProperty("waterLevel") > controlValue[i] - headTolerance)
					edges[controlEdgeIndex[i]]->status = controlStatus[i];
					//changeEdgeStatus(controlEdgeIndex[i], controlStatus[i]);
			}
			else{
				if(edges[controlNodeIndex[i]]->getDoubleProperty("waterLevel") < controlValue[i] + headTolerance)
					edges[controlEdgeIndex[i]]->status = controlStatus[i];
					//changeEdgeStatus(controlEdgeIndex[i], controlStatus[i]);
			}
		}
		//TODO
		//else if(controlType[i] == "Node")
		//{
		//}
		else
			cout << endl << "!WARNING! [CONTROLS] control type unkown: " << controlType[i] << endl;
	}
}

//-------------------------------------------------------------------
void SeriesHydraulics::updateRule()
{
	for(int i=0; i<rules.size(); i++)
	{	
		// HANDLING CONDITIONS
		bool cond = true;
		for(int j=0; j<rules[i]->conditionID.size(); j++)
		{
			if(rules[i]->conditionType[j] == "SYSTEM")
			{
				if(rules[i]->conditionID[j] == "CLOCKTIME")
				{
					cond *= booleanWithString(rules[i]->conditionRelation[j], clockTime, rules[i]->conditionValue[j]);
				}
			}
			else if(rules[i]->conditionType[j] == "NODE")
			{
				cond *= booleanWithString(rules[i]->conditionRelation[j], edges[rules[i]->conditionIndex[j]]->getDoubleProperty("waterLevel"), rules[i]->conditionValue[j]);
			}
		}

		// EXECUTING THE ACTIONS
		for(int j=0; j<rules[i]->actionID.size(); j++)
		{
			if(cond*!rules[i]->actionCounter[j]) // THEN
			{

			}
			if(!cond*rules[i]->actionCounter[j]) // ELSE
			{

			}
		}



	}
}

//-------------------------------------------------------------------
bool SeriesHydraulics::booleanWithString(string op, double left, double right)
{
	if((op == "<=") && (left <= right))
		return true;
	else
		return false;
	if((op == ">=") && (left >= right))
		return true;
	else
		return false;
	if((op == "<") && (left < right))
		return true;
	else
		return false;
	if((op == ">") && (left > right))
		return true;
	else
		return false;
	if((op == "=") && (left == right))
		return true;
	else
		return false;
	if((op == "ABOVE") && (left > right))
		return true;
	else
		return false;
	if((op == "BELOW") && (left < right))
		return true;
	else
		return false;
}

//-------------------------------------------------------------------
void SeriesHydraulics::seriesInfo()
{
	cout << endl << endl;
	int hour = time/3600.;
	int minute = (time - hour*3600.)/60.;
	if(minute < 10)
		printf(" [*] time: %3i:0%1i", hour, minute);
	else
		printf(" [*] time: %3i:%2i", hour, minute);
}

//-------------------------------------------------------------------
void SeriesHydraulics::loadTimeSettings()
{
	// FOR DEMAND PATTERNS

	ifstream fileIn;
  fileIn.open(getDefinitionFile());
  string line;
  if(fileIn.is_open())
  {
	  while(getline(fileIn,line))
	  {
	  	// LOADING THE DEMANDS AND PATTERNS FOR SERIES CALC
	  	if(line.substr(0,11) == "[JUNCTIONS]"){
	  		int i=0;
				while(true){
					getline(fileIn,line);
					if(line.length()<=1 || line.substr(0,5) == "[END]")
						break;
					if(line[0] != ';'){
						vector<string> sv=line2sv(line);
						if(sv.size()==3){
							nodes[i]->vectorDemand.push_back(demandUnit*stod(sv[2],0));
							nodes[i]->vectorPatternID.push_back("");
						}
						if(sv.size()>3){
							nodes[i]->vectorDemand.push_back(demandUnit*stod(sv[2],0));
							nodes[i]->vectorPatternID.push_back(sv[3]);
						}
						i++;
					}
				}
			}

			// LOADING THE DEMANDS FOR THE NODES
			// Here we load every demand with pattern data
	  	if(line.substr(0,9) == "[DEMANDS]"){
				while(true){
					getline(fileIn,line);
					if(line.length()<=1 || line.substr(0,5) == "[END]")
						break;
					if(line[0] != ';'){
						vector<string> sv=line2sv(line);
						if(sv.size()>=2)
						{
							string id = sv[0];
							for(int i=0; i<numberNodes; i++)
							{
								if(id == nodes[i]->getName())
								{
									nodes[i]->vectorDemand.push_back(demandUnit*stod(sv[1],0));
									if(sv.size()>=3)
										nodes[i]->vectorPatternID.push_back(sv[2]);
									else
										nodes[i]->vectorPatternID.push_back("");
									break;
								}
								if(i==numberNodes-1)
									cout << "\n!WARNING! demand ID " << id << " was not found" << endl;
							}
						}
					}
				}
			}

	  	// LOADING THE DEMANDS AND PATTERNS FOR SERIES CALC
			if(line.substr(0,10) == "[PATTERNS]")
			{
				vector<double> v;
				while(true)
				{
					getline(fileIn,line);
					if(line[0] != ';')
					{
						if(line.length()<=1 || line.substr(0,5) == "[END]")
						{
							patternValue.push_back(v);
							break;
						}
						vector<string> sv=line2sv(line);
						if(patternID.size()==0)
						{
							patternID.push_back(sv[0]);
							for(vector<string>::iterator i=sv.begin()+1; i!=sv.end(); i++)
								v.push_back(stod(*i,0));
						}
						else
						{
							if(sv[0] == patternID.back())
							{
								for(vector<string>::iterator i=sv.begin()+1; i!=sv.end(); i++)
									v.push_back(stod(*i,0));
							}
							else
							{
								patternValue.push_back(v);
								v.clear();
								patternID.push_back(sv[0]);
								for(vector<string>::iterator i=sv.begin()+1; i!=sv.end(); i++)
									v.push_back(stod(*i,0));
							}
						}
					}
				}
			}
			// TIME SETTINGS OF SERIES CALCULATIONS
			if(line.substr(0,7) == "[TIMES]")
			{
				while(true)
				{
					getline(fileIn,line);
					if(line.length()<=1 || line.substr(0,5) == "[END]")
						break;
					if(line[0] != ';'){
						vector<string> sv=line2sv(line);
						if(sv.size()>=2){
							if(sv[0] == "Duration" || sv[0] == "DURATION"){
								if(sv.size() == 2)
									duration = timeToSeconds(sv[1],"");
								else
									duration = timeToSeconds(sv[1],sv[2]);
							}
							if((sv[0] == "HYDRAULIC" && sv[1] == "TIMESTEP") || (sv[0] == "Hydraulic" && sv[1] == "Timestep")){
								if(sv.size() == 3)
									hydraulicTimeStepOriginal = timeToSeconds(sv[2],"");
								else
									hydraulicTimeStepOriginal = timeToSeconds(sv[2],sv[3]);
							}
							if((sv[0] == "PATTERN" && sv[1] == "TIMESTEP") || (sv[0] == "Pattern" && sv[1] == "Timestep")){
								if(sv.size() == 3)
									patternTimeStep = timeToSeconds(sv[2],"");
								else
									patternTimeStep = timeToSeconds(sv[2],sv[3]);
							}
							if((sv[0] == "START" && sv[1] == "CLOCKTIME") || (sv[0] == "Start" && sv[1] == "ClockTime")){
								if(sv.size() == 3)
									startClock = timeToSeconds(sv[2],"");
								else
									startClock = timeToSeconds(sv[2],sv[3]);
							}
						}
					}
				}
			}
	  	// LOADING CONTROLS
  		// TODO: do this in more general way, handling time contorls as well (maybe struct or class)
	  	if(line.substr(0,10) == "[CONTROLS]"){
				while(true){
					getline(fileIn,line);
					if(line.length()<=1 || line.substr(0,5) == "[END]")
						break;
					if(line[0] != ';'){
						vector<string> sv=line2sv(line);
						controlEdgeID.push_back(sv[1]);
						for(int i=0; i<numberEdges; i++){
							if(controlEdgeID.back() == edges[i]->getEdgeStringProperty("name")){
								controlEdgeIndex.push_back(i);
								break;
							}
							if(i == numberEdges-1)
								cout << endl << "!WARNING! Edge in [CONTROLS] " << controlEdgeID.back() << " was not found in edge list." << endl;
						}

						if(sv[2] == "OPEN" || sv[2] == "Open" || sv[2] == "open")
							controlStatus.push_back(true);
						else if(sv[2] == "CLOSED" || sv[2] == "Close" || sv[2] == "close")
							controlStatus.push_back(false);
						else{
							controlStatus.push_back(true);
							cout << endl << "!WARNING! [CONTROLS] Unkown input: " << sv[2] << " at " << sv[1] << ". Changed to OPEN." << endl;
						}

						controlNodeID.push_back(sv[5]);
						//for(int i=0; i<numberNodes; i++)
						//	if(controlNodeID.back() == nodes[i]->getName()){
						//		controlNodeIndex.push_back(i);
						//		controlType.push_back("Node");
						//	}
						for(int i=0; i<numberEdges; i++)
							if(edges[i]->getEdgeStringProperty("type") == "Pool")
								if(controlNodeID.back() == edges[i]->getEdgeStringProperty("name")){
									controlNodeIndex.push_back(i);
									controlType.push_back("Pool");
									break;
								}
						if(controlNodeIndex.size() != controlNodeID.size())
							cout << endl << "!WARNING! Node in [CONTROLS] " << controlNodeID.back() << " was not found in node list." << endl;

						if(sv[6] == "ABOVE" || sv[6] == "Above" || sv[6] == "above")
							controlAbove.push_back(true);
						else if(sv[6] == "BELOW" || sv[6] == "Below" || sv[6] == "below")
							controlAbove.push_back(false);
						else{
							controlAbove.push_back(true);
							cout << endl << "!WARNING! [CONTROLS] Unkown input: " << sv[6] << " at " << sv[0] << "  " << sv[1] << ". Changed to ABOVE." << endl;
						}

						if(unit == "SI")
							controlValue.push_back(stod(sv[7],0));
						else // US
							controlValue.push_back(0.3048*stod(sv[7],0));
					}
				}
			}

			// RULES
			// TODO: lot of other setting/cases are not considered yet
			// TODO: LOADING NOT OKAY FOR ANDs!
			if(line.substr(0,7) == "[RULES]"){
				int numberRule=-1;
				while(true){
					getline(fileIn,line);
					if(line.length()<=1 || line.substr(0,5) == "[END]")
						break;
					if(line[0] != ';'){
						vector<string> sv=line2sv(line);
						if(sv[0] == "RULE"){
							rules.push_back(new Rule(sv[1]));
							numberRule++;
						}
						if(sv[0] == "IF" || sv[0] == "AND"){
							if(sv[1] == "SYSTEM"){
								rules[numberRule]->conditionType.push_back(sv[1]);
								rules[numberRule]->conditionID.push_back(sv[2]);
								rules[numberRule]->conditionIndex.push_back(-1); // We do not need this in this case
								rules[numberRule]->conditionRelation.push_back(sv[3]);
								rules[numberRule]->conditionValue.push_back(timeToSeconds(sv[4],sv[5]));
							}
							if(sv[1] == "NODE"){
								rules[numberRule]->conditionType.push_back(sv[1]);
								rules[numberRule]->conditionID.push_back(sv[2]);
								rules[numberRule]->conditionIndex.push_back(edgeIDtoIndex(sv[2]));
								rules[numberRule]->conditionRelation.push_back(sv[4]);
								rules[numberRule]->conditionValue.push_back(stod(sv[5],0));
							}
						}
						if(sv[0] == "THEN" || sv[0] == "ELSE"){
							rules[numberRule]->actionType.push_back(sv[1]);
							rules[numberRule]->actionID.push_back(sv[2]);
							if(sv[3] == "SETTING"){
								rules[numberRule]->actionSetting.push_back(true);
								rules[numberRule]->actionValue.push_back(stod(sv[5],0));
							}
							else{
								rules[numberRule]->actionSetting.push_back(false);
								if(sv[5] == "OPEN")
									rules[numberRule]->actionValue.push_back(1.);
								else
									rules[numberRule]->actionValue.push_back(-1.);
							}
							if(sv[0] == "THEN")
								rules[numberRule]->actionCounter.push_back(false);
							else
								rules[numberRule]->actionCounter.push_back(true);
						}
					}
				}
			}
		}//end of while(getline)
	}//end of if(fileIn.is_open())
} 

//-------------------------------------------------------------------
double SeriesHydraulics::timeToSeconds(string s1, string s2)
{
	double time;
	if(s2 == "") // HANDLING 12:00, 14:30, 09:45 etc.
	{
		time = stod(s1.substr(0,s1.find(":")),0) * 3600. + stod(s1.substr(s1.find(":")+1,s1.length()),0) * 60.;
	}
	else // HANDLING 9:45 AM, 2 Days, 4 Hours etc.
	{
		if(s2 == "DAYS" || s2 == "Days" || s2 == "days")
			time = stod(s1,0) * 86400.;
		else if(s2 == "HOURS" || s2 == "Hours" || s2 == "hours")
			time = stod(s1,0) * 3600.;
		else if(s2 == "MIN" || s2 == "Min" || s2 == "min")
			time = stod(s1,0) * 60.;
		else if(s2 == "AM" || s2 == "am")
		{
			double t1 = stod(s1.substr(0,s1.find(":")),0);
			if(t1 >= 12.)
				t1 -= 12.;
			if(s1.find(":") <= s1.length())
				time =  t1* 3600. + stod(s1.substr(s1.find(":")+1,s1.length()),0) * 60.;
			else
				time =  t1* 3600.;
		}
		else if(s2 == "PM" || s2 == "pm")
		{
			double t1 = stod(s1.substr(0,s1.find(":")),0);
			t1 += 12.;
			if(s1.find(":") <= s1.length())
				time =  t1* 3600. + stod(s1.substr(s1.find(":")+1,s1.length()),0) * 60.;
			else
				time =  t1* 3600.;
		}
		else
		{
			cout << endl << "!WARNING! Unkown input: " << s2 << "\nAvailable inputs: DAYS | HOURS | MIN | AM | PM" << endl;
			time = 0.;
		}
	}
	return time;
}

//-------------------------------------------------------------------
void SeriesHydraulics::patternIDtoIndex()
{
 	for(int i=0; i<numberNodes; i++)
 	{
 		for(int j=0; j<nodes[i]->vectorPatternID.size(); j++)
 		{	
 			nodes[i]->vectorPatternIndex.resize(nodes[i]->vectorPatternID.size(),-1);
 			for(int k=0; k<patternID.size(); k++)
 			{
 				if(nodes[i]->vectorPatternID[j] == patternID[k])
 				{
 					nodes[i]->vectorPatternIndex[j] = k;
 					break;
 				}
 			}
 		}
 	}
}
