#include <string>
#include <fstream>
#include <vector>
#include "Node.h"
#include "Edge.h"
#include "xmlParser.h"

class IOxml
{

public:
    /// Konstruktor
    IOxml(const char *xml_fnev);
    void load_system(vector<Node *> &nodes, vector<Edge *> &edges);
    void load_ini_values(vector<Node *> &nodes, vector<Edge *> &edges);
    void writingTagValue(XMLNode node, string tag_name, double tag_value);
    void save_results(double fluidVolume, double sum_of_inflow, double sum_of_demand, vector<Node *> nodes, vector<Edge *> edges, bool conv_reached, int staci_debug_level);
    void save_mod_prop(vector<Node *> nodes, vector<Edge *> edges, string eID, string pID, bool is_property_general);
    void save_mod_prop_all_elements(vector<Node *> nodes, vector<Edge *> edges, string pID);
    //void save_transport(int mode, vector<Node *> nodes, vector<Edge *> edges);
    string read_setting(string which);

private:
    /// debug info a kepernyore
    const char *xml_fnev;
    /// debug info a kepernyore
    bool debug;
    /// Nodeok es agak szama
    int nodeNumber, edgeNumber;
    /// agelem tipusok
    vector<string> edge_type;
    int edge_type_number;
    vector<int> edge_type_occur;
    /// gorbek kiolvasasa
    void curve_reader(const string id, const XMLNode node, vector<double> &, vector<double> &);
    double string_to_double( const string &s , const string &elem_name, const string &tag_name, const double &def_value);
};
