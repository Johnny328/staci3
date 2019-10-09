#include <iostream>
#include <math.h>

using namespace std;

int main()
{
	cout << endl;
	double time = 3843;
	int hour = time/3600.;
	int minute = (time - hour*3600.)/60.;
	printf(" [*] time: %3i:%2i\n", hour, minute);

	cout << endl << endl;
	return 0;
}