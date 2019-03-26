#include "Calibration.h"

using namespace std;

Calibration::Calibration(string caseFileName) : Sensitivity(caseFileName){}
Calibration::~Calibration(){}

void Calibration::generateMeasurement(const vector<double> &fric, vector<double> demandNoiseValue, string demandNoiseMode){

  string caseFileName = getDefinitionFile();
  string measurementFolder = caseFileName.substr(0,caseFileName.rfind('.')) + '/';

  // loading the operation point where the simulated measuremts will occur
  loadMeasurement(caseFileName,false);

  int numberNodes = nodes.size(), numberEdges = edges.size();

  // Saving original frics and demands
  vector<double> fricNominal(numberNodes,0.), demandNominal(numberEdges,0.);
  for(int k=0;k<numberNodes;k++)
    demandNominal[k] = nodes.at(k)->getProperty("demand");
  for(int k=0;k<numberEdges;k++)
    if(edges[k]->getEdgeStringProperty("type") == "Pipe")
      fricNominal[k] = edges.at(k)->getDoubleProperty("roughness");

  //if((demandNoiseMode == "Uniform" || demandNoiseMode == "Normal" || demandNoiseMode == "None") == false){
  if((demandNoiseMode == "None") == false){
    cout << "\n!!!!! ERROR !!!!!\nCalibration:generateMeasurement function\nPerturbation methods available: Uniform || Normal || None\n\"" << demandNoiseMode << "\" is not existing.\nChanging to None!!! Continouing...\n\n";
    demandNoiseMode = "None";
  }

  double demandSumNominal = 0., cons; // Sum of every demands at nominal operational point
  for(int i=0; i<nodes.size(); i++){
    cons = nodes.at(i)->getProperty("demand");
    if (cons > 0.)
      demandSumNominal += cons;
  }
  double demandSumMeasuredNomnial=0.; // Sum of measured demands at nominal operational point
  for(int i=0; i<numberDemand; i++)
    demandSumMeasuredNomnial += nodes[measuredDemandIndex[i]]->getProperty("demand");

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
    if(edges[k]->getEdgeStringProperty("type") == "Pipe")
      edges[k]->setDoubleProperty("roughness",fric[k]);
  for(int j=0;j<numberOperationalPoint;j++){
    double demandSumMeasuredReal=0.;
    for(int k=0;k<numberDemand;k++){
      nodes.at(measuredDemandIndex[k])->setProperty("demand",measuredDemand(j,k));
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
        if(nodes[k]->getProperty("demand") > 0. && !measured)
          nodes[k]->setProperty("demand",(measuredDemandSum(j)*demandSumNominal-demandSumMeasuredReal)/(demandSumNominal-demandSumMeasuredNomnial)*demandNominal[k]*UniformDist(demandNoiseValue[0],demandNoiseValue[1]));
      if(demandNoiseMode == "Normal")
        if(nodes[k]->getProperty("demand") > 0. && !measured)
          nodes[k]->setProperty("demand",(measuredDemandSum(j)*demandSumNominal-demandSumMeasuredReal)/(demandSumNominal-demandSumMeasuredNomnial)*demandNominal[k]*NormalDist(demandNoiseValue[0],demandNoiseValue[1]));
      if(demandNoiseMode == "None")
        if(nodes[k]->getProperty("demand") > 0. && !measured)
          nodes[k]->setProperty("demand",(measuredDemandSum(j)*demandSumNominal-demandSumMeasuredReal)/(demandSumNominal-demandSumMeasuredNomnial)*demandNominal[k]); 
    }
    // Adjusting the sum of the consumptions in case of noise
    if(demandNoiseMode == "Uniform" || demandNoiseMode == "Normal"){
      measuredDemandSum(j) = 0.;
      for(int k=0; k<numberNodes; k++)
        if(nodes[k]->getProperty("demand")>0.)
          measuredDemandSum(j) += nodes[k]->getProperty("demand");
      measuredDemandSum(j) = measuredDemandSum(j)/demandSumNominal;
    }
    solveSystem();
    for(int k=0;k<numberPressure;k++){
      measuredPressure(j,k) = nodes[measuredPressureIndex[k]]->getProperty("head");
      if(p_file!=NULL)
	      fprintf(p_file,"%8.5e;",measuredPressure(j,k));
    }
    if(p_file!=NULL)
	    fprintf(p_file,"\n");
    for(int k=0; k<numberNodes; k++){
      everyPressure(j,k) = nodes[k]->getProperty("head");
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
    nodes[k]->setProperty("demand",demandNominal[k]);
  for(int k=0;k<numberEdges;k++)
    if(edges[k]->getEdgeStringProperty("type") == "Pipe")
      edges[k]->setDoubleProperty("roughness",fricNominal[k]);
  solveSystem();

  if(getDebugLevel()>1)
    cout << endl << "Generating measurement data: OK" << endl;
}

void Calibration::loadMeasurement(string caseFileName, bool isPressure){
  string measurementFolder = caseFileName.substr(0,caseFileName.rfind('.')) + '/';

  // Reading the sum of the consumptions
  measuredDemandSum = readVectorDouble((measurementFolder+"measuredDemandSum.txt").c_str());
  numberOperationalPoint = measuredDemandSum.rows();
  if(numberOperationalPoint==0){
    cout << endl << "ERROR!!! measuredDemandSum is empty i.e. the sum of the overall consumptions must be given in " << measurementFolder + "/measuredDemandSum.txt file" << endl;
    exit(-1);
  }

  // Reading the measured pressure values
  if(isPressure)
    measuredPressure = readMatrixDouble((measurementFolder+"measuredPressure.txt").c_str(),';');

  // Reading the IDs of the measured nodes
  measuredPressureID = readVectorString((measurementFolder+"measuredPressureID.txt").c_str());
  measuredPressureIndex = ID2Index(measuredPressureID);
  numberPressure = measuredPressureIndex.size();
  if(numberPressure==0){
    cout << endl << "ERROR!!! measuredPressureID is empty i.e. the node IDs are not given in " << measurementFolder + "/measuredDemandSum.txt file" << endl;
    exit(-1);
  }

  // Reading the individually measured demands
  measuredDemand = readMatrixDouble((measurementFolder+"measuredDemand.txt").c_str(),';');
  measuredDemandID = readVectorString((measurementFolder+"measuredDemandID.txt").c_str());
  measuredDemandIndex = ID2Index(measuredDemandID);
  numberDemand = measuredDemandIndex.size();

  // IMPORTANT: CHECKING THE SIZE OF THE VARIABLES
  if(isPressure){
    if(measuredPressure.rows() != numberOperationalPoint){
      cout << endl << "ERROR!!! measuredPressure row size doest not match numberOperationalPoint(" << numberOperationalPoint << ") / measuredPressure(" << measuredPressure.rows() << ")" << endl << "Th size of measured pressure values does not match with the measured operational points" << endl;
      exit(-1);
      if(measuredPressure.cols() != numberPressure){
        cout << endl << "ERROR!!! measuredPressure col size doest not match numberOperationalPoint(" << numberOperationalPoint << ") / measuredPressureID(" << measuredPressure.cols() << ")" << endl << "The size of measured pressure values does not match with the measured node IDs" << endl;
        exit(-1);
      }
    }
  }
  if(numberDemand>0){
    if(measuredDemand.rows() != numberOperationalPoint){
      cout << endl << "ERROR!!! measuredDemand size doest not match numberOperationalPoint(" << numberOperationalPoint << ") / measuredDemandSum(" << measuredDemand.rows() << ")" << endl << "The measured demand values does not match with the measured operational points" << endl;
      exit(-1);
      if(measuredDemand.rows()>0){
        if(measuredDemand.cols() != numberDemand){
          cout << endl << "ERROR!!! measuredDemand size doest not match numberDemand(" << measuredDemand.cols() << ") / measuredDemandID(" << numberDemand << ")" << endl << "The measured pressure values does not match with the measured node IDs" << endl;
          exit(-1);
        }
      }
    }
  }
 if(getDebugLevel()>1)
  cout << endl << "Loading measurement data: OK" << endl; 
}
