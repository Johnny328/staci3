#include "Graph.h"

using namespace std;

double GlobalSegmentClustering(vector<int> segmentEdgeVector)
{
    igraph_t graph;
    igraph_vector_t v2;
    igraph_real_t edges[segmentEdgeVector.size()], res;
    for(int i = 0; i < segmentEdgeVector.size(); i++)
    {
    //    cout << i << " edge: " << segmentEdgeVector[i] << endl;
        edges[i] = segmentEdgeVector[i];
    //    cout << "edges: " << edges[i] << " edgeVector: " << segmentEdgeVector[i] << " v: "<<VECTOR(v)[i] << endl;
    }
    igraph_vector_view(&v2, edges, sizeof(edges)/sizeof(double));
    igraph_create(&graph, &v2, 0, IGRAPH_UNDIRECTED);
    igraph_transitivity_undirected(&graph, &res, IGRAPH_TRANSITIVITY_ZERO);
    cout << "Clustering: " << res << endl;
    return res;
}
vector<double> LocalSegmentClustering(vector<int> segmentEdgeVector)
{
    igraph_t graph; 
    igraph_vector_t v2, res;
    igraph_vs_t vs;
    igraph_real_t edges[segmentEdgeVector.size()];
    igraph_vector_init(&res, 0);
    igraph_integer_t size;
    vector<double> LocalClustering;
    for(int i = 0; i < segmentEdgeVector.size(); i++)
    {
    //    cout << i << " edge: " << segmentEdgeVector[i] << endl;
        edges[i] = segmentEdgeVector[i];
    //    cout << "edges: " << edges[i] << " edgeVector: " << segmentEdgeVector[i] << " v: "<< VECTOR(v)[i] << endl;
    }
    igraph_vector_view(&v2, edges, sizeof(edges)/sizeof(double));
    igraph_create(&graph, &v2, 0, IGRAPH_UNDIRECTED);
    igraph_transitivity_local_undirected(&graph, &res, igraph_vss_all(), IGRAPH_TRANSITIVITY_ZERO);
    cout << "EDDIG MEGVAN" << endl;
    igraph_vs_all(&vs);
    igraph_vs_size(&graph, &vs, &size);
    for (int i = 0; i < size; ++i)
    {
        cout << "local clustering[" << i << "] helyen: "<< VECTOR(res)[i] << endl;
    }
    return LocalClustering;
}

double AvPathLength(vector<int> segmentEdgeVector)
{
    double AvPath = 0.;
    igraph_t graph; 
    igraph_vector_t v2;
    igraph_real_t edges[segmentEdgeVector.size()], res;
    for(int i = 0; i < segmentEdgeVector.size(); i++)
    {
    //    cout << i << " edge: " << segmentEdgeVector[i] << endl;
        edges[i] = segmentEdgeVector[i];
    //    cout << "edges: " << edges[i] << " edgeVector: " << segmentEdgeVector[i] << " v: "<< VECTOR(v)[i] << endl;
    }
    igraph_vector_view(&v2, edges, sizeof(edges)/sizeof(double));
    igraph_create(&graph, &v2, 0, IGRAPH_UNDIRECTED);
    igraph_average_path_length(&graph, &res, 0, 0);
    cout << "EDDIG MEGVAN" << endl;
    AvPath = res;
    return AvPath;
}

int GraphDiameter(vector<int> segmentEdgeVector)
{
    int Diameter;   
    igraph_t graph; 
    igraph_vector_t v2, res;
    igraph_vs_t vs;
    igraph_real_t edges[segmentEdgeVector.size()];
    igraph_vector_init(&res, 0);
    igraph_integer_t result;
    igraph_integer_t size;
    for(int i = 0; i < segmentEdgeVector.size(); i++)
    {
    //    cout << i << " edge: " << segmentEdgeVector[i] << endl;
        edges[i] = segmentEdgeVector[i];
    //    cout << "edges: " << edges[i] << " edgeVector: " << segmentEdgeVector[i] << " v: "<< VECTOR(v)[i] << endl;
    }
    igraph_vector_view(&v2, edges, sizeof(edges)/sizeof(double));
    igraph_create(&graph, &v2, 0, IGRAPH_UNDIRECTED);
    igraph_diameter(&graph, &result, 0, 0, 0, IGRAPH_UNDIRECTED, 0);
    cout << "EDDIG MEGVAN" << endl;
    igraph_vs_all(&vs);
    igraph_vs_size(&graph, &vs, &size);
    Diameter = result;
    return Diameter;
}

vector<double> DegreeDistribution(vector<int> segmentEdgeVector, bool selfloop)
{
    igraph_t graph; 
    igraph_vector_t v2, res;
    igraph_vs_t vs;
    igraph_bool_t selector = selfloop;
    igraph_real_t edges[segmentEdgeVector.size()];
    igraph_vector_init(&res, 0);
    igraph_integer_t size;
    vector<double> DegreeDistribution;
    for(int i = 0; i < segmentEdgeVector.size(); i++)
    {
    //    cout << i << " edge: " << segmentEdgeVector[i] << endl;
        edges[i] = segmentEdgeVector[i];
    //    cout << "edges: " << edges[i] << " edgeVector: " << segmentEdgeVector[i] << " v: "<< VECTOR(v)[i] << endl;
    }
    igraph_vector_view(&v2, edges, sizeof(edges)/sizeof(double));
    igraph_create(&graph, &v2, 0, IGRAPH_UNDIRECTED);
    igraph_degree(&graph, &res, igraph_vss_all(), IGRAPH_ALL, selector);
    cout << "EDDIG MEGVAN" << endl;
    igraph_vs_all(&vs);
    igraph_vs_size(&graph, &vs, &size);
    for (int i = 0; i < size; ++i)
    {
  //      cout << "Degree distribution[" << i << "] helyen: "<< VECTOR(res)[i] << endl;
        DegreeDistribution.push_back(VECTOR(res)[i]);
    }
    return DegreeDistribution;
}

vector< vector<int> > segmenter2(vector<int> SledgeVector)
{
    vector<int> Segment;
    vector<int> edgeVector = SledgeVector;
    vector< vector<int> > Segments;
    bool repeat = true;
    while(edgeVector.size() != 0)//22//1433villasori//445//923ferto445
    {
        repeat = true;
        cout << " Szegmens meret " << Segment.size() << " Vektor meret: " << edgeVector.size() << endl;
        if(Segment.size() == 0)
        {
            Segment.push_back(edgeVector[0]);
            Segment.push_back(edgeVector[1]);
            edgeVector.erase(edgeVector.begin());
            edgeVector.erase(edgeVector.begin());
            cout << " Szegmens meret " << Segment.size() << " Vektor meret: " << edgeVector.size() << endl;
        }
        while(repeat == true)
        { 
            repeat = false;
            cout << " Szegmens meret " << Segment.size() << " Vektor meret: " << edgeVector.size() << endl;
            for (int j = 0; j < edgeVector.size(); j++)
            {
                for (int k = 0; k < Segment.size(); k++)
                {
                    if(repeat==true)
                    {
                        break;
                    }        
        //            cout << " belep 54 " << endl;
                    if(Segment[k] == edgeVector[j] && j % 2 != 0)
                    {
                        cout << " belep 60 " << "j size: " << j << endl;
                        cout << " el1: " << edgeVector[j-1] << " el2: ";
                        cout << edgeVector[j];
                        Segment.push_back(edgeVector[j-1]);
                        Segment.push_back(edgeVector[j]);
                        edgeVector.erase(edgeVector.begin() + j-1);
                        edgeVector.erase(edgeVector.begin() + j-1);
                        repeat = true;
                    }
                    if(Segment[k] == edgeVector[j] && j % 2 == 0)
                    {
                        cout << " belep 70 " << Segment.size() << " " << edgeVector.size() << " " << j << endl;
                        cout << " el2: " << edgeVector[j] << " el1: ";
                        cout << edgeVector[j+1];
                        Segment.push_back(edgeVector[j]);
                        Segment.push_back(edgeVector[j+1]);
                        edgeVector.erase(edgeVector.begin() + j);
                        edgeVector.erase(edgeVector.begin() + j);
                        repeat = true;
                    }
                }        
            }
        }
        if(edgeVector.size() > 2 && repeat != true)
        {
            Segments.push_back(Segment);
            Segment.clear();
        }
        if(edgeVector.size() == 2 && repeat != true)
        {
            Segments.push_back(Segment);
            Segment.clear();
            cout << " belep 100 " << endl;
            cout << " bemegy " << endl;
            Segment.push_back(edgeVector[0]);
            Segment.push_back(edgeVector[1]);
            Segments.push_back(Segment);
            edgeVector.clear();
            cout << " kijon " << endl;
        }
    }
    cout << " exiting ";
    return Segments;
}

vector< vector<int> > segmenter(vector<int> SledgeVector)
{
    //tipp: A szukseges ciklusszam number of tolozarak+1
    vector<int> Segment;
    vector<int> edgeVector = SledgeVector;
    vector< vector<int> > Segments;
    int collector = 0, First_round = 1;
    bool Goon = true, ItWorks = true;
    while(Goon == true)//22//1433villasori//445//923ferto445
    {
        ItWorks = false;
        cout << " Szegmens meret " << Segment.size() << " Vektor meret: " << edgeVector.size() << endl;
        if(Segment.size() == 0 && edgeVector.size() != 0)
        {
            Segment.push_back(edgeVector[0]);
            Segment.push_back(edgeVector[1]);
            edgeVector.erase(edgeVector.begin());
            edgeVector.erase(edgeVector.begin());
            cout << " Szegmens meret " << Segment.size() << " Vektor meret: " << edgeVector.size() << endl;
            First_round = 0;
        }
        else
        {
            cout << " Szegmens meret " << Segment.size() << " Vektor meret: " << edgeVector.size() << endl;
            for (int k = 0; k < Segment.size(); k++)
            {
        //        cout << " belep 51 " << endl;
                for (int j = 0; j < edgeVector.size(); j++)
                {
        //            cout << " belep 54 " << endl;
                    if(edgeVector[j] == Segment[k] && edgeVector.size() > 2)
                    {
        //                cout << " belep 57 " << endl;
                        if(j % 2 != 0)
                        {
                            cout << " belep 60 " << "j size: " << j << endl;
                            cout << " el1: " << edgeVector[j-1] << " el2: ";
                            cout << edgeVector[j];
                            Segment.push_back(edgeVector[j-1]);
                            Segment.push_back(edgeVector[j]);
                            edgeVector.erase(edgeVector.begin() + j-1);
                            edgeVector.erase(edgeVector.begin() + j-1);
                        }
                        else
                        {
                            cout << " belep 70 " << Segment.size() << " " << edgeVector.size() << " " << j << endl;
                            cout << " el2: " << edgeVector[j] << " el1: ";
                            cout << edgeVector[j+1];
                            Segment.push_back(edgeVector[j]);
                            Segment.push_back(edgeVector[j+1]);
                            edgeVector.erase(edgeVector.begin() + j);
                            edgeVector.erase(edgeVector.begin() + j);
                        }
                    }
                    else
                    {
                        collector += 1;
                    }
                }        
            }
            if(collector == edgeVector.size()-1)
            {
                cout << " belep 85 " << endl;
                cout << " Teljesul " << endl;
                Segments.push_back(Segment);
                Segment.clear();
            }
            if(edgeVector.size() == 2)
            {
                cout << " belep 100 " << endl;
                cout << " bemegy " << endl;
                Segment.push_back(edgeVector[0]);
                Segment.push_back(edgeVector[1]);
                Segments.push_back(Segment);
                edgeVector.clear();
                cout << " kijon " << endl;
            }
        }
        cout << "eddig oke";
        if (edgeVector.size() == 0)
        {
            cout << " EV. size() = " << edgeVector.size() << endl;
            Goon = false;
        }
    }
    cout << " exiting ";
    return Segments;
}

vector<vector<int> > segmenterWR(vector<int> edgeVector){
  vector<vector<int> > everySegment;
  vector<int> segment;

  while(edgeVector.size() !=0)
  {
    segment.push_back(edgeVector[0]);
    segment.push_back(edgeVector[1]);
    edgeVector.erase(edgeVector.begin());
    edgeVector.erase(edgeVector.begin());

    for(int j=0; j<segment.size(); j+=2)
    {
      for(int i=0; i<edgeVector.size(); i+=2)
      {
        if(edgeVector[i] == segment[j] || edgeVector[i+1] == segment[j] || edgeVector[i] == segment[j+1] || edgeVector[i+1] == segment[j+1])
        {
          segment.push_back(edgeVector[i]);
          segment.push_back(edgeVector[i+1]);
          edgeVector.erase(edgeVector.begin() + i);
          edgeVector.erase(edgeVector.begin() + i);
          j=-2; // setting back j to the first item that is 0th
          break;
        }
      }
    }
    everySegment.push_back(segment);
    segment.clear();
  }

  return everySegment;
}