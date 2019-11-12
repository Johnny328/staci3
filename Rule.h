/*===================================================================*\
                                  Rule
                                  ----
  TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO
 
  staci3 is using Eigen, see http://eigen.tuxfamily.org

    staci3
    Cs. Hos, R. Weber, T. Huzsvar
    https://github.com/weberrichard/staci3
\*==================================================================*/

#ifndef RULE_H
#define RULE_H

#include <string>
#include <vector>

using namespace std;

class Rule
{
public:
  Rule(string id){
    ID = id;
  }
  ~Rule();

  string ID; // ID of the rule

  // Handling conditions
  vector<string> conditionType; // SYSTEM, NODE, LINK etc.
  vector<string> conditionID;
  vector<int> conditionIndex;
  vector<string> conditionRelation; // BELOW, ABOVE, <, >, =, <=, >=
  vector<double> conditionValue; // double number

  // Taking actions
  vector<string> actionType; // LINK, PUMP, PIPE, VALVE
  vector<string> actionID;
  vector<int> actionIndex;
  vector<bool>   actionSetting; // if SETTING true, if STATUS false
  vector<double> actionValue;
  vector<bool>   actionCounter; // if THEN ... false, if ELSE ... true
};

#endif
