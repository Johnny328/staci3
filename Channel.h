#include "Edge.h"
using namespace std;

class Channel : public Edge {
private:
	/// A Channel hossza [m]-ben
	double L;

	/// A Channel elej�nek �s v�g�nek geodetikus magass�ga
	double ze, zv;

	/// lejt�s %-ban, pozit�v sz�m, ha elej�t�l v�ge fel� lejt
	double lejtes;

	/// geometria tipusa
	int geo_tipus;

	/// S�rl�d�si t�nyez� & friends
	double erdesseg, lambda, surl, headloss;

	/// A geometri�ra jellemz� param�terek:
	/// geo_tipus = 0 -> t�glalap: B, Hmax
	/// geo_tipus = 1 -> k�r: dia
	double B, Hmax, dia, y;
	vector<double> yb, ab, ya, aa, yk, ak;

	/// Numerikus l�p�sk�z: dx=L/db;
	int db;

	/// Az f �s df f�ggv�ny ki�rt�kel�s�t sz�molja
	int f_count, df_count;

	/// Az �gegyenlet aktu�lis �rt�ke (hogy ne kelljen df sz�m�t�sakor �jra kisz�m�tani
	double ff;

	/// Esetsz�tv�laszt�s, az @see f
	string eset;

	/// Ha integr�ltunk egyet, megjegyezz�k az integr�lt, �gy gyorsabb df ki�rt�kel�se ill. ye-t megjegyezzuk
	double yi, yyee, yyvv;

	/// Au integr�l�s utani sebesseg- es szinteloszl�sok
	vector<double> xres, yf, Hres, yres, vres;

	/// Az ode_megoldonak ez mondja meg, hogy az yres-be es a vres-be betegye-e az eredmenyeket (amikor Jacobit szamolunk, nem kell)
	bool write_res;

	/// A vegso eloszlasok kimenet
	vector<double> xfere, yfere, xere, yere, yereH, vere;

	/// A vegso eloszlasok mar elkeszultek?
	bool res_ready;

	/// Kl�reloszl�s jellemz�k
	double cl_k, cl_w;

	/// Eredm�nyeloszl�sok kisz�m�t�sa
	void build_res();

	/// Ki kell-e __numerikusan__ sz�m�tani a Jacobi elemeit: [df_dye, df_dyv, df_dQ]
	vector<bool> num_eval_jac;

	/// A Jacobi elemei: [df_dye, df_dyv, df_dQ]
	vector<double> jac;

	/// Norm�lszint sz�m�t�sa
	vector<double> normal_szint(const double Q);

	/// Kritikus szint sz�m�t�sa
	double kritikus_szint(const double Q);

	/// Kritikus �s norm�lszint sz�m�t�s�n�l maxim�lis iter�ci�sz�m
	const static unsigned int iter_max = 1000;

	// double f_simple(vector<double>);
	// double f_full(vector<double>);

	double f_telt(const double ye, const double yv, const double mp);
	double f_0(const double ye, const double yv, const double mp);
	double f_1(const double ye, const double yv, const double mp);
	double f_1_old(const double ye, const double yv, const double mp);
	double f_2c(const double ye, const double yv, const double mp);
	double f_2c_old(const double ye, const double yv, const double mp);
	double f_2aii(const double ye, const double yv, const double mp);
	double f_2ai(const double ye, const double yv, const double mp);

	/// Z�rtszelv�ny� cs�h�z (@see f_0) kell
	double D_fake;

	/// Kapcsol�d� akna nyom�sok �s fen�kszintek (@see f())
	double pe, pv, he, hv;

	/// Meg kellett forditani a Channelt?
	bool is_reversed;

	void which_case(const double ye, const double yv);

	double (Channel::*pt2fun)(double, double, double);

	bool is_simplified;
	bool is_switched;
	double vmean;
	double y_crit;
	vector<double> y_normal;

public:
	/// Konstruktor t�glalap km. eset�n
	//	Channel(const string nev, const string a_cspe_nev,
	//			const string a_cspv_nev, const double Aref, const double L,
	//			const double ze, const double zv, const double surl,
	//			const int int_steps, const int debugl, const double width,
	//			const double Hmax, const double a_cl_k, const double a_cl_w);

	/// Konstruktor k�r km. eset�n
	Channel(const string nev, const string a_cspe_nev,
	         const string a_cspv_nev, const double a_ro, const double Aref, const double L,
	         const double ze, const double zv, const double surl,
	         const int int_steps, const int debugl, const double dia,
	         const double a_cl_k, const double a_cl_w, const bool is_reversed, const double a_mp);

	/// Konstruktor user-defined km. eset�n
	//	Channel(const string a_nev, const string a_cspe_nev,
	//			const string a_cspv_nev, const double Aref, const double L,
	//			const double ze, const double zv, const double surl,
	//			const int int_steps, const int debugl, const vector<double> ayb,
	//			const vector<double> aab, const vector<double> aya,
	//			const vector<double> aaa, const vector<double> ayk,
	//			const vector<double> aak, const double a_cl_k, const double a_cl_w);

	/// Destruktor
	~Channel();

	/// Inform�ci�
	string Info();

	/// �gegyenlet �rt�ke
	double f(vector<double>);

	/// �gegyenlet derv�ltja
	vector<double> df(vector<double>);
	// vector<double> df_simple(vector<double>);

	/// Inicializ�ci�
	void Ini(int mode, double value);

	/// Keresztmetszeti jellemzok szamitasa
	void keresztmetszet(const double y, double &A, double &B, double &Rh);

	/// Norm�l- �s kritikus szint sz�m�t�sa adott t�rfogat�ramhoz
	void norm_krit(const double Q);

	/// A ny�ltfelsz�n� �raml�s le�r� DE
	double nyf_ode(const double x, const double y, const double mp);

	/// ODE integr�l�s
	double ode_megoldo(double ystart, double dx, double xstart, double mp);

	/// debug f�jl �r�sa
	///void logfile_write(string msg, int msg_debug_level);

	/// Az eredmenyeloszlasok elkeszitese
	void compute_distr();

	/// Eredm�nyeloszl�sok visszaad�sa
	vector<double> Get_res(string mit);

	/// Surl�dosi modell
	double surlodas();

	/// Get Double property
	double Get_dprop(string mit);

	/// Folyadekterfogat az elemben
	double Get_FolyTerf();

	/// Hibauzenet
	void error(string fv, string msg);

	/// Figyelmeztetes
	void warning(string fv, string msg);

	string GetType() {
		return "Channel";
	}

	void Set_dprop(string mit, double mire);

	/// Overriding Edge::SetLogFile()
	// void virtual SetLogFile();

	double virtual Get_v()
	{
		if (!res_ready)
			build_res();
		return vmean;
	}

	bool Get_res_ready() {
		return res_ready;
	}


};
