#include "SeriesHydraulics.h"

//-------------------------------------------------------------------
SeriesHydraulics::SeriesHydraulics(string fileName) : HydraulicSolver(fileName)
{
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
	vectorTime.push_back(time);
	clockTime = startClock;

	while(time<=duration && convOk)
	{
		// printing basic info to consol
		seriesInfo();	

		updateDemand();
		updateRule();
		convOk = solveSystem();

		// saving head, consumptions for nodes and flows for edges
		saveOutput();

		hydraulicTimeStep = newHydraulicTimeStep();
		time += hydraulicTimeStep;
		vectorTime.push_back(time);

		// updating the settings, pools etc.
		updatePool();
		updateControl();

		clockTime = time;
		while(clockTime>=86400.)
			clockTime -= 86400.;

		//cout << endl << edges[12]->name << "  vf: " << edges[12]->volumeFlowRate << "  wl: " << edges[12]->getDoubleProperty("waterLevel") << endl;
		cin.get();
	}

	if(!convOk)
		cout << endl << "!ERROR! Convergence was not achieved in solveSystem!" << endl;
}

//-------------------------------------------------------------------
void SeriesHydraulics::saveOutput()
{
	for(int i=0; i<numberNodes; i++)
	{
		nodes[i]->vectorHead.push_back(nodes[i]->head);
		nodes[i]->vectorConsumption.push_back(nodes[i]->getProperty("consumption"));
		nodes[i]->vectorStatus.push_back(nodes[i]->status);
	}
	for(int i=0; i<numberEdges; i++)
	{
		edges[i]->vectorVolumeFlowRate.push_back(edges[i]->volumeFlowRate);
		edges[i]->vectorStatus.push_back(edges[i]->status);
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
			{
				demand += nodes[i]->vectorDemand[j];
			}
			else
			{
				int timeIndex = time/patternTimeStep;
				int ptIndex = nodes[i]->vectorPatternIndex[j];
				// making patternvalues periodic
				while(timeIndex>=patternValue[ptIndex].size())
					timeIndex -= patternValue[ptIndex].size();
				demand += nodes[i]->vectorDemand[j]*patternValue[ptIndex][timeIndex];
			}
		}
		nodes[i]->demand = demand;
	}
}

//-------------------------------------------------------------------
double SeriesHydraulics::newHydraulicTimeStep()
{	
	double dt = hydraulicTimeStepOriginal, dt2 = hydraulicTimeStepOriginal;

	// Checking the [CONTROLS]
	for(int i=0; i<controlEdgeID.size(); i++)
	{
		double wl = edges[controlNodeIndex[i]]->getDoubleProperty("waterLevel");
		double aref = edges[controlNodeIndex[i]]->referenceCrossSection;
		double set = controlValue[i];
		double vf = edges[controlNodeIndex[i]]->volumeFlowRate;
		if(controlType[i] == "Pool") // && edges[i]->status == 1 ???
		{

			if(vf > 0. && controlAbove[i]) // FILLING THE TANK
				dt2 = (set - wl) * aref / vf;
			else if(vf < 0. && !controlAbove[i])
				dt2 = (set - wl) * aref / vf;

			if(dt2<0.)
				cout << endl << "!WARNING! Something is wrong, dt is negative at Control " << controlEdgeID[i] << endl;
			else if(dt2<dt)
				dt = dt2;
		}
		else
			cout << endl << "!WARNING! [CONTROLS] control type unkown: " << controlType[i] << endl;

		cout << endl << "ID: " << controlEdgeID[i] << " set: " << set << " vf: " << vf << " wl: " << wl <<  " dt : " << dt << endl;
	}

	// Checking the tank fillups
	for(int i=0; i<poolIndex.size(); i++)
	{
		int idx = poolIndex[i];
		if(edges[i]->status == 1)
		{
			double vf = edges[idx]->volumeFlowRate;
			double wl = edges[idx]->getDoubleProperty("waterLevel");
			double max_wl = edges[idx]->getDoubleProperty("maxLevel");
			double min_wl = edges[idx]->getDoubleProperty("minLevel");
			double aref = edges[idx]->referenceCrossSection;

			if(vf > 0.) // VF > 0. -> FILLING UP
				dt2 = (max_wl-wl) * aref / vf;
			else if(vf < 0.) // VF < 0. -> EMPTYING
				dt2 = (min_wl-wl) * aref / vf;

			if(dt2<0.)
				cout << endl << "!WARNING! Something is wrong, dt is negative at Tank Fillup: " << i << " th edge, name: " << edges[idx]->name << endl;
			else if(dt2<dt)
				dt = dt2;
		}

		cout << edges[idx]->name << " dt : " << dt << endl;
	}

	// Checking the [RULES]
	for(int i=0; i<rules.size(); i++)
	{
		for(int j=0; j<rules[i]->conditionID.size(); j++)
		{
			if(rules[i]->conditionID[j] == "CLOCKTIME" || rules[i]->conditionID[j] == "TIME")
			{
				double eps = 0;
				string relation = rules[i]->conditionRelation[j];
				if(relation == "<=" || relation == ">")
					eps = hydraulicTimeStepOriginal/10.;

				if(rules[i]->conditionID[j] == "CLOCKTIME")
					dt2 = rules[i]->conditionValue[j] - clockTime;
				else
					dt2 = rules[i]->conditionValue[j] - time;

				if(dt2 < dt && ((dt2 >= 0. && eps > 0.) || (dt2 > 0. && eps == 0.)))
					dt = dt2 + eps;
			}

			if(rules[i]->conditionType[j] == "NODE")
			{	
				int idx = rules[i]->conditionIndex[j];
				double set = rules[i]->conditionValue[j];
				double vf = edges[idx]->volumeFlowRate;
				double wl = edges[idx]->getDoubleProperty("waterLevel");
				double aref = edges[idx]->referenceCrossSection;

				bool above;
				string rel = rules[i]->conditionRelation[j];
				if(rel == "ABOVE" || rel == ">" || rel == ">=")
					above = true;
				else
					above = false;

				if(vf > 0. && above) // VF > 0. -> FILLING UP
					dt2 = (set-wl) * aref * 1000. / vf + hydraulicTimeStepOriginal/100.; // just to be sure we add 1/100 part of the original time step
				else if(vf < 0. && !above) // VF < 0. -> EMPTYING
					dt2 = (set-wl) * aref * 1000. / vf + hydraulicTimeStepOriginal/100.;

				bool cond = checkCondition(i, dt2);

				if(dt2<0.)
					cout << endl << "!WARNING! Something is wrong, dt is negative at Rule: " << i << ", name: " << rules[i]->ID << endl;
				else if(dt2<dt && cond)
					dt = dt2;
			}
		}
		cout << "rules dt : " << dt << endl;
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
	{
		dt = hydraulicTimeStepOriginal/100.;
		cout << endl << "!WARNING! Time step is too small, dt: " << dt << "\n System clock: " << secondsToTime(time) << endl;
	}

	return dt;
}

//-------------------------------------------------------------------
void SeriesHydraulics::updatePool()
{
	for(int i=0; i<poolIndex.size(); i++)
	{	
		int idx = poolIndex[i]; // index of pool in edges list
		double wl = edges[idx]->getDoubleProperty("waterLevel");
		double vf = edges[idx]->volumeFlowRate;
		double aref = edges[idx]->referenceCrossSection;
		double min_wl = edges[idx]->getDoubleProperty("minLevel");
		double max_wl = edges[idx]->getDoubleProperty("maxLevel");

		if(edges[idx]->status == 0) // CLOSED i.e. FULL
		{
			if(nodes[edges[idx]->startNodeIndex]->head < wl)
				edges[idx]->status = 1;
		}
		else // OPEN
		{
			wl += vf * hydraulicTimeStep / aref;
			if(wl <= min_wl)
				edges[idx]->setDoubleProperty("waterLevel",min_wl);
			else if(wl >= edges[idx]->getDoubleProperty("maxLevel"))
			{
				edges[idx]->setDoubleProperty("waterLevel",max_wl);
				edges[idx]->status = 0;
			}
			else
				edges[idx]->setDoubleProperty("waterLevel",wl);
		}
	}

	/*for(int i=0; i<numberEdges; i++)
	{	
		if(edges[i]->type == "Pool")
		{
			if(edges[i]->status == 0) // CLOSED i.e. FULL
			{
				if(nodes[edges[i]->startNodeIndex]->head < edges[i]->getDoubleProperty("waterLevel"))
					edges[i]->status = 1;
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
	}*/
}

//-------------------------------------------------------------------
void SeriesHydraulics::updateControl()
{
	for(int i=0; i<controlEdgeID.size(); i++)
	{
		if(controlType[i] == "Pool")
		{		
			if(controlAbove[i])
			{
				if(edges[controlNodeIndex[i]]->getDoubleProperty("waterLevel") > controlValue[i] - headTolerance)
				{
					edges[controlEdgeIndex[i]]->status = controlStatus[i];
				}
			}
			else
			{
				if(edges[controlNodeIndex[i]]->getDoubleProperty("waterLevel") < controlValue[i] + headTolerance)
				{
					edges[controlEdgeIndex[i]]->status = controlStatus[i];
				}
			}
		}
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
		// ONLY AND IS CONSIDERED, TODO: OR, see ROSSMANN, 2000
		bool cond = checkCondition(i, 0.);

		// EXECUTING THE ACTIONS
		for(int j=0; j<rules[i]->actionID.size(); j++)
		{
			bool act = cond*!rules[i]->actionCounter[j] + !cond*rules[i]->actionCounter[j]; // THEN + ELSE
			if(act)
			{
				if(rules[i]->actionSetting[j]) // SETTING
				{
					edges[rules[i]->actionIndex[j]]->setting = rules[i]->actionValue[j];
				}
				else // STATUS
				{
					edges[rules[i]->actionIndex[j]]->status = rules[i]->actionValue[j];
					if(edges[rules[i]->actionIndex[j]]->status == 0)
						edges[rules[i]->actionIndex[j]]->volumeFlowRate = 0.0;
				}
			}
		}
	}
}

//-------------------------------------------------------------------
bool SeriesHydraulics::checkCondition(int i, double dt)
{
	bool cond = true;
	for(int j=0; j<rules[i]->conditionID.size(); j++)
	{
		if(rules[i]->conditionType[j] == "SYSTEM")
		{
			if(rules[i]->conditionID[j] == "CLOCKTIME")
			{
				cond *= booleanWithString(rules[i]->conditionRelation[j], clockTime + dt, rules[i]->conditionValue[j]);
			}
			else if(rules[i]->conditionID[j] == "TIME")
			{
				cond *= booleanWithString(rules[i]->conditionRelation[j], time + dt, rules[i]->conditionValue[j]);
			}
		}
		else if(rules[i]->conditionType[j] == "NODE") // actually waterlevel of a tank (that is an edge in staci3)
		{
			cond *= booleanWithString(rules[i]->conditionRelation[j], edges[rules[i]->conditionIndex[j]]->getDoubleProperty("waterLevel"), rules[i]->conditionValue[j]);
		}
		else
		{
			cout << endl << "!WARNING! [Rules] Condition type is unkown: " << rules[i]->conditionType[j] << " at Rule " << rules[i]->ID << endl;
		}
	}

	return cond;
}

//-------------------------------------------------------------------
bool SeriesHydraulics::booleanWithString(string op, double left, double right)
{
	bool out = false;
	if((op == "<=") && (left <= right))
		out = true;

	if((op == ">=") && (left >= right))
		out = true;

	if((op == "<") && (left < right))
		out = true;

	if((op == ">") && (left > right))
		out = true;

	if((op == "=") && (left == right))
		out = true;

	if((op == "ABOVE") && (left > right))
		out = true;

	if((op == "BELOW") && (left < right))
		out = true;
	return out;
}

//-------------------------------------------------------------------
void SeriesHydraulics::seriesInfo()
{
	cout << endl << "[*] time: " << secondsToTime(time) << endl;
}

//-------------------------------------------------------------------
void SeriesHydraulics::loadTimeSettings()
{
	ifstream fileIn;
  fileIn.open(getDefinitionFile());
  string line;
  if(fileIn.is_open())
  {
	  getline(fileIn,line);
	  while(true)
	  {
	  	while(line[0] != '[')
	  		getline(fileIn,line);

	  	string line_old = line;

	  	if(line.length() >= 6 && line.substr(0,5) == "[END]") // reaching the end of the file
	  		break;

	  	// LOADING THE DEMANDS AND PATTERNS FOR SERIES CALC
	  	if(line.substr(0,11) == "[JUNCTIONS]")
	  	{
	  		int i=0;
				while(getline(fileIn,line))
				{
					while(line.length() <= 1) // skipping the empty lines
		  			getline(fileIn,line);

					if(line[0] == '[') // reaching the end of this section
						break;

					if(line[0] != ';')
					{
						vector<string> sv=line2sv(line);
						if(sv.size()==3)
						{
							nodes[i]->vectorDemand.push_back(demandUnit*stod(sv[2],0));
							nodes[i]->vectorPatternID.push_back("");
						}
						if(sv.size()>3)
						{
							nodes[i]->vectorDemand.push_back(demandUnit*stod(sv[2],0));
							nodes[i]->vectorPatternID.push_back(sv[3]);
						}
						i++;
					}
				}
			}

			// LOADING THE DEMANDS FOR THE NODES
			// Here we load every demand with pattern data
	  	if(line.substr(0,9) == "[DEMANDS]")
	  	{
				while(getline(fileIn,line))
				{
					while(line.length() <= 1) // skipping the empty lines
		  			getline(fileIn,line);

					if(line[0] == '[') // reaching the end of this section
						break;

					if(line[0] != ';')
					{
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
				while(getline(fileIn,line))
				{
					while(line.length() <= 1) // skipping the empty lines
		  			getline(fileIn,line);

					if(line[0] == '[') // reaching the end of this section
					{
						patternValue.push_back(v);
						break;
					}

					if(line[0] != ';')
					{
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
				while(getline(fileIn,line))
				{
					while(line.length() <= 1) // skipping the empty lines
		  			getline(fileIn,line);

					if(line[0] == '[') // reaching the end of this section
						break;

					if(line[0] != ';')
					{
						vector<string> sv=line2sv(line);
						if(sv.size()>=2)
						{
							if(sv[0] == "Duration" || sv[0] == "DURATION")
							{
								if(sv.size() == 2)
									duration = timeToSeconds(sv[1],"");
								else
									duration = timeToSeconds(sv[1],sv[2]);
							}
							if((sv[0] == "HYDRAULIC" && sv[1] == "TIMESTEP") || (sv[0] == "Hydraulic" && sv[1] == "Timestep"))
							{
								if(sv.size() == 3)
									hydraulicTimeStepOriginal = timeToSeconds(sv[2],"");
								else
									hydraulicTimeStepOriginal = timeToSeconds(sv[2],sv[3]);
							}
							if((sv[0] == "PATTERN" && sv[1] == "TIMESTEP") || (sv[0] == "Pattern" && sv[1] == "Timestep"))
							{
								if(sv.size() == 3)
									patternTimeStep = timeToSeconds(sv[2],"");
								else
									patternTimeStep = timeToSeconds(sv[2],sv[3]);
							}
							if((sv[0] == "START" && sv[1] == "CLOCKTIME") || (sv[0] == "Start" && sv[1] == "ClockTime"))
							{
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
	  	if(line.substr(0,10) == "[CONTROLS]")
	  	{
				while(getline(fileIn,line))
				{
					while(line.length() <= 1) // skipping the empty lines
		  			getline(fileIn,line);

					if(line[0] == '[') // reaching the end of this section
						break;

					if(line[0] != ';')
					{
						vector<string> sv=line2sv(line);
						controlEdgeID.push_back(sv[1]);
						for(int i=0; i<numberEdges; i++)
						{
							if(controlEdgeID.back() == edges[i]->getEdgeStringProperty("name"))
							{
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
						else
						{
							controlStatus.push_back(true);
							cout << endl << "!WARNING! [CONTROLS] Unkown input: " << sv[2] << " at " << sv[1] << ". Changed to OPEN." << endl;
						}

						controlNodeID.push_back(sv[5]);

						for(int i=0; i<numberEdges; i++)
						{
							if(edges[i]->getEdgeStringProperty("type") == "Pool")
							{
								if(controlNodeID.back() == edges[i]->getEdgeStringProperty("name"))
								{
									controlNodeIndex.push_back(i);
									controlType.push_back("Pool");
									break;
								}
							}
						}
						if(controlNodeIndex.size() != controlNodeID.size())
							cout << endl << "!WARNING! Node in [CONTROLS] " << controlNodeID.back() << " was not found in node list." << endl;

						if(sv[6] == "ABOVE" || sv[6] == "Above" || sv[6] == "above")
							controlAbove.push_back(true);
						else if(sv[6] == "BELOW" || sv[6] == "Below" || sv[6] == "below")
							controlAbove.push_back(false);
						else
						{
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
			if(line.substr(0,7) == "[RULES]"){
				int numberRule=-1;
				while(getline(fileIn,line))
				{
					while(line.length() <= 1) // skipping the empty lines
		  			getline(fileIn,line);

					if(line[0] == '[') // reaching the end of this section
						break;

					if(line[0] != ';')
					{
						vector<string> sv=line2sv(line);
						if(sv[0] == "RULE")
						{
							rules.push_back(new Rule(sv[1]));
							numberRule++;
						}
						if(sv[0] == "IF" || sv[0] == "AND") // TODO: AND / OR handling
						{
							if(sv[1] == "SYSTEM")
							{
								rules[numberRule]->conditionType.push_back(sv[1]); // SYSTEM
								rules[numberRule]->conditionID.push_back(sv[2]); // CLOCKTIME or TIME
								rules[numberRule]->conditionIndex.push_back(-1); // We do not need this in this case
								rules[numberRule]->conditionRelation.push_back(sv[3]);
								rules[numberRule]->conditionValue.push_back(timeToSeconds(sv[4],sv[5]));
							}
							if(sv[1] == "NODE")
							{
								rules[numberRule]->conditionType.push_back(sv[1]);
								rules[numberRule]->conditionID.push_back(sv[2]);
								rules[numberRule]->conditionIndex.push_back(edgeIDtoIndex(sv[2]));
								rules[numberRule]->conditionRelation.push_back(sv[4]);
								double value = stod(sv[5],0);
								if(unit == "US") // converting from feet to meter
									value = 0.3048*value;
								rules[numberRule]->conditionValue.push_back(value);
							}
						}
						if(sv[0] == "THEN" || sv[0] == "ELSE") // only one THEN action and one ELSE action is considered
						{
							rules[numberRule]->actionType.push_back(sv[1]); // LINK, NODE, PUMP
							rules[numberRule]->actionID.push_back(sv[2]); // ID of LINK, NODE, PUMP

							if(sv[1] == "LINK" || sv[1] == "PUMP")
								rules[numberRule]->actionIndex.push_back(edgeIDtoIndex(sv[2])); // Index of LINK, NODE, PUMP
							else if(sv[1] == "NODE")
								rules[numberRule]->actionIndex.push_back(nodeIDtoIndex(sv[2])); // Index of LINK, NODE, PUMP
							else
								cout << endl << "!WARNING! Unkown attribute: " << sv[2] << "  at Rule: " << rules[numberRule]->ID << endl;

							if(sv[3] == "SETTING")
							{
								rules[numberRule]->actionSetting.push_back(true);
								rules[numberRule]->actionValue.push_back(stod(sv[5],0));
							}
							else if(sv[3] == "STATUS")
							{
								rules[numberRule]->actionSetting.push_back(false);
								if(sv[5] == "OPEN")
									rules[numberRule]->actionValue.push_back(1.);
								else
									rules[numberRule]->actionValue.push_back(0.);
							}
							else
								cout << endl << "!WARNING! Unkown attribute: " << sv[3] << "  at Rule: " << rules[numberRule]->ID << endl;

							if(sv[0] == "THEN")
								rules[numberRule]->actionCounter.push_back(false);
							else if(sv[0] == "ELSE")
								rules[numberRule]->actionCounter.push_back(true);
						}
					}
				}
			}
			if(line == line_old) // [XYZ] is not intresting
				if(!getline(fileIn,line)) // if we reached the end of file break
					break;
		}//end of while(getline)
	}//end of if(fileIn.is_open())
} 

//-------------------------------------------------------------------
void SeriesHydraulics::timeTableNode(vector<string> ID, string unit)
{
	vector<int> idx;
	for(int i=0; i<ID.size(); i++)
	{	
		int k = nodeIDtoIndex(ID[i]);
		if(k!=-1)
	  	idx.push_back(k);
	}
	timeTableNode(idx,unit);
}
//-------------------------------------------------------------------
void SeriesHydraulics::timeTableNode(vector<int> idx, string unit)
{
	double convertUnit = 1.;
	if(unit == "psi")
		convertUnit = 1/0.7032;
	else unit = " m ";

	cout << endl;
	printf(" |   ID   |");
	for(int i=0; i<idx.size(); i++)
		printf(" %-19s |",nodes[idx[i]]->name.c_str());

	printf("\n |  time  |");
	for(int i=0; i<idx.size(); i++)
		printf(" head [%3s] | status |",unit.c_str());// | status |");

	printf("\n |--------");
	for(int i=0; i<idx.size(); i++)
		printf("+---------------------");
	printf("|");

	for(int i=0; i<vectorTime.size()-1; i++)
	{
		printf("\n");
		int hour = vectorTime[i]/3600.;
		int minute = (vectorTime[i] - hour*3600.)/60.;
		if(minute < 10)
			printf(" | %3i:0%1i |", hour, minute);
		else
			printf(" | %3i:%2i |", hour, minute);

		for(int j=0; j<idx.size(); j++)
		{
			printf(" %10.3f |", nodes[idx[j]]->vectorHead[i]*convertUnit);
			if(nodes[idx[j]]->vectorStatus[i] == 1)
				printf("  open  |");
			else if(nodes[idx[j]]->vectorStatus[i] == 0)
				printf(" closed |");
			else
				printf(" lofasz |");
		}
	}
	cout << endl;
}
//-------------------------------------------------------------------
void SeriesHydraulics::timeTableEdge(vector<string> ID, string unit)
{
	vector<int> idx;
	for(int i=0; i<ID.size(); i++)
	{	
		int k = edgeIDtoIndex(ID[i]);
		if(k!=-1)
	  	idx.push_back(k);
	}
	timeTableEdge(idx, unit);
}
//-------------------------------------------------------------------
void SeriesHydraulics::timeTableEdge(vector<int> idx, string unit)
{
	double convertUnit = 1000.;
	if(unit == "gpm")
		convertUnit = 1/0.06309*1000.;
	else
		unit = "lps";

	cout << endl;
	printf(" |   ID   |");
	for(int i=0; i<idx.size(); i++)
		printf(" %-19s |",edges[idx[i]]->name.c_str());

	printf("\n |  time  |");
	for(int i=0; i<idx.size(); i++)
		printf(" flow [%3s] | status |",unit.c_str());

	printf("\n |--------");
	for(int i=0; i<idx.size(); i++)
		printf("+---------------------");
	printf("|");

	for(int i=0; i<vectorTime.size()-1; i++)
	{
		printf("\n");
		int hour = vectorTime[i]/3600.;
		int minute = (vectorTime[i] - hour*3600.)/60.;
		if(minute < 10)
			printf(" | %3i:0%1i |", hour, minute);
		else
			printf(" | %3i:%2i |", hour, minute);

		for(int j=0; j<idx.size(); j++)
		{
			printf(" %10.3f |", edges[idx[j]]->vectorVolumeFlowRate[i]*convertUnit);
			if(edges[idx[j]]->vectorStatus[i] == 2)
				printf(" active |");
			else if(edges[idx[j]]->vectorStatus[i] == 1)
				printf("  open  |");
			else if(edges[idx[j]]->vectorStatus[i] == 0)
				printf(" closed |");
			else if(edges[idx[j]]->vectorStatus[i] == -1)
				printf(" CLOSED |");
			else
				printf(" lofasz |");
		}
	}
	cout << endl;
}

//-------------------------------------------------------------------
string SeriesHydraulics::secondsToTime(double seconds)
{
	string out;
	int hour = seconds/3600.;
	int minute = (seconds - hour*3600.)/60.;
	if(minute < 10)
		out = to_string(hour) + ":0" + to_string(minute);
	else
		out = to_string(hour) + ":" + to_string(minute);
	return out;
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
