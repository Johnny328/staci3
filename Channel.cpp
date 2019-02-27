//using namespace std;
#include <cmath>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <vector>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <fstream>
#include "Edge.h"
#include "Channel.h"

//Channel::Channel(const string a_nev, const string a_cspe_nev,
//      const string a_cspv_nev, const double Aref, const double a_L,
//      const double a_ze, const double a_zv, const double a_erd,
//      const int a_int_steps, const int a_debugl, const double a_width,
//      const double a_Hmax, const double a_cl_k, const double a_cl_w) :
//  Edge(a_nev, Aref) {
//  //Kotelezo adatok minden Edgenel:
//  tipus="Channel";
//  csp_db=2;
//  cspe_nev=a_cspe_nev;
//  cspv_nev=a_cspv_nev;
//  // specifikus adatok
//  L =a_L;
//  ze =a_ze;
//  zv =a_zv;
//  lejtes =(ze-zv)/L;
//  erdesseg =a_erd;
//  geo_tipus=0;
//  // Numerikus eredmenyek tarolasara:
//  db=a_int_steps;
//  // Debug f�jl be�ll�t�sa.
//  df_name=nev+".out";
//  debug_level=a_debugl;
//  if (debug_level>4)
//      debug_level=4;
//  f_count=0;
//  df_count=0;
//  Hmax=a_Hmax;
//  B=a_width;
//  res_ready=false;
//  cl_k=a_cl_k;
//  cl_w=a_cl_w;
//}

//--------------------------------------------------------------
/// K�r km.
Channel::Channel(const string a_nev, const string a_cspe_nev,
                   const string a_cspv_nev, const double a_ro, const double Aref, const double a_L,
                   const double a_ze, const double a_zv, const double a_erd,
                   const int a_int_steps, const int a_debugl, const double a_dia,
                   const double a_cl_k, const double a_cl_w, const bool a_is_reversed, const double a_mp) :
	Edge(a_nev, a_dia * a_dia * pi / 4, a_mp, a_ro) {
	//Kotelezo adatok minden Edgenel:
	tipus = "Channel";
	csp_db = 2;
	cspe_nev = a_cspe_nev;
	cspv_nev = a_cspv_nev;
	// specifikus adatok
	L = a_L;
	ze = a_ze;
	zv = a_zv;
	lejtes = (ze - zv) / L;
	erdesseg = a_erd;
	geo_tipus = 1;
	// Numerikus eredmenyek tarolasara:
	db = 1000.;//a_int_steps * 10;

	// Set debug level
	debug_level = a_debugl;
	if (debug_level > 4)
		debug_level = 4;

	// debug_level = 2;

	f_count = 0;
	df_count = 0;
	dia = a_dia;
	Hmax = dia;
	D_fake = dia;

	res_ready = false;
	write_res = false;
	cl_k = a_cl_k;
	cl_w = a_cl_w;
	is_reversed = a_is_reversed;

	if (fabs(erdesseg) == 0.0)
		error("Konstruktor",
		      "A Channel erdessege zerus, ez nem megengedett!!! Kerem javitsa az adatot!");

	if (fabs(lejtes * 100.) < (0.01 / 100.))
		warning("Konstruktor",
		        "A Channel lejtese kisebb, mint 0.1mm/m (0.01%), ez biztosan helyes adat?");

	if (fabs(lejtes * 100.) < (10. / 100.))
		warning("Konstruktor",
		        "A Channel lejtese nagyobb, mint 0.1m/m (10%), ez biztosan helyes adat?");


	double (Channel::*pt2fun)(double, double, double) = NULL;

	is_simplified = false;
	is_switched = true;
	force_more_iter = false;
	vmean = 0.;

	double y_crit;
	vector<double> y_normal;

	if (debug_level > 0)
		Edge::SetLogFile();
}

//--------------------------------------------------------------
Channel::~Channel() {
}

/// Info
/**
 * @return info string
 */
string Channel::Info() {
	ostringstream strstrm;
	strstrm << Edge::Info();
	cout << setprecision(3);
	strstrm << endl << "       tipusa : " << tipus;
	strstrm << endl << "  kapcsolodas : " << cspe_nev << "(index:" << cspe_index
	        << ") ==> " << cspv_nev << "(index:" << cspv_index << ")";
	strstrm << endl << "       adatok : hossz [m]         : " << L;
	strstrm << endl << "                z eleje [m]       : " << ze;
	strstrm << endl << "                z vege [m]        : " << zv;
	strstrm << endl << "                lejtes [%]        : " << (lejtes * 100.);
	strstrm << endl << "                Hmax [m]          : " << Hmax;
	strstrm << endl << "                erdesseg [mm]             : " << erdesseg;
	surlodas();
	strstrm << endl << "                lambda [-]                : " << lambda;
	strstrm << endl << "                Manning surl.teny.(n) [-] : " << surl;
	strstrm << endl << "                geometria tipus           : ";
	switch (geo_tipus) {
	case 0:
		strstrm << B << "m szelessegu, " << Hmax
		        << "m magassagu teglalap keresztmetszet";
		break;
	case 1:
		strstrm << dia << "m atmeroju kor keresztmetszet";
		break;
	default:
		error("Info()", " Ismeretlen geometria tipus!");
	}
	strstrm << endl << "                numerikus megoldo beallitasai (db)     : "
	        << db;
	strstrm << endl << "                klor lebomlasi allando (cl_k)          : "
	        << cl_k;
	strstrm << endl << "                klor lebomlasi allando a falnal (cl_w) : "
	        << cl_w;
	strstrm << endl << "                debug szint                            : "
	        << debug_level << endl;
	strstrm << endl << "                logfile                                : "
	        << out_file << endl;


	if (debug_level > 0) {
		time_t ido = time(0);
		ostringstream strstrm1;
		strstrm1 << "Staci\nBME Hidrodinamikai Rendszerek Tanszek\n" << ctime(&ido)
		         << "\n";
		ofstream outfile(out_file.c_str(), ios::trunc);
		outfile << strstrm1.str();
		outfile.close();
		logfile_write(strstrm.str(), 1);
	}
	return strstrm.str();
}

/// �gegyenlet �s deriv�ltak sz�m�t�sa
/**
 * @param x[0] = pe/ro/g - nyom�s [v.o.m.] az �gelem elej�n
 * @param x[1] = pv/ro/g - nyom�s [v.o.m.] az �gelem v�g�n
 * @param x[2] = he/ro/g - �gelem elej�n a csom�pont nullszintje [m]
 * @param x[3] = hv/ro/g - �gelem v�g�n a csom�pont nullszintje [m]
 * @return f �rt�ke
 *
 * �gelem elej�n az abszol�t nyom�sszint: he(aknafen�k) + pe/ro/g = x[0]+x[1]
 * �gelem elej�n az abszol�t nyom�sszint: hv(aknafen�k) + pv/ro/g = x[0]+x[1]
 *
 * Az elj�r�s kisz�m�tja a deriv�ltakat is, �gy @see df csak visszaadja az �rt�ket.
 */

// double Channel::f(vector<double> x) {
//
// if (is_simplified)
// return f_simple(x);
// else
// return f_full(x);
// }

// double Channel::f_simple(vector<double> x) {
// pt2fun = &Channel::f_telt;
// double ere, tag1;
// double pe = x[0] * ro * g;
// double pv = x[1] * ro * g;
// double he = x[2];
// double hv = x[3];

// double A = dia*dia*3.1416/4.;
// double v = mp / ro / A;
// double headloss = lambda * L / dia * ro / 2. * v * fabs(v);
// tag1 = ro * g * (hv - he);

// ere = pv - pe + tag1 + headloss;
// f_count++;

// eset = "teltszelveny";

// return ere / ro / g;
// }

double Channel::f(vector<double> x) {
	pe = x[0] * ro * g;
	pv = x[1] * ro * g;
	he = x[2];
	hv = x[3];
	double ye = pe / ro / g + he - ze;
	double yv = pv / ro / g + hv - zv;
	ostringstream strstrm;

	headloss = fabs(ye - yv);

	strstrm << endl << endl << " evaluating f (#" << f_count << ")" << endl
	        << "-----------------------------------------";
	strstrm << scientific << showpos;
	// strstrm << endl << "  Data:"  << " pe=" << pe / ro / g << "m, ze   ="<< he << "m, pv=" << pv / ro / g << "m, zv   =" << hv << "m, mp=" << mp;
	strstrm << endl << " Data: ye=" << ye << "m, ye+ze=" << (ze + ye) << "m, yv=" << yv << ", yv+zv=" << (zv + yv) << "m, mp=" << mp << "kg/s";
	logfile_write(strstrm.str(), 1);

	jac.clear();
	num_eval_jac.clear();
	for (unsigned int ii = 0; ii < 3; ii++) {
		jac.push_back(0.0);
		num_eval_jac.push_back(true);
	}
	force_more_iter = false;

	//***************
	// SZAMITASOK
	//***************

	logfile_write("\n\n  * evaluating f...", 2);
	which_case(ye, yv);
	double ff = (*this.*pt2fun)(ye, yv, mp);

	// Jacobi elemeinek szamitasa
	double dx, df;
	double TINY_NUM = 1.e-4;

	// dfdye szamitasa:
	logfile_write("\n\n  * evaluating df/dye...", 2);
	if (num_eval_jac[0]) {
		double ye_old = ye;
		dx = -0.001 * ye;
		if (fabs(ye) < TINY_NUM)
			dx = TINY_NUM;
		// which_case(ye + dx, yv);
		df = (*this.*pt2fun)(ye + dx, yv, mp);
		jac[0] = (df - ff) / dx;
		ye = ye_old;
	}

	// dfdyv szamitasa:
	logfile_write("\n\n  * evaluating df/dyv...", 2);
	if (num_eval_jac[1]) {
		double yv_old = yv;
		dx = -0.001 * yv;
		if (fabs(yv) < TINY_NUM)
			dx = TINY_NUM;
		// which_case(ye, yv + dx);
		df = (*this.*pt2fun)(ye, yv + dx, mp);
		jac[1] = (df - ff) / dx;
		yv = yv_old;
	}

	// dfdmp szamitasa:
	logfile_write("\n\n  * evaluating df/dmp...", 2);
	if (num_eval_jac[2]) {
		double mdot_old = mp;
		dx = -0.001 * mp;
		if (fabs(mp) < TINY_NUM)
			dx = TINY_NUM;
		mp = mp + dx;
		which_case(ye, yv);
		df = (*this.*pt2fun)(ye, yv, mp);
		jac[2] = (df - ff) / dx;
		mp = mdot_old;
	}

	// konstans tag, csak linearizalas eseten van jelentosege
	jac.push_back(0.);

	f_count++;

	strstrm.str("");
	strstrm << "\n\n\n\t      f  = " << scientific << ff;
	strstrm << "\n\t df/dye  = " << jac[0];
	strstrm << "\n\t df/dyv  = " << jac[1];
	strstrm << "\n\t df/dmp  = " << jac[2];
	logfile_write(strstrm.str(), 1);


	// Some checking...
	if (isnan(ff)) {
		cout << endl << nev << ", eset:" << eset << ", ff=nan!" << endl;
		exit(-1);
	}
	for (int ii = 0; ii < 3; ii++)
		if (isnan(jac[ii])) {
			cout << endl << nev << ", eset:" << eset << ", jac[" << ii << "]=nan!" << endl;
			exit(-1);
		}

	if (isinf(ff)) {
		cout << endl << nev << ", eset:" << eset << ", ff=inf!" << endl;
		exit(-1);
	}
	for (int ii = 0; ii < 3; ii++)
		if (isinf(jac[ii])) {
			cout << endl << nev << ", eset:" << eset << ", jac[" << ii << "]=inf!" << endl;
			exit(-1);
		}

	double jac_sum, jac_sum_tol = 1e-5;

	jac_sum = fabs(jac[0]) + fabs(jac[1]);
	if (jac_sum < jac_sum_tol) {
		cout << endl << nev << ", eset:" << eset << ", jac[0]+jac[1]<" << jac_sum_tol << " !" << endl;
		exit(-1);
	}

	jac_sum = fabs(jac[1]) + fabs(jac[2]);
	if (jac_sum < jac_sum_tol) {
		cout << endl << nev << ", eset:" << eset << ", jac[1]+jac[2]<" << jac_sum_tol << " !" << endl;
		exit(-1);
	}

	jac_sum = fabs(jac[0]) + fabs(jac[2]);
	if (jac_sum < jac_sum_tol) {
		cout << endl << nev << ", eset:" << eset << ", jac[0]+jac[2]<" << jac_sum_tol << " !" << endl;
		exit(-1);
	}

	return ff;
}


void Channel::which_case(const double ye, const double yv) {
	ostringstream strstrm;

	strstrm.str("");
	if (fabs(mp) > 0.0001) {
		if ((ye > dia) && (yv > dia)) {
			// telt eset
			eset = "teltszelveny";
			strstrm << "\n\n\t " << eset << ": ye=" << ye << "m > D and yv=" << yv << "m >D (D=" << dia << "m)";
			pt2fun = &Channel::f_telt;
		}
		else {
			if (ye < 0.0) {
				if (yv + zv < ze) {
					// 0.a. eset
					eset = "0.a.";
					strstrm << "\n\n\t case " << eset << " (empty channel):  ye=" << ye << "m <0 and yv=" << yv << "m < ze=" << ze << "m, yv+zv<ze";
					pt2fun = &Channel::f_0;
				} else {
					if (mp > 0) {
						// 0.b.i. eset
						eset = "0.b.i.";
						strstrm << "\n\n\t case " << eset << ":  ye=" << ye << "m <0, yv=" << yv << "m >0, mp=" << mp
						        << " kg/s >0, yv+zv>ze (should be backflow)";

						pt2fun = &Channel::f_0;
					} else {
						// 0.b.ii. eset
						eset = "0.b.ii.";
						strstrm << "\n\n\t case " << eset << ":  ye=" << ye << "m <0, yv=" << yv << "m >0, mp=" << mp
						        << " kg/s <0, yv+zv>ze (backflow)";

						pt2fun = &Channel::f_1;
						//eval_jac[0]= false;
					}
				}
			} else {
				if (mp < 0) {
					// 1. eset
					eset = "1.";
					strstrm << "\n\n\t case " << eset << ":  ye=" << ye << "m >0, mp=" << mp << " kg/s <0";

					pt2fun = &Channel::f_1;
				} else {
					vector<double> yn = normal_szint(mp / ro);
					double yc = kritikus_szint(mp / ro);

					switch (yn.size()) {
					case 1:
						if (yn[0] > yc) {
							// 2.a.i. eset
							eset = "2.a.i.";
							strstrm << "\n\n\t case " << eset << ":  ye=" << ye << " m >0, mp=" << mp
							        << " kg/s >0, yn>yc";
							strstrm << "\n\t\t(1 db normalszint: yn=" << yn[0] << " m > yc=" << yc << " m)";

							pt2fun = &Channel::f_2ai;
						} else {
							// 2.a.ii. eset
							eset = "2.a.ii.";
							strstrm << "\n\n\t case " << eset << ":  ye=" << ye << " m >0, mp=" << mp
							        << " kg/s >0, yn<yc";
							strstrm << "\n\t\t(1 db normalszint: yn=" << yn[0] << " m < yc=" << yc << " m)";

							pt2fun = &Channel::f_2aii;
						}
						break;

					case 2:

						double yn1, yn2;
						// yn1<yn2, rendez�s sz�ks�ges lehet.
						if (yn[0] > yn[1]) {
							yn1 = yn[1];
							yn2 = yn[0];
						} else {
							yn1 = yn[0];
							yn2 = yn[1];
						}

						if (yc < yn1) {
							eset = "2.b.i.";
							strstrm << "\n\n\t case " << eset << ":  ye=" << ye << "m >0, mp=" << mp
							        << "kg/s >0, yc<yn1";
							strstrm << "\n\t\t(2 db normalszint: yc=" << yc << "m < yn1=" << yn1 << "m, yn2=" << yn2
							        << "m)";

							pt2fun = &Channel::f_2c;
						} else {
							if (yc < yn2) {
								eset = "2.b.ii.";
								strstrm << "\n\n\t case " << eset << ":  ye=" << ye << "m >0, mp=" << mp
								        << "kg/s >0, yn1<yc<yn2";
								strstrm << endl << "\n\t\t(2 db normalszint: yn1=" << yn1 << "m < yc=" << yc
								        << "m < yn2=" << yn2 << "m";

								pt2fun = &Channel::f_2aii;
							} else {
								eset = "2.b.iii.";
								strstrm << "\n\n\t case " << eset << ":  ye=" << ye << "m >0, mp=" << mp
								        << "kg/s >0, yn2<yc";
								strstrm << "\n\t\t(2 db normalszint: yn1=" << yn1 << "m < yn2=" << yn2 << "m < yc="
								        << yc << "m";

								//pt2fun = &Channel::f_2c;
								pt2fun = &Channel::f_2aii;
							}
						}
						break;

					case 0:
						eset = "2.c.";
						strstrm << "\n\n\t case " << eset << ":  ye=" << ye << "m >0, mp=" << mp << "kg/s >0, yn nincs";
						strstrm << "\n\t\t(0 db normalszint, yc=" << yc << "m)";

						pt2fun = &Channel::f_2c;

						break;

					default:
						strstrm.str("Bajjjj van: yn.size()=");
						strstrm << yn.size();
						error("f()", strstrm.str());
					}
				}
			}
		}
	}
	else {
		eset = "0.a.";
		strstrm << "\n\n\t case " << eset << ":  ye=" << ye << "m >0, mp=" << mp << "kg";
		strstrm << "\n\t\t(all a viz a Channelban)";

		pt2fun = &Channel::f_0;
	}
	logfile_write(strstrm.str(), 1);
}

/// �gegyenlet deriv�ltjai (sz�m�t�s: @see f)
/**
 * @return df �rt�ke
//  */
vector<double> Channel::df(vector<double> x) {

	return jac;
}

//--------------------------------------------------------------
// vector<double> Channel::df_simple(vector<double> x) {
// 	// double pe=x[0]*ro*g;
// 	// double pv=x[1]*ro*g;
// 	double he = x[2];
// 	double hv = x[3];
// 	vector<double> ere;
// 	ere.push_back(-ro * g);
// 	ere.push_back(+ro * g);
// 	double A = dia * dia * 3.1416 / 4.;
// 	double der = lambda * L / pow(dia, 5) * 8 / ro / pow(3.1416, 2.) * 2. *
// 	             abs(mp);  // Pa/(kg/s)
// 	ere.push_back(der);
// 	ere.push_back(ro * g * (hv - he));

// 	for (unsigned int i = 0; i < ere.size(); i++) ere.at(i) /= ro * g;
// 	return ere;
// }


/// �gelem inicializ�l�sa
/**
 * @param mode - (0) automatikus, (1) mp=value
 * @param value csak mode=1 eset�n
 */

void Channel::Ini(int mode, double value) {
	if (mode == 0)
		mp = 1.e-3;
	else
		mp = value;
}

/// Keresztmetszeti jellemz�k sz�m�t�sa
/**
 * @param y v�zszint
 * @param &A nedves�tett ter�let
 * @param &B nedves�tett ker�let
 * @param &Rh hidraulikai sug�r (A/B)
 */

void Channel::keresztmetszet(const double yy, double & A, double & B, double & Rh) {
	double K = 0.0;
	double y = yy;
	if (y < 0) {
		//ostringstream msg;
		//msg.str("");
		//msg<<"Negat�v szint, y="<<y;
		//warning("keresztmetszet()", msg.str());
		y = dia / 1000.;
	} else {
		switch (geo_tipus) {
		case 0:   // teglalap
		{
			A = B * y;
			K = B + 2 * y;
			break;
		}
		case 1:   // kor
		{
			if (y < dia) {
				double teta, R = dia / 2;
				teta = acos(1 - y / R);
				K = 2 * R * teta;
				A = dia * dia / 4. * (teta - sin(teta) * cos(teta));
				B = dia * sin(teta);
			} else {
				double h = dia / 100;
				A = dia * dia * pi / 4.;
				B = h;
				K = dia * pi;
			}
			break;
		}
		default:
			ostringstream strstrm;
			strstrm.str("");
			strstrm << "Ismeretlen geometria tipus:" << geo_tipus;
			error("keresztmetszet()", strstrm.str());
		}

		Rh = A / K;
	}
}

/// Kritikus szint sz�m�t�sa k�r keresztmetszet eset�n
/**
 * yc(0) -> -inf, yc(D)->1, �gy a kett� k�z�tt pontosan egy gy�khely van.
 * Megold�s: intervallumfelez�ssel
 *
 * @param Q t�rfogat�ram
 * @return kritikus szint
 */

double Channel::kritikus_szint(const double Q) {
	double fun = 1.0, yj = dia, yb = dia, A, B, Rh, Q2 = Q * Q;
	ostringstream strstrm;

	strstrm << scientific << showpos;
	strstrm << "\n\n\tKritikus szint szamitasa: Q=" << Q << " m3/s";

	// Cs�kken� yb mellett keres�nk egy negat�v �rt�ket
	unsigned int iter = 0;
	strstrm << "\n\t\tEl�sz�m�t�s: csokkeno yb menten negativ f ertek keresese:";
	while (fun > 0) {
		yb /= 2.;
		keresztmetszet(yb, A, B, Rh);
		fun = 1. - Q2 * B / (A * A * A) / g;
		iter++;

		strstrm << "\n\t\t\t iter=" << iter << ", y=" << yb << " m, f=" << fun;
		if (iter > iter_max) {
			logfile_write(strstrm.str(), 0);
			error("kritikus_szint()", "HIBA (1) - Nem tal�lok negat�v �rt�ket!");
		}
	}
	logfile_write(strstrm.str(), 4);

	// H�rm�dszer
	fun = 1e10;
	iter = 0;
	double yk = 0.0;

	strstrm.str("");
	strstrm << "\n\n\t\tSz�m�t�s:";

	keresztmetszet(yb, A, B, Rh);
	double fb = 1. - Q2 * B / (A * A * A) / g;
	keresztmetszet(yj, A, B, Rh);
	double fj = 1. - Q2 * B / (A * A * A) / g;

	while ((fabs(fun) > 1.e-10) && (fabs(yj - yb) > 1.e-10)) {
		strstrm << "\n\t\t\titer=" << iter << ": yb=" << yb << "(fb=" << fb << "),  yj=" << yj
		        << "(fj=" << fj << ")";

		yk = (yj + yb) / 2;
		//yk=yb-fb*(yj-yb)/(fj-fb);

		keresztmetszet(yk, A, B, Rh);
		fun = 1. - Q2 * B / (A * A * A) / g;

		if (fun > 0) {
			yj = yk;
			fj = fun;
		} else {
			yb = yk;
			fb = fun;
		}

		iter++;

		if (iter > iter_max) {
			logfile_write(strstrm.str(), 0);
			error("kritkus_szint()", "HIBA (2) - T�l sok iter�ci�!");
		}
	}
	logfile_write(strstrm.str(), 4);

	strstrm.str("");
	strstrm << "\n\n\t\tKritikus szint: yc = " << yk << " m";
	logfile_write(strstrm.str(), 3);

	return yk;
}

/// Norm�lszint sz�m�t�sa k�r keresztmetszet eset�n
/**
 * yn(0) -> -inf
 * ha Q<Q_D -> 1db gy�khely
 * ha Q_D<Q<Q_cs -> 2db gy�khely
 * ha Q>Q_cs -> 0db gy�khely
 * Megold�s: h�rm�dszer
 *
 * @param Q t�rfogat�ram
 * @return norm�lszint(ek)
 */

vector<double> Channel::normal_szint(const double Q) {
	vector<double> yn;

	double y_cs = 0.938181 * dia;
	double A, B, Rh, Q2 = Q * Q;

	keresztmetszet(y_cs, A, B, Rh);
	double C = pow(Rh, 1.0 / 6.0) / surlodas();
	double Q_cs = sqrt(lejtes * A * A * C * C * Rh);

	keresztmetszet(dia, A, B, Rh);
	C = pow(Rh, 1.0 / 6.0) / surlodas();
	double Q_D = sqrt(lejtes * A * A * C * C * Rh);

	ostringstream strstrm;
	strstrm << scientific << showpos;
	strstrm << "\n\n\tNormalszint szamitasa: Q=" << Q << " m3/s, Q_cs=" << Q_cs
	        << "m3/s, Q_D=" << Q_D << "m3/s";
	logfile_write(strstrm.str(), 4);

	if (Q < Q_cs) {
		strstrm.str("\n\n\t\t1. normalszint szamitasa:");
		unsigned int iter = 0;
		double fun = 1.e10;
		double y = dia / 2;
		while (fun > 0) {
			keresztmetszet(y, A, B, Rh);
			C = pow(Rh, 1.0 / 6.0) / surlodas();
			fun = lejtes - Q2 / A / A / C / C / Rh;
			strstrm << "\n\t\t -> " << " y=" << y << ", f=" << fun;

			y /= 2.;
			iter++;
			if (iter > iter_max) {
				logfile_write(strstrm.str(), 0);
				error("normal_szint()",
				      "HIBA (1) - Nem talalok negativ erteket!");
			}
		}
		logfile_write(strstrm.str(), 4);

		// 1. norm�lszint sz�m�t�sa: 0<y<y_cs
		// H�rm�dszer
		fun = 1e10;
		iter = 0;
		double yk = 0.0;

		strstrm.str("\n\n\t\tSzamitas:");

		double yb = y, yj = y_cs;
		keresztmetszet(yb, A, B, Rh);
		C = pow(Rh, 1.0 / 6.0) / surlodas();
		double fb = lejtes - Q2 / A / A / C / C / Rh;
		keresztmetszet(yj, A, B, Rh);
		C = pow(Rh, 1.0 / 6.0) / surlodas();
		double fj = lejtes - Q2 / A / A / C / C / Rh;

		while ((fabs(fun) > 1e-10) && fabs(yb - yj) > 1e-10) {
			//yk=yb-fb*(yj-yb)/(fj-fb);
			yk = (yb + yj) / 2.;

			keresztmetszet(yk, A, B, Rh);
			C = pow(Rh, 1.0 / 6.0) / surlodas();
			fun = lejtes - Q2 / A / A / C / C / Rh;

			strstrm << "\n\t\t\titer=" << iter << ": yb=" << yb << "(fb=" << fb << ")  yk="
			        << yk << "(fk=" << fun << "), yj=" << yj << "(fj=" << fj << ")";
			//cout<<"\n\t\t\titer="<<iter<<": yb="<<yb<<"(fb="<<fb<<")  yk=" <<yk <<"(fk="<<fun <<"), yj="<<yj<<"(fj="<<fj<<")";;
			if (fun > 0) {
				yj = yk;
				fj = fun;
			} else {
				yb = yk;
				fb = fun;
			}
			iter++;

			if (iter > iter_max) {
				logfile_write(strstrm.str(), 0);
				error("normal_szint()",
				      "HIBA (2) - T�l sok iter�ci�, 1. norm�lszint sz�m�t�sa!");
			}
		}
		yn.push_back(yk);
		logfile_write(strstrm.str(), 4);

		if (Q > Q_D) {
			// 2. norm�lszint sz�m�t�sa: 0<y<y_cs
			strstrm.str("");
			strstrm << "\n\n\t2. norm�lszint sz�m�t�sa:";

			double yb = y_cs, yj = dia;
			keresztmetszet(yb, A, B, Rh);
			C = pow(Rh, 1.0 / 6.0) / surlodas();
			double fb = lejtes - Q2 / A / A / C / C / Rh;
			keresztmetszet(yj, A, B, Rh);
			C = pow(Rh, 1.0 / 6.0) / surlodas();
			double fj = lejtes - Q2 / A / A / C / C / Rh;

			double fun = 1e10, yk;
			unsigned int iter = 0;
			while ((fabs(fun) > 1e-10) && (fabs(yb - yj) > 1e-10)) {
				//yk=yb-fb*(yj-yb)/(fj-fb);
				yk = (yb + yj) / 2.;
				keresztmetszet(yk, A, B, Rh);
				C = pow(Rh, 1.0 / 6.0) / surlodas();
				fun = lejtes - Q2 / A / A / C / C / Rh;

				strstrm << "\n\t\t\titer=" << iter << ": yb=" << yb << "(fb=" << fb
				        << ")  yk=" << yk << "(fk=" << fun << "), yj=" << yj << "(fj="
				        << fj << ")  |yb-yj|=" << fabs(yb - yj);
				if (fun > 0) {
					yj = yk;
					fj = fun;
				} else {
					yb = yk;
					fb = fun;
				}
				iter++;

				if (iter > iter_max) {
					logfile_write(strstrm.str(), 0);
					error("normal_szint()",
					      "HIBA (3) - T�l sok iter�ci� h�rm�dszer k�zben, 2. norm�lszint sz�m�t�sa!");
				}
				//cout<<endl<<iter<<": yk="<<yk<<"  |yb-yj|="<<fabs(yb-yj)<<"  fun="<<fabs(fun);

			}
			yn.push_back(yk);
			logfile_write(strstrm.str(), 4);
		}
	}

	strstrm.str("");
	strstrm << "\n\n\t\tNormalszint(ek):";
	switch (yn.size()) {
	case 0:
		strstrm << " Nincs";
		logfile_write(strstrm.str(), 3);
		break;
	case 1:
		strstrm << " yn1 = " << yn[0] << " m";
		logfile_write(strstrm.str(), 3);
		break;
	case 2:
		strstrm << " yn1 = " << yn[0] << " m";
		strstrm << " yn2 = " << yn[1] << " m";
		logfile_write(strstrm.str(), 3);
		break;
	default:
		strstrm.str("");
		strstrm << " yn.size()=" << yn.size() << " ???";
		error("normal_szint", strstrm.str());
	}

	return yn;
}

/// Folyad�kfelsz�nt le�r� dy/dx=f(x,y) KDE jobboldala
/**
 * @param x x koordin�ta
 * @param y y koordin�ta
 * @return dy/dx �rt�ke
 */

double Channel::nyf_ode(const double x, const double y, const double mp) {
	double Q, A, B, C, Rh, ere = 0.0;
	Q = mp / ro;

	if (y < dia) {
		keresztmetszet(y, A, B, Rh);
		C = pow(Rh, 1. / 6.) / surlodas();
		ere = (lejtes - fabs(Q) * Q / A / A / C / C / Rh) / (1 - Q * Q / A / A / A / g * B);
	} else {
		A = dia * dia * pi / 4.;
		Rh = dia / 4.0;
		C = pow(Rh, 1. / 6.) / surlodas();
		ere = lejtes - fabs(Q) * Q / A / A / C / C / Rh;
	}

	return ere;
}

/// ODE solver
/**
 * @param y0  depth at x=0
 * @param dx0 initial dx step
 * @param x0  initial x value (defines direction of integration)
 * @return y result of integration
 */

double Channel::ode_megoldo(double y0, double dx0, double x0, double mp) {
	ostringstream strstrm;
	double A, B, Rh, y = y0, x = x0, dx = dx0, sumdx = 0, yy = y0;
	int i = 0;

	// clear vectors and save initial point
	if (write_res) {
		xres.clear();
		yf.clear();
		yres.clear();
		vres.clear();
		xres.push_back(x);
		if (dx0 > 0)
			yf.push_back(ze - lejtes * x);
		else
			yf.push_back(zv + lejtes * (L - x));
		yres.push_back(yy);

		keresztmetszet(yy, A, B, Rh);
		vres.push_back(mp / ro / A);
	}

	strstrm.str("");
	strstrm << endl << endl << "\t\tSolving open-surface DE in channel " << nev << " ...\n";
	// strstrm << scientific << setprecision(4) << showpos;
	// strstrm << "\t\t x=" << x << " y=" << y << " (mp=" << mp << " kg/s)";
	logfile_write(strstrm.str(), 2);

	double c2 = 1. / 4., a21 = 1. / 4.;
	double c3 = 3. / 8., a31 = 3. / 32., a32 = 9. / 32.;
	double c4 = 12. / 13., a41 = 1932. / 2197., a42 = -7200. / 2197., a43 = 7296. / 2197.;
	double c5 = 1., a51 = 439. / 216., a52 = -8., a53 = 3680. / 513., a54 = -845. / 4104;
	double c6 = 1. / 2., a61 = -8. / 27., a62 = 2., a63 = -3544. / 2565., a64 = 1859. / 4104., a65 = -11. / 40.;
	double b1 = 16. / 135., b2 = 0., b3 = 6656. / 12825., b4 = 28561. / 56430., b5 = -9. / 50., b6 = 2. / 55.;
	double d1 = 25. / 216., d2 = 0., d3 = 1408. / 2565., d4 = 2197. / 4104., d5 = -1. / 5., d6 = 0.;
	double k1, k2, k3, k4, k5, k6;

	bool euler_integ = false;
	bool last_step = false;
	double hiba_max = 1e-7, dx_min = 1.e-9,  dxuj, dx_max = L / 10.;

	strstrm.str("");
	strstrm << endl << "\t\t x=" << scientific << setprecision(4) << showpos << x
	        << " dx=" << dx << " y=" << y << ", hiba=" << 0.0 << ", hiba_max=" << hiba_max;
	logfile_write(strstrm.str(), 2);

	double hiba = 2.0 * hiba_max;
	while (!last_step) {
		strstrm.str("");
		double y1, y2, f_at_y, f_at_y2;
		hiba = 2.0 * hiba_max;

		while (hiba > hiba_max) {
			// Is this the last step?
			if (x + dx > L) {
				dx = L - x;
				last_step = true;
			}
			if (x + dx < 0) {
				dx = -x;
				last_step = true;
			}

			if (euler_integ) {
				// one full step...        .
				f_at_y = nyf_ode(x, y, mp);
				y1 = y + dx * f_at_y;

				// two falf steps...
				y2 = y + dx / 2 * f_at_y;
				f_at_y2 = nyf_ode(x + dx / 2, y2, mp);
				y2 = y2 + dx / 2 * f_at_y2;
			}
			else {
				// RK45
				k1 = nyf_ode(x, y, mp);
				k2 = nyf_ode(x + c2 * dx, y + a21 * dx * k1, mp);
				k3 = nyf_ode(x + c3 * dx, y + a31 * dx * k1 + a32 * dx * k2, mp);
				k4 = nyf_ode(x + c4 * dx, y + a41 * dx * k1 + a42 * dx * k2 + a43 * dx * k3, mp);
				k5 = nyf_ode(x + c5 * dx, y + a51 * dx * k1 + a52 * dx * k2 + a53 * dx * k3 + a54 * dx * k4, mp);
				k6 = nyf_ode(x + c6 * dx, y + a61 * dx * k1 + a62 * dx * k2 + a63 * dx * k3 + a64 * dx * k4 + a65 * dx * k5, mp);
				y1 = y + dx * (b1 * k1 + b2 * k2 + b3 * k3 + b4 * k4 + b5 * k5 + b6 * k6);
				y2 = y + dx * (d1 * k1 + d2 * k2 + d3 * k3 + d4 * k4 + d5 * k5 + d6 * k6);
			}

			// compare results
			hiba = fabs(y1 - y2);

			// Info
			strstrm.str("");
			strstrm << endl << "\t\t x=" << scientific << setprecision(4) << showpos << x
			        << " dx=" << dx << " y=" << y << ", hiba=" << hiba << ", hiba_max=" << hiba_max;
			logfile_write(strstrm.str(), 4);
			strstrm.str("");

			// Lepes beallitasa
			double TINY_WATER = 0.001 / 100.;
			if ((hiba > hiba_max) || (y2 < 0.)) {
				dxuj = dx / 2.;
				logfile_write(" dx -> dx/2", 4);
				if (fabs(dxuj) < dx_min) {
					last_step = true;

					strstrm << endl << "!!! Feladom, a " << nev
					        << " Channelban az eloirt dx_min=" << dx_min
					        << " lepeskozzel sem tudom elerni a megadott "
					        << hiba_max << " hibahatart!!! \nVizszintek: y0=" << y0 << ", y1=" << y1 << "m, tomegaram: " << mp << " kg/s" << endl;
					// cout << strstrm.str();
					error("ode_megoldo()", strstrm.str());
				}
				// if (y2 < TINY_WATER) {
				// 	last_step = true;
				// 	strstrm << endl << "!!! A " << nev
				// 	        << " Channelban y=" << y2 << "m, tomegaram: " << mp << " kg/s" << endl;
				// 	error("ode_megoldo()", strstrm.str());
				// }
			}
			else {
				strstrm.str("");
				strstrm << endl << "\t\t x=" << scientific << setprecision(4) << showpos << x
				        << " dx=" << dx << " y=" << y2 << ", f=" << nyf_ode(x + dx, y2, mp) << ", hiba=" << hiba << ", hiba_max=" << hiba_max;
				logfile_write(strstrm.str(), 3);

				logfile_write(" OK", 4);
				if (hiba < hiba_max / 10.) {
					dxuj = dx * 2.;
					if (fabs(dxuj) > dx_max) {
						if (dxuj > 0)
							dxuj = dx_max;
						else
							dxuj = -dx_max;
					}
					logfile_write(" dx -> dx*2", 4);
				} else
					dxuj = dx;

				// Lepes lezarasa
				y = y2;
				x += dx;
				sumdx += fabs(dx);
				i++;
			}
			dx = dxuj;
		}

		// Collect results
		if (write_res) {
			xres.push_back(x);
			if (dx0 > 0)
				yf.push_back(ze - lejtes * x);
			else
				yf.push_back(zv + lejtes * (L - x));
			yres.push_back(y);
			keresztmetszet(y, A, B, Rh);
			vres.push_back(mp / ro / A);
		}
		logfile_write(strstrm.str(), 4);
	}
	strstrm.str("");
	strstrm << endl << "\t\t x=" << scientific << setprecision(4) << showpos << x
	        << " dx=" << dx << " y=" << y << ", hiba=" << hiba << ", hiba_max=" << hiba_max;
	logfile_write(strstrm.str(), 2);

	return y;
}

/// Logfile-ba �r�s
/**
 * @param msg �zenet
 * @param msg_debug_level csak akkor �r�dik be msg a nev.out f�jlba, ha msg_debug_level>=debug_level
 */

/*void Channel::logfile_write(string msg, int msg_debug_level) {
    if (debug_level>=msg_debug_level) {
        ofstream outfile(df_name.c_str(), ios::app);
        outfile<<msg;
        outfile.close();
    }
}*/

/// Eredm�nyvektorok �p�t�se
/**
 * xres: x koordinata
 * yf  : fenek y koordinata
 * yres: nyomasvonal y koordinata
 * Hres: vizszint y koordinata
 * vres: sebesseg
 * !!! Folyasiranyban kell rajzolni!
 */

void Channel::build_res() {

// Throw a warning if not switched back from simplified to proper model.
	ostringstream strstrm;
	if (is_switched == false) {
		strstrm << endl << "WARNING!!!! " << nev << " did not switch back from simplified to proper model!";
		logfile_write(strstrm.str(), 1);
		cout << strstrm.str();
	}

	write_res = true;
	res_ready = true;
	double ye = pe / ro / g + he - ze;
	double yv = pv / ro / g + hv - zv;

	xres.clear();
	yf.clear();
	yres.clear();
	Hres.clear();
	vres.clear();

	// Empty channel hadnled separately
	bool megvolt = false;
	if (!strcmp(eset.c_str(), "teltszelveny")) {
		megvolt = true;
		xres.push_back(0);
		xres.push_back(L);
		yf.push_back(ze);
		yf.push_back(zv);
		yres.push_back(ye);
		yres.push_back(yv);
	}

	if (!strcmp(eset.c_str(), "0.a.") || !strcmp(eset.c_str(), "0.b.i.")) {
		// f_0
		megvolt = true;
		xres.push_back(0);
		xres.push_back(L);
		yf.push_back(ze);
		yf.push_back(zv);
		yres.push_back(ye);
		yres.push_back(yv);
	}

	if (!strcmp(eset.c_str(), "0.b.ii.") ) {
		// f_1
		megvolt = true;
		double ff = f_1(ye, yv, mp);
		// Az eredmenyvektorokat folyasiranyban rajzoljuk fel!
		is_reversed = false; // 2016.12.06.: False volt!
	}

	if (!strcmp(eset.c_str(), "1.")) {
		// f_1
		megvolt = true;
		double ff = f_1(ye, yv, mp);
		// Az eredmenyvektorokat folyasiranyban rajzoljuk fel!
		is_reversed = false;
	}

	if (!strcmp(eset.c_str(), "2.a.i.")) {
		// f_2ai
		megvolt = true;
		vector<double> yn = normal_szint(mp / ro);
		xres.push_back(0);
		xres.push_back(L);
		yf.push_back(ze);
		yf.push_back(zv);
		yres.push_back(yn.at(0));
		yres.push_back(yn.at(0));
	}

	if (!strcmp(eset.c_str(), "2.a.ii.") || !strcmp(eset.c_str(), "2.b.ii.")) {
		// f_2aii
		megvolt = true;
		double ff = f_2aii(ye, yv, mp);
	}

	if (!strcmp(eset.c_str(), "2.b.i.") || !strcmp(eset.c_str(), "2.b.iiii.") || !strcmp(eset.c_str(), "2.c.")) {
		// f_2c
		megvolt = true;
		double ff = f_2c(ye, yv, mp);
	}

	if (!megvolt) {
		string msg = "Ismeretlen eset: " + eset;
		error("build_res()", msg);
	}

	// Hmax korrekcio
	for (unsigned int i = 0; i < yres.size(); i++) {
		// Ha a Laci majd jol rajzolja ki, vissza kell rakni az offset-et 0-ra.
		double offset = yf.at(i);
		if (yres.at(i) > dia)
			Hres.push_back(offset + dia);
		else if (yres.at(i) < 0)
			Hres.push_back(offset + 0.0);
		else
			Hres.push_back(offset + yres.at(i));
	}

	// Az eredm�nyek konzisztenci�ja miatt mindig utolag szamitjuk yres-bol.
	double A, B, Rh;
	vmean = 0.;
	for (unsigned int i = 0; i < yres.size(); i++) {
		keresztmetszet(Hres.at(i), A, B, Rh);
		vres.push_back(mp / ro / A);
		vmean += mp / ro / A;
	}
	vmean /= yres.size();

	// Ha meg volt forditva, vissza kell forgatni a cuccost.
	if (is_reversed) {
		vector<double> tmp_xres(xres.size()), tmp_yf(xres.size()),
		       tmp_yres(xres.size()), tmp_Hres(xres.size()),
		       tmp_vres(xres.size());
		int max = xres.size();
		for (unsigned int i = 0; i < xres.size(); i++) {
			tmp_xres.at(i) = L - xres.at(max - i - 1);
			tmp_yf.at(i) = yf.at(max - i - 1);
			tmp_yres.at(i) = yres.at(max - i - 1);
			tmp_Hres.at(i) = Hres.at(max - i - 1);
			tmp_vres.at(i) = vres.at(max - i - 1);
		}
		xres.clear();
		yf.clear();
		yres.clear();
		Hres.clear();
		vres.clear();

		xres = tmp_xres;
		yf = tmp_yf;
		yres = tmp_yres;
		Hres = tmp_Hres;
		vres = tmp_vres;
	}

	// Info
	strstrm.str("");
	strstrm << "\n\nBuilding solution vectors...";
	strstrm << "\n---------------------------------";
	strstrm << "\n\t ye+ze = " << (pe / ro / g + he) << "m, ye=" << ye << "m";
	strstrm << "\n\t yv+zv = " << (pv / ro / g + hv) << "m, yv=" << yv << "m";
	strstrm << "\n\t mp    = " << mp << "kg/s";
	strstrm << "\n\t case  : " << eset;
	strstrm << "\n\t length of solution vectors : " << yres.size() << "\n";

	strstrm << "\n Solution vectors:\n";
	strstrm << "\n\t  i    x      \t   yf     \t  yf+D   \t   y   \t     p (mwc) \t  yf+p    \t   v    \t     A";
	strstrm << scientific << setprecision(3);
	double sumh;

	double AA, BB, RRh;
	for (unsigned int i = 0; i < yres.size(); i++) {
		keresztmetszet(Hres.at(i), AA, BB, RRh);

		// !!!!!!!!!!!!!!!!!!
		// At kell allitani Aref-et, mert kulonben a sebesseg a teljes keresztmetszettel lesz szamolva es az hulyeseg.
		if (i == 0)
			Set_Aref(AA);
		// !!!!!!!!!!!!!!!!!!

		sumh = yf.at(i) + yres.at(i);
		strstrm << "\n\t  " << i << "\t" << xres.at(i) << "\t" << yf.at(i) << "\t";
		strstrm << (yf.at(i) + dia) << "\t" << Hres.at(i) << "\t";
		strstrm << yres.at(i) << "\t" << sumh << "\t" << vres.at(i) << "\t" << AA;
	}
	logfile_write(strstrm.str(), 2);
}

//--------------------------------------------------------------
vector<double> Channel::Get_res(string mit) {
	vector<double> empty;

	if (!res_ready)
		build_res();

	if (mit == "xf")
		return xres;
	else if (mit == "x")
		return xres;
	else if (mit == "yf")
		return yf;
	else if (mit == "y")
		return Hres;
	else if (mit == "p")
		return yres;
	else if (mit == "v")
		return vres;
	else {
		ostringstream strstrm;
		strstrm.str("");
		strstrm << "Nincs ilyen valtozo: " << mit;
		error("Get_res()", strstrm.str());
		return empty;
	}
}

//--------------------------------------------------------------
double Channel::surlodas() {
	// surl: Manning-allando
	if (erdesseg <= 0) {
		lambda = -erdesseg;
		surl = -erdesseg;
	} else {
		if (f_count >= 0) {
			Hmax = dia;
			double ize = 2.0 * log(14.8 * (Hmax / 2) / (erdesseg / 1000));
			lambda = 1 / ize / ize;
		} else
			lambda = 0.02;
		surl = pow(Hmax / 2., 1. / 6.) * sqrt(lambda / 8.0 / g);
	}
	//cout<<endl<<nev<<": surl="<<surl<<" (lambda="<<lambda<<")";

	return surl;
}

//--------------------------------------------------------------
double Channel::Get_dprop(string mit) {
	double out = 0.0;
	if (mit == "Aref")
		out = Aref;
	else if (mit == "lambda")
		out = lambda;
	else if (mit == "mass_flow_rate")
		out = mp;
	else if ((mit == "L") || (mit == "length"))
		out = L;
	else if ((mit == "diameter") || (mit == "D"))
		out = dia;
	else if (mit == "Rh") {
		double A, B, Rh;
		keresztmetszet(Hmax, A, B, Rh);
		out = Rh;
	} else if (mit == "cl_k")
		out = cl_k;
	else if (mit == "cl_w")
		out = cl_w;
	else if (mit == "headloss")
		out = headloss;
	else if (mit == "headloss_per_unit_length")
		out = headloss / L;
	else if (mit == "ze")
		out = ze;
	else if (mit == "zv")
		out = zv;
	else if (mit == "lejtes")
		out = lejtes;
	else if ((mit == "concentration") || (mit == "konc_atlag"))
		out = konc_atlag;
	else {
		ostringstream msg;
		msg.str("");
		msg << "Ismeretlen bemenet: mit=" << mit;
		error("Get_dprop()", msg.str());
	}
	return out;
}

//--------------------------------------------------------------
double Channel::Get_FolyTerf() {
	double A, B, Rh, szum = 0;
	if (eset == "-2") {
		szum = 0;
	} else {
		// cout<<"\n Hres.size()="<<Hres.size()<<endl;
		// cout<<"\n xres.size()="<<xres.size()<<endl;
		// cout<<"\n xere.size()="<<xere.size()<<endl;
		for (unsigned int i = 1; i < Hres.size(); i++) {
			keresztmetszet(Hres.at(i), A, B, Rh);
			szum += A * fabs(xres.at(i) - xres.at(i - 1));
			// cout<<endl<<"\t\t i="<<i<<", x(i)="<<xres.at(i)<<", x(i-1)="<<xres.at(i
			// -1) <<", dV="<<A*fabs(xres.at(i)-xres.at(i-1));
		}
	}
	// cout<<endl<<nev<<"\t V= "<<szum<<" m^3";
	FolyTerf = szum;
	return FolyTerf;
}

/// �gegyenlet (0.a.) ye<0, yv<0 �s (0.b.i.) ye<0, yv>0, Q>0 esetben
/**
 * D/100 �tm�r�j� sz�v�sz�lon teltszelv�ny� �raml�s ye-yv nyom�sk�l�nbs�g hat�s�ra
 * @param ye eleje v�zszint
 * @param yv v�ge v�zszint
 * @param mp t�meg�ram
 * @return �gegyenlet �rt�ke
 */
double Channel::f_telt(double ye, double yv, double mp) {
	// Matlab kod:
	//  Df=D/100;
	//  Af=Df^2*pi/4;
	//  ff=yea-yva-0.02*L/Df/2/9.81*Q*abs(Q)/Af^2;
	//  eval_jac=[1 1 1];

	ostringstream strstrm;

	// D_fake=dia;
	double v = mp / ro / (D_fake * D_fake * pi / 4.);
	lambda = surlodas();
	double dh = lambda * L / D_fake / 2. / 9.81 * v * fabs(v);
	double f = ze + ye - (zv + yv) - dh;

	// If the pipe was empty in the previous step, we reduce the diameter.
	// if (D_fake > dia) {
	// 	strstrm.str("\n\t\t  f_telt: fake pipe, D_fake=");
	// 	strstrm << D_fake << "m > D=" << dia << "m";
	// 	if (fabs(f) < 1.e-3) {
	// 		D_fake /= 1.5;
	// 		v = mp / ro / (D_fake * D_fake * pi / 4);
	// 		dh = lambda * L / D_fake / 2 / 9.81 * v * fabs(v);
	// 		f = ze + ye - (zv + yv) - dh;
	// 		strstrm << "\n\t\t\t  D=" << D_fake << ", f=" << f;
	// 	}
	// 	if (D_fake > dia)
	// 		D_fake = dia;
	// }
	// if (D_fake<dia)
	// 	D_fake = dia;
	// end of d_fake update

	strstrm << "\n\t\t\t mp = " << mp << ", dh = " << dh << ", f = (ze+ye)-(zv+yv)-dp = " << f;
	logfile_write(strstrm.str(), 3);

	jac[0] = +1.0;
	num_eval_jac[0] = false;
	jac[1] = -1.0;
	num_eval_jac[1] = false;
	jac[2] = -lambda * L / D_fake * ro / 2. * 2.*fabs(mp) / (pow(D_fake, 4.) * 3.1416 / 4.*3.1416 / 4.) / ro / ro / (ro * g);
	num_eval_jac[2] = false;

	return f;
}

/// �gegyenlet (0.a.) ye<0, yv<ze
/**
 * D/100 �tm�r�j� sz�v�sz�lon teltszelv�ny� �raml�s ye-yv nyom�sk�l�nbs�g hat�s�ra
 * @param ye eleje v�zszint
 * @param yv v�ge v�zszint
 * @param mp t�meg�ram
 * @return �gegyenlet �rt�ke
 */
double Channel::f_0(double ye, double yv, double mp) {

	//  Matlab k�d:
	//  Df=D/100;
	//  Af=Df^2*pi/4;
	//  ff=yea-yva-0.02*L/Df/2/9.81*Q*abs(Q)/Af^2;
	//  eval_jac=[1 1 1];

	ostringstream strstrm;
	strstrm.str("");
	// strstrm << "\n\t\t  " << nev << " f_0: fake cso, D=" << D_fake;

	double v = mp / ro / (D_fake * D_fake * pi / 4);
	lambda = surlodas();
	double dh = lambda * L / D_fake / 2 / 9.81 * v * fabs(v);
	double f = ze + ye - (zv + yv) - dh;

	double D_ratio_max = 100.;

	if (D_fake > dia / D_ratio_max)
		force_more_iter = true;
	else {
		force_more_iter = false;
		update_diameter = false;
	}

	if (update_diameter) {
		D_fake /= 1.1;
		strstrm << "\n\t  " << nev << " : updating D_fake/D=" << D_fake / dia << " > " << 1. / D_ratio_max;
		cout << strstrm.str();
		update_diameter = false;
	}

	// if (D_fake > dia / D_ratio_max) {
	// 	force_more_iter = true;
	// 	if (fabs(f) < 1.e-3) {
	// 		D_fake /= 2.;
	// 		strstrm << "\n\t  " << nev << " seems to be empty, updating D_fake/D=" << D_fake / dia << " > " << 1. / D_ratio_max;
	// 		cout << strstrm.str();
	// 	}
	// }
	// else {
	// 	force_more_iter = false;
	// }

	jac[0] = +1.0;
	num_eval_jac[0] = false;
	jac[1] = -1.0;
	num_eval_jac[1] = false;
	jac[2] = -lambda * L / D_fake * ro / 2. * 2.*fabs(mp) / (pow(D_fake, 4.) * 3.1416 / 4.*3.1416 / 4.) / ro / ro / (ro * g);
	num_eval_jac[2] = false;

	logfile_write(strstrm.str(), 2);
	return f;
}

/// �gegyenlet negat�v t�meg�ram eset�n
/**
 * @param ye eleje v�zszint
 * @param yv v�ge v�zszint
 * @param mp t�meg�ram
 * @return �gegyenlet �rt�ke
 */
double Channel::f_1_old(double ye, double yv, double mp) {
	//  Matlab k�d
	//  eval_jac=[1 1 1];
	//  ykrit=kritikus_szint(Q,D,g,n);
	//  if ye<ykrit
	//      ye=1.01*ykrit;
	//      eval_jac(1)=0;
	//  end
	//  [x,y]=ode15s(@nyf_ode,[0,L],ye,options,Q);
	//  ff=yv-y(end);

	ostringstream strstrm;
	strstrm.str("\n\t\t  f_1: ellentetes aramlas, integralas iranya: -->");
	logfile_write(strstrm.str(), 2);

	double ykrit = kritikus_szint(mp / ro);

	// strstrm.str("");
	// strstrm << "\n\t\t\t y_krit=" << ykrit; //<<", ye_min="<<ye_min;
	double f;
	if (ye < ykrit) {
		// ye = ykrit * 1.01;
		jac[0] = 0.0;
		num_eval_jac[0] = false;
		// strstrm
		//         << "\n\t\t\t o !!! Nem lehetseges a kritikus szint alatti bearamlas";
		// strstrm << "\n\t\t\t          -> ye=1.01*ykrit=" << ye << " �s eval_jac[0]=0";
		// Integralas
		// f = yv - ode_megoldo(ye, L / db, 0., mp);
		f = 0.;
		jac[1] = 0.0;
		num_eval_jac[1] = false;
	}
	else {
		// ye = ykrit * 1.01;
		// jac[0] = 0.0;
		// num_eval_jac[0] = false;
		// strstrm
		//         << "\n\t\t\t o !!! Nem lehetseges a kritikus szint alatti bearamlas";
		// strstrm << "\n\t\t\t          -> ye=1.01*ykrit=" << ye << " �s eval_jac[0]=0";
		// Integralas
		f = yv - ode_megoldo(ye, L / db, 0., mp);
		jac[1] = 1.0;
		num_eval_jac[1] = false;
	}
	logfile_write(strstrm.str(), 2);


	strstrm.str("");
	strstrm << "\n\t\t\t f = yv-yv_int = " << f;
	logfile_write(strstrm.str(), 2);

	return f;
}

/// �gegyenlet negat�v t�meg�ram eset�n
/**
 * @param ye eleje v�zszint
 * @param yv v�ge v�zszint
 * @param mp t�meg�ram
 * @return �gegyenlet �rt�ke
 */
double Channel::f_1(double ye, double yv, double mp) {
	//  Matlab k�d
	//  eval_jac=[1 1 1];
	//  ykrit=kritikus_szint(Q,D,g,n);
	//  if ye<ykrit
	//      ye=1.01*ykrit;
	//      eval_jac(1)=0;
	//  end
	//  [x,y]=ode15s(@nyf_ode,[0,L],ye,options,Q);
	//  ff=yv-y(end);

	ostringstream strstrm;
	strstrm.str("\n\t\t  f_1: ellentetes aramlas, integralas iranya: -->");
	logfile_write(strstrm.str(), 2);

	double ykrit = kritikus_szint(mp / ro);

	// strstrm.str("");
	// strstrm << "\n\t\t\t y_krit=" << ykrit; //<<", ye_min="<<ye_min;

	if (ye < ykrit) {
		ye = ykrit * 1.01;
		jac[0] = 0.0;
		num_eval_jac[0] = false;
		strstrm
		        << "\n\t\t\t o !!! Nem lehetseges a kritikus szint alatti bearamlas";
		strstrm << "\n\t\t\t          -> ye=1.01*ykrit=" << ye << " �s eval_jac[0]=0";
	}
	logfile_write(strstrm.str(), 2);

	// Integralas
	double f = yv - ode_megoldo(ye, L / db, 0., mp);
	jac[1] = 1.0;
	num_eval_jac[1] = false;

	strstrm.str("");
	strstrm << "\n\t\t\t f = yv-yv_int = " << f;
	logfile_write(strstrm.str(), 2);

	return f;
}

/// Open-channel flow equation for positive flow rate, y_normal > y_crit
/**
 * @param ye water depth @ x=0
 * @param yv water depth @ x=L
 * @param mp mass flow rate
 * @return error of branch equation
 */
double Channel::f_2c_old(double ye, double yv, double mp) {
	ostringstream strstrm;
	strstrm.str("\n\t\t  f_2c: normal depth not found, integrating backwards: <--");
	logfile_write(strstrm.str(), 2);

	double ykrit = kritikus_szint(mp / ro);

	strstrm.str("");
	strstrm << "\n\t\t\t y_crit=" << ykrit;
	double f;
	if (yv < ykrit) {
		jac[1] = 0.0;
		num_eval_jac[1] = false;
		f = 0;
		double yvv = ode_megoldo(ye, L / db, L, mp);
		jac[0] = 0.0;
		num_eval_jac[0] = false;

		strstrm.str("");
		strstrm << "\n\t\t\t f =  " << f << " !!! (f_2c, yv<ycrit)";
		logfile_write(strstrm.str(), 2);
	}
	else {

		// Integrate backwards
		f = ye - ode_megoldo(yv, -L / db, L, mp);
		jac[0] = 1.0;
		num_eval_jac[0] = false;

		strstrm.str("");
		strstrm << "\n\t\t\t f = y(0)-y(0)_integrated = " << f;
		logfile_write(strstrm.str(), 2);
	}

	return f;
}

/// Open-channel flow equation for positive flow rate, y_normal > y_crit
/**
 * @param ye water depth @ x=0
 * @param yv water depth @ x=L
 * @param mp mass flow rate
 * @return error of branch equation
 */
double Channel::f_2c(double ye, double yv, double mp) {
	ostringstream strstrm;
	strstrm.str("\n\t\t  f_2c: normal depth not found, integrating backwards: <--");
	logfile_write(strstrm.str(), 2);

	double ykrit = kritikus_szint(mp / ro);

	strstrm.str("");
	// strstrm << "\n\t\t\t y_crit=" << ykrit;

	if (yv < ykrit) {
		yv = ykrit * 1.01;

		jac[1] = 0.0;
		num_eval_jac[1] = false;
		strstrm
		        << "\n\t\t\t o !!! y(x=L) < y_crit, which is not possible.";
		strstrm << "\n\t\t\t          -> setting y(L) = 1.01 * y_crit=" << yv << " and eval_jac[1]=0";
	}
	logfile_write(strstrm.str(), 2);

	// Integrate backwards
	double f = ye - ode_megoldo(yv, -L / db, L, mp);
	jac[0] = 1.0;
	num_eval_jac[0] = false;

	strstrm.str("");
	strstrm << "\n\t\t\t f = y(0)-y(0)_integrated = " << f;
	logfile_write(strstrm.str(), 2);

	return f;
}

/// Open-channel flow equation for positive flow rate, y_normal > y_crit
/**
 * @param ye water depth @ x=0
 * @param yv water depth @ x=L
 * @param mp mass flow rate
 * @return error of branch equation
 */
double Channel::f_2ai(double ye, double yv, double mp) {
	ostringstream strstrm;

	double f;
	double ykrit = kritikus_szint(mp / ro);
	vector<double> yn = normal_szint(mp / ro);
	if (yn.size() != 1)
		error("Channel::f_2ai", "WTF????");

	bool simple = true;

	if (!simple) {
		strstrm.str("");
		strstrm << "\n\t\t  f_2ai: y(0) < yc, integrating backwards <--";
		double yvv = yv;
		if (yvv < ykrit) {
			yvv = ykrit * 1.01;
			strstrm << "yv < ycrit, overwriting by 1.01*ycrit";
		}
		double yee = ode_megoldo(yvv, -L / db, L, mp);
		f = ye - yee;
		jac[0] = 1.0;
		num_eval_jac[0] = false;

		strstrm << "\n\t\t\t f = y(0)-y(0)_integrated = " << f << ", y(0)-ynorm = " << (yee - yn.at(0)) << " = 0 ?";
		logfile_write(strstrm.str(), 2);
	}
	else {
		// f = (ye - yn.at(0)) * (ye - yn.at(0)) + (yv - yn.at(0)) * (yv - yn.at(0));
		// strstrm << "\n\t\t\t (simplified model) f = (y(0)-yn)^2+ (y(L)-yn)^2= " << f ;
		// logfile_write(strstrm.str(), 2);
		f = ye - yn.at(0);
		strstrm << "\n\t\t\t (simplified model) f = y(0)-yn= " << f ;
		jac[0] = 1.0;
		num_eval_jac[0] = false;
		logfile_write(strstrm.str(), 2);
	}
	return f;
}


/// �gegyenlet pozit�v t�meg�ram eset�n, yn<yk
/**
 * @param ye eleje v�zszint
 * @param yv v�ge v�zszint
 * @param mp t�meg�ram
 * @return �gegyenlet �rt�ke
 */
double Channel::f_2aii(double ye, double yv, double mp) {
	//  Matlab k�d:
	//  eval_jac=[1 1 1];
	//  [x,y]=ode15s(@nyf_ode,[0,L],ye,options,Q);
	//
	//  ykrit=kritikus_szint(Q,D,g,n);
	//  if ye<ykrit
	//      ff=ye-normal_szint(Q,D,lejt,n);
	//      eval_jac(2)=0;
	//      fprintf('\n\t\t f_2aii: (b) normal   ye=%+5.3e, yv=%+5.3e, Q=%+5.3e, ff=%+5.3e, yv=%+5.3e',ye,yv,Q,ff,yv);
	//  else
	//      ff=yv-y(end);
	//      fprintf('\n\t\t f_2aii: (c) szubkritikus ye=%+5.3e, yv=%+5.3e, Q=%+5.3e, ff=%+5.3e, yv=%+5.3e',ye,yv,Q,ff,yv);
	//  end

	ostringstream strstrm;
	strstrm.str("\n\t\t  f_2aii: yn<ykrit, integralas iranya: -->");
	logfile_write(strstrm.str(), 2);

	double f, ykrit = kritikus_szint(mp / ro);

	if (ye < ykrit) {
		vector<double> ynv = normal_szint(mp / ro);
		double yn;
		if (ynv.size() == 1)
			yn = ynv[0];
		else {
			if (ynv[0] > ynv[1])
				yn = ynv[1];
			else
				yn = ynv[0];
		}

		f = ye - yn;
		jac[0] = 1.0;
		num_eval_jac[0] = false;

		if (yv < yn)
			jac[1] = 0.0;
		else
			jac[1] = 0.0;
		num_eval_jac[1] = false;

		strstrm.str("");
		strstrm << "\n\t\t\t o !!! ye=" << ye << " < ykrit=" << ykrit << ",  jac[1]=0";
		strstrm << "\n\t\t\t f = ye-yn = " << f;
		logfile_write(strstrm.str(), 2);

		// build_res
		if (write_res) {
			if (yv > yn) {
				//cout<<"\n\n"<<nev
				//      <<": 2.a.ii. : vizugras folyadekszint szamitasa...\n";
				double dx = -L / 100., x = L, y1 = yv, f_at_y;

				while ((x > 0) && (y1 > ykrit)) {
					xres.push_back(x);
					yf.push_back((L - x) / L * (ze - zv) + zv);
					yres.push_back(y1);
					//cout<<"\n\t x="<<x<<", yf="<<x/L*(ze-zv)<<", y="<<y1;
					f_at_y = nyf_ode(x, y1, mp);
					y1 += dx * f_at_y;
					x += dx;
				}
				xres.push_back(x);
				yf.push_back((L - x) / L * (ze - zv) + zv);
				yres.push_back(yn);

				xres.push_back(0);
				yf.push_back(ze);
				yres.push_back(yn);
			} else {
				xres.push_back(0);
				xres.push_back(L);
				yres.push_back(yn);
				yres.push_back(yn);
				yf.push_back(ze);
				yf.push_back(zv);
			}
		}
	} else {
		f = yv - ode_megoldo(ye, L / db, 0., mp);
		strstrm.str("");
		strstrm << "\n\t\t\t f = yv-yv_int = " << f;
		jac[1] = 1.0;
		num_eval_jac[1] = false;
	}

	logfile_write(strstrm.str(), 2);

	return f;
}

/// Figyelmeztetes k�perny�re �s eredm�nyf�jlba
/**
 * @param fv h�v� f�ggv�ny
 * @param msg �zenet
 */
void Channel::warning(string fv, string msg) {
	ostringstream strstrm;
	strstrm.str("");
	strstrm << "\n\n******** FIGYELMEZTETES *********";
	strstrm << "\n\telem neve: " << nev;
	strstrm << "\n\tfuggveny : " << fv;
	strstrm << "\n\tuzenet   : " << msg << "\n\n";
	logfile_write(strstrm.str(), 0);
	cout << strstrm.str();
}

/// Hibauzenet k�perny�re �s eredm�nyf�jlba
/**
 * @param fv h�v� f�ggv�ny
 * @param msg �zenet
 */
void Channel::error(string fv, string msg) {
	ostringstream strstrm;
	strstrm.str("");
	strstrm << "\n\n******** HIBA *********";
	strstrm << "\n\telem neve: " << nev;
	strstrm << "\n\tfuggveny : " << fv;
	strstrm << "\n\tuzenet   : " << msg << "\n\n";
	logfile_write(strstrm.str(), 0);
	cout << strstrm.str();
	exit(0);
}

//--------------------------------------------------------------
void Channel::Set_dprop(string mit, double mire) {
	//    if (mit=="diameter")
	//      D=mire;
	//    else
	//      {
	cout << endl << "HIBA! Channel::Set_dprop(mit), ismeretlen bemenet: mit="
	     << mit << endl << endl;
	//      }
}

