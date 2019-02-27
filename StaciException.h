class StaciException{
private:
 string hibauzenet; //verbal description of the error
public:
 StaciException(string d):hibauzenet(d){}
 string getDescription() {return hibauzenet;}
};
