#include <iomanip>
#include <string>
#include <vector>
#include "Edge.h"
#include "Pump.h"

using namespace std;

Pump::Pump(const string a_name, const string a_startNodeName, const string a_endNodeName, const double a_density, const double referenceCrossSection, vector<double> a_q, vector<double> a_H, const double a_mp) : Edge(a_name, referenceCrossSection, a_mp, a_density) {
    //Kotelezo adatok minden Edgenel:
    type = "Pump";
    numberNode = 2;
    startNodeName = a_startNodeName;
    endNodeName = a_endNodeName;
    // jelleggorbe adatok
    q = a_q;
    H = a_H;
    order = 3; // sum a_i x^i, tehat ha order=2, csak egyenesrol van szo.

    // A terfogataram m3/h-ban erkezik:
    for (unsigned int i = 0; i < q.size(); i++) q.at(i) /= 3600.;

    //TODO: nr könyvtár helyett eigen/saját algoritmusok

    /*//cout<<"\n Gorbeillesztes a jellegorbe pontokra:\n";
    Mat_DP A(order, order);
    Vec_DP b(order);
    Vec_INT indx(order);
    DP d;
    //    for (int i=0; i<q.size(); i++) cout<<endl<<"++++ q("<<i<<")="<<q.at(i)<<", H("<<i<<")="<<H.at(i);
    // b eloallitasa:
    for (int i = 0; i < order; i++) {
        b[i] = 0.0;
        for (unsigned int k = 0; k < q.size(); k++)
            b[i] += H.at(k) * pow(q.at(k), i);
        //        cout<<"\nb["<<i<<"]="<<b[i]<<endl<<"A["<<i<<",:]=";
        for (int j = 0; j < order; j++) {
            A[i][j] = 0.0;
            for (unsigned int k = 0; k < q.size(); k++)
                A[i][j] += pow(q.at(k), i) * pow(q.at(k), j);
            //            cout<<"\t"<<A[i][j];
        }
    }
    //cout<<endl;
    NR::ludcmp(A, indx, d);
    NR::lubksb(A, indx, b);

    // A polinom adatainak visszatoltese:
    for (int i = 0; i < order; i++)
        p.push_back(b[i]);

    mer_szorzo = 10;

    if (p.at(1) > 0)
        if (debug_level > 0)
            cout << endl << "\tWarning! PUMP: " << name << ":  dH/dQ(0)=" << p.at(1) << " > 0 !!!";*/

    //cout<<endl<<"Az illesztett polinom egyenlete:"<<endl;
    //for (int i=0; i<order; i++) cout<<"\t"<<p[i];
    //cout<<endl;

    /*      cout << endl << endl;
          vector<double> xx;
          vector<double> yy;
          double Qmax = q.at(q.size() - 1);
          cout << endl << name << ": Qmax=" << Qmax * 3600;
          cin.get();
          double xmin = -0.2*Qmax;
          double xmax = 1.2 * Qmax;
          double dx = (xmax - xmin) / 50;
          double x, y;
          for (unsigned int i = 0; i < 50; i++)
          {
              x = xmin + i * dx;
              y = 0;
              cout << endl << x * 3600 << "\t" << pumpCharCurve(x);
          }
          cout << endl;
          cin.get();
      */
}

//--------------------------------------------------------------
Pump::~Pump() {}

//--------------------------------------------------------------
string Pump::info() {
    /*ostringstream strstrm;
    strstrm << Edge::info();
    strstrm << endl << "       tipusa : " << type;
    strstrm << endl << "  kapcsolodas : " << startNodeName << "(index:" << cspe_index << ") --> " << endNodeName << "(index:"
            << cspv_index << ")\n";
    cout << setprecision(3);
    vector<double>::iterator it;
    strstrm << "       adatok : Q [m3/h]= ";
    for (unsigned int i = 0; i < q.size(); i++) strstrm << q.at(i) * 3600 << "  ";
    strstrm << "\n";
    strstrm << "                H [m]   = ";
    for (unsigned int i = 0; i < H.size(); i++) strstrm << H.at(i) << "  ";
    strstrm << "\n";
    strstrm << "       illesztett polinom: H= sum(i=0,order) p_i * q^i" << endl << "\t";
    strstrm << scientific << setprecision(3);
    for (unsigned int i = 0; i < p.size(); i++) strstrm << "p[" << i << "]=" << p.at(i) << "  ";
    strstrm << "\n";
    cin.get();
    return strstrm.str();*/
    return "OK";
}

//! Pump branch equation
/*!
The function evaluates the curve fit by the constructor \sa Pump()
\param x vector<double>
\param x(0)=pstart/density/gravity
\param x(1)=pend/density/gravity
\param x(2)=zstart
\param x(3)=zend
\return (double) function error (should be zero)
\sa pumpCharCurve()
*/
double Pump::function(vector<double> x) {
    double result;
    double pe = x[0] * density * gravity;
    double pv = x[1] * density * gravity;
    double he = x[2];
    double hv = x[3];

    result = (pv - pe) / density / gravity - pumpCharCurve(massFlowRate / density) + (hv - he);

    return result;
}

//! Pump performance curve evaluation
/*!
The function evaluates the curve fit by the constructor \sa Pump()
\param qq (double) flow rate in m^3/s
\return (double) head in m
\sa Pump() and f()
*/
double Pump::pumpCharCurve(double qq) {

    double He = 0.0;
    double qmax = q.at(q.size() - 1);
    double Hmin = H.at(H.size() - 1);
    if (qq < 0) {
        He = -mer_szorzo * p[0] / qmax * qq + p[0];
        //if (debug_level > 0)
        //    cout << endl << "\tWarning! PUMP: " << name << ": Q=" << (qq * 3600) << " m^3/h < 0, H=" << He << " m, p[0]=H(0)=" << p[0];
    } else {
        if (qq < qmax)
            for (int i = 0; i < order; i++) He += p[i] * pow(qq, i);
        else {
            He = -mer_szorzo * p[0] / qmax * (qq - qmax) + Hmin;
            //if (debug_level > 0)
            //    cout << endl << "\tWarning! PUMP: " << name << ": Q=" << (qq * 3600) << " m^3/h > Qmax, extrapolating on the performace curve gives H=" << He << " m";
        }
    }
    return He;
}


//--------------------------------------------------------------
vector<double> Pump::functionDerivative(vector<double> x) {
    vector<double> result;
    result.push_back(-1.0);
    result.push_back(+1.0);

    double der = 0.0;
    double qmax = q.at(q.size() - 1);
    if ((massFlowRate < 0) || (massFlowRate > qmax * density))
        der = -mer_szorzo * p[0] / qmax;
    else
        for (int i = 1; i < order; i++) der += p[i] * i * pow(massFlowRate / density, i - 1);
    der /= -density;

    // in case of negative derivative, we overwrite with zero!
    if (der < 0)
        der = 0;

    result.push_back(der);

    result.push_back(0.0);

    return result;
}

//--------------------------------------------------------------
void Pump::initialization(int mode, double value) {
    if (mode != 0)
        massFlowRate = value;
}

//--------------------------------------------------------------
void Pump::setProperty(string mit, double mire) {
    cout << endl << "HIBA! Pump::setProperty(mit), ismeretlen bemenet: mit=" << mit << endl << endl;
}

//--------------------------------------------------------------
double Pump::getProperty(string mit) {

    double out = 0.0;
    if (mit == "referenceCrossSection")
        out = referenceCrossSection;
    else if (mit == "massFlowRate")
        out = massFlowRate;
    else if (mit == "headLoss")
        out = abs(pumpCharCurve(massFlowRate / density));
    else if (mit == "headloss_per_unit_length")
        out = abs(pumpCharCurve(massFlowRate / density));
    else if ((mit == "length") || (mit == "L"))
        out = 0.5;
    else {
        cout << endl << "HIBA! Pump::getProperty(mit), ismeretlen bemenet: mit="
             << mit << endl << endl;
        out = 0.0;
    }
    return out;
}
