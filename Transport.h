#include "HydraulicSolver.h"

class Transport : public HydraulicSolver
{
public:
	double tt_length, cl_length; //, mp_init, p_init;
  double transp_dt, ido; //, transp_dt_ki;

	Transport();
	~Transport();
	void set_up_transport();
	void solve_transport(int mode);
	void save_transport(int mode);
	double get_oldest();
	void transport_step(double dt);
	double teta(double konc, const int i);
	void solve_residence_time();
	void residence_time_step(string & max_ID, double & max_VAL, double & mean_VAL);
	string convert_to_hr_min(double input_seconds);
};