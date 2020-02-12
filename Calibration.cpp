#include "Calibration.h"

using namespace std;

Calibration::Calibration(string fileName) : SeriesHydraulics(fileName)
{
  measuredTimeConvert = 60.; // i.e. time must in minutes or this must be change accordingly
  loadMeasurement();
}

Calibration::~Calibration(){}

//--------------------------------------------------------------
void Calibration::loadMeasurement()
{
  // Reading the measured pressure values
  measuredPressure = readMatrixXdDouble((caseName+"/pressureData.txt").c_str(),';');

  // Loading the time appointments and converting to seconds
  measuredTime = readVectorDouble((caseName+"/time.txt").c_str());
  for(int i=0; i<measuredTime.size(); i++)
  {
    measuredTime[i] = measuredTimeConvert*measuredTime[i];
  }
  numberOperationalPoint = measuredTime.size();

  // Reading the IDs of the measured nodes
  measuredPressureID = readVectorString((caseName+"/pressureID.txt").c_str());
  for(int i=0; i<measuredPressureID.size(); i++)
  {
    int idx = nodeIDtoIndex(measuredPressureID[i]);
    if(idx != -1)
      measuredPressureIndex.push_back(idx);
  }
  numberPressure = measuredPressureIndex.size();

  // IMPORTANT: CHECKING THE SIZE OF THE VARIABLES
  if(measuredPressure.rows() != numberOperationalPoint)
  {
    cout << endl << "ERROR!!! measuredPressure row size doest not match numberOperationalPoint(" << numberOperationalPoint << ") / measuredPressure(" << measuredPressure.rows() << ")" << endl << "Th size of measured pressure values does not match with the measured operational points" << endl;
    exit(-1);
  }
  if(measuredPressure.cols() != numberPressure)
  {
    cout << endl << "ERROR!!! measuredPressure col size doest not match numberPressure(" << numberPressure << ") / measuredPressure(" << measuredPressure.cols() << ")" << endl << "The size of measured pressure values does not match with the measured node IDs" << endl;
    exit(-1);
  }
}

//--------------------------------------------------------------
void Calibration::savePipes()
{
  FILE *wFile;
  wFile = fopen((caseName+"/Pipe.txt").c_str(),"w");

  for(int i=0; i<pipeIndex.size(); i++)
  {
    int idx = pipeIndex[i];
    string cv = "";
    if(edges[idx]->isCheckValve)
      cv = "cv";

    fprintf(wFile, " %10s %10s %10s %10.3f %10.3f %10.3f %10s\n", edges[idx]->name.c_str(), edges[idx]->startNodeName.c_str(), edges[idx]->endNodeName.c_str(), edges[idx]->getDoubleProperty("length"), edges[idx]->getDoubleProperty("diameter"), 0.0, cv.c_str());
  }

  fclose(wFile);
}
