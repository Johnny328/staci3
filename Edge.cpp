#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <cmath>
#include <ctime>
#include <stdlib.h>
#include "Edge.h"

Edge::Edge(const string a_name, const double a_referenceCrossSection, const double a_massFlowRate, const double a_density) {
    string type;
    massFlowRate = a_massFlowRate;
    density = a_density;
    name = a_name;
    referenceCrossSection = a_referenceCrossSection;
    startNodeIndex = -1;
    endNodeIndex = -1;
    string startNodeName = "nincs_cspe_nev";
    endNodeName = "nincs_cspv_nev";
    fluidVolume = 0;

    if (fabs(a_density) < 1.0e-3)
        error("Edge constructor", "Density (density) is zero up to machine precision!");

    user1=0.;
    user2=0.;
}

//--------------------------------------------------------------
/*Edge::Edge(const string a_name, const double a_referenceCrossSection, const double a_massFlowRate, const double a_density, const double a_tt) {
    string type;
    massFlowRate = a_massFlowRate;
    density = a_density;
    name = a_name;
    referenceCrossSection = a_referenceCrossSection;
    startNodeIndex = -1;
    endNodeIndex = -1;
    string startNodeName = "nincs_cspe_nev";
    endNodeName = "nincs_cspv_nev";
    fluidVolume = 0;

    startTravelTime = a_tt * 3600;
    endTravelTime = a_tt * 3600;

    if (fabs(a_density) < 1.0e-3)
        error("Edge constructor", "Density (density) is zero up to machine precision!");
}*/

//--------------------------------------------------------------
Edge::~Edge() {
}

//--------------------------------------------------------------
string Edge::info() {
    ostringstream strstrm;
    strstrm << "\n Edge name             : " << name;
    strstrm << "\n type                  : " << type;
    strstrm << "\n density               : " << density << " [kg/m^3]";
    strstrm << "\n referenceCrossSection : " << referenceCrossSection << " [m^2]";
    strstrm << "\n mass flow rate        : " << massFlowRate / density * 3600 << " [m3/h]";
    return strstrm.str();
}

//--------------------------------------------------------------
void Edge::addNodes(const int a_startNodeIndex, const int a_endNodeIndex) {
    startNodeIndex = a_startNodeIndex;
    endNodeIndex = a_endNodeIndex;
}

//--------------------------------------------------------------
void Edge::error(string fv, string msg) {
    ostringstream strstrm;
    strstrm.str("");
    strstrm << "\n\n******** ERROR *********";
    strstrm << "\n\telement name: " << name;
    strstrm << "\n\tmethod      : " << fv;
    strstrm << "\n\tmessage     : " << msg << "\n\n";
    //logfileWrite(strstrm.str(), 0);
    cout << strstrm.str();
    exit(0);
}

//--------------------------------------------------------------
//void Edge::setLogFile()
//{
//    outputFileName = name + ".out";
//    ofstream outputFile;
//    outputFile.open(outputFileName.c_str());
//    outputFile.close();
//};

//--------------------------------------------------------------
//vector<double> Edge::Get_res(string mit) {
//    vector<double> x;
//    return x;
//}

//--------------------------------------------------------------
/*vector<double> Edge::interp(vector<double> x, vector<double> y, vector<double> xg) {
    vector<double> yg;
    double xp, xn, yp, yn;

    double xmin = 1e100, xmax = -1e100;
    for (unsigned int i = 0; i < x.size(); i++) {
        if (x.at(i) < xmin)
            xmin = x.at(i);
        if (x.at(i) > xmax)
            xmax = x.at(i);
    }
    if (abs(xmin) < 1e-8)
        xmin = 0.0;

    for (unsigned int i = 0; i < xg.size(); i++) {
        if (xg.at(i) < xmin || xg.at(i) > xmax) {
            cout << endl << endl
                 << "!!!element/basic/interp interp hiba!!! Interpolacio kiserlet a tartmanyon kivulre:";
            cout << endl << "\t xmin=" << xmin << " <? " << xg.at(i) << " <? xmax=" << xmax
                 << endl;
        }
        //      if (xg.at(i)<xmin*0.999)
        //          xg.at(i)=xmin;
        //      if (xg.at(i)>xmax*1.001)
        //          xg.at(i)=xmax;
    }

    for (unsigned int i = 0; i < xg.size(); i++) {
        unsigned int j = 0;
        bool megvan = false;
        while ((!megvan) && (j < x.size() - 1)) {
            double ize = (xg.at(i) - x.at(j)) * (xg.at(i) - x.at(j + 1));
            //cout<<endl<<"xg="<<xg.at(i)<<"  x.at("<<j<<")="<<x.at(j)<<"  x.at("<<j+1<<")="<<x.at(j+1)<<"  y.at("<<j<<")="<<y.at(j)<<"  y.at("<<j+1<<")="<<y.at(j+1)<<" ize="<<ize;
            if (ize < 1e-10)
                megvan = true;
            else
                j++;
        }
        if (j == x.size() - 1)
            j--;
        xp = x.at(j);
        xn = x.at(j + 1);
        yp = y.at(j);
        yn = y.at(j + 1);
        yg.push_back(yp + (xg.at(i) - xp) * (yn - yp) / (xn - xp));
        //cout<<endl<<"==>xg="<<xg.at(i)<<"  x.at(j)="<<x.at(j)<<"  x.at(j+1)="<<x.at(j+1)<<"  y.at(j)="<<y.at(j)<<"  y.at(j+1)="<<y.at(j+1)<<" yi="<<yg.at(i);
        //int int1; cin>>int1;
    }
    
     //cout<<endl<<endl<<"Csatorna::interp, az eredeti vektorok:";
     //for (int i=0; i<x.size(); i++) cout<<endl<<scientific<<"\t x ="<<x.at(i)<<" y ="<<y.at(i);
     //cout<<endl<<"Az interpolalt ertekek ";
     //for (int i=0; i<xg.size(); i++) cout<<endl<<scientific<<"\t xg="<<xg.at(i)<<" yg="<<yg.at(i);
     //cout<<endl<<"Csatorna::interp: kesz (kerek egy egesz erteket...)"<<endl<<endl;
     //int int1; cin>>int1;
     
    return yg;
}*/

//--------------------------------------------------------------
//void Edge::set_up_grid(double a_konc, vector<double> a_vel, double a_cL) {
//    //konc.clear(); vel.clear();
//    // Vizminoseg adatok:
//    for (unsigned int i = 0; i < a_vel.size(); i++)
//        konc.push_back(a_konc);
//    for (unsigned int i = 0; i < a_vel.size(); i++)
//        vel.push_back(a_vel.at(i));
//    cL = a_cL;
//    cT = cL / fabs(mean(vel));
//    cdt = cT / vel.size();
//}

//--------------------------------------------------------------
//string Edge::show_grid(double ido) {
//    ostringstream strstrm;
//    strstrm << endl << "A " << name << " Edge seb.- es konc.eloszlasa  t=" << ido
//            << "s-ban, cL=" << cL << "m, cT=" << cT << "s, cdt=" << cdt << "s\n";
//    strstrm << "\n\tv= ";
//    for (unsigned int i = 0; i < vel.size(); i++)
//        strstrm << scientific << showpos << setprecision(2) << vel.at(i) << " ";
//    strstrm << "\n\tc= ";
//    for (unsigned int i = 0; i < vel.size(); i++)
//        strstrm << konc.at(i) << " ";
//    strstrm << "\n";
//    return strstrm.str();
//}

//--------------------------------------------------------------
/*double Edge::mean(vector<double> x) {
    double mean = 0;
    for (unsigned int i = 0; i < x.size(); i++)
        mean += x.at(i);
    return mean / x.size();
}*/

//--------------------------------------------------------------
//void Edge::logfileWrite(string msg, int msg_debug_level) {
//    if (debug_level >= msg_debug_level) {
//        ofstream outfile(outputFileName.c_str(), ios::app);
//        outfile << msg;
//        outfile.close();
//    }
//}