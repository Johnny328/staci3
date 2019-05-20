#include "Shutdown.h"

//--------------------------------------------------------------
Shutdown::Shutdown(string spr_filename) : Staci(spr_filename)
{
  edgeVector.clear();
  for(int i=0; i<edges.size(); i++)
  {   
    if(edges[i]->getEdgeStringProperty("type") == "Pipe" || edges[i]->getEdgeStringProperty("type") == "Pump")
    {
      edgeVector.push_back(edges[i]->getEdgeIntProperty("startNodeIndex"));
      edgeVector.push_back(edges[i]->getEdgeIntProperty("endNodeIndex"));
    }
  }
}
Shutdown::~Shutdown(){}

//--------------------------------------------------------------
void Shutdown::buildSegmentGraph()
{
  // finding the isolation valves connection points
  vector<int> connectingNodes;
  for(int i=0; i<edges.size(); i++)
  {	
  	if(edges[i]->getEdgeStringProperty("type") == "Valve")
  	{
  		connectingNodes.push_back(edges[i]->getEdgeIntProperty("startNodeIndex"));
  		connectingNodes.push_back(edges[i]->getEdgeIntProperty("endNodeIndex"));
  	}
  }

  // searching for connecting nodes thats rank is not equal to two (or zero) and deleteing them
  vector<int> connectionErrors = findConnectionError(connectingNodes);
  for(int i=0; i<connectionErrors.size(); i++)
  {
    for(int j=0; j<connectingNodes.size(); j++)
    {
      if(connectionErrors[i] == connectingNodes[j])
      {
        connectingNodes.erase(connectingNodes.begin() + j);
        connectingNodes.erase(connectingNodes.begin() + j);
      }
    }
  }

  // disconnecting the network by multiplying on of the common connecting nodes with (-1)
  vector<int> edgeVectorMod = edgeVector;
  for(int i=0; i<connectingNodes.size(); i++)
  {
    for(int j=0; j<edgeVectorMod.size(); j++)
    {
      if(connectingNodes[i] == edgeVectorMod[j])
      {
        edgeVectorMod[j] *= -1;
        break;
      }
    }
  }

  // creating the edge vector of the segments
  segmentVector = segmenterWR(edgeVectorMod); 
  cout << endl << "segment vector size: " << segmentVector.size() << endl;
  for(int i=0; i<segmentVector.size(); i++){
    for(int j=0; j<segmentVector[i].size(); j++)
      cout << segmentVector[i][j] << ", ";
    cout << endl;
  }

  cout << endl << "connecting nodes: " << endl;
  for(int i=0; i<connectingNodes.size(); i++)
    cout << connectingNodes[i] << ", ";
  cout << endl;

  // creating the edge vector of the segment graph
  segmentEdgeVector.clear();
  for(int i=0; i<connectingNodes.size(); i++)
  {
    for(int j=0; j<segmentVector.size(); j++)
    {
      for(int k=0; k<segmentVector[j].size(); k++)
      {
        if(connectingNodes[i] == abs(segmentVector[j][k]))
        {
          segmentEdgeVector.push_back(j);
        }
      }
    }
  }

  cout << endl << "segmentEdgeVector: " << segmentEdgeVector.size() << endl;
  for(int i=0; i<segmentEdgeVector.size(); i+=2)
    cout << segmentEdgeVector[i] << "-" << segmentEdgeVector[i+1] << ", ";

  cout << endl << "no of valves: " << connectingNodes.size()/2. << endl;

  for(int i=0; i<segmentVector.size(); i++)
  {
    for(int j=0; j<segmentVector[i].size(); j+=2)
    { 
      nodes[abs(segmentVector[i][j])]->setSegment(i);
      nodes[abs(segmentVector[i][j+1])]->setSegment(i);
      for(int k=0; k<edges.size(); k++)
      {
        if(abs(segmentVector[i][j]) == edges[k]->getEdgeIntProperty("startNodeIndex") && abs(segmentVector[i][j+1]) == edges[k]->getEdgeIntProperty("endNodeIndex"))
          edges[k]->setEdgeIntProperty("segment",i);
      }
    }
  }

  cout << endl << "NODES" << endl;
  for(int i=0; i<nodes.size(); i++)
    printf("%15s %3i %3i\n", nodes[i]->getName().c_str(), i, nodes[i]->getSegment());

  cout << endl << "PIPES" << endl;
  for(int i=0; i<edges.size(); i++)
    printf("%15s %3i %3i\n", edges[i]->getEdgeStringProperty("name").c_str(), i, edges[i]->getEdgeIntProperty("segment"));
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

//--------------------------------------------------------------
/*void Shutdown::createShutdownPlan(string edgeID)
{
    int cspe = 0, cspv = 0;
    for (int i = 0; i < edges.size(); ++i)
    {
        //cout << " eddig megvan " << endl;
        if (edges[i]->getEdgeStringProperty("type") == "Pipe" && edgeID == edges[i]->getEdgeStringProperty("name"))
        {
             cspe = edges[i]->getEdgeIntProperty("startNodeIndex");
             cspv = edges[i]->getEdgeIntProperty("endNodeIndex");
        }
    }
    vector<string> Tolzar_list;
    vector<int> TolzarCSPID;
    vector<double> TolzarMeret;
    for (int k = 0; k < segmentVector.size(); ++k)
    {
        for (int l = 0; l < segmentVector[k].size(); ++l)
        {
            if ((segmentVector[k][l] == cspe && segmentVector[k][l+1] == cspv) || (segmentVector[k][l] == cspe && segmentVector[k][l-1] == cspv))
            {
                for (int i = 0; i < edges.size(); ++i)
                {
                    if (edges[i]->getEdgeStringProperty("type") == "Valve")
                    {
                        for (int j = 0; j < segmentVector[k].size(); ++j)
                        {
                           if (segmentVector[k][j] == edges[i]->getEdgeIntProperty("startNodeIndex") || segmentVector[k][j] == edges[i]->getEdgeIntProperty("endNodeIndex"))
                           {
                               Tolzar_list.push_back(edges[i]->getEdgeStringProperty("name"));
                               TolzarCSPID.push_back(edges[i]->getEdgeIntProperty("startNodeIndex"));
                           }
                        }
                    }
                }
            }
        }
    }
    for (int i = 0; i < Tolzar_list.size(); ++i)
    {
        for (int j = 0; j < edges.size(); ++j)
        {
           if ((edges[j]->getEdgeStringProperty("type") == "Pipe" && TolzarCSPID[i] == edges[j]->getEdgeIntProperty("startNodeIndex")) || (edges[j]->getEdgeStringProperty("type") == "Pipe" && TolzarCSPID[j] == edges[j]->getEdgeIntProperty("endNodeIndex")))
           {
                cout << "megvan" << endl;
                TolzarMeret.push_back(edges[j]->getEdgeDoubleProperty("diameter"));
                break;
           }
        }
    }
    int sorszam = 0;
    vector< vector<string> > TeljesListaID;
    vector<double> TeljesListaAtm;
    for (int i = 0; i < Tolzar_list.size(); ++i)
    {
        sorszam = 0;
        for (int j = 0; j < Tolzar_list.size(); ++j)
        {
            if (TolzarMeret[i] < TolzarMeret[j])
            {
                sorszam += 1;
            }
        }
        while(TeljesListaID.size() <= sorszam)
        {
            TeljesListaID.push_back(vector<string>());
        }
        cout << "eddig oke" << endl;
        TeljesListaID[sorszam].push_back(Tolzar_list[i]);
        TeljesListaAtm.push_back(TolzarMeret[i]);
    }
    cout << endl;
    cout << "*** A kiválasztott csőszakasz ***" << endl;
    cout << "*          " << edgeID << "         *" << endl;
    cout << "*********************************" << endl;
    cout << "* A kizárandó tolózárak listája *" << endl;
    for (int i = 0; i < TeljesListaID.size(); ++i)
    {   
        for (int j = 0; j < TeljesListaID[i].size(); ++j)
        {
            cout << "*      " << TeljesListaID[i][j] << "   D: " << TeljesListaAtm[i] <<"     *" << endl;//<< "          *" << endl;//
        }
    }
    cout << "*********************************" << endl;
}*/



/*  int count = edgeVector.size();
  vector< vector<int> > connections(connectingEdges.size());
  vector<int> connectionsOriginal(connectingEdges.size());
  int untilThis=1;
  for(int i=0; i<connectingEdges.size(); i++)
  {
    for(int j=0; j<edgeVector.size(); j++)
    {
      if(connectingEdges[i] == edgeVector[j])
      {
        connectionsOriginal[i] = edgeVector[j];
        connections[i].push_back(count + untilThis);
        edgeVector[j] = (count + untilThis);
        untilThis++;
        changeTo += 1;
      }
    }
  }

  cout << endl << "Network blow up: OK" << endl;

  vector< vector<int> > segmentVector = segmenter(edgeVector);
  vector< vector<int> > connectionsBTWSegments;
  vector<int> segmentEdgeVector;

  for (int i = 0; i < connectionsOriginal.size(); ++i)
  {
      connectionsBTWSegments.push_back(vector<int>());
      for (int j = 0; j < connections[i].size(); ++j)
      {
          for (int k = 0; k < segmentVector.size(); ++k)
          {
              for (int l = 0; l < segmentVector[k].size(); ++l)
              {
                  if (segmentVector[k][l] == connections[i][j])
                  {
                      segmentVector[k][l] = connectionsOriginal[i];
                      connectionsBTWSegments[i].push_back(k);
                      changeBack += 1;
                  }
              }
          }
      }
  }
  if( changeTo == changeBack )
  {
      cout << "segmentation completed" << endl;
  }
  int connectionStack, connectionCounter;
  for (int i = 0; i < connectionsBTWSegments.size(); ++i)
  {
      connectionCounter = 0;
      for (int j = 0; j < connectionsBTWSegments[i].size(); ++j)
      {
          connectionCounter += 1;
          if(connectionCounter == 1)
          {
              connectionStack = connectionsBTWSegments[i][j];
          }
          if (connectionCounter == 2)
          {
              cout << "EV1: " << connectionStack;
              segmentEdgeVector.push_back(connectionStack);
              cout << "EV2: " << connectionsBTWSegments[i][j] << " ";
              segmentEdgeVector.push_back(connectionsBTWSegments[i][j]);
          }
      }
      cout << endl;
  }
  for (int i = 0; i < segmentEdgeVector.size(); ++i)
  {
      cout << "Szegmens: " << segmentEdgeVector[i] << endl;
  }
  cin.get();
  vector<int> segmentsCoordinatesX;
  vector<int> segmentsCoordinatesY;
  for (int i = 0; i < segmentVector.size(); ++i)
  {
      segmentsCoordinatesX.push_back(0);
      segmentsCoordinatesY.push_back(0);
      for (int j = 0; j < segmentVector[i].size(); ++j)
      {
  //        cout << "  " << nodes.at(segmentVector[i][j])->Get_nev() << endl;//nodes.at(segmentVector[i][j])->getEdgeDoubleProperty("xcoord");
          segmentsCoordinatesX[i] += nodes.at(segmentVector[i][j])->getProperty("xPosition");
          segmentsCoordinatesY[i] += nodes.at(segmentVector[i][j])->getProperty("yPosition");
      }
      segmentsCoordinatesX[i] /= segmentVector[i].size();
      segmentsCoordinatesY[i] /= segmentVector[i].size();
      cout << i <<". Szegmens X: " << segmentsCoordinatesX[i] << endl;
      cout << i <<". Szegmens Y: " << segmentsCoordinatesY[i] << endl;
  }
  // planner("PIPE_6290143", segmentVector);//PIPE_1351966PIPE_777945,PIPE_1386965,PIPE_1388906, PIPE_1388906,PIPE_780652,PIPE_1351966
  if(segmentEdgeVector.size() % 2 != 0)
  {
      cout << "INVALID EDGE VECTOR" << endl;
      cin.get();
  }
  for (int i = 0; i < nodes.size(); ++i)
  {
      cout << i << ". segment's demand " << nodes.at(i)->getProperty("demand") << endl;
      if(nodes.at(i)->getProperty("demand") != 0)
      {
          sum += nodes.at(i)->getProperty("demand");
      }
      cout << "The summarized demand of the system: " << sum << endl;
  }*/