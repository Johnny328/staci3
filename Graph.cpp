#include "Graph.h"

//--------------------------------------------------------------
vector<vector<int> > segmenter(vector<int> ev)
{
  vector<vector<int> > everySegment;
  vector<int> segment;

  while(ev.size() !=0)
  {
    segment.push_back(ev[0]);
    segment.push_back(ev[1]);
    ev.erase(ev.begin());
    ev.erase(ev.begin());

    for(int j=0; j<segment.size(); j+=2)
    {
      for(int i=0; i<ev.size(); i+=2)
      {
        if(ev[i] == segment[j] || ev[i+1] == segment[j] || ev[i] == segment[j+1] || ev[i+1] == segment[j+1])
        {
          segment.push_back(ev[i]);
          segment.push_back(ev[i+1]);
          ev.erase(ev.begin() + i);
          ev.erase(ev.begin() + i);
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


double globalClustering(vector<int> ev)
{
    igraph_t graph;
    igraph_vector_t v2;
    igraph_real_t edges[ev.size()], out;
    for(int i = 0; i < ev.size(); i++)
    {
        edges[i] = ev[i];
    }
    igraph_vector_view(&v2, edges, sizeof(edges)/sizeof(double));
    igraph_create(&graph, &v2, 0, IGRAPH_UNDIRECTED);
    igraph_transitivity_undirected(&graph, &out, IGRAPH_TRANSITIVITY_ZERO);
    return out;
}

double avPathLength(vector<int> ev)
{
    igraph_t graph; 
    igraph_vector_t v2;
    igraph_real_t edges[ev.size()], out;
    for(int i = 0; i < ev.size(); i++)
    {
        edges[i] = ev[i];
    }
    igraph_vector_view(&v2, edges, sizeof(edges)/sizeof(double));
    igraph_create(&graph, &v2, 0, IGRAPH_UNDIRECTED);
    igraph_average_path_length(&graph, &out, 0, 0);
    return out;
}

int graphDiameter(vector<int> ev)
{
    igraph_t graph; 
    igraph_vector_t v2, res;
    igraph_vs_t vs;
    igraph_real_t edges[ev.size()];
    igraph_vector_init(&res, 0);
    igraph_integer_t out;
    igraph_integer_t size;
    for(int i = 0; i < ev.size(); i++)
    {
        edges[i] = ev[i];
    }
    igraph_vector_view(&v2, edges, sizeof(edges)/sizeof(double));
    igraph_create(&graph, &v2, 0, IGRAPH_UNDIRECTED);
    igraph_diameter(&graph, &out, 0, 0, 0, IGRAPH_UNDIRECTED, 0);
    igraph_vs_all(&vs);
    igraph_vs_size(&graph, &vs, &size);
    return out;
}

int articulationPoints(vector<int> ev)
{
    igraph_t graph; 
    igraph_vector_t v2, res;
    igraph_vs_t vs;
    igraph_real_t edges[ev.size()];
    igraph_vector_init(&res, 0);
    igraph_integer_t size;
    for(int i = 0; i < ev.size(); i++)
    {
        edges[i] = ev[i];
    }
    igraph_vector_view(&v2, edges, sizeof(edges)/sizeof(double));
    igraph_create(&graph, &v2, 0, IGRAPH_UNDIRECTED);
    igraph_articulation_points(&graph, &res);
    return igraph_vector_size(&res);
}
