/*===================================================================*\
                            SeriesHydraulics
                            ----------------
  TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO
  TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO
  TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO
  TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO
  TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO
  TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO
 
  staci3 is using Eigen, see http://eigen.tuxfamily.org

    staci3
    Cs. Hos, R. Weber, T. Huzsvar
    https://github.com/weberrichard/staci3
\*==================================================================*/

#ifndef SERIESHYDRAULICS_H
#define SERIESHYDRAULICS_H

#include "HydraulicSolver.h"
#include "Rule.h"

class SeriesHydraulics : public HydraulicSolver
{
public:

	SeriesHydraulics(string fileName);
	~SeriesHydraulics();

  // calculating the hydraulics in time
  void seriesSolve();

  // Printing results to console
  void timeTableNode(vector<string> ID, double convertUnit);
  void timeTableNode(vector<int> idx, double convertUnit);
  void timeTableEdge(vector<string> ID, double convertUnit);
  void timeTableEdge(vector<int> idx, double convertUnit);

private:
  // loading the series settings from INP file
  void loadTimeSettings();
    
  // saving heads, consumtions for nodes and flow rate for edges
  void saveOutput();

  // Calculating new time step based on controls and rules (tank filling etc.)
  double newHydraulicTimeStep();

  // updating stuffs
  void updateDemand();
  void updatePool();
  void updateControl();
  void updateRule();

  // converting string to a boolean operator between val1 and val2, e.g. val1 op val2 -> val1 > val2
  bool booleanWithString(string op, double left, double right);

  // info about the seires calculation
  void seriesInfo();

  // Converting everything to seconds like 24:00 or 7.0 DAYS
  double timeToSeconds(string s1, string s2);

  // time stuff
  double duration, startClock=0., time, clockTime;
  double patternTimeStep, hydraulicTimeStep, hydraulicTimeStepOriginal;
  vector<double> vectorTime;

  // demand patterns
  vector<string> patternID;
  vector<vector<double> > patternValue;

  // CONTROLS
  // TODO: do this in more general way, handling time controls as well (maybe struct or class)
  vector<string> controlEdgeID; // controlled edge ID
  vector<int> controlEdgeIndex; // controlled edge INDEX in edges list
  vector<bool> controlStatus; // Open (true) or closed (false)
  vector<string> controlNodeID; // checked node (or tank) pressure ID
  vector<int> controlNodeIndex; // checked node (or tank) pressure INDEX
  vector<string> controlType; // Pool or Node
  vector<bool> controlAbove; // true if ABOVE, false if BELOW
  vector<double> controlValue; // head [m]

  // RULES
  vector<Rule*> rules;

  // filling up vectorPatternIndex in Nodes
  void patternIDtoIndex();
};

#endif