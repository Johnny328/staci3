#include "BasicFileIO.h"

// Reading lines into string from file
//--------------------------------------------------------------
vector<string> readVectorString(string fileName)
{
  ifstream ifile(fileName);
  string line;
  vector<string> M;
  if(ifile.is_open())
  {
    while(getline(ifile,line))
    { 
      if(line[0] != '/')
      {
        string tmp = line.substr(0,line.length()-1);
        M.push_back(tmp);
      }
    }
  }
  else
  {
    cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!ERROR!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl << "readStrings(), File is not open when calling CSVRead function!!!" << endl;
    exit(-1);
  }
  ifile.close();
  return M;
}

// Reading doubles from file into MatrixXd, separeted with "separator"
//--------------------------------------------------------------
MatrixXd readMatrixXdDouble(string fileName, char separator)
{
  if(separator == '/')
  {
    cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!ERROR!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl << "CSVRead(), Separator cannot be '/', that is for comments ONLY!!!" << endl;
    exit(-1);
  }

  ifstream ifile(fileName);
  vector<int> rowscols = countRowsCols(fileName,separator);
  int rows = rowscols[0], cols = rowscols[1];
  MatrixXd M = MatrixXd::Zero(rows,cols);

  string temp, line;
  if(ifile.is_open())
  {
    int midx=0;
    while(getline(ifile,line))
    {
      if(line[0] != '/')
      {
        VectorXd v = VectorXd::Zero(cols);
        int vidx=0;
        for (string::iterator j=line.begin(); j!=line.end(); j++){
          if(*j!=separator)
          {
            temp +=*j;
          }
          else
          {
            v(vidx) = stod(temp,0);
            vidx++;
            temp = "";
          }
        }
        M.row(midx) = v;
        midx++;
      }
    }
  }
  else
  {
    cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!ERROR!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl << "Staci::CSVRead(), File is not open when calling CSVRead function!!!" << endl;
    exit(-1);
  }
  ifile.close();
  return M;
}

// Reading doubles from file into vector<double>
//--------------------------------------------------------------
vector<double> readVectorDouble(string fileName)
{
  //int rows = countRows(fileName);
  ifstream ifile(fileName);
  vector<double> out;
  string line;
  if(ifile.is_open())
  {
    while(getline(ifile,line))
    {
      if(line[0] != '/')
      {
        out.push_back(stod(line,0));
      }
    }
  }
  else
  {
    cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!ERROR!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl << "Staci::CSVRead(), File is not open when calling CSVRead function!!!" << endl;
    exit(-1);
  }
  ifile.close();
  return out;
}


// Counting the rows of a file
//--------------------------------------------------------------
int countRows(string fileName)
{
  ifstream ifile(fileName);
  string line;
  int rows=0;
  while(getline(ifile,line))
    rows++;
  ifile.close();
  return rows;
}

// Counting the rows and cols of a file
//--------------------------------------------------------------
vector<int> countRowsCols(string fileName, char separator)
{
  ifstream ifile(fileName);
  string line;
  int cols=0, rows;
  vector<int> v;
  getline(ifile,line);
  for(string::iterator j=line.begin(); j!=line.end(); j++)
  {
    if(*j == separator)
      cols++;
  }
  rows = countRows(fileName);
  v.push_back(rows);
  v.push_back(cols);
  ifile.close();
  return v;
}
