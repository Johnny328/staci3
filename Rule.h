/*===================================================================*\
                                  Rule
                                  ----
  TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO
  TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO
  TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO
  TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO
  TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO
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
  Rule(string id);
  ~Rule();

  string ID; // ID of the rule

  vector<string> conditionType; // SYSTEM, NODE, LINK etc.
  vector<string> conditionID;
  vector<double> conditionValue; // simple double number
  vector<string> conditionRelation; // BELOW, ABOVE, <, >, =, <=, >=

  vector<string> actionType; // LINK, PUMP, PIPE, VALVE
  vector<string> actionID;
  vector<bool>   actionSetting; // if SETTING true, if STATUS false
  vector<double> actionValue;
  vector<bool>   actionCounter; // if THEN ... false, if ELSE ... true
};

#endif
