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

#include "Staci.h"

class Shutdown : public Staci
{
public:
	Shutdown(string spr_filename);
	~Shutdown();
	void buildSegmentGraph();
	void createShutdownPlan(string edgeID);

	vector<int> getEdgeVector(vector<int> edgeVector)
	{
		return edgeVector;
	}
	vector<int> getSegmentEdgeVector(vector<int> segmentEdgeVector)
	{
		return segmentEdgeVector;
	}
	vector<vector<int> > getSegmentVector(vector<int> segmentEdgeVector)
	{
		return segmentVector;
	}

private:
	vector<int> edgeVector; // edge vector of the original network
	vector<int> segmentEdgeVector; // edge vector of the segment graph
	vector<vector<int> > segmentVector; // contains the original node indicies of pipes

	vector<int> findConnectionError(vector<int> connectingNodes);

};
