/**
* @file Staci.h
* @brief Header file of Staci class
* @author Csaba Hos
* @date 09/20/2017
*/
#ifndef STACI_H
#define STACI_H

#include "AnyOption.h"
#include "Node.h"
#include "Edge.h"
#include "GetNew.h"
#include "Statistic.h"
#include "IOxml.h"
#include "StaciException.h"


#include </usr/include/eigen3/Eigen/Eigen>
//#include "/home/rweber/C_libraries/Eigen/Eigen/Eigen"

#include <string>
#include <iomanip>
#include <vector>

using namespace Eigen;

class Staci
{
public:
    Statistic *statTools;
    vector<Node *> nodes;
    vector<Edge *> edges;
    void exportNodesEdges();
    Staci(int argc, char *argv[]);
    Staci(string spr_filename);
    ~Staci();
    //GetNew *InsertNewElement(vector<Node *> nodes, vector<Edge *> edges);
    //InsertNewElement << nodes;
    //InsertNewElement << edges;
    string getOutputFile()
    {
        return outputFile.c_str();
    }
    int getDebugLevel()
    {
        return debugLevel;
    }
    void setDebugLevel(int a)
    {
        debugLevel = a;
    }
    void buildSystem();
    void listSystem();
    void saveProperties(bool is_general_property);
    void saveAllProperties(string propertyID);
    void writeLogFile(string msg, int msg_debug_level);
    void setResultFile(string xml_fnev)
    {
        resultFile = xml_fnev;
    }
    string getResultFile()
    {
        return resultFile;
    }
    void setInitializationFile(string xml_fnev)
    {
        initializationFile = xml_fnev;
    }
    void setOutputFile(string fnev)
    {
        outputFile = fnev;
    }
    string getDefinitionFile()
    {
        return definitionFile;
    }
    int getCalculationType()
    {
        return calculationType;
    }

    void copyFile(const string in_f_nev, const string out_f_nev);
    void setIsInitialization(const bool van_e)
    {
        isInitialization = van_e;
    }
    double getElementProperty(string ID, string prop);
    void setElementProperty(string ID, string prop, double val);

    // ADATMODOSITASHOZ
    string newDefinitionFile, elementID, propertyID;
    double newValue;

    void listAllElements();

    bool performDemandSensitivityAnalysis;
    stringstream consolePrint;

    // TODO
    vector<int> Id2Idx(const vector<string> &id);
    vector<string> ReadStrings(ifstream &file);
    vector<vector<double> > CSVRead(ifstream &file, char separator);
    double get_sum_of_pos_consumption();
    
//private:

    int calculationType, debugLevel;
    AnyOption *opt;
    string definitionFile;
    bool isInitialization;
    string outputFile, initializationFile, resultFile;

    void setInitialParameters();

    void getCommandLineOptions(int argc, char *argv[]);
    string trim(string s, const string drop);
};

#endif //STACI_H