#include "Node.h"

Node::Node(const string a_name, const double a_xPosition, const double a_yPosition, const double a_geodeticHeight, const double a_demand, const double a_head, const double a_density)
{
    density = a_density;
    demand = a_demand / 3600. * density;
    geodeticHeight = a_geodeticHeight;
    xPosition = a_xPosition;
    yPosition = a_yPosition;
    name = a_name;
    head = a_head;
    user1 = 0.;
    user2 = 0.;
}

//--------------------------------------------------------------
Node::~Node()
{
}

//--------------------------------------------------------------
string Node::info(bool check_if_lonely)
{
    ostringstream strstrm;
    strstrm << "\n       Node name: " << name;
    strstrm << "\n          height: " << geodeticHeight << " m";
    strstrm << "\n            head: " << head << " m (=p[Pa]/density/g)";
    strstrm << "\n        pressure: " << head*density * 9.81 << " Pa";
    strstrm << "\n         desnity: " << density << " kg/m3";
    strstrm << "\n     consumption: " << demand << " kg/s = " << demand * 3600 / density << " m3/h";
    strstrm << "\n  incoming edges: ";
    for (vector<int>::iterator it = edgeIn.begin(); it != edgeIn.end(); it++)
        strstrm << *it << " ";
    strstrm << "\n  outgoing edges: ";
    for (vector<int>::iterator it = edgeOut.begin(); it != edgeOut.end(); it++)
        strstrm << *it << " ";
    strstrm << endl;

    if (check_if_lonely && ((edgeIn.size() + edgeOut.size()) == 0))
    {
        strstrm << "\n!!! PANIC !!! Lonely node: " << name
                << " !!!\n";
        cout << strstrm.str();
        exit(-1);
    }

    return strstrm.str();
}

//--------------------------------------------------------------
void Node::initialization(int mode, double value)
{
    if (mode == 0)
        head = 300. - geodeticHeight;
    else
        head = value - geodeticHeight;
}

//--------------------------------------------------------------
void Node::setProperty(string mit, double value)
{
    if(mit == "demand")
        demand = value / 3600 * density;
}

//--------------------------------------------------------------
double Node::getProperty(string mit)
{

    bool gotIt = false;
    double dataOut = 0.0;

    if(mit == "demand")
    {
        gotIt = true;
        // kg/s-ban taroljuk, de m3/h-ban adjuk vissza
        dataOut = demand * 3600 / density;
    }

    if(mit == "head")
    {
        gotIt = true;
        dataOut = head / density / 9.81;
    }
    if(mit == "pressure")
    {
        gotIt = true;
        dataOut = head;
    }

    if(mit == "density")
    {
        gotIt = true;
        dataOut = density;
    }

    if(mit == "height")
    {
        gotIt = true;
        dataOut = geodeticHeight;
    }

    if(!gotIt)
    {
        cout << endl << endl << "Node::Get_dprop() wrong argument:" << mit;
        cout << ", right values: chlorineIn|concentration|demand|pressure|head|density|travelTime|height" << endl << endl;
    }
    return dataOut;
}
