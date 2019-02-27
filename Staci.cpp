#include "Staci.h"
#include <string.h>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <string>
#include "PressurePoint.h"

// TODO: Clear up debug level behaviour in the doc!
// TODO: log file cleanup!

using namespace Eigen;

Staci::Staci(int argc, char *argv[]) {
  debugLevel = 1;
  // Initiate with command line arguments
  //perform_demand_sensitivity_analysis = false;
  opt = new AnyOption();
  getCommandLineOptions(argc, argv);
  setInitialParameters();
}

Staci::Staci(string spr_filename) {
  calculationType = 0;
  debugLevel = 1;
  opt = new AnyOption();
  definitionFile = spr_filename;
  setInitialParameters();
}

void Staci::setInitialParameters() {


// Add header
  consolePrint.str("");
  consolePrint << endl << "======================================";
  consolePrint << endl << " STACI v.2.0";
  consolePrint << endl << " (c) BME Dept. of Hydrodynamic Systems";
  consolePrint << endl << " (c) C. Hos (cshos@hds.bme.hu)";
  consolePrint << endl << " info: www.hds.bme.hu\\staci_web";
  consolePrint << endl << "======================================" << endl;
  time_t ido = time(0);
  consolePrint << endl << " date: " << ctime(&ido);
  consolePrint << endl << " input file: " << definitionFile << endl;
  writeLogFile(consolePrint.str(), 1);
  consolePrint.str("");

  isInitialization = false;

  if (calculationType >= 0) {
    stringstream ss;
    ss << endl << " Loading system...";
    writeLogFile(ss.str(), 1);

    IOxml datta_io(definitionFile.c_str());
    datta_io.load_system(nodes, edges);

    ss.str("");
    ss << " ready." << endl;
    writeLogFile(ss.str(), 1);

    debugLevel = atoi(datta_io.read_setting("debugLevel").c_str());

    outputFile = datta_io.read_setting("outputFile");
  }

  setOutputFile(definitionFile + ".ros");

  ostringstream msg;
  msg << "\n Trying to delete previous logfile " << outputFile.c_str() << "... ";
  if (remove(outputFile.c_str()) != 0)
    msg << " file not found, cannot delete it." << endl;
  else
    msg << "file successfully deleted." << endl;
  writeLogFile(msg.str(), 1);
}

//--------------------------------------------------------------
Staci::~Staci() { 
  delete opt; 
}

//--------------------------------------------------------------
void Staci::getCommandLineOptions(int argc, char *argv[]) {
  opt->setVerbose();         /* print warnings about unknown options */
  opt->autoUsagePrint(true); /* print usage for bad options */

  /// This function sets members 'calculationType' and 'definitionFile'.
  /// Possible argumentums:
  /// -h --help
  /// -s --stac              (calculationType=0)
  /// -i --ini
  /// -t --travel_time       (calculationType=1)
  /// -c --conc_transp       (calculationType=2)
  /// -m --mod_prop          (calculationType=3)
  /// -e --elementID
  /// -p --propertyID
  /// -n --newValue
  /// -o --outfile
  /// -l --list_all_elements (calculationType=4)
  /// -g --get_data          (calculationType=5)
  /// -e --elementID
  /// -p --propertyID
  /// -r sensitivity         (calculationType=6)
  /// -d demand_sensitivity         (calculationType=0)
  /// -e export community structure (calculationType=7)

  opt->addUsage("");
  opt->addUsage("staci hasznalata: ");
  opt->addUsage("");
  opt->addUsage("\t help:");
  opt->addUsage(
    "\t\t -h  --help                          Help nyomtatasa a kepernyore");
  opt->addUsage(" ");
  opt->addUsage("\t stacioner halozatszamitas: ");
  opt->addUsage(
    "\t\t -s  (--stac) <halofile>.xml           Definicios file, kotelezo");
  opt->addUsage(
    "\t\t -i  (--ini) <resfile>.xml             Inicializacios file, nem "
    "kotelezo");
  opt->addUsage(" ");
  opt->addUsage("\t tartozkodasi ido szamitasa: ");
  opt->addUsage(
    "\t\t -t  (--travel_time) <halofile>.xml    Tartozkodasi ido szamitas a "
    "halofile.xml feladaton");
  opt->addUsage(" ");
  opt->addUsage("\t koncentracioeloszlas szamitasa: ");
  opt->addUsage(
    "\t\t -c  (--conc_transp) <halofile>.xml    Koncentracio eloszlas "
    "szamitasa a halofile.xml feladaton");
  opt->addUsage(" ");
  opt->addUsage("\t parameter megvaltoztatasa: ");
  opt->addUsage(
    "\t\t -m  (--mod_prop) <halofile_regi>.spr -e (--elementID) <ID_ag/csp> "
    "-p (--propertyID) <ID_adat> -n (--newValue) <uj_ertek> -o (--outfile) "
    "<halofajl_uj>.spr    Adatmodositas: halofajl_regi.xml -> "
    "halofajl_uj.xml");
  opt->addUsage(" ");
  opt->addUsage("\t minden elem listazasa a kepernyora: ");
  opt->addUsage("\t\t -l  (--list_all_elements) <halofile_regi>.spr");
  opt->addUsage(" ");
  opt->addUsage("\t adat kiolvasasa: ");
  opt->addUsage(
    "\t\t -g  (--get_data) <halofile_regi>.spr -e (--elementID) <ID_ag/csp> "
    "-p (--propertyID) <ID_adat>");
  opt->addUsage(" ");
  
  opt->addUsage("\t erzekenysegvizsgalat: ");
  opt->addUsage(
    "\t\t -r  (--sensitivity) <halofile_regi>.spr -e (--elementID) "
    "<ID_ag/csp> -p (--propertyID) <ID_adat>");
  opt->addUsage(" ");

  opt->addUsage("\t hydraulics + residence time + demand sensitivity: ");
  opt->addUsage(
    "\t\t -d  (--demand_sensitivity) <halofile_regi>.spr");
  opt->addUsage(" ");


  opt->addUsage("\t check for islands: ");
  opt->addUsage(
    "\t\t -x  (--export_to_connectivity_check) <halofile_regi>.spr");

  opt->addUsage(" ");
  opt->addUsage(" ");

  opt->setOption("stac", 's');
  opt->setOption("ini", 'i');
  opt->setOption("travel_time", 't');
  opt->setOption("conc_transp", 'c');
  opt->setOption("mod_prop", 'm');
  opt->setOption("elementID", 'e');
  opt->setOption("propertyID", 'p');
  opt->setOption("newValue", 'n');
  opt->setOption("outfile", 'o');
  opt->setOption("list_all_elements", 'l');
  opt->setOption("get_data", 'g');
  opt->setOption("sensitivity", 'r');
  opt->setOption("demand_sensitivity", 'd');
  opt->setOption("export_for_connectivity_check", 'x');

  opt->processCommandArgs(argc, argv);

  if (!opt->hasOptions()) {
    calculationType = -1;
    opt->printUsage();
  } else {
    if (opt->getFlag("help") || opt->getFlag('h')) opt->printUsage();

    if (opt->getValue('s') != NULL || opt->getValue("stac") != NULL) {
      calculationType = 0;
      //perform_demand_sensitivity_analysis = false;
      definitionFile = opt->getValue('s');
      if (debugLevel > 0) {
        string msg = "\n Steady-state hydraulic simulation, data file: " +
                     definitionFile + "\n";
        if (debugLevel > 0) {
          // cout << msg;
          writeLogFile(msg, 1);
        }
      }
    }

    if (opt->getValue('d') != NULL || opt->getValue("demand_sensitivity") != NULL) {
      calculationType = 0;
      //perform_demand_sensitivity_analysis = true;
      definitionFile = opt->getValue('d');
      if (debugLevel > 0) {
        string msg = "\n Steady-state hydraulic simulation with demand sensitivity computation, data file: " +
                     definitionFile + "\n";
        if (debugLevel > 0) {
          // cout << msg;
          writeLogFile(msg, 1);
        }
      }
    }

    if (opt->getValue('i') != NULL || opt->getValue("ini") != NULL) {
      isInitialization = true;
      calculationType = 0;
      initializationFile = opt->getValue('i');
      if (debugLevel > 0) {
        string msg = "\n Stacioner halozatszamitas, inicializacios file : " +
                     initializationFile + "\n";
        if (debugLevel > 0) {
          // cout << msg;
          writeLogFile(msg, 1);
        }
      }
    }

    if (opt->getValue('t') != NULL || opt->getValue("travel_time") != NULL) {
      calculationType = 1;
      definitionFile = opt->getValue('t');
      cout << "Tartozkodasi ido szamitas, a feladatot tartalmazo file: "
           << definitionFile << endl;
    }

    if (opt->getValue('c') != NULL || opt->getValue("conc_transp") != NULL) {
      calculationType = 2;
      definitionFile = opt->getValue('c');
      cout << "Koncentracioeloszlas szamitas, a feladatot tartalmazo file: "
           << definitionFile << endl;
    }

    if (opt->getValue('r') != NULL || opt->getValue("sens") != NULL) {
      calculationType = 0;
      definitionFile = opt->getValue('r');
      cout << endl
           << "Steady-state hydraulic simulation and sensitivity analysis, "
           "data file: "
           << definitionFile << endl;
    }

    // ADATMODOSITAS
    //-----------------------------
    if (opt->getValue('m') != NULL || opt->getValue("mod_prop") != NULL) {
      calculationType = 3;
      definitionFile = opt->getValue('m');
      cout << endl << "Adatmodositas" << endl;
      /*cout << "\n regi adatfajl: "<<definitionFile;*/
    }
    if (opt->getValue('e') != NULL || opt->getValue("elementID") != NULL) {
      elementID = opt->getValue('e');
      /* cout << "\n elementID: "<< elementID ;*/
    }
    if (opt->getValue('p') != NULL || opt->getValue("propertyID") != NULL) {
      propertyID = opt->getValue('p');
      /* cout << "\n propertyID: "<< propertyID ;*/
    }
    if (opt->getValue('n') != NULL || opt->getValue("newValue") != NULL) {
      newValue = atof(opt->getValue('n'));
      /* cout << "\n newValue: "<< newValue;*/
    }
    if (opt->getValue('o') != NULL || opt->getValue("outfile") != NULL) {
      newDefinitionFile = opt->getValue('o');
      /* cout << "\n newDefinitionFile: "<< newDefinitionFile<<endl;*/
    }

    // Minden elem listazasa
    //-----------------------------
    if (opt->getValue('l') != NULL ||
        opt->getValue("list_all_elements") != NULL) {
      calculationType = 4;
      definitionFile = opt->getValue('l');
      /*cout << endl << "Adatkiolvasas"<< endl ;*/
    }
    // elementID es propertyID fent

    // Adat kiolvasasa
    //-----------------------------
    if (opt->getValue('g') != NULL || opt->getValue("get_data") != NULL) {
      calculationType = 5;
      definitionFile = opt->getValue('g');
      /*cout << endl << "Adatkiolvasas"<< endl ;*/
    }
    // elementID es propertyID fent

    // Erzekenysegvizsgalat
    //-----------------------------
    if (opt->getValue('r') != NULL || opt->getValue("sensitivity") != NULL) {
      calculationType = 6;
      definitionFile = opt->getValue('r');
    }
    // elementID es propertyID fentAdatkiolvasas"<< endl ;*/

    // Export for connectivity check
    //-----------------------------
    if (opt->getValue('x') != NULL || opt->getValue("export_for_connectivity_check") != NULL) {
      calculationType = 7;
      definitionFile = opt->getValue('x');
    }
    // elementID es propertyID fentAdatkiolvasas"<< endl ;*/
  }
}


//--------------------------------------------------------------
double Staci::getElementProperty(string in_elementID, string in_propertyID) {
  bool elementGotIt = false;
  bool propertyGotIt = false;
  double outdata = 0.0;

  for (unsigned int i = 0; i < edges.size(); i++) {
    if ((edges[i]->getName()) == in_elementID) {
      elementGotIt = true;
      if ((in_propertyID == "diameter") ||
          (in_propertyID == "mass_flow_rate") ||
          (in_propertyID == "bottom_level") ||
          (in_propertyID == "water_level") || (in_propertyID == "position")) {
        outdata = edges[i]->getProperty(in_propertyID);
        propertyGotIt = true;
      }
    }
  }

  for (unsigned int i = 0; i < nodes.size(); i++) {
    if ((nodes[i]->getName()) == in_elementID) {
      elementGotIt = true;
      if ((in_propertyID == "pressure") || (in_propertyID == "demand") ||
          (in_propertyID == "head")) {
        outdata = nodes[i]->getProperty(in_propertyID);
        propertyGotIt = true;
      }
    }
  }

  if (!elementGotIt) {
    cout << endl << endl << "ERROR!!! Staci::getElementProperty(): Element not found: " << in_elementID << endl << endl;
    exit(-1);
  } else {
    if (!propertyGotIt) {
      cout << endl << endl << "ERROR!!! Staci::getElementProperty(): Element: " << in_elementID << ", property not found: " << in_propertyID << endl << endl;
      exit(-1);
    } else
      return outdata;
  }
}

//--------------------------------------------------------------
void Staci::setElementProperty(string in_elementID, string in_propertyID, double in_newValue) {
  bool elementGotIt = false;
  bool propertyGotIt = false;

  for (unsigned int i = 0; i < edges.size(); i++) {
    if ((edges[i]->getName()) == in_elementID) {
      elementGotIt = true;
      if (in_propertyID == "diameter") {
        edges[i]->setProperty(in_propertyID, in_newValue);
        propertyGotIt = true;
      }

      if (in_propertyID == "bottom_level") {
        edges[i]->setProperty(in_propertyID, in_newValue);
        propertyGotIt = true;
      }

      if (in_propertyID == "water_level") {
        edges[i]->setProperty(in_propertyID, in_newValue);
        propertyGotIt = true;
      }

      if (in_propertyID == "position") {
        edges[i]->setProperty(in_propertyID, in_newValue);
        propertyGotIt = true;
      }
    }
  }

  for (unsigned int i = 0; i < nodes.size(); i++) {
    if ((nodes[i]->getName()) == in_elementID) {

      elementGotIt = true;
      if (in_propertyID == "demand") {
        nodes[i]->setProperty(in_propertyID, in_newValue);
        propertyGotIt = true;
      }
    }
  }

  if (!elementGotIt) {
    cout << endl << endl << "ERROR!!! Staci::setElementProperty(): Element not found: " << in_elementID << endl << endl;
    exit(-1);
  }
  if (!propertyGotIt) {
    cout << endl << endl << "ERROR!!! Staci::setElementProperty(): Element: " << in_elementID << ", property not found: " << in_propertyID << endl << endl;
    exit(-1);
  }
}

//--------------------------------------------------------------
void Staci::buildSystem() {
  ostringstream msg1;

  bool stop = false;

  msg1 << endl << " Building system...";
  writeLogFile(msg1.str(), 1);
  msg1.str("");

  writeLogFile("\n Azonos ID-k keresese....", 3);
  string nev1, nev2;
  for (unsigned int i = 0; i < edges.size(); i++) {
    nev1 = edges.at(i)->getName();
    for (unsigned int j = 0; j < edges.size(); j++) {
      nev2 = edges.at(j)->getName();
      if (i != j) {
        if (nev1 == nev2) {
          ostringstream msg;
          msg << "\n !!!ERROR!!! edge #" << i << ": " << nev1 << " and edge #" << j << ": " << nev2 << " with same ID!" << endl;
          writeLogFile(msg.str(), 1);
          stop = true;
        }
      }
    }

    for (unsigned int j = 0; j < nodes.size(); j++) {
      nev2 = nodes.at(j)->getName();
      if (i != j) {
        if (nev1 == nev2) {
          ostringstream msg;
          msg << "\n !!!ERROR!!! edge #" << i << ": " << nev1 << " and node #" << j << ": " << nev2 << " with same ID!" << endl;
          writeLogFile(msg.str(), 1);
          stop = true;
        }
      }
    }
  }

  if (stop)
    exit(-1);
  else
    writeLogFile("\t ok.", 3);

  writeLogFile("\n\n Building system...", 3);
  bool startGotIt = false;
  bool endGotIt = false;
  unsigned int j = 0;
  int startNodeIndex = -1, endNodeIndex = -1;
  ostringstream strstrm;

  for (unsigned int i = 0; i < edges.size(); i++) {
    startGotIt = false;
    j = 0;
    while ((j < nodes.size()) && (!startGotIt)) {
      // log
      strstrm.str("");
      strstrm << "\n\t" << edges[i]->getName().c_str()
              << " startNodeIndex: " << edges[i]->getStartNodeName().c_str() << " =? "
              << nodes[j]->getName().c_str();
      writeLogFile(strstrm.str(), 5);
      if ((edges[i]->getStartNodeName()).compare(nodes[j]->getName()) == 0) {
        startGotIt = true;
        startNodeIndex = j;
        nodes[j]->edgeOut.push_back(i);
      }
      j++;
    }
    if (!startGotIt) {
      strstrm.str("");
      strstrm << "\n!!! Not found " << edges[i]->getName().c_str() << " start node: " << edges[i]->getStartNodeName() << " !!!";
      writeLogFile(strstrm.str(), 1);
      StaciException csphiba(strstrm.str());
      throw csphiba;
    }
    if (edges[i]->getNumberNode() == 2) {
      endGotIt = false;
      j = 0;
      while ((j < nodes.size()) && (!endGotIt)) {
        if ((edges[i]->getEndNodeName()).compare(nodes[j]->getName()) == 0) {
          endGotIt = true;
          endNodeIndex = j;
          nodes[j]->edgeIn.push_back(i);
        }
        j++;
      }
      if (!endGotIt) {
        strstrm.str("");
        strstrm << "\n!!! Not found " << edges[i]->getName().c_str() << " end node!";
        writeLogFile(strstrm.str(), 1);
      }
    } else {
      strstrm.str("");
      strstrm << "\n\t" << edges[i]->getName().c_str() << " endNodeIndex: - "
              << endNodeIndex;
    }

    if (edges[i]->getNumberNode() == 2) 
      edges[i]->addNodes(startNodeIndex, endNodeIndex);
    else
      edges[i]->addNodes(startNodeIndex, -1);
  }

  for (unsigned int i = 0; i < edges.size(); i++) {
    if (edges[i]->getType() == "Pipe") {
      edges[i]->setFrictionModel("DW");
    }
  }

  writeLogFile("\t ok.", 3);

  msg1.str("");
  msg1 << " ready." << endl;
  writeLogFile(msg1.str(), 1);

}

//--------------------------------------------------------------
void Staci::writeLogFile(string msg, int msg_debugLevel) {
  if (debugLevel >= msg_debugLevel) {
    ofstream outfile(outputFile.c_str(), ios::app);
    outfile << msg;
    outfile.close();
    cout << msg << flush;
  }
}

//--------------------------------------------------------------
void Staci::listSystem() {
  writeLogFile("\n\n Nodes:\n--------------------------", 3);
  for (unsigned int i = 0; i < nodes.size(); i++)
    writeLogFile(nodes[i]->info(true), 3);
  writeLogFile("\n\n Edges:\n--------------------------", 3);
  for (unsigned int i = 0; i < edges.size(); i++)
    writeLogFile(edges[i]->info(), 3);
}

//--------------------------------------------------------------
void Staci::saveProperties(bool is_general_property) {
  IOxml datta_io(resultFile.c_str());
  datta_io.save_mod_prop(nodes, edges, elementID, propertyID, is_general_property);
}

//--------------------------------------------------------------
void Staci::saveAllProperties(string propertyID) {
  IOxml datta_io(resultFile.c_str());
  datta_io.save_mod_prop_all_elements(nodes, edges, propertyID);
}

//--------------------------------------------------------------
void Staci::exportNodesEdges() {
  ofstream nodelistfile("nodelist.txt", ios::trunc);
  for (unsigned int i = 0; i < nodes.size(); i++)
    nodelistfile << nodes[i]->getName() << "\n";
  nodelistfile.close();

  ofstream nodefile("connected_nodes.txt", ios::trunc);
  for (unsigned int i = 0; i < edges.size(); i++) {
    if (edges[i]->getNumberNode() == 2) {
      string from = nodes[edges[i]->getStartNodeIndex()]->getName();
      string to = nodes[edges[i]->getEndNodeIndex()]->getName();
      string name = edges[i]->getName();
      nodefile << "\n" << from << "," << to;
    }
  }
  nodefile.close();
}

//--------------------------------------------------------------
string Staci::trim(string s, const string drop) {
  string r = s.erase(s.find_last_not_of(drop) + 1);
  return r.erase(0, r.find_first_not_of(drop));
}

//--------------------------------------------------------------
void Staci::copyFile(const string in_f_nev, const string out_f_nev) {
  ifstream infile(in_f_nev.c_str(), ifstream::in);
  ofstream outfile(out_f_nev.c_str(), ifstream::trunc);
  outfile << infile.rdbuf();
}

//--------------------------------------------------------------
void Staci::listAllElements() {
  stringstream str;
  for (unsigned int i = 0; i < edges.size(); i++)
    str << edges.at(i)->getType() << ";\t" << edges.at(i)->getName()
        << endl;
  for (unsigned int i = 0; i < nodes.size(); i++)
    str << "Node;\t" << nodes.at(i)->getName() << endl;

  cout << endl
       << endl
       << "Element list is in file element_list.txt." << endl
       << endl;

  ofstream OutFile;
  OutFile.open("element_list.txt");
  OutFile << str.str();
  OutFile.close();
}

/*! WR: find the the given node IDs and gives back the Indicies
  *id:  vector containing the IDs
  IMPORTANT:  if only one ID not found, drops ERRRO with exit(-1)
*/
vector<int> Staci::Id2Idx(const vector<string> &id){
  int n_id = id.size();
  bool megvan = false;
  vector<int> idx(n_id);
  for(int j=0;j<n_id;j++){
    int i=0;
    megvan = false;
    //cout << endl << id[j] << " : " << endl << (id[j]).length() << endl;
    while(!megvan && i<nodes.size()){
      if(id[j] == nodes[i]->getName()){
        idx[j] = i;
        megvan = true;
      }
      i++;
    }
    if(megvan == false){
      cout << "\n!!!!! ERROR !!!!!\nStaci:Id2Idx function\nNode is not existing, id: " << id[j] << endl<< "\nexiting...";
      exit(-1);
    }
  }
  return idx;
}

/// WR Reading doubles from file, separeted with ','
vector<string> Staci::ReadStrings(ifstream &file){
  string temp,line;
  vector<string> M;
  if(file.is_open()){
    while(getline(file,line)){
      if(line[0] != '/')
        M.push_back(line);
    }
  }else
    cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!ERROR!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl << "Staci::CSVRead(), File is not open when calling CSVRead function!!!" << endl;
  return M;
}

/// WR Reading doubles from file, separeted with "separator"
vector<vector<double> > Staci::CSVRead(ifstream &file, char separator){
  if(separator == '/')
    cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!ERROR!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl << "Staci::CSVRead(), Separator cannot be '/', that is for comments ONLY!!!" << endl;
  string temp,line;
  vector<vector<double> > M;
  if(file.is_open()){
    while(getline(file,line)){
      if(line[0] != '/'){
        vector<double> v;
        for (string::iterator j=line.begin(); j!=line.end(); j++){
          if(*j!=separator)
            temp +=*j;
          else{
            v.push_back(stod(temp,0));
            temp = "";
          }
        }
        M.push_back(v);
      }
    }
  }else
    cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!ERROR!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl << "Staci::CSVRead(), File is not open when calling CSVRead function!!!" << endl;
  return M;
}

//--------------------------------------------------------------
double Staci::get_sum_of_pos_consumption() {
  double sum = 0.0, cons;
  for (unsigned int i = 0; i < nodes.size(); i++) {
    cons = nodes.at(i)->getDemand();
    if (cons > 0.) sum += cons;
  }
  return sum;
}