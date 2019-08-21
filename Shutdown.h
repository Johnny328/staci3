/*===================================================================*\
                                Shutdown
                            ---------------
	
	This class is capable of build the segment graph from the original
	network topology. Also creates shutdown plan.
 
  staci3 is using Eigen, see http://eigen.tuxfamily.org

    staci3
    Cs. Hos, R. Weber, T. Huzsvar
    https://github.com/weberrichard/staci3
\*==================================================================*/

#ifndef SHUTDOWN_H
#define SHUTDOWN_H

#include "Staci.h"

class Shutdown : public Staci
{
public:
	Shutdown(string spr_filename);
	~Shutdown();
	void buildSegmentGraph();
	vector<int> closeDisconnectedParts(); // gives back closed segments
	vector<string> shutdownPlan(string pipeID);	// gives back id of closed valves i.e. shutdownplan
	vector<string> closeSegment(int segmentToClose); // gives back id of closed valves i.e. shutdownplan
	vector<int> getEdgeVector()
	{
		return edgeVector;
	}
	vector<int> getSegmentEdgeVector()
	{
		return segmentEdgeVector;
	}
	vector<vector<int> > getSegmentVector()
	{
		return segmentVector;
	}
	int getNumberSegment()
	{
		return numberSegment;
	}

private:
	vector<int> edgeVector; // edge vector of the original network
	vector<int> segmentEdgeVector; // edge vector of the segment graph
	vector<vector<int> > segmentVector; // contains the original node indicies of pipes
	int numberSegment;

	void updateEdgeVector();
	vector<int> findConnectionError(vector<int> connectingNodes);
};

#endif