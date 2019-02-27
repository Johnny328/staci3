#ifndef NODE_H
#define NODE_H

#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <cmath>
#include <ctime>
#include <stdlib.h>

using namespace std;

class Node
{
public:
    vector<int> edgeIn;
    vector<int> edgeOut;

private:
    string name;
    double head;
    double density;
    double xPosition, yPosition;
    double geodeticHeight;
    double demand;
    double user1, user2;

public:
    Node(const string a_name, const double a_xPosition, const double a_yPosition, const double a_geodeticHeight, const double a_demand, const double a_head, const double a_density);
    ~Node();
    string info(bool check_if_lonely);
    void setHead(double x)
    {
        head = x;
    }
    double getXPosition()
    {
        return xPosition;
    }
    double getYPosition()
    {
        return yPosition;
    }
    double getDemand()
    {
        return demand;
    }
    void setDemand(double a_fogy)
    {
        demand = a_fogy;
    }
    string getName()
    {
        return name;
    }
    double getHead()
    {
        return head;
    }
    double getGeodeticHeight()
    {
        return geodeticHeight;
    }
    void setGeodeticHeight(double a_h)
    {
        geodeticHeight = a_h;
    }
    void setUser1(double val) {
        user1 = val;
    }
    void setUser2(double val) {
        user2 = val;
    }
    double getUser1() {
        return user1;
    }
    double getUser2() {
        return user2;
    }

    void initialization(int mode, double value);
    void setProperty(string mit, double value);
    double getProperty(string mit);
};
#endif
