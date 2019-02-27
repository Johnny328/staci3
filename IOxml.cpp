#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <iomanip>
#include <vector>
#include <math.h>
#include <string.h>
#include "PressurePoint.h"
#include "Pipe.h"
#include "Pump.h"
#include "Valve.h"
#include "Pool.h"
#include "Channel.h"
#include "Weir.h"
#include "CheckValve.h"
#include "IOxml.h"
#include "xmlParser.h"

// TODO
// 2014.05.07. HCs. Ha a felhasznalo veletlenul kitorli pl. a klor adatot dnal, elhasal az egesz! Hibavisszajelzest!


IOxml::IOxml(const char *a_xml_fnev) {
    xml_fnev = a_xml_fnev;

    // Itt kenytelen vagyok kulon definialni a rednelkezesre allo elemeket
    // Jobb lenne valahol mashol letrehozni oket, de sajna csak itt megy.
    edge_type.push_back("press");
    edge_type.push_back("pipe");
    edge_type.push_back("pump");
    edge_type.push_back("valve");
    edge_type.push_back("pool");
    edge_type.push_back("channel"); // rectangle cross-section
    edge_type.push_back("channel1"); // circle cross-section
    edge_type.push_back("channel2"); // arbitrary cross-section
    edge_type.push_back("overflow"); // overflow
    edge_type.push_back("checkvalve"); // check valve
    edge_type_number = edge_type.size();

    for (int i = 0; i < edge_type_number; i++)
        edge_type_occur.push_back(0);

    XMLNode xMainNode = XMLNode::openFileHelper(xml_fnev, "staci");

    string cpp_xml_debug = xMainNode.getChildNode("settings").getChildNode("cpp_xml_debug").getText();
    if (strcmp(cpp_xml_debug.c_str(), "true") == 0)
        debug = true;
    else
        debug = false;

}

//--------------------------------------------------------------------------------
void IOxml::load_system(vector<Node *> &nodes, vector<Edge *> &edges) {
    XMLNode xMainNode = XMLNode::openFileHelper(xml_fnev, "staci");

    XMLNode Node_settings = xMainNode.getChildNode("settings");
    XMLNode Node_nodes = xMainNode.getChildNode("nodes");
    XMLNode Node_edges = xMainNode.getChildNode("edges");

    // Nodeok es agak szamanak es nevenek kiolvasasa...
    nodeNumber = Node_nodes.nChildNode("node");
    edgeNumber = Node_edges.nChildNode("edge");
    if (debug)
        cout << endl << "Adatfajl: " << xml_fnev << endl << "\tNodeok szama: "
             << nodeNumber << endl << "\tagak szama:        " << edgeNumber;

    // Nodeok reszletes kiolvasasa...
    if (debug)
        cout << endl << endl << "A Nodeok alapadatai reszletesen: " << endl
             << "-----------------------------------------" << endl;

    string id, is_endnode;
    double height, demand, pressure, density, xpos, ypos;
    int j = 0;
    for (int i = 0; i < nodeNumber; i++) {
        is_endnode = Node_nodes.getChildNode("node", i).getChildNode("endnode").getText();
        //cout << endl << " NODE #" << i << "/" << nodeNumber << ": endnode=" << is_endnode << "   " << (is_endnode == "false");

        if (is_endnode == "false") {
            id = Node_nodes.getChildNode("node", i).getChildNode("id").getText();

            xpos = string_to_double(Node_nodes.getChildNode("node", i).getChildNode("xpos").getText(), id, "<xpos>", 0.0);

            ypos = string_to_double(Node_nodes.getChildNode("node", i).getChildNode("ypos").getText(), id, "<ypos>", 0.0);

            height = string_to_double(Node_nodes.getChildNode("node", i).getChildNode("height").getText(), id, "<height>", 0.0);

            demand = string_to_double(Node_nodes.getChildNode("node", i).getChildNode("demand").getText(), id, "<demand>", 0.0);

            //cl_be = string_to_double(Node_nodes.getChildNode("node", i).getChildNode("cl_input").getText(), id, "<cl_input>", 0.0);

            pressure = string_to_double(Node_nodes.getChildNode("node", i).getChildNode("pressure").getText(), id, "<pressure>", 0.0);

            density = string_to_double(Node_nodes.getChildNode("node", i).getChildNode("density").getText(), id, "<pressure>", 0.0);

            //tt = string_to_double(Node_nodes.getChildNode("node", i).getChildNode("travel_time").getText(), id, "<travel_time>", 0.0);

            nodes.push_back(new Node(id, xpos, ypos, height, demand, pressure, density));

            if (debug)
                cout << nodes.at(j)->info(false);
            j++;
        }
    }
    // nodeNumber = j--;

    if (debug)
        cin.get();

    // Agak reszletes kiolvasasa...
    // if (debug)
    //     cout << endl << endl << "Az agak: " << endl
    //          << "-----------------------------------------" << endl;
    // Eloszor csak az info kedveert megszamoljuk a elemszamot
    int count;
    bool gotIt;
    if (debug) {
        for (int i = 0; i < edgeNumber; i++) {
            gotIt = false;
            count = 0;
            for (int j = 0; j < edge_type_number; j++) {
                count = Node_edges.getChildNode("edge", i).getChildNode("edge_spec", 0).nChildNode(
                         edge_type.at(j).c_str());
                if (count == 1) {
                    edge_type_occur.at(j) += 1;
                    gotIt = true;
                }
            }
            //cout<<endl<<"\t"<<(i+1)<<". edge: ";
            //if (gotIt)
            //  cout<<edge_type.at(ezaz);
            //else
            //  cout<<"????";
        }
    }

    if (debug)
        cout << endl << endl << "Az agak reszletesen:" << endl
             << "-----------------------------------------" << endl;
    string node_from, node_to;
    double aref, mass_flow_rate;
    for (int i = 0; i < edgeNumber; i++) {
        id = Node_edges.getChildNode("edge", i).getChildNode("id").getText();
        // Laci vicceskedett, vesszovel irta be, igy lecsereljuk pontra.
        // Fasza, 5 oram ment el ezzel.
        string s = Node_edges.getChildNode("edge", i).getChildNode("aref").getText();

        replace( s.begin(), s.end(), ',', '.');
        aref = atof(s.c_str());

        if (aref < 1.e-3 * 1.e-3) {
            printf("\nWARNING!!! Reference area of %s is %g, overriding with 1. m^2.", id.c_str(), aref);
            cout << endl << "String in xml file: " << s;
            aref = 1.;
            cin.get();
        }

        node_from = Node_edges.getChildNode("edge", i).getChildNode("node_from").getText();

        node_to = Node_edges.getChildNode("edge", i).getChildNode("node_to").getText();
  
        density = atof(Node_edges.getChildNode("edge", i).getChildNode("density").getText());
 
        mass_flow_rate = atof(Node_edges.getChildNode("edge", i).getChildNode("mass_flow_rate").getText());
  
        //travel_time = atof(Node_edges.getChildNode("edge", i).getChildNode("travel_time").getText());
        if (debug)
            cout << endl << (i + 1) << ". edge: " << id << ", node_from=" << node_from << ", node_to=" << node_to << ", density=" << density << ", aref=" << aref;

        for (int j = 0; j < edge_type_number; j++) {
            count = Node_edges.getChildNode("edge", i).getChildNode("edge_spec").nChildNode(edge_type.at(j).c_str());
            if (count == 1) {
                XMLNode elem = Node_edges.getChildNode("edge", i).getChildNode("edge_spec").getChildNode(
                                   edge_type.at(j).c_str());
                switch (j) {
                case 0: // PRESSUREPOINT
                {
                    double pres = atof(elem.getChildNode("pressure").getText());
                    if (debug)
                        cout << ", pressure=" << pres << "Pa";
                    edges.push_back(new PressurePoint(id, aref, node_from, density, pres, mass_flow_rate));
                    if (debug)
                        cout << " OK";
                    break;
                }

                case 1: // PIPE
                {
                    double L = atof(elem.getChildNode("length").getText());
                    double D = atof(elem.getChildNode("diameter").getText());
                    double roughness = atof(elem.getChildNode("roughness").getText());
                    //double cl_k = atof(elem.getChildNode("cl_k").getText());
                    //double cl_w = atof(elem.getChildNode("cl_w").getText());
                    //double b=atof(elem.getChildNode("distr_consumption").getText());
                    if (debug)
                        cout << ", length=" << L << "m, diameter=" << D << "m, roughness=" << roughness << "mm";
                    edges.push_back(new Pipe(id, node_from, node_to, density, L, D, roughness, mass_flow_rate));
                    if (debug)
                        cout << " OK";
                    break;
                }

                case 2: // PUMP
                {
                    int numberCurve = elem.getChildNode("curve").getChildNode("points").nChildNode("point_x");
                    vector<double> Q(numberCurve), H(numberCurve);
                    curve_reader(id, elem.getChildNode("curve"), Q, H);
                    edges.push_back(new Pump(id, node_from, node_to, density, aref, Q, H, mass_flow_rate));
                    if (debug)
                        cout << " OK";
                    break;
                }

                case 3: // VALVE
                {
                    double allas = atof(elem.getChildNode("position").getText());

                    if (debug)
                        cout << ", actual setting=" << allas;
                    int numberCurve = elem.getChildNode("curve").getChildNode("points").nChildNode("point_x");
                    vector<double> e(numberCurve), zeta(numberCurve);
                    curve_reader(id, elem.getChildNode("curve"), e, zeta);
                    if (aref < 1e-5) {
                        double Aref_min = 3.1416e-04; // 2cm-es Pipe belvilaga
                        aref = Aref_min;
                        cout << "Warning! element " << id << " Aref=" << aref << ", overwriting with " << Aref_min << endl;
                    }
                    if (allas > e.at(e.size() - 1))
                        cout << "Warning! element " << id << " actual setting=" << allas << " > e(end) " << e.at(e.size() - 1) << endl;
                    if (allas < e.at(0))
                        cout << "Warning! element " << id << " actual setting=" << allas << " < e(0) " << e.at(0) << endl;

                    edges.push_back(new Valve(id, node_from, node_to, density, aref, e, zeta, allas, mass_flow_rate));
                    if (debug)
                        cout << " OK";
                    break;
                }

                case 4: //POOL
                {
                    double Hb = atof(elem.getChildNode("bottom_level").getText());
                    double Hw = atof(elem.getChildNode("water_level").getText());

                    if (debug)
                        cout << ", bottom_level=" << Hb << "m" << ", water_level=" << Hw << "m";

                    edges.push_back(new Pool(id, node_from, density, aref, Hb, Hw, mass_flow_rate));
                    if (debug)
                        cout << " OK";
                    break;
                }

                //              case 5: //channel0 = T�glalap km. Channel
                //              {
                //                  double L = atof(elem.getChildNode("length").getText());
                //                  double ze = atof(elem.getChildNode("start_height").getText());
                //                  double zv = atof(elem.getChildNode("end_height").getText());
                //                  //      double inc   = atof(elem.getChildNode("inclination").getText())/100;
                //                  double roughness=atof(elem.getChildNode("roughness").getText());
                //                  int int_steps= atoi(elem.getChildNode("integral_steps").getText());
                //                  int debugl = atoi(elem.getChildNode("debug_level").getText());
                //                  double cl_k=atof(elem.getChildNode("cl_k").getText());
                //                  double cl_w=atof(elem.getChildNode("cl_w").getText());
                //                  if (debug)
                //                      cout<<", length="<<L<<"m"<<", start_height="<<ze
                //                              <<"m, end_height="<<zv<<"m, roughness="
                //                              <<roughness <<", integral_steps="<<int_steps
                //                              <<", debug_level=" <<debug;
                //                  double width = atof(elem.getChildNode("width").getText());
                //                  double Hmax = atof(elem.getChildNode("max_height").getText());
                //
                //                  if (debug)
                //                      cout<<endl<<"\t\t teglalap geometria: B="<<width
                //                              <<"m, Hmax="<<Hmax;
                //                  edges.push_back(new Channel(id,node_from,node_to,aref,L,ze,zv,roughness,int_steps,debugl,width,Hmax,cl_k,cl_w));
                //                  break;
                //              }

                /*case 6: // kor rm.
                {
                    double L = atof(elem.getChildNode("length").getText());
                    double ze = atof(elem.getChildNode("start_height").getText());
                    double zv = atof(elem.getChildNode("end_height").getText());
                    bool is_reversed = false;
                    if (ze < zv) {
                        cout
                                << "\n\n\tNegativ lejtes -> Node CSERE!!!\n\t elem neve: "
                                << id;
                        cout << "\n\t elotte: node_from:" << node_from
                             << ", node_to:" << node_to << ", ze=" << ze << ", zv="
                             << zv;

                        string s_tmp = node_from;
                        node_from = node_to;
                        node_to = s_tmp;

                        double d_tmp = ze;
                        ze = zv;
                        zv = d_tmp;
                        cout << "\n\t utana : node_from:" << node_from
                             << ", node_to:" << node_to << ", ze=" << ze << ", zv="
                             << zv;
                        is_reversed = true;
                    }
                    double roughness = atof(elem.getChildNode("roughness").getText());
                    int int_steps = atoi(elem.getChildNode("integral_steps").getText());
                    int debugl = atoi(elem.getChildNode("debug_level").getText());
                    double cl_k = atof(elem.getChildNode("cl_k").getText());
                    double cl_w = atof(elem.getChildNode("cl_w").getText());
                    if (debug)
                        cout << ", length=" << L << "m" << ", start_height=" << ze
                             << "m, end_height=" << zv << "m, roughness="
                             << roughness << ", integral_steps=" << int_steps
                             << ", debug_level=" << debug;
                    double dia = atof(elem.getChildNode("diameter").getText());

                    if (debug)
                        cout << endl << "\t\t kor geometria: D=" << dia;
                    edges.push_back(
                        new Channel(id, node_from, node_to, density, dia * dia * M_PI / 4., L, ze, zv,
                                     roughness, int_steps, debugl, dia, cl_k, cl_w, is_reversed,
                                     mass_flow_rate));
                    break;
                }*/

                //              case 7: // Felhasznalo altal definialt tipus
                //              {
                //                  double L = atof(elem.getChildNode("length").getText());
                //                  double ze = atof(elem.getChildNode("start_height").getText());
                //                  double zv = atof(elem.getChildNode("end_height").getText());
                //                  //                        double inc   = atof(elem.getChildNode("inclination").getText())/100;
                //                  double roughness=atof(elem.getChildNode("roughness").getText());
                //                  int int_steps= atoi(elem.getChildNode("integral_steps").getText());
                //                  int debugl = atoi(elem.getChildNode("debug_level").getText());
                //                  double cl_k=atof(elem.getChildNode("cl_k").getText());
                //                  double cl_w=atof(elem.getChildNode("cl_w").getText());
                //                  if (debug)
                //                      cout<<", length="<<L<<"m"<<", start_height="<<ze
                //                              <<"m, end_height="<<zv<<"m, roughness="
                //                              <<roughness <<", integral_steps="<<int_steps
                //                              <<", debug_level=" <<debug;
                //                  int jgpszb, jgpsza, jgpszk;
                //                  for (int cdb=0; cdb<elem.nChildNode("curve"); cdb++) {
                //                      if (elem.getChildNode("curve",cdb).getChildNode("id").getText()=="curve_b")
                //                          jgpszb=elem.getChildNode("curve").getChildNode("points").nChildNode("point_x");
                //                      if (elem.getChildNode("curve",cdb).getChildNode("id").getText()=="curve_a")
                //                          jgpsza=elem.getChildNode("curve").getChildNode("points").nChildNode("point_x");
                //                      if (elem.getChildNode("curve",cdb).getChildNode("id").getText()=="curve_k")
                //                          jgpszk=elem.getChildNode("curve").getChildNode("points").nChildNode("point_x");
                //                  }
                //                  vector<double> yb(jgpszb), b(jgpszb), ya(jgpsza), a(jgpsza),
                //                          yk(jgpszk), k(jgpszk);
                //
                //                  for (int cdb=0; cdb<elem.nChildNode("curve"); cdb++) {
                //                      if (elem.getChildNode("curve",cdb).getChildNode("id").getText()=="curve_b")
                //                          curve_reader(id, elem.getChildNode("curve"), yb, b);
                //                      if (elem.getChildNode("curve",cdb).getChildNode("id").getText()=="curve_a")
                //                          curve_reader(id, elem.getChildNode("curve"), ya, a);
                //                      if (elem.getChildNode("curve",cdb).getChildNode("id").getText()=="curve_k")
                //                          curve_reader(id, elem.getChildNode("curve"), yk, k);
                //                  }
                //                  edges.push_back(new Channel(id,node_from,node_to,aref,L,ze,zv,roughness,int_steps,debugl,yb,b,ya,a,yk,k,cl_k,cl_w));
                //                  break;
                //              }

                case 8: // WEIR
                {
                    string iso = elem.getChildNode("is_opened").getText();
                    double bh = atof(elem.getChildNode("bottom_height").getText());
                    bool is_opened = false;
                    if (iso == "true")
                        is_opened = true;
                    double wi = atof(elem.getChildNode("width").getText());
                    double oh = atof(elem.getChildNode("overflow_height").getText());
                    double dc = atof(elem.getChildNode("discharge_coeff").getText());
                    double vc = atof(elem.getChildNode("valve_coeff").getText());
                    if (debug)
                        cout << endl << "is_opened:" << iso << ", width=" << wi << ", overflow_height=" << oh << ", discharge_coeff=" << dc << ", valve_coeff=" << vc;
                    edges.push_back(new Weir(id, node_from, node_to, density, aref, bh, is_opened, wi, oh, dc, vc, mass_flow_rate));
                    if (debug)
                        cout << "  OK";
                    break;
                }
                case 9: // CHECK VALVE
                {
                    double lce = atof(elem.getChildNode("loss_coeff_f").getText()) / 1e5;
                    double lcv = atof(elem.getChildNode("loss_coeff_b").getText()) / 1e5;
                    if (debug) {
                        cout << endl << "loss_coeff_f(orward):" << lce;
                        cout << endl << "loss_coeff_b(ack):" << lcv;
                    }
                    edges.push_back(new CheckValve(id, node_from, node_to, density, aref, lce, lcv, mass_flow_rate));
                    if (debug)
                        cout << "  OK";
                    break;
                }
                default: {
                    cout << endl << endl
                         << "xml adatfajl feldolgozasa: HIBA!!! Sz@r van, nem talalok ilyen tipusu agelemet => "
                         << i << ". edge, id:" << id << endl;
                }
                }
            }
        }
        if (debug)
            cout << edges.at(i)->info();
    }

    if (debug) {
        cout << endl << endl << endl << "Number of edge types:";
        cout << endl << "\tnode:\t" << nodeNumber;
        for (int j = 0; j < edge_type_number; j++)
            cout << endl << "\t" << edge_type.at(j) << ":\t" << edge_type_occur.at(j);
    }

    if (debug)
        cin.get();
}

//--------------------------------------------------------------------------------
void IOxml::curve_reader(const string id, const XMLNode elem,
                           vector<double> &px, vector<double> &py) {

    string curve_id = elem.getChildNode("id").getText();
    string x_val = elem.getChildNode("x_val").getText();
    string y_val = elem.getChildNode("y_val").getText();
    string x_dim = elem.getChildNode("x_dim").getText();
    string y_dim = elem.getChildNode("y_dim").getText();
    int numberX = elem.getChildNode("points").nChildNode("point_x");
    int numberY = elem.getChildNode("points").nChildNode("point_y");

    string tmp;

    if (debug)
        cout << endl << "\t datta_io::curvereader() -> curve_id: " << curve_id << "  x_jgsz=" << numberX << ", y_jgpsz=" << numberY;

    if (numberX == numberY) {
        if (debug)
            cout << ", OK, Reading the file..." << endl;
        double x, y;
        for (int i = 0; i < numberX; i++) {
            tmp = elem.getChildNode("points").getChildNode("point_x", i).getText();
            replace(tmp.begin(), tmp.end(), ',', '.');
            x = atof(tmp.c_str());

            tmp = elem.getChildNode("points").getChildNode("point_y", i).getText();
            replace(tmp.begin(), tmp.end(), ',', '.');
            y = atof(tmp.c_str());

            px.at(i) = x;
            py.at(i) = y;
            if (debug)
                cout << "\t\t " << x_val << "[" << x_dim << "]=" << x << "  " << y_val << "[" << y_dim << "]=" << y << endl;
        }
    } else
        cout << endl << "xml reading: ERROR! number of X =" << numberX << " <-> number of Y =" << numberY << endl << "Moving on " << id << " element..." << endl << endl;

}

/*! WR: function to change the value in a tag inside on the second level + handles the existince of the tag (if there is a new tag it will be added)
*/
void IOxml::writingTagValue(XMLNode node, string tag_name, double tag_value){
    ostringstream os;
    if(node.getChildNode(tag_name.c_str()).isEmpty())
    {
        node.addChild(XMLNode::parseString(("<" + tag_name + "> </" + tag_name + ">").c_str()));
        if(debug)
            cout << "\n The following child has been added: " << tag_name << '\n';
    }
    node.getChildNode(tag_name.c_str()).deleteText();
    os.str("");
    os << scientific << setprecision(5) << tag_value;
    node.getChildNode(tag_name.c_str()).addText(os.str().c_str());
}

//--------------------------------------------------------------------------------
void IOxml::save_results(double fluidVolume, double sum_of_inflow, double sum_of_demand, vector<Node *> nodes, vector<Edge *> edges, bool conv_reached, int staci_debug_level) {

    XMLNode xMainNode = XMLNode::openFileHelper(xml_fnev, "staci");

    if (staci_debug_level > 0)
        cout << endl << endl << "Output file name:" << xml_fnev << ", saving results...";

    XMLNode Node_settings = xMainNode.getChildNode("settings");
    XMLNode Node_nodes = xMainNode.getChildNode("nodes");
    XMLNode Node_edges = xMainNode.getChildNode("edges");

    Node_settings.getChildNode("solution_exists").deleteText();

    ostringstream os;
    if (conv_reached) {
        Node_settings.getChildNode("solution_exists").addText("true");
        Node_settings.getChildNode("fluid_volume").deleteText();
        os.str("");
        os << scientific << setprecision(5) << fluidVolume;
        Node_settings.getChildNode("fluid_volume").addText(os.str().c_str());
    } else
        Node_settings.getChildNode("solution_exists").addText("false");

    writingTagValue(Node_settings, "sum_of_inflow", sum_of_inflow);
    writingTagValue(Node_settings, "sum_of_demand", sum_of_demand);    

    nodeNumber = Node_nodes.nChildNode("node");
    edgeNumber = Node_edges.nChildNode("edge");

    string id, is_endnode;
    bool gotIt;
    double p;
    for (int i = 0; i < nodeNumber; i++) {
        id = Node_nodes.getChildNode("node", i).getChildNode("id").getText();
        gotIt = false;
        if (debug)
            cout << endl << "\tcsp id: " << id << "  ";
        is_endnode = Node_nodes.getChildNode("node", i).getChildNode("endnode").getText();
        if (is_endnode == "false") {
            for (unsigned int j = 0; j < nodes.size(); j++) {
                if (id == (nodes.at(j)->getName())) {
                    Node_nodes.getChildNode("node", i).getChildNode("pressure").deleteText();
                    p = nodes.at(j)->getProperty("density") * 9.81 * nodes.at(j)->getHead();
                    ostringstream os;
                    os << scientific << setprecision(5) << p;
                    Node_nodes.getChildNode("node", i).getChildNode("pressure").addText(os.str().c_str());
                    gotIt = true;

                    Node_nodes.getChildNode("node", i).getChildNode("head").deleteText();
                    p = nodes.at(j)->getHead();
                    os.str("");
                    os << scientific << setprecision(5) << p;
                    Node_nodes.getChildNode("node", i).getChildNode("head").addText(os.str().c_str());

                    writingTagValue(Node_nodes.getChildNode("node",i), "user1", nodes.at(j)->getUser1());
                    writingTagValue(Node_nodes.getChildNode("node",i), "user2", nodes.at(j)->getUser2());
                    writingTagValue(Node_nodes.getChildNode("node",i), "height", nodes.at(j)->getGeodeticHeight());
                }
            }
            if (debug && !gotIt)
                cout << "not found, moving on...";
        } else {
            if (debug)
                cout << " this is endnode ...";
        }
    }
    if (debug) {
        cout << endl << endl << "Writing results of nodes is done." << endl;
    }

    double mp, q, v, dh, dhpL, aref;
    for (int i = 0; i < edgeNumber; i++) {
        id = Node_edges.getChildNode("edge", i).getChildNode("id").getText();
        gotIt = false;
        if (debug)
            cout << endl << "\tag id : " << id << "  ";
        for (unsigned int j = 0; j < edges.size(); j++) {
            if (id == (edges.at(j)->getName())) {
                mp = edges.at(i)->getMassFlowRate();
                Node_edges.getChildNode("edge", i).getChildNode("mass_flow_rate").deleteText();
                ostringstream os;
                os << scientific << setprecision(5) << mp;
                Node_edges.getChildNode("edge", i).getChildNode("mass_flow_rate").addText(os.str().c_str());

                q = edges.at(i)->getVolumeFlowRate();
                Node_edges.getChildNode("edge", i).getChildNode("volume_flow_rate").deleteText();
                os.str("");
                os << scientific << setprecision(5) << q * 3600;
                Node_edges.getChildNode("edge", i).getChildNode("volume_flow_rate").addText(os.str().c_str());

                v = edges.at(i)->getVelocity();
                Node_edges.getChildNode("edge", i).getChildNode("velocity").deleteText();
                os.str("");
                os << scientific << setprecision(5) << v;
                Node_edges.getChildNode("edge", i).getChildNode("velocity").addText(os.str().c_str());

                dh = edges.at(i)->getProperty("headloss");
                Node_edges.getChildNode("edge", i).getChildNode("headloss").deleteText();
                os.str("");
                os << scientific << setprecision(5) << dh;
                Node_edges.getChildNode("edge", i).getChildNode("headloss").addText(os.str().c_str());

                dhpL = edges.at(i)->getProperty("headloss_per_unit_length");
                Node_edges.getChildNode("edge", i).getChildNode("head_loss_per_unit_length").deleteText();
                os.str("");
                os << scientific << setprecision(5) << dhpL;
                Node_edges.getChildNode("edge", i).getChildNode("head_loss_per_unit_length").addText(os.str().c_str());

                //tt = (edges.at(i)->Get_tt_start() + edges.at(i)->Get_tt_end()) / 2. / 3600.;
                //Node_edges.getChildNode("edge", i).getChildNode("travel_time").deleteText();
                //os.str("");
                //os << scientific << setprecision(5) << tt;
                //Node_edges.getChildNode("edge", i).getChildNode("travel_time").addText(os.str().c_str());

                aref = edges.at(i)->getReferenceCrossSection();
                Node_edges.getChildNode("edge", i).getChildNode("aref").deleteText();
                os.str("");
                os << scientific << setprecision(5) << aref;
                Node_edges.getChildNode("edge", i).getChildNode("aref").addText(os.str().c_str());

                writingTagValue(Node_edges.getChildNode("edge",i), "user1", edges.at(i)->getUser1());
                writingTagValue(Node_edges.getChildNode("edge",i), "user2", edges.at(i)->getUser2());

                gotIt = true;
                if (debug)
                    cout << " \t=> mass_flow_rate=" << mp
                         << " kg/s, volume_flow_rate=" << q
                         << " m^3/s, velocity=" << v << " m/s"
                         << " m/s, head_loss=" << dh << " m";

                // Valve with interpolated values
                if (edges.at(j)->getType() == "Valve") {
                    XMLNode akt_node;

                    if (Node_edges.getChildNode("edge", i).getChildNode("edge_spec").nChildNode("valve") > 0)
                        akt_node = Node_edges.getChildNode("edge", i).getChildNode("edge_spec").getChildNode("valve").getChildNode("adzeta");

                    akt_node.deleteText();
                    ostringstream os;
                    os << scientific << setprecision(5) << edges.at(j)->getProperty("veszt");
                    akt_node.addText(os.str().c_str());
                }

                if (edges.at(j)->getType() == "Channel" || edges.at(j)->getType() == "Pipe") {
                    XMLNode akt_node;
                    if (Node_edges.getChildNode("edge", i).getChildNode("edge_spec").nChildNode("channel1") > 0)
                        akt_node = Node_edges.getChildNode("edge", i).getChildNode("edge_spec").getChildNode(
                                       "channel1");
                    if (Node_edges.getChildNode("edge", i).getChildNode("edge_spec").nChildNode("pipe") > 0)
                        akt_node = Node_edges.getChildNode("edge", i).getChildNode("edge_spec").getChildNode("pipe");

                    Node_edges.getChildNode("edge", i).getChildNode("aref").deleteText();

                    ostringstream os;
                    os << scientific << setprecision(5) << edges.at(j)->getProperty("Aref");
                    Node_edges.getChildNode("edge", i).getChildNode("aref").addText(os.str().c_str());
                }

                if (edges.at(j)->getType() == "Channel") {
                    XMLNode akt_node;
                    if (Node_edges.getChildNode("edge", i).getChildNode("edge_spec").nChildNode("channel1") > 0)
                        akt_node = Node_edges.getChildNode("edge", i).getChildNode("edge_spec").getChildNode("channel1");

                    Node_edges.getChildNode("edge", i).getChildNode("node_from").deleteText();
                    Node_edges.getChildNode("edge", i).getChildNode("node_from").addText(
                        (edges.at(j)->getStartNodeName()).c_str());

                    Node_edges.getChildNode("edge", i).getChildNode("node_to").deleteText();
                    Node_edges.getChildNode("edge", i).getChildNode("node_to").addText(
                        (edges.at(j)->getEndNodeName()).c_str());
                }

                if(edges.at(j)->getType() == "Pipe")
                    writingTagValue(Node_edges.getChildNode("edge",i), "roughness", edges.at(i)->getProperty("roughness"));

                if (edges.at(j)->getType() == "Channel" || edges.at(j)->getType() == "Pipe") {
                    XMLNode akt_node;
                    if (Node_edges.getChildNode("edge", i).getChildNode("edge_spec").nChildNode("channel") > 0)
                        akt_node = Node_edges.getChildNode("edge", i).getChildNode("edge_spec").getChildNode("channel");
                    if (Node_edges.getChildNode("edge", i).getChildNode("edge_spec").nChildNode("channel0") > 0)
                        akt_node = Node_edges.getChildNode("edge", i).getChildNode("edge_spec").getChildNode(
                                       "channel0");
                    if (Node_edges.getChildNode("edge", i).getChildNode("edge_spec").nChildNode("channel1") > 0)
                        akt_node = Node_edges.getChildNode("edge", i).getChildNode("edge_spec").getChildNode(
                                       "channel1");
                    if (Node_edges.getChildNode("edge", i).getChildNode("edge_spec").nChildNode("channel2") > 0)
                        akt_node = Node_edges.getChildNode("edge", i).getChildNode("edge_spec").getChildNode(
                                       "channel2");
                    if (Node_edges.getChildNode("edge", i).getChildNode("edge_spec").nChildNode("pipe") > 0)
                        akt_node = Node_edges.getChildNode("edge", i).getChildNode("edge_spec").getChildNode("pipe");

                    akt_node.getChildNode("friction_coeff").deleteText();
                    ostringstream os;
                    os << scientific << setprecision(5) << edges.at(j)->getProperty("lambda");
                    akt_node.getChildNode("friction_coeff").addText(os.str().c_str());
                }

                if (edges.at(j)->getType() == "Channel") {
                    XMLNode akt_node;
                    if (Node_edges.getChildNode("edge", i).getChildNode("edge_spec").nChildNode("channel") > 0)
                        akt_node = Node_edges.getChildNode("edge", i).getChildNode("edge_spec").getChildNode("channel");
                    if (Node_edges.getChildNode("edge", i).getChildNode("edge_spec").nChildNode("channel0") > 0)
                        akt_node = Node_edges.getChildNode("edge", i).getChildNode("edge_spec").getChildNode("channel0");
                    if (Node_edges.getChildNode("edge", i).getChildNode("edge_spec").nChildNode("channel1") > 0)
                        akt_node = Node_edges.getChildNode("edge", i).getChildNode("edge_spec").getChildNode("channel1");
                    if (Node_edges.getChildNode("edge", i).getChildNode("edge_spec").nChildNode("channel2") > 0)
                        akt_node = Node_edges.getChildNode("edge", i).getChildNode("edge_spec").getChildNode("channel2");

                    akt_node.getChildNode("start_height").deleteText();
                    ostringstream os;
                    os.str("");
                    os << scientific << setprecision(5) << edges.at(j)->getProperty("ze");
                    akt_node.getChildNode("start_height").addText(os.str().c_str());

                    akt_node.getChildNode("end_height").deleteText();
                    os.str("");
                    os << scientific << setprecision(5) << edges.at(j)->getProperty("zv");
                    akt_node.getChildNode("end_height").addText(os.str().c_str());

                    akt_node.getChildNode("inclination").deleteText();
                    os.str("");
                    os << scientific << setprecision(5) << (100 * (edges.at(j)->getProperty("lejtes")));
                    akt_node.getChildNode("inclination").addText(os.str().c_str());
                }

                /*if (edges.at(j)->getType() == "Channel") {
                    XMLNode akt_node;
                    if (Node_edges.getChildNode("edge", i).getChildNode("edge_spec").nChildNode("channel") > 0)
                        akt_node = Node_edges.getChildNode("edge", i).getChildNode("edge_spec").getChildNode("channel");
                    if (Node_edges.getChildNode("edge", i).getChildNode("edge_spec").nChildNode("channel0") > 0)
                        akt_node = Node_edges.getChildNode("edge", i).getChildNode("edge_spec").getChildNode("channel0");
                    if (Node_edges.getChildNode("edge", i).getChildNode("edge_spec").nChildNode("channel1") > 0)
                        akt_node = Node_edges.getChildNode("edge", i).getChildNode("edge_spec").getChildNode("channel1");
                    if (Node_edges.getChildNode("edge", i).getChildNode("edge_spec").nChildNode("channel2") > 0)
                        akt_node = Node_edges.getChildNode("edge", i).getChildNode("edge_spec").getChildNode("channel2");


                    for (int nn = 0; nn < akt_node.nChildNode("curve"); nn++) {
                        XMLNode xnode = akt_node.getChildNode("curve", nn);
                        string melyik = xnode.getChildNode("id").getText();
                        xnode.getChildNode("points").deleteNodeContent();
                        xnode.addChild(XMLNode::parseString("<points> </points>"));
                        ostringstream os;
                        os.str("");
                        os << scientific << setprecision(5);

                        if (melyik == "curve_yf") {
                            vector<double> xf = edges.at(j)->Get_res("xf");
                            vector<double> yf = edges.at(j)->Get_res("yf");

                            for (unsigned int pdb = 0; pdb < xf.size(); pdb++)
                                os << "<point_x>" << xf.at(pdb) << "</point_x><point_y>" << yf.at(pdb) << "</point_y>";
                            if (debug)
                                cout << endl << " \t\t fenek kesz";
                        }

                        if (melyik == "curve_y") {
                            vector<double> x = edges.at(j)->Get_res("x");
                            vector<double> y = edges.at(j)->Get_res("y");
                            for (unsigned int pdb = 0; pdb < x.size(); pdb++)
                                os << "<point_x>" << x.at(pdb) << "</point_x><point_y>" << y.at(pdb) << "</point_y>";
                            if (debug)
                                cout << endl << " \t\t vizfelszin kesz";
                        }

                        if (melyik == "curve_p") {
                            vector<double> x = edges.at(j)->Get_res("x");
                            vector<double> y = edges.at(j)->Get_res("p");
                            for (unsigned int pdb = 0; pdb < x.size(); pdb++)
                                os << "<point_x>" << x.at(pdb) << "</point_x><point_y>" << y.at(pdb) << "</point_y>";
                            if (debug)
                                cout << endl << " \t\t vizfelszin kesz";
                        }

                        if (melyik == "curve_v") {
                            vector<double> x = edges.at(j)->Get_res("x");
                            vector<double> v = edges.at(j)->Get_res("v");
                            for (unsigned int pdb = 0; pdb < x.size(); pdb++)
                                os << "<point_x>" << x.at(pdb) << "</point_x><point_y>" << v.at(pdb) << "</point_y>";
                            if (debug)
                                cout << endl << " \t\t sebesseg kesz";
                        }
                        xnode.getChildNode("points").addChild(XMLNode::parseString(os.str().c_str()));
                    }
                }*/
            }
        }
        if (debug && !gotIt)
            cout << "not found, moving on ...";
    }
    xMainNode.writeToFile(xml_fnev);

    if (staci_debug_level > 0)
        cout << "  ok.\n\n";
}

//--------------------------------------------------------------------------------
void IOxml::save_mod_prop_all_elements(vector<Node *> nodes, vector<Edge *> edges, string pID) {

    debug = false;

    XMLNode xMainNode = XMLNode::openFileHelper(xml_fnev, "staci");
    XMLNode Node_settings = xMainNode.getChildNode("settings");
    XMLNode Node_nodes = xMainNode.getChildNode("nodes");
    XMLNode Node_edges = xMainNode.getChildNode("edges");

    for (unsigned int i = 0; i < edges.size(); i++) {
        XMLNode akt_node = Node_edges.getChildNode("edge", i);
        akt_node.getChildNode(pID.c_str()).deleteText();
        ostringstream os;
        os << scientific << setprecision(5) << edges.at(i)->getProperty(pID);
        akt_node.getChildNode(pID.c_str()).addText(os.str().c_str());

    }

    int i = 0;
    double val;
    for (unsigned int j = 0; j < Node_nodes.nChildNode("node"); j++) {
        XMLNode akt_node = Node_nodes.getChildNode("node", j);

        string is_endnode = akt_node.getChildNode("endnode").getText();
        if (is_endnode == "false") {
            val = nodes.at(i)->getProperty(pID);
            i++;
        }
        else
            val = 0.0;

        akt_node.getChildNode(pID.c_str()).deleteText();
        ostringstream os;
        os << scientific << setprecision(5) << val;
        akt_node.getChildNode(pID.c_str()).addText(os.str().c_str());
    }
    xMainNode.writeToFile(xml_fnev);
}

//--------------------------------------------------------------------------------
void IOxml::save_mod_prop(vector<Node *> nodes, vector<Edge *> edges, string eID, string pID, bool is_property_general) {

    debug = false;

    XMLNode xMainNode = XMLNode::openFileHelper(xml_fnev, "staci");

    if (debug)
        cout << endl << "Output file name:" << xml_fnev << ", modified data saving ... (eID:" << eID << ", pID=" << pID << ")";

    XMLNode Node_settings = xMainNode.getChildNode("settings");
    XMLNode Node_nodes = xMainNode.getChildNode("nodes");
    XMLNode Node_edges = xMainNode.getChildNode("edges");

    if (debug)
        cout << endl << endl << "cpp_xml_debug=" << Node_settings.getChildNode("cpp_xml_debug").getText() << " => adatfajl irasa kozbeni debug bekapPipelasa..." << endl;

    nodeNumber = Node_nodes.nChildNode("node");
    edgeNumber = Node_edges.nChildNode("edge");

    string id;
    bool gotIt;

    gotIt = false;

    int i = 0;
    while ((i < edges.size()) && (!gotIt)) {
        id = Node_edges.getChildNode("edge", i).getChildNode("id").getText();
        for (unsigned int j = 0; j < edges.size(); j++) {
            if ((id == (edges.at(j)->getName())) && (id == eID)) {
                gotIt = true;
                if (debug) {
                    cout << endl << "\tFOUND! xml id : " << id << ",  eID : " << eID << ", edges.at(" << j << ")->getName() : " << edges.at(j)->getName();
                    cout << endl << "\tpID : " << pID;
                    cout << endl << "\tval : " << edges.at(j)->getProperty(pID);
                    cout << endl << "\ttip : " << edges.at(j)->getType();
                }
                if (is_property_general) {
                    XMLNode akt_node = Node_edges.getChildNode("edge", i);

                    akt_node.getChildNode(pID.c_str()).deleteText();
                    ostringstream os;
                    os << scientific << setprecision(5) << edges.at(j)->getProperty(pID);
                    akt_node.getChildNode(pID.c_str()).addText(os.str().c_str());
                } else {

                    if (edges.at(j)->getType() == "Pipe") {
                        XMLNode akt_node;

                        if (Node_edges.getChildNode("edge", i).getChildNode("edge_spec").nChildNode("pipe") > 0)
                            akt_node = Node_edges.getChildNode("edge", i).getChildNode("edge_spec").getChildNode("pipe");

                        akt_node.getChildNode(pID.c_str()).deleteText();
                        ostringstream os;
                        os << scientific << setprecision(5) << edges.at(j)->getProperty(pID);
                        akt_node.getChildNode(pID.c_str()).addText(os.str().c_str());
                    }

                    if (edges.at(j)->getType() == "Pool") {
                        XMLNode akt_node;

                        if (Node_edges.getChildNode("edge", i).getChildNode("edge_spec").nChildNode("pool") > 0)
                            akt_node = Node_edges.getChildNode("edge", i).getChildNode("edge_spec").getChildNode("pool");

                        akt_node.getChildNode(pID.c_str()).deleteText();
                        ostringstream os;
                        os << scientific << setprecision(5) << edges.at(j)->getProperty(pID);
                        akt_node.getChildNode(pID.c_str()).addText(os.str().c_str());
                    }

                    if (edges.at(j)->getType() == "Valve") {
                        XMLNode akt_node;

                        if (Node_edges.getChildNode("edge", i).getChildNode("edge_spec").nChildNode("valve") > 0)
                            akt_node = Node_edges.getChildNode("edge", i).getChildNode("edge_spec").getChildNode("valve");

                        akt_node.getChildNode(pID.c_str()).deleteText();
                        ostringstream os;
                        os << scientific << setprecision(5) << edges.at(j)->getProperty(pID);
                        akt_node.getChildNode(pID.c_str()).addText(os.str().c_str());

                        if (pID == "position") {
                            akt_node.getChildNode("position").deleteText();
                            ostringstream os;
                            os << scientific << setprecision(5) << edges.at(j)->getProperty("position");
                            akt_node.getChildNode("position").addText(os.str().c_str());

                            akt_node.getChildNode("adzeta").deleteText();
                            os.str("");
                            os << scientific << setprecision(5) << edges.at(j)->getProperty("adzeta");
                            akt_node.getChildNode("adzeta").addText(os.str().c_str());
                        }
                    }
                }
                break;
            } 
        }
        i++;
    }

    if (debug && !gotIt)
        cout << "\n\n eID " << eID << " could not be found among edges, is it a node? ...";

    i = 0;
    double newval;
    while ((i < nodeNumber) && (!gotIt)) {
        id = Node_nodes.getChildNode("node", i).getChildNode("id").getText();
        if (debug)
            cout << endl << "\tcsp id : " << id << "  ";
        for (unsigned int j = 0; j < nodes.size(); j++) {
            if ((id == (nodes.at(j)->getName())) && (id == eID)) {
                gotIt = true;
                XMLNode akt_node = Node_nodes.getChildNode("node", i);
                akt_node.getChildNode(pID.c_str()).deleteText();
                ostringstream os;
                os << scientific << setprecision(5) << nodes.at(j)->getProperty(pID);
                newval = nodes.at(j)->getProperty(pID);
                akt_node.getChildNode(pID.c_str()).addText(os.str().c_str());
                break;
            }
        }
        i++;
    }

    if (!gotIt) {
        cout << "\n\n ERROR!!! IOxml.save_mod_prop() -> eID " << eID << " was not found!!!\n\n";
        exit(-1);
    } else {
        xMainNode.writeToFile(xml_fnev);
        if (debug) {
            cout << "\n modified element saved to data file: " << pID << " = " << newval << "\n\n";
        }
    }
}

//--------------------------------------------------------------------------------
void IOxml::load_ini_values(vector<Node *> &nodes, vector<Edge *> &edges) {

    XMLNode xMainNode = XMLNode::openFileHelper(xml_fnev, "staci");

    XMLNode Node_settings = xMainNode.getChildNode("settings");
    XMLNode Node_nodes = xMainNode.getChildNode("nodes");
    XMLNode Node_edges = xMainNode.getChildNode("edges");

    if (debug)
        cout << endl << "cpp_xml_debug= true => inicializacios fajl olvasasa kozbeni debug bekapPipelasa..." << endl;

    nodeNumber = Node_nodes.nChildNode("node");
    edgeNumber = Node_edges.nChildNode("edge");
    if (debug)
        cout << endl << "initialization fajl: " << xml_fnev << endl << "\tNodeok szama: " << nodeNumber << endl << "\tagak szama:        " << edgeNumber << endl;

    string id;
    double pressure;
    for (int i = 0; i < nodeNumber; i++) {
        id = Node_nodes.getChildNode("node", i).getChildNode("id").getText();
        for (unsigned int j = 0; j < nodes.size(); j++) {
            if (nodes.at(j)->getName() == id) {
                pressure = atof(Node_nodes.getChildNode("node", i).getChildNode("pressure").getText()) / 1000 / 9.81;
                nodes.at(j)->initialization(1, pressure);
                if (debug)
                    cout << endl << "\t id: " << id << " =>  p=" << (pressure * 1000 * 9.81)
                         << "Pa = " << pressure << "vom";
            }
        }
    }

    double mass_flow_rate;
    for (int i = 0; i < edgeNumber; i++) {
        id = Node_edges.getChildNode("edge", i).getChildNode("id").getText();
        for (unsigned int j = 0; j < edges.size(); j++) {
            if (edges.at(j)->getName() == id) {
                mass_flow_rate = atof(Node_edges.getChildNode("edge", i).getChildNode("mass_flow_rate").getText());
                if (debug)
                    cout << endl << "\t id: " << id << " => mp=" << mass_flow_rate << " kg/s";
                edges.at(j)->initialization(1, mass_flow_rate);
            }
        }
    }
}

//--------------------------------------------------------------------------------
/*void IOxml::save_transport(int mode, vector<Node *> nodes, vector<Edge *> edges) {
    XMLNode xMainNode = XMLNode::openFileHelper(xml_fnev, "staci");

    XMLNode Node_settings = xMainNode.getChildNode("settings");
    XMLNode Node_nodes = xMainNode.getChildNode("nodes");
    XMLNode Node_edges = xMainNode.getChildNode("edges");

    if (debug)
        cout << endl << endl << "cpp_xml_debug=" << Node_settings.getChildNode("cpp_xml_debug").getText()
             << " => adatfajl irasa kozbeni debug bekapPipelasa..." << endl;

    nodeNumber = Node_nodes.nChildNode("node");
    edgeNumber = Node_edges.nChildNode("edge");

    string tagname;
    if (mode == 1) {
        tagname = "travel_time";
        Node_settings.getChildNode("tsolution_exists").deleteText();
        Node_settings.getChildNode("tsolution_exists").addText("true");
    }
    if (mode == 2) {
        tagname = "concentration";
        Node_settings.getChildNode("csolution_exists").deleteText();
        Node_settings.getChildNode("csolution_exists").addText("true");
    }

    string id, is_endnode;
    bool gotIt;
    for (int i = 0; i < nodeNumber; i++) {
        id = Node_nodes.getChildNode("node", i).getChildNode("id").getText();
        gotIt = false;
        if (debug)
            cout << endl << "\tcsp id: " << id << "  ";
        is_endnode = Node_nodes.getChildNode("node", i).getChildNode("endnode").getText();
        if (is_endnode == "false") {
            for (unsigned int j = 0; j < nodes.size(); j++) {
                if (id == (nodes.at(j)->getName())) {
                    Node_nodes.getChildNode("node", i).getChildNode(tagname.c_str()).deleteText();
                    double c = nodes.at(j)->getProperty("konc_atlag");
                    if (mode == 1)
                        c = c / 3600;

                    ostringstream os;
                    os << scientific << setprecision(5) << c;
                    Node_nodes.getChildNode("node", i).getChildNode(tagname.c_str()).addText(os.str().c_str());
                    gotIt = true;
                    if (debug)
                        cout << " => " << tagname << ": " << c;
                }
            }
            if (debug && !gotIt)
                cout << "nincs meg, atugrom...";
        } else {
            if (debug)
                cout << " ez endnode, nem erdekes...";
        }
    }

    double mp;
    for (int i = 0; i < edgeNumber; i++) {
        id = Node_edges.getChildNode("edge", i).getChildNode("id").getText();
        gotIt = false;
        if (debug)
            cout << endl << "\tag id : " << id << "  ";
        for (unsigned int j = 0; j < edges.size(); j++) {
            if (id == (edges.at(j)->getName())) {
                mp = edges.at(i)->getMassFlowRate();
                Node_edges.getChildNode("edge", i).getChildNode(tagname.c_str()).deleteText();
                ostringstream os;
                double konc = edges.at(j)->mean(edges.at(j)->konc);
                if (mode == 1)
                    konc = konc / 3600;
                os << scientific << setprecision(5) << konc;
                Node_edges.getChildNode("edge", i).getChildNode(tagname.c_str()).addText(os.str().c_str());

                gotIt = true;
            }
        }
        if (debug && !gotIt)
            cout << "nincs meg, atugrom...";
    }
    xMainNode.writeToFile(xml_fnev);
}*/

//--------------------------------------------------------------------------------
string IOxml::read_setting(string which) {
    XMLNode xMainNode = XMLNode::openFileHelper(xml_fnev, "staci");

    XMLNode Node_settings = xMainNode.getChildNode("settings");

    string out;
    if (Node_settings.nChildNode(which.c_str()) == 1) {
        out = xMainNode.getChildNode("settings").getChildNode(which.c_str()).getText();
        return out;
    } else
        return "nincs ilyen node!";
}

//--------------------------------------------------------------------------------
double IOxml::string_to_double(const string &s, const string &elem_name, const string &tag_name, const double &def_value) {
    std::istringstream i(s);
    double x;
    if (s.empty()) {
        cout << endl << endl << "Error! Element: " << elem_name << ", tag: " << tag_name << " is empty." << endl;
        cout << endl << "  Returning default value of " << def_value << " but you should check the input file." << endl << endl;
        cin.get();
        return def_value;
    } else {
        if (!(i >> x)) {
            cout << endl << endl << "Error! Element: " << elem_name << ", tag: " << tag_name << " - non-numeric value: " << s << endl;
            cout << endl << "  Returning default value of " << def_value << " but you should check the input file." << endl << endl;
            cin.get();
            return def_value;
        } else {
            return x;
        }
    }
}
