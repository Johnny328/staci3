#include "Shutdown.h"

//--------------------------------------------------------------
Shutdown::Shutdown(string spr_filename) : Staci(spr_filename)
{
  buildSegmentGraph();
}
Shutdown::~Shutdown(){}

//--------------------------------------------------------------
void Shutdown::buildSegmentGraph()
{ 
  updateEdgeVector();

  // creating the edge vector of the segments
  segmentVector = segmenterWR(edgeVector);
  numberSegment = segmentVector.size();

  // Setting the segment variables of nodes and edges
  for(int i=0; i<segmentVector.size(); i++)
  {
    for(int j=0; j<segmentVector[i].size(); j+=2)
    {
      nodes[abs(segmentVector[i][j])]->setSegment(i);
      nodes[abs(segmentVector[i][j+1])]->setSegment(i);
      for(int k=0; k<edges.size(); k++)
      {
        string type = edges[k]->getEdgeStringProperty("type");
        if(type == "Pipe" || type == "Pump"){
          if(abs(segmentVector[i][j]) == edges[k]->getEdgeIntProperty("startNodeIndex") && abs(segmentVector[i][j+1]) == edges[k]->getEdgeIntProperty("endNodeIndex"))
            edges[k]->setEdgeIntProperty("segment",i);
        }
        if(type == "Pool" || type == "PressurePoint"){
          if(abs(segmentVector[i][j]) == edges[k]->getEdgeIntProperty("startNodeIndex") || abs(segmentVector[i][j+1]) == edges[k]->getEdgeIntProperty("startNodeIndex"))
            edges[k]->setEdgeIntProperty("segment",i);
        }
      }
    }
  }

  // Creating the edge vector of the segment graph
  segmentEdgeVector.clear();
  for(int i=0; i<edges.size(); i++)
  {
    if(edges[i]->getEdgeStringProperty("type") == "ValveISO")
    {
      int idx_from, idx_to;
      idx_from = edges[i]->getEdgeIntProperty("startNodeIndex");
      idx_to = edges[i]->getEdgeIntProperty("endNodeIndex");
      segmentEdgeVector.push_back(nodes[idx_from]->getSegment());
      segmentEdgeVector.push_back(nodes[idx_to]->getSegment());

      // Filling up the Valves which segments are they connecting
      edges[i]->setIntProperty("startSegment",nodes[idx_from]->getSegment());
      edges[i]->setIntProperty("endSegment",nodes[idx_to]->getSegment());
    }
  }
}

//--------------------------------------------------------------
vector<string> Shutdown::shutdownPlan(string pipeID){
  int idx=0;
  bool gotIt = false;
  while(!gotIt && idx<edges.size()){
    if(edges[idx]->getEdgeStringProperty("name") == pipeID)
    {
      gotIt = true;
    }
    idx++;
  }

  vector<string> valves;
  if(idx==edges.size())
    cout << endl << "Shutdown::shutdownPlan(pipeID) : pipeID (" << pipeID << ") not found" << endl;
  else{
    int segment = edges[idx-1]->getEdgeIntProperty("segment");
    valves = closeSegment(segment);
  }

  return valves;
}

//--------------------------------------------------------------
vector<string> Shutdown::closeSegment(int segmentToClose){

  vector<string> closedValves;
  for(int i=0; i<edges.size(); i++)
  {
    if(edges[i]->getEdgeIntProperty("segment") == segmentToClose)
      changeEdgeStatus(edges[i]->getEdgeStringProperty("name"),false);
    if(edges[i]->getEdgeStringProperty("type") == "ValveISO"){
      if(edges[i]->getIntProperty("startSegment") == segmentToClose || edges[i]->getIntProperty("startSegment") == segmentToClose){
        changeEdgeStatus(edges[i]->getEdgeStringProperty("name"),false);
        closedValves.push_back(edges[i]->getEdgeStringProperty("name"));
      }
    }
  }

  vector<int> closedSegment = closeDisconnectedParts();

  // Clearing the inner isolation valves
  for(int i=0; i<closedValves.size(); i++)
  {
    for(int j=0; j<edges.size(); j++)
    {
      if(edges[j]->getEdgeStringProperty("type") == "ValveISO")
      {
        if(edges[j]->getEdgeStringProperty("name") == closedValves[i])
        { 
          for(int k=0; k<closedSegment.size(); k++)
          {
            if(edges[j]->getIntProperty("startSegment") == closedSegment[k] || edges[j]->getIntProperty("endSegment") == closedSegment[k])
            {
              closedValves[i] = "no";
            }
          }
        }
      }
    }
  }

  for(int i=0; i<closedValves.size(); i++)
    if(closedValves[i] == "no")
      closedValves.erase(closedValves.begin() + i);

  return closedValves;
}

//--------------------------------------------------------------
vector<int> Shutdown::closeDisconnectedParts(){
  // build up edge vector that contains the isolation valves as well
  vector<int> edgeVectorWithValves;
  vector<int> pressurePrescribed;
  for(int i=0; i<edges.size(); i++)
  {
    if(!edges[i]->isClosed)
    {
      string type = edges[i]->getEdgeStringProperty("type");
      if(type == "Pipe" || type == "Pump" || type == "ValveISO")
      {
        edgeVectorWithValves.push_back(edges[i]->getEdgeIntProperty("startNodeIndex"));
        edgeVectorWithValves.push_back(edges[i]->getEdgeIntProperty("endNodeIndex"));
      }
      if(type == "PressurePoint" || type == "Pool")
      {
        pressurePrescribed.push_back(edges[i]->getEdgeIntProperty("startNodeIndex"));
      }
    }
  }

  // checking the integrity
  vector<vector<int> > segmentVectorWithValves = segmenterWR(edgeVectorWithValves);

  vector<int> closedSegment;
  // if we have more than one "part", checking which ones have pressure prescribed
  if(segmentVectorWithValves.size()>1)
  {
    vector<bool> isPressurePrescribed(segmentVectorWithValves.size(),false);
    for(int k=0; k<pressurePrescribed.size(); k++)
    {
      for(int i=0; i<segmentVectorWithValves.size(); i++)
      {
        for(int j=0; j<segmentVectorWithValves[i].size(); j++)
        {
          if(segmentVectorWithValves[i][j] == pressurePrescribed[k])
          {
            isPressurePrescribed[i] = true;
          }
        }
      }
    }

    // closing the nodes those are not connected to pressure node
    for(int i=0; i<isPressurePrescribed.size(); i++)
    {
      if(!isPressurePrescribed[i])
      {
        closedSegment.push_back(nodes[segmentVectorWithValves[i][0]]->getSegment());
        for(int j=0; j<segmentVectorWithValves[i].size(); j++)
        {
          nodes[segmentVectorWithValves[i][j]]->isClosed = true;
        }
      }
    }

    // closing the edges as well
    for(int i=0; i<edges.size(); i++)
    {
      int idx_from = edges[i]->getEdgeIntProperty("startNodeIndex");
      if(nodes[idx_from]->isClosed){
        changeEdgeStatus(edges[i]->getEdgeStringProperty("name"),false);
      }
    }
  }
  return closedSegment;
}

//--------------------------------------------------------------
// Building up the edge vector from pipes and pumps
void Shutdown::updateEdgeVector(){
  edgeVector.clear();
  for(int i=0; i<edges.size(); i++)
  {
    if(!edges[i]->isClosed)
    {
      if(edges[i]->getEdgeStringProperty("type") == "Pipe" || edges[i]->getEdgeStringProperty("type") == "Pump")
      {
        edgeVector.push_back(edges[i]->getEdgeIntProperty("startNodeIndex"));
        edgeVector.push_back(edges[i]->getEdgeIntProperty("endNodeIndex"));
      }
    }
  }
}

//--------------------------------------------------------------
vector<int> Shutdown::findConnectionError(vector<int> connectingNodes)
{ 
  vector<int> connectionErrors;
  vector<int> rank(nodes.size());
  for(int i=0; i<edges.size(); i++)
  { 
    string type = edges[i]->getEdgeStringProperty("type");

    if(type == "Pipe" || type == "Pump")
    {
      rank[edges[i]->getEdgeIntProperty("startNodeIndex")]++;
      rank[edges[i]->getEdgeIntProperty("endNodeIndex")]++;
    }
    if(type == "PressurePoint" || type == "Pool")
    { 
      rank[edges[i]->getEdgeIntProperty("startNodeIndex")]++;
    }
  }

  for(int i=0; i<connectingNodes.size(); i++){
    if(rank[connectingNodes[i]]!=2 && rank[connectingNodes[i]]!=0){
      connectionErrors.push_back(connectingNodes[i]);
    }
  }
  if(connectionErrors.size()>0)
    cout << endl << "[*] Cleared connection nodes (with  rank): ";
  for(int i=0; i<connectionErrors.size(); i++){
    cout << nodes[connectionErrors[i]]->getName() << "(" << rank[connectionErrors[i]] << ")" << ", ";
  }

  return connectionErrors;
}
