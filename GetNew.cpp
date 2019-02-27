#include "/mnt/d/Linux/Staci_30/staci_3.0/GetNew.h"
//#include "/mnt/d/Linux/Staci_30/staci_3.0/Staci.h"
//#include "IOxml.h"

GetNew::GetNew(vector<Node *> &nodes, vector<Edge *> &edges){
  newedges = edges;
  newnodes = nodes;
  cout << endl << "newnodes: " << newnodes.size() << endl; 
  cin.get();
}
GetNew::~GetNew(){
}
void GetNew::PiPe(const string nodeOfInflow, const string nodeOfOutflow, const double density, const double D, const double erdesseg)
{
  double X1, X2, Y1, Y2, cl_k = 0.0, cl_w = 0.0, mass_flow_rate = 0.0, L;
  for (int i = 0; i < newnodes.size(); ++i)
  {
    if(newnodes[i]->getName() == nodeOfInflow)
    {
      newnodes[i]->edgeOut.push_back(newedges.size());
      X1 = newnodes[i]->getXPosition();
      Y1 = newnodes[i]->getYPosition();
    }
    if(newnodes[i]->getName() == nodeOfOutflow)
    {
      newnodes[i]->edgeIn.push_back(newedges.size());
      X2 = newnodes[i]->getXPosition();
      Y2 = newnodes[i]->getYPosition();
    }
  }
  //L = sqrt((X2-X1)^2 + (Y2-Y1)^2);
  L = 15; 
  newedges.push_back(new Pipe("NEW_PIPE_1", nodeOfInflow, nodeOfOutflow, density, L, D, erdesseg, mass_flow_rate));
  HaveExtraItems(true);
}
double GetNew::readMassFlow()
{
  double massFlowRate = 0.0;
  for (int i = 0; i < newedges.size(); ++i)
  {
    if (newedges.at(i)->getName() == "NEW_PIPE_1")
    {
      massFlowRate = newedges.at(i)->getMassFlowRate();
    }
  }
  return massFlowRate;
}
void GetNew::ListPipes()
{
  for (int i = 0; i < newedges.size(); ++i)
  {
    cout << "pipe " << i << " ID: " << newedges.at(i)->getName() << endl;
  }
}
bool GetNew::HaveExtraItems(bool Used)
{
  return Used;
}