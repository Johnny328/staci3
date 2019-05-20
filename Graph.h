/*===================================================================*\
                                  Graph
                            ---------------
	
	This file provides multiple algorithms corresponding to Graphs and
	their structural properties (e.g. diameter, clustering coeff.).
	
  Independently works from staci3.
 
  staci3 is using Eigen, see http://eigen.tuxfamily.org

    staci3
    Cs. Hos, R. Weber, T. Huzsvar
    https://github.com/weberrichard/staci3
\*==================================================================*/

#ifndef GRAPH_H
#define GRAPH_H

#include <vector>
#include <iostream>
#include <igraph.h>

using namespace std;

int GraphDiameter(vector<int> Segment_EV);
vector< vector<int> > segmenter(vector<int> Sledge_vector);
vector< vector<int> > segmenter2(vector<int> Sledge_vector);
vector<vector<int> > segmenterWR(vector<int> edveVector);
double GlobalSegmentClustering(vector<int> Segment_EV);
double AvPathLength(vector<int> Segment_EV);
vector<double> LocalSegmentClustering(vector<int> Segment_EV);
vector<double> DegreeDistribution(vector<int> Segment_EV, bool selfloop);

#endif
