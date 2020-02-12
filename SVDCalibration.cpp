#include "SVDCalibration.h"

using namespace std;
using namespace Eigen;

SVDCalibration::SVDCalibration(string fileName) : Calibration(fileName)
{
  /// in default we use every measured data
  for(int i=0; i<measuredTime.size(); i++)
  { 
    measuredUsedIndex.push_back(i);
    measuredUsedTime.push_back(measuredTime[i]);
  }
}

SVDCalibration::~SVDCalibration(){}

//-----------------------------------------------------------------
int SVDCalibration::calibrate()
{
  // selecting the used measurements
  numberMeasuredUsed = measuredUsedTime.size();
  MatrixXd measuredUsedPressure;
  measuredUsedPressure.resize(numberMeasuredUsed, numberPressure);
  for(int i=0; i<measuredUsedTime.size(); i++)
  {
    measuredUsedPressure(i,all) = measuredPressure(measuredUsedIndex[i],all);
  }

  //if(printLevelSVD>1)
  //  cout << "measuredUsedPressure: " << measuredUsedPressure.rows() << "x" << measuredUsedPressure.cols() << endl << measuredUsedPressure << endl;

  // saving the original roughness coefficients
  int numberPipes = pipeIndex.size();
  VectorXd roughnessOriginal(numberPipes);
  for(int i=0; i<numberPipes; i++)
  { 
    int idx = pipeIndex[i];
    roughnessOriginal(i) = edges[idx]->getDoubleProperty("roughness");
  }

  // Start of main iteration
  if(printLevelSVD>0)
    cout << "\n\n---------------------------------\n******Starting Calibration******\n---------------------------------\n\n";

  seriesSensitivityControl.parameter = "roughness";
  seriesSensitivityControl.rowIndex = measuredPressureIndex;
  seriesSensitivityControl.isPressure = true;
  seriesSensitivityControl.time = measuredUsedTime;

  int convSVD=0;
  double e_p_norm=1e10;

  // clearing vectors
  seriesInitialization(); // setting back tanks and clearing outputs

  bool convSolveSystem = true;
  time = startTime;
  clockTime = startTime;

  MatrixXd everyRoughness(numberPipes,numberMeasuredUsed);

  for(int i=0; i<measuredUsedTime.size(); i++)
  {
    //for(int k=0; k<pipeIndex.size(); k++)
    //{
    //  int idx=pipeIndex[k];
    //  edges[idx]->setDoubleProperty("roughness",roughnessOriginal(k));
    //}

    for(int j=0; j<iterMax; j++)
    {
      //cout << endl << "r: " << endl;
      //for(int k=0; k<pipeIndex.size(); k++)
      //  cout << edges[pipeIndex[k]]->name << "  " << edges[pipeIndex[k]]->getDoubleProperty("roughness") << " | ";
      //cout << endl;

      while(time<=measuredUsedTime[i] && convSolveSystem)
      {
        // printing basic info to consol
        if(printLevel > 1)
          seriesInfo();

        updateDemand();
        updatePressurePointPattern();
        updateControl();
        updateRule();

        if(isSensitivityCalculation(seriesSensitivityControl))
        {     
          convSolveSystem = calculateSensitivity(seriesSensitivityControl.parameter);
          saveSensitivityMatrix(seriesSensitivityControl);
          if(printLevel > 1)
            cout << endl << " Sensitivity calculation was done. " << endl;
          break;
        }
        else
        {
          convSolveSystem = solveSystem();
        }

        if(time<measuredUsedTime[i])
        {
          // saving heads and volume flow rates
          saveOutput();

          hydraulicTimeStep = newHydraulicTimeStep();
          time += hydraulicTimeStep;

          // updating the settings, pools etc.
          updatePool();
        }

        clockTime = time;
        while(clockTime>=86400.)
          clockTime -= 86400.;
      }

      // constructin SMred and ep
      MatrixXd SMred = seriesSensitivity.back();
      //cout << endl << "SMred: " << endl << SMred << endl;

      VectorXd calculatedPressure;
      calculatedPressure.resize(numberPressure);
      // collecting the calculated pressures
      for(int j=0; j<numberPressure; j++)
        calculatedPressure(j) = nodes[measuredPressureIndex[j]]->head;

      VectorXd ep = measuredUsedPressure(i,all).transpose() - calculatedPressure;
      //if(printLevelSVD>1)
      //{
      //  cout << endl << "mp: " << measuredUsedPressure(i,all).transpose().rows() << endl << measuredUsedPressure(i,all).transpose() << endl;
      //  cout << endl << "cp: " << calculatedPressure.rows() << endl << calculatedPressure << endl;
      //  cout << endl << "ep: " << ep.rows() << endl << ep << endl;
      //}

      // performing SVD and calculate dr
      MatrixXd U,S,V;
      eigenSVD(SMred,SVDTolerance,U,S,V);
      MatrixXd SMred_pinv = pinvSVD(U,S,V);
      VectorXd dr = SMred_pinv*ep;

      // Checking the largest relative change
      double satur=1., absMax=dr.cwiseAbs().maxCoeff();
      if(absMax > maxChange)
      {
        satur = maxChange/absMax;
        dr = dr*satur;
      }

      // applying changes
      for(int k=0; k<pipeIndex.size(); k++)
      {
        int idx = pipeIndex[k];
        double ri = edges[idx]->getDoubleProperty("roughness");
        double dri = dr(idx);
        edges[idx]->setDoubleProperty("roughness", ri+dri);
      }

      if(printLevelSVD>0)
      {
        printf("\n *** SVDCALIB");
        printf("  meas idx: %3i", i);
        printf("  iter: %3i", j);
        printf("  dr: %2.4e",dr.norm());
        printf("  e_p: %2.4e", ep.norm());
        printf("  e_p_r: %2.4e", e_p_norm);
        printf("  satur:  %1.3e",satur);

        double min_r=1e5, max_r=0.0;
        for(int k=0; k<pipeIndex.size(); k++)
        {
          int idx=pipeIndex[k];
          double r=edges[idx]->getDoubleProperty("roughness");
          if(r < min_r)
            min_r = r;
          if(r > max_r)
            max_r = r;
        }
        printf("  min_r:  %6.2f",min_r);
        printf("  max_r:  %6.2f",max_r);
        //if(printLevelSVD>1)
        //  cin.get();
      }
      e_p_norm = ep.norm();

      // checking convSVD
      if(ep.cwiseAbs().maxCoeff() < errorPressureStop || dr.cwiseAbs().maxCoeff() < dlambdaStop)
      {
        convSVD = 1;

        // saving head, consumptions for nodes and flows for edges
        saveOutput();

        hydraulicTimeStep = newHydraulicTimeStep();
        time += hydraulicTimeStep;

        // updating the settings, pools etc.
        updatePool();

        for(int k=0; k<pipeIndex.size(); k++)
        {
          int idx=pipeIndex[k];
          //cout << endl << "eR: " << everyRoughness.rows() << "x" << everyRoughness.cols() << endl << everyRoughness << endl;
          everyRoughness(k,i) = edges[idx]->getDoubleProperty("roughness");
        }
        cout << endl << "OK" << endl;
        break;
      }

      if(j==iterMax-1)
      {
        cout << "\nWARNING!! Convergence is not sufficient!" << endl;
        exit(-1);
      }
    }
  }

  if(printLevelSVD>0)
    cout << "\n\n---------------------------------\n******Ending Calibration******\n---------------------------------\n\n";

  if(printLevelSVD > 0)
  {
    VectorXd roughnessNew(numberPipes);
    printf("    edge name    |   roughness \n");
    for(int i=0; i<pipeIndex.size(); i++)
    {
      int idx=pipeIndex[i];
      double ri = edges[idx]->getDoubleProperty("roughness");
      printf(" %15s | %8.3f\n", edges[idx]->name.c_str(), ri);
      roughnessNew(i) = ri;
    }
    cout << endl << "(roughnessNew - roughnessOriginal).norm: " << (roughnessNew - roughnessOriginal).norm() << endl;
  }

  if(convSVD == 0)
  {
    for(int k=0; k<pipeIndex.size(); k++)
    {
      int idx = pipeIndex[k];
      edges[idx]->setDoubleProperty("roughness",roughnessOriginal(k)); 
    }
  }

  cout << endl << "roughnesses: " << endl << everyRoughness << endl;
  cout << endl << "eR: " << everyRoughness.rows() << "x" << everyRoughness.cols() << endl << everyRoughness << endl;

  return convSVD;
}

//-----------------------------------------------------------------
void SVDCalibration::eigenSVD(const MatrixXd &A, double &tol, MatrixXd &U, MatrixXd &S, MatrixXd &V)
{
  JacobiSVD<MatrixXd> svd(A, ComputeThinU | ComputeThinV);
  U = svd.matrixU();
  V = svd.matrixV();
  VectorXd v = svd.singularValues();
  S = MatrixXd::Constant(v.rows(),v.rows(),0.);
  //S.resize(v.rows(),v.rows());

  if(tol>1. || tol<0.)
    tol = .9;

  double tr=0.;
  for(int i=0; i<v.rows(); i++)
    tr += v(i)*v(i);
  
  double sum_tr = 0.;
  for(int i=0; i<v.rows(); i++)
  {
    if(sum_tr/tr<tol)
    {
      S(i,i) = v(i);
      sum_tr += v(i)*v(i);
    }
    else
    {
      break;
    }
  }

  if(printLevelSVD>1)
  {
    printf("\n*Singuar values of Sr (tol: %5.2f %%) : ", tol*100.);
    int i=0;
    while(i<v.rows() && S(i,i)>0.)
    {
      printf("%6.3e | ",v(i));
      i++;
    }
    printf("OFF: ");
    while(i<v.rows())
    {
      printf("%6.3e | ",v(i));
      i++;
    }
    cout << endl;
  }
}

//-----------------------------------------------------------------
MatrixXd SVDCalibration::pinvSVD(const MatrixXd &U, const MatrixXd &S, const MatrixXd &V)
{
  MatrixXd S_inv = MatrixXd::Constant(S.rows(),S.cols(),0.);
  //S_inv.resize(S.rows(),S.cols());
  for(int i=0; i<S.rows(); i++)
  {
    if(S(i,i)>1e-15)
      S_inv(i,i) = 1./S(i,i);
    else
      S_inv(i,i) = 0.0;
  }

  return V*(S_inv*U.transpose()); //See Numerical Recipes, Press, Teukolsky
}
