#include "Calibration.h"

using namespace std;

Calibration::Calibration(string caseFileName) : Sensitivity(caseFileName){}
Calibration::~Calibration(){}

void Calibration::generateMeasurement(const vector<double> &fric, vector<double> demandNoiseValue, string demandNoiseMode){

  string caseFileName = getDefinitionFile();
  string measurementFolder = caseFileName.substr(0,caseFileName.rfind('.')) + '/';

  measuredDemandSum = readVectorDouble((measurementFolder+"measuredDemandSum.txt").c_str());
  numberOperationalPoint = measuredDemandSum.rows();
  if(numberOperationalPoint==0){
    cout << endl << "ERROR!!! measuredDemandSum is empty i.e. the sum of the overall consumptions must be given in " << measurementFolder + "/measuredDemandSum.txt file" << endl;
    exit(-1);
  }

  measuredPressureID = readVectorString((measurementFolder+"measuredPressureID.txt").c_str());
  measuredPressureIndex = ID2Index(measuredPressureID);
  numberPressure = measuredPressureIndex.size();
  if(numberPressure==0){
    cout << endl << "ERROR!!! measuredPressureID is empty i.e. the node IDs are not given in " << measurementFolder + "/measuredDemandSum.txt file" << endl;
    exit(-1);
  }

  measuredDemand = readMatrixDouble((measurementFolder+"measuredDemand.txt").c_str(),';');
  numberDemand = measuredDemandIndex.size();

  int numberNodes = nodes.size(), numberEdges = edges.size();

  // Saving original frics and demands
  vector<double> fricNominal(numberNodes,0.), demandNominal(numberEdges,0.);
  for(int k=0;k<numberNodes;k++)
    demandNominal[k] = nodes.at(k)->getDemand();
  for(int k=0;k<numberEdges;k++)
    if(edges[k]->getType() == "Pipe")
      fricNominal[k] = edges.at(k)->getProperty("roughness");

  //if((demandNoiseMode == "Uniform" || demandNoiseMode == "Normal" || demandNoiseMode == "None") == false){
  if((demandNoiseMode == "None") == false){
    cout << "\n!!!!! ERROR !!!!!\nCalibration:generateMeasurement function\nPerturbation methods available: Uniform || Normal || None\n\"" << demandNoiseMode << "\" is not existing.\nChanging to None!!! Continouing...\n\n";
    demandNoiseMode = "None";
  }

  double demandSumNominal = 0., cons; // Sum of every demands at nominal operational point
  for(int i=0; i<nodes.size(); i++){
    cons = nodes.at(i)->getDemand();
    if (cons > 0.)
      demandSumNominal += cons;
  }
  double demandSumMeasuredNomnial=0.; // Sum of measured demands at nominal operational point
  for(int i=0; i<numberDemand; i++)
    demandSumMeasuredNomnial += nodes[measuredDemandIndex[i]]->getDemand();

  string save_folder = getDefinitionFile();
  save_folder = save_folder.substr(0,save_folder.rfind('.')) + '/';

  FILE *p_file, *pall_file;
  p_file = fopen((save_folder+"measuredPressure.txt").c_str(),"w");
  if(p_file==NULL)
    cout << endl << "Warning!!! measurementFolder named: " << save_folder << " does not exist! No saving will be performed in generateMeasurement..." << endl;
  pall_file = fopen((save_folder+"everyPressure.txt").c_str(),"w");
  if(pall_file==NULL)
    cout << endl << "Warning!!! measurementFolder named: " << save_folder << " does not exist! No saving will be performed in generateMeasurement..." << endl;

  // Clearing vectors
  measuredPressure = MatrixXd::Zero(numberOperationalPoint, numberPressure);
  everyPressure = MatrixXd::Zero(numberOperationalPoint, numberNodes);

  for(int k=0;k<numberEdges;k++)
    if(edges[k]->getType() == "Pipe")
      edges[k]->setProperty("roughness",fric[k]);
  for(int j=0;j<numberOperationalPoint;j++){
    double demandSumMeasuredReal=0.;
    for(int k=0;k<numberDemand;k++){
      nodes.at(measuredDemandIndex[k])->setDemand(measuredDemand(j,k));
      demandSumMeasuredReal += measuredDemand(j,k);
    } 
    for(int k=0;k<numberNodes;k++){
      bool measured=false; int l=0;
      while(l<measuredDemandID.size() && !measured){
        if(measuredDemandID[l] == nodes[k]->getName())
          measured = true;
        l++;
      }
      if(demandNoiseMode == "Uniform")
        if(nodes[k]->getDemand() > 0. && !measured)
          nodes[k]->setDemand((measuredDemandSum(j)*demandSumNominal-demandSumMeasuredReal)/(demandSumNominal-demandSumMeasuredNomnial)*demandNominal[k]*UniformDist(demandNoiseValue[0],demandNoiseValue[1]));
      if(demandNoiseMode == "Normal")
        if(nodes[k]->getDemand() > 0. && !measured)
          nodes[k]->setDemand((measuredDemandSum(j)*demandSumNominal-demandSumMeasuredReal)/(demandSumNominal-demandSumMeasuredNomnial)*demandNominal[k]*NormalDist(demandNoiseValue[0],demandNoiseValue[1]));
      if(demandNoiseMode == "None")
        if(nodes[k]->getDemand() > 0. && !measured)
          nodes[k]->setDemand((measuredDemandSum(j)*demandSumNominal-demandSumMeasuredReal)/(demandSumNominal-demandSumMeasuredNomnial)*demandNominal[k]); 
    }
    // Adjusting the sum of the consumptions in case of noise
    if(demandNoiseMode == "Uniform" || demandNoiseMode == "Normal"){
      measuredDemandSum(j) = 0.;
      for(int k=0; k<numberNodes; k++)
        if(nodes[k]->getDemand()>0.)
          measuredDemandSum(j) += nodes[k]->getDemand();
      measuredDemandSum(j) = measuredDemandSum(j)/demandSumNominal;
    }
    solveSystem();
    for(int k=0;k<numberPressure;k++){
      measuredPressure(j,k) = nodes[measuredPressureIndex[k]]->getHead();
      if(p_file!=NULL)
	      fprintf(p_file,"%8.5e;",measuredPressure(j,k));
    }
    if(p_file!=NULL)
	    fprintf(p_file,"\n");
    for(int k=0; k<numberNodes; k++){
      everyPressure(j,k) = nodes[k]->getHead();
      if(pall_file!=NULL)
	      fprintf(pall_file,"%8.5e;",everyPressure(j,k));
    }
    if(pall_file!=NULL)
	    fprintf(pall_file,"\n");
  }

  if(p_file!=NULL)
  	fclose(p_file);
  if(pall_file!=NULL)
  	fclose(pall_file);

  // setting back original frics and demands
  for(int k=0;k<numberNodes;k++)
    nodes[k]->setDemand(demandNominal[k]);
  for(int k=0;k<numberEdges;k++)
    if(edges[k]->getType() == "Pipe")
      edges[k]->setProperty("roughness",fricNominal[k]);
  solveSystem();

  if(getDebugLevel()>1)
    cout << endl << "Generating measurement data: OK" << endl;
}

void Calibration::loadMeasurement(string caseFileName){
  string measurementFolder = caseFileName.substr(0,caseFileName.rfind('.')) + '/';

  // Reading the measured pressure values
  measuredPressure = readMatrixDouble((measurementFolder+"measuredPressure.txt").c_str(),';');

  // Reading the IDs of the measured nodes
  measuredPressureID = readVectorString((measurementFolder+"measuredPressureID.txt").c_str());
  measuredPressureIndex = ID2Index(measuredPressureID);
  numberPressure = measuredPressureIndex.size();

  // Reading the sum of the consumptions
  measuredDemandSum = readVectorDouble((measurementFolder+"measuredDemandSum.txt").c_str());
  numberOperationalPoint = measuredDemandSum.rows();

  // Reading the individually measured demands
  measuredDemand = readMatrixDouble((measurementFolder+"measuredDemand.txt").c_str(),';');
  measuredDemandID = readVectorString((measurementFolder+"measuredDemandID.txt").c_str());
  measuredDemandIndex = ID2Index(measuredDemandID);
  numberDemand = measuredDemandIndex.size();

  // IMPORTANT: CHECKING THE SIZE OF THE VARIABLES
  if(measuredPressure.rows() != numberOperationalPoint){
    cout << endl << "ERROR!!! measuredPressure row size doest not match numberOperationalPoint(" << numberOperationalPoint << ") / measuredPressure(" << measuredPressure.rows() << ")" << endl << "The measured pressure values does not match with the measured operational points" << endl;
    exit(-1);
    if(measuredPressure.rows()>0){
      if(measuredPressure.cols() != numberPressure){
        cout << endl << "ERROR!!! measuredPressure col size doest not match numberOperationalPoint / measuredPressureID" << endl << "The measured pressure values does not match with the measured node IDs" << endl;
        exit(-1);
      }
    }
  }
  if(numberDemand>0){
    if(measuredDemand.rows() != numberOperationalPoint){
      cout << endl << "ERROR!!! measuredDemand size doest not match numberOperationalPoint / measuredDemandSum" << endl << "The measured demand values does not match with the measured operational points" << endl;
      exit(-1);
      if(measuredDemand.rows()>0){
        if(measuredPressure.cols() != numberDemand){
          cout << endl << "ERROR!!! measuredDemand size doest not match numberDemand / measuredDemandID" << endl << "The measured pressure values does not match with the measured node IDs" << endl;
          exit(-1);
        }
      }
    }
  }
 if(getDebugLevel()>1)
  cout << endl << "Loading measurement data: OK" << endl; 
}
/*
void Calibration::loadDemandSum(string caseFileName){
  string measurementFolder = caseFileName.substr(0,caseFileName.rfind('.')) + '/';
  ifstream ifile;
  // Reading the sum of the consumptions
  ifile.open((measurementFolder+"measuredDemandSum.txt").c_str());
  if(ifile.is_open()){
    vector<string> temp = readStrings(ifile);    
    measuredDemandSum.clear();
    for(int i=0; i<temp.size(); i++)
      measuredDemandSum.push_back(stod(temp[i],0));
    numberOperationalPoint = measuredDemandSum.rows();
    if(numberOperationalPoint==0)
      cout << endl << "Warning! File " << measurementFolder+"measuredDemandSum.txt" << " does exist, but does NOT contain anything!" << endl;
  }
  else{
    cout << endl << "ERROR!!! File " << measurementFolder+"measuredDemandSum.txt" << " does NOT exist!" << endl << "Exiting..." << endl;
    exit(-1);
  }
  ifile.close();
}

void Calibration::loadPressureID(string caseFileName){
  string measurementFolder = caseFileName.substr(0,caseFileName.rfind('.')) + '/';
  ifstream ifile;
(measurementFolder+"measuredPressureID.txt").c_str()
  // Reading the IDs of the measured nodes
  ifile.open((measurementFolder+"measuredPressureID.txt").c_str());
  if(ifile.is_open()){
    measuredPressureID.clear();
    measuredPressureIndex.clear();
    measuredPressureID = readVectorString((measurementFolder+"measuredPressureID.txt").c_str());
    measuredPressureIndex = ID2Index(measuredPressureID);
    numberPressure = measuredPressureIndex.size();
    if(numberPressure==0)
      cout << endl << "Warning! File " << measurementFolder+"measuredPressureID.txt" << " does exist, but does NOT contain anything!" << endl;
  }
  else{
    cout << endl << "ERROR!!! File " << measurementFolder+"measuredPressureID.txt" << " does NOT exist!" << endl << "Exiting..." << endl;
    exit(-1);
  }
  ifile.close();
}

void Calibration::loadDemand(string caseFileName){
  // Reading the measured consumptions if they existed
;

  // Reading the ids of the nodes where consumptions were measured if they existed
  ifile.open((measurementFolder+"measuredDemandID.txt").c_str());
  if(ifile.is_open()){
    measuredDemandID.clear();
    measuredDemandIndex.clear();
    measuredDemandID = readStrings((measurementFolder+"measuredDemandID.txt").c_str());
    measuredDemandIndex = ID2Index(measuredDemandID);
  }
  else{
    if(getDebugLevel()>2){
      cout << endl << "Warning!!! File " << measurementFolder+"measuredDemandID.txt" << " does NOT exist!" << endl << "Exiting..." << endl;
    }
  }
  ifile.close();
}*/