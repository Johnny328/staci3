#ifndef EDGE_H
#define EDGE_H
#include <iostream>
#include <vector>
using namespace std;

class Edge
{
protected:
    /// Tomegaram, [kg/s]
    double massFlowRate;
    /// Suruseg, [kg/m3]
    double density;
    /// Referencia keresztmetszet, [m2], a sebesseg kiszamitasahoz hasznaljuk
    double referenceCrossSection;
    /// Pi
    //const double pi = 3.1416;
    /// g
    const double gravity = 9.81;
    /// Az Edge neve
    string name;
    /// Az Edge tipusa, pl. Cso, Csatorna, stb.
    string type;
    /// Az elejen es vegen levo csompont indexe
    int startNodeIndex, endNodeIndex;
    /// Az elejen es vegen levo csompont neve
    string startNodeName, endNodeName;
    int numberNode;
    /// Folyadekterfogat
    double fluidVolume;
    /// Nyomaseses
    double headLoss;
    /// Standard error message
    void error(string fv, string msg);
    string outputFileName;
    //int debug_level;
    /// vizkor az elejen
    //double startTravelTime;
    /// vizkor a vegen
    //double endTravelTime;
    /// custom data
    double user1, user2;

public:
    //void logfileWrite(string msg, int msg_debug_level);
    /// Konstruktor
    Edge(const string a_name, const double a_referenceCrossSection, const double a_massFlowRate, const double a_densitiy);
    /// Konstruktor with travel time
    //Edge(const string name, const double referenceCrossSection, const double massFlowRate, const double density, const double tt);
    /// Destruktor
    virtual ~Edge();

    /// Csomopont beallitasa
    virtual void addNodes(const int startNodeIndex, const int endNodeIndex);
    /// Az agegyenlet erteke, nullara rendezve, v.o.m.-ben
    virtual double function(vector<double>) = 0;
    /// Jacobi: df/dhe, df/dhv, df/dmp, konstans tag
    virtual vector<double> functionDerivative(vector<double>) = 0;
    /// Inicializacio, mode=0 -> automatikus, mode=1 -> value beirasa
    virtual void initialization(int mode, double value) = 0;
    /// Get double property, Cso es Csatorna akarja elulirja
    virtual double getProperty(string mit) = 0;
    /// Informacio
    virtual string info();
    /// Get double property, Cso es Csatorna akarja felulirja
    double getReferenceCrossSection()
    {
        return referenceCrossSection;
    }
    /// Get equation derivative w.r.t. parameter
    virtual double getFunctionDerivative(string mit)
    {
        return 0.0;
    }
    virtual void setProperty(string mit, double mire) {

    };

    virtual void setFrictionModel(string friction_model) {};

    double getMassFlowRate()
    {
        return massFlowRate;
    }
    void setMassFlowRate(double x)
    {
        massFlowRate = x;
    }
    double getVolumeFlowRate()
    {
        return massFlowRate / density;
    }
    double virtual getVelocity()
    {
        return massFlowRate / density / referenceCrossSection;
    }
    string getName()
    {
        return name;
    }
    string getType()
    {
        return type;
    }
    string getStartNodeName()
    {
        return startNodeName;
    }
    string getEndNodeName()
    {
        return endNodeName;
    }
    int getStartNodeIndex()
    {
        return startNodeIndex;
    }
    int getEndNodeIndex()
    {
        return endNodeIndex;
    }
    int getNumberNode()
    {
        return numberNode;
    }
    //double Get_tt_start()
    //{
    //    return startTravelTime;
    //}
    //double Get_tt_end()
    //{
    //    return endTravelTime;
    //}
    //void Set_tt_start(double tmp)
    //{
    //    startTravelTime = tmp;
    //}
    //void Set_tt_end(double tmp)
    //{
    //    endTravelTime = tmp;
    //}
    virtual double getFluidVolume()
    {
        return fluidVolume;
    }
    virtual void setFluidVolume()
    {
    }
    //virtual void build_res()
    //{
    //}
    //virtual string GetType() = 0;
    /// Eredmenyvektor visszaadasa (csak Csatorna eseten)
    //virtual vector<double> Get_res(string which);
    /// Matlab-szeru linearis interpolacio
    //vector<double> interp(vector<double> x, vector<double> y, vector<double> xg);
    /// Atlag szamitasa
    //double mean(vector<double> x);
    /// Vizminoseghez numerikus halo
    //void set_up_grid(double a_konc, vector<double> a_vel, double a_cL);
    /// Vizminoseghez numerikus halo kiirasa
    //string show_grid(double time);
    /// Vizminoseghez koncentracio- es sebessegeloszlas
    //vector<double> concentrationDistribution, velocityDistribution;
    /// Atlagos koncentracio
    //double concentration;
    /// Vizminoseg szamitashoz belso time nyilvantartasa
    //double time;
    /// Hossz, Csatorna es Cso eseten maga a hossz, az osszes tobbi elemnel 1[m]
    //double cL;
    /// Az Edge vegigjarasahoz szukseges time es idolepes
    //double cT, cdt;
    double getHeadLoss()
    {
        return headLoss;
    }
    void setHeadLoss(double a_headLoss)
    {
        headLoss = a_headLoss;
    }
    double getDensity()
    {
        return density;
    }
    //void Set_cdt(double a_cdt)
    //{
    //    cdt = a_cdt;
    //}
    //void SetLogFile();

    void setReferenceCrossSection(double a_Aref)
    {
        referenceCrossSection = a_Aref;
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
};
#endif
