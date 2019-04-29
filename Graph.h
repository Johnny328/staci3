#include "Sensitivity.h"

using namespace std;

int GraphDiameter(vector<int> Segment_EV);
vector< vector<int> > segmenter(vector<int> Sledge_vector);
vector< vector<int> > segmenter2(vector<int> Sledge_vector);
double GlobalSegmentClustering(vector<int> Segment_EV);
double AvPathLength(vector<int> Segment_EV);
vector<double> LocalSegmentClustering(vector<int> Segment_EV);
vector<double> DegreeDistribution(vector<int> Segment_EV, bool selfloop);
vector<double> Atriculation_point(vector<int> Segment_EV);
class Graph
{
    int V;    // No. of vertices in graph
    list<int> *adj; // Pointer to an array containing adjacency lists
 
    // A recursive function used by printAllPaths()
    void printAllPathsUtil(int , int , bool [], int [], int &);
 
public:
    Graph(int V);   // Constructor
    void addEdge(int u, int v);
    void printAllPaths(int s, int d);
};