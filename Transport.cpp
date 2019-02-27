#include "Transport.h"

//--------------------------------------------------------------
void Transport::set_up_transport() {
  unsigned int i;
  isInitialization = true;
  IOxml datta_io(definitionFile.c_str());
  datta_io.load_ini_values(nodes, edges);
  solveSystem();

  vector<double> v;
  double hossz;
  for (i = 0; i < edges.size(); i++) {
    v.clear();
    // cout<<endl<<"Agelem: "<<edges.at(i)->Get_nev();
    if (edges.at(i)->Get_Tipus() == "Csatorna") {
      v = edges.at(i)->Get_res("v");
      hossz = edges.at(i)->Get_dprop("L");
      edges.at(i)->set_up_grid(0.0, v, hossz);
    } else {
      if (edges.at(i)->Get_Tipus() == "Cso") {
        for (int j = 0; j < 10; j++) {
          v.push_back(edges.at(i)->Get_v());
        }
        hossz = edges.at(i)->Get_dprop("L");
        edges.at(i)->set_up_grid(0.0, v, hossz);
      } else {
        v.push_back(edges.at(i)->Get_v());
        v.push_back(edges.at(i)->Get_v());
        hossz = 10.0;
        edges.at(i)->set_up_grid(0.0, v, hossz);
        edges.at(i)->Set_cdt(10 * 60);
      }
    }

    // cout<<edges.at(i)->Info();
    // cout<<edges.at(i)->show_grid(ido);
    // int int1;cin>>int1;
  }
  transp_dt = 1e100;
  for (i = 0; i < edges.size(); i++) {
    bool kell_eloszlas = false;

    if (edges.at(i)->Get_Tipus() == "Cso") {
      if (fabs(edges.at(i)->Get_dprop("erdesseg")) > 1e-5)
        kell_eloszlas = true;
    }
    if (edges.at(i)->Get_Tipus() == "Csatorna") kell_eloszlas = true;

    if (kell_eloszlas) {
      // cout << ", dt=" << edges.at(i)->cdt;
      if (transp_dt > edges.at(i)->cdt) {
        transp_dt = edges.at(i)->cdt;
        // cout << "*";
      }
    }
  }
  // double transp_dt_min=1; // s
  // if (transp_dt<transp_dt_min)
  //    transp_dt=transp_dt_min;

  tt_length = atof(datta_io.read_setting("tt_length").c_str());

  ostringstream strstrm;
  strstrm << endl
          << endl
          << "transp_dt=" << transp_dt << "s, tt_length=" << tt_length << "h"
          << endl;
  strstrm << endl << "Grid setup complete." << endl;
  writeLogFile(strstrm.str(), 2);
  cout << strstrm.str();
}

//--------------------------------------------------------------
void Transport::solve_transport(int calculationType) {
  ostringstream strstrm;
  strstrm << "Starting computation..." << endl;
  writeLogFile(strstrm.str(), 2);
  cout << strstrm.str();

  cout << fixed << setprecision(2) << setw(5) << setfill(' ') << right;
  ido = 0.;
  double counter = 0, szazalek;
  while (ido < tt_length * 3600) {
    szazalek = round(ido / (tt_length * 3600) * 100);
    // szazalek=round(10*szazalek)/10;
    //if ((debugLevel) > 0) progress_file_write(szazalek);
    if (counter < szazalek) {
      cout << "\n\t\t" << szazalek << " %, time = " << (ido / 3600.);
      cout << "h, oldest fluid particle: " << (get_oldest() / 3600.) << "h";
      counter += 1;
    }
    transport_step(transp_dt);
    ido += transp_dt;
  }
}

//--------------------------------------------------------------
void Transport::save_transport(int calculationType) {
  resultFile = definitionFile;
  cout << endl << "Kimeneti fajl neve:" << resultFile << endl;
  IOxml datta_io(resultFile.c_str());
  datta_io.save_transport(calculationType, nodes, edges);
}

//--------------------------------------------------------------
double Transport::get_oldest() {
  double tmax = 0;
  double tmp;
  for (unsigned int i = 0; i < nodes.size(); i++) {
    tmp = nodes.at(i)->Get_dprop("konc_atlag");
    if (tmp > tmax) tmax = tmp;
  }
  return tmax;
}

//--------------------------------------------------------------
void Transport::transport_step(double dt) {
  bool transp_debug = false;
  vector<int> temp;
  double seb;

  // 1. lepes: csomopontok atlagos koncentraciojanak szamitasa
  double szaml, nevezo, c, m;
  for (unsigned int i = 0; i < nodes.size(); i++) {
    // Ha meg van adva beadagolt klor, a csomopontban pontosan ennyi
    if (fabs(nodes.at(i)->Get_dprop("cl_be")) > 1e-10) {
      nodes.at(i)->Set_dprop("konc_atlag", nodes.at(i)->Get_dprop("cl_be"));
    }
    // Ha nincs megadva, kevered�st kell sz�molni
    else {
      szaml = 0.0;
      nevezo = 0.0;

      if (transp_debug) cout << endl << "\t bemeno agak: (";
      temp.clear();
      temp = nodes.at(i)->ag_be;
      if (transp_debug) cout << temp.size() << "db)";

      if (temp.size() > 0) {
        for (unsigned int j = 0; j < temp.size(); j++) {
          // pozitiv atlagsebessegu agak kellenek csak
          // edges.at(temp.at(j))->show_grid(ido);
          seb = edges.at(temp.at(j))->mean(edges.at(temp.at(j))->vel);
          if (transp_debug)
            cout << endl
                 << "\t\t" << edges.at(temp.at(j))->Get_nev()
                 << ", vatlag=" << fixed << setprecision(3) << seb;
          int utolso = edges.at(temp.at(j))->vel.size() - 1;
          if (edges.at(temp.at(j))->vel.at(utolso) > 0) {
            c = edges.at(temp.at(j))->konc.at(utolso);
            m = edges.at(temp.at(j))->Get_mp();
            if (transp_debug) cout << " mp= " << m << " c=" << c;
            szaml += fabs(m) * c;
            nevezo += fabs(m);
          }
        }
      }

      if (transp_debug) cout << endl << "\t kimeno agak: (";
      temp.clear();
      temp = nodes.at(i)->ag_ki;
      if (transp_debug) cout << temp.size() << "db)";

      if (temp.size() > 0) {
        for (unsigned int j = 0; j < temp.size(); j++) {
          // negativ sebessegu agak kellenek csak
          seb = edges.at(temp.at(j))->mean(edges.at(temp.at(j))->vel);
          if (transp_debug)
            cout << endl
                 << "\t\t" << edges.at(temp.at(j))->Get_nev()
                 << ", vatlag=" << fixed << setprecision(3) << seb;

          if (edges.at(temp.at(j))->vel.at(0) < 0) {
            c = edges.at(temp.at(j))->konc.at(0);
            m = edges.at(temp.at(j))->Get_mp();
            if (transp_debug) cout << " mp= " << m << " c=" << c;
            szaml += fabs(m) * c;
            nevezo += fabs(m);
          }
        }
      }
      double c, fogy, cl_be;
      fogy = nodes.at(i)->Get_dprop("fogy");
      cl_be = nodes.at(i)->Get_dprop("cl_be");
      // Ha "kifele" megy a viz, attol a csomopontban nem higul
      if (fogy > 0) {
        fogy = 0.0;
        cl_be = 0.0;
      }

      if (calculationType == 1) {
        // if (fabs(nevezo - fogy) > 1.e-10)
        c = (szaml) / (nevezo - fogy);
        // else
        //{
        // TODO: writeLogFile-ba!
        // cout << endl << "ERROR!!!! Transport::transport_step():
        // fabs(nevezo-fogy)=" << fabs(nevezo - fogy) << "<1.e-10" << endl <<
        // endl;
        // exit(-1);
        //}
      } else {
        // if (nevezo + fabs(fogy) > 1.e-10)
        c = (szaml + cl_be * fabs(fogy)) / (nevezo + fabs(fogy));
        // else
        //{
        // TODO: writeLogFile-ba!
        //    cout << endl << "ERROR!!!! Transport::transport_step(): nevezo +
        //    fabs(fogy)" << (nevezo + fabs(fogy)) << "<1.e-10" << endl << endl;
        // exit(-1);
        //}
      }

      nodes.at(i)->Set_dprop("konc_atlag", c);
      if ((nodes.at(i)->Get_nev() == "NODE_1346650") ||
          (nodes.at(i)->Get_nev() == "NODE_1337960")) {
        cout << "\n\t" << nodes.at(i)->Get_nev() << ": szaml=" << szaml
             << ", cl_be*fabs(fogy)=" << (cl_be * fabs(fogy))
             << ", nevezo=" << nevezo;
        cout << "\n\t\t atlagos koncentracio: " << c << endl;
      }
    }
  }

  // cout << endl << "Node update complete.\n";
  // cin.get();

  // 2. lepes: edges leptetese
  for (unsigned int i = 0; i < edges.size(); i++) {
    // cout << endl << "\t updating " << edges.at(i)->Get_nev() << "...";

    // Ha nulla az �rdess�g, akkor csak �sszek�t� cs� �s szkippelj�k
    bool kell_eloszlas = false;

    if (edges.at(i)->Get_Tipus() == "Cso") {
      if ((fabs(edges.at(i)->Get_dprop("erdesseg"))) > 1e-5)
        kell_eloszlas = true;
    }
    if (edges.at(i)->Get_Tipus() == "Csatorna") kell_eloszlas = true;

    //      cout<<endl<<edges.at(i)->Get_nev()<<",
    //      kell_eloszlas="<<kell_eloszlas;
    //      int int1;
    //      cin>>int1;

    if (kell_eloszlas) {
      if ((edges.at(i)->mean(edges.at(i)->vel)) > 0) {
        double konc_eleje, konc, konc_e, vel_e, dx;
        dx = edges.at(i)->cL / edges.at(i)->vel.size();
        if (edges.at(i)->Get_Csp_db() == 1) {
          konc_eleje = 0.0;
          edges.at(i)->konc.at(0) = 0.0;
          edges.at(i)->konc.at(1) = 0.0;
        } else {
          konc_eleje = nodes.at(edges.at(i)->Get_Cspe_Index())
                       ->Get_dprop("konc_atlag");
          for (unsigned int j = edges.at(i)->vel.size() - 1; j > 0; j--) {
            konc_e = edges.at(i)->konc.at(j - 1);
            vel_e = edges.at(i)->vel.at(j - 1);
            konc = edges.at(i)->konc.at(j);
            edges.at(i)->konc.at(j) =
              konc - dt / dx * vel_e * (konc - konc_e) + dt * teta(konc_e, i);

            /*if (edges.at(i)->Get_nev()=="PIPE116") {
              cout<<endl<<"i="<<i<<", konc="<<konc<<", konc_e="
              <<konc_e<<", vel_e="<<vel_e<<", teta="<<teta;
              }*/

            if (edges.at(i)->konc.at(j) < 0)
              edges.at(i)->konc.at(j) = 0.0;
          }
          edges.at(i)->konc.at(0) = konc_eleje;
        }
      } else {
        double konc_vege, konc, konc_u, vel_u, dx;
        dx = edges.at(i)->cL / edges.at(i)->vel.size();
        if (edges.at(i)->Get_Csp_db() == 1) {
          konc_vege = nodes.at((unsigned long)edges.at(i)->Get_Cspv_Index())
                      ->Get_dprop("konc_atlag");
          edges.at(i)->konc.at(0) = konc_vege;
          edges.at(i)->konc.at(1) = konc_vege;
        } else {
          konc_vege = nodes.at((unsigned long)edges.at(i)->Get_Cspv_Index())
                      ->Get_dprop("konc_atlag");
          for (unsigned int j = 0; j < edges.at(i)->vel.size() - 1; j++) {
            konc_u = edges.at(i)->konc.at(j + 1);
            vel_u = edges.at(i)->vel.at(j + 1);
            konc = edges.at(i)->konc.at(j);
            edges.at(i)->konc.at(j) =
              konc - dt / dx * vel_u * (konc_u - konc) + dt * teta(konc_u, i);
            if (edges.at(i)->konc.at(j) < 0)
              edges.at(i)->konc.at(j) = 0.0;
          }
          edges.at(i)->konc.at(edges.at(i)->konc.size() - 1) = konc_vege;
        }
      }
    } else {
      double konc =
        nodes.at(edges.at(i)->Get_Cspe_Index())->Get_dprop("konc_atlag");

      if (edges.at(i)->Get_Csp_db() == 2) {
        double konc_vege =
          nodes.at(edges.at(i)->Get_Cspv_Index())->Get_dprop("konc_atlag");
        konc = (konc + konc_vege) / 2;
      }

      // cout << endl << "dt=" << dt << ", " << edges.at(i)->show_grid(0.0)
      // << "\n END OF GRID\n";

      for (unsigned int j = 0; j < edges.at(i)->konc.size(); j++) {
        // cout << "\t edges.at(i)->konc.size()=" <<
        // edges.at(i)->konc.size();
        // cin.get();
        edges.at(i)->konc.at(j) = konc;
      }
    }
    if (edges.at(i)->Get_nev() == "PIPE_602407") {
      cout << endl << "dt=" << dt << ", " << edges.at(i)->show_grid(0.0);
      /*cin.get();*/
    }
  }
  // cout << endl << "Edge update complete.\n";
  // cin.get();
}

//--------------------------------------------------------------
double Transport::teta(double konc, const int i) {
  if (calculationType == 1)
    return 1.0;
  else {
    double Rh, cl_k, cl_w;
    if (edges.at(i)->Get_Tipus() == "Csatorna") {
      Rh = edges.at(i)->Get_dprop("Rh");
      cl_k = edges.at(i)->Get_dprop("cl_k");
      cl_w = edges.at(i)->Get_dprop("cl_w");
    } else {
      if (edges.at(i)->Get_Tipus() == "Cso") {
        Rh = edges.at(i)->Get_dprop("Rh");
        cl_k = edges.at(i)->Get_dprop("cl_k");
        cl_w = edges.at(i)->Get_dprop("cl_w");
      } else {
        Rh = 1;
        cl_k = 0.0;
        cl_w = 0.0;
      }
    }
    double Sh = 3.65;
    double kf = Sh;
    double tag1 = -cl_k * konc;
    double tag2 = -kf / Rh * (konc - cl_w);
    return (tag1 + 0 * tag2);
  }
}


//--------------------------------------------------------------
void Transport::solve_residence_time() {
  string max_ID;
  double max_VAL, mean_VAL, VAL_prev = -100., d_VAL = 1000.;

  consolePrint.str("");
  consolePrint
      << "\n\nComputing residence time...\n===================================="
      << endl;
  if (debugLevel > 0) {
    writeLogFile(consolePrint.str(), 1);
  }
  int step = 1;
  int step_max = 100000;

  while ((step < step_max) && (d_VAL > 0.1)) {
    residence_time_step(max_ID, max_VAL, mean_VAL);
    if (debugLevel > 0) {
      if ((step % 100) == 0)
        cout << endl
             << "\t step #" << step
             << " max. res. time: " << convert_to_hr_min(max_VAL) << " ("
             << max_ID << "), mean: " << convert_to_hr_min(mean_VAL);
    }
    d_VAL = fabs(mean_VAL - VAL_prev);
    VAL_prev = mean_VAL;
    step++;
  }
  if (debugLevel > 0) {
    consolePrint.str("");
    consolePrint << endl
         << "\t step #" << step
         << " max. res. time: " << convert_to_hr_min(max_VAL) << " (" << max_ID
         << "), mean: " << convert_to_hr_min(mean_VAL) << endl << "Finished computing residence time." << endl;
    writeLogFile(consolePrint.str(), 1);
    // cout << consolePrint.str();
  }
}

//--------------------------------------------------------------
void Transport::residence_time_step(string & max_ID, double & max_VAL, double & mean_VAL) {
  bool transp_debug = false;
  double mv = -1.;
  double sum = 0;
  double TINY_MASS_FLOW_RATE = 1.e-3;
  double TINY_VEL = 0.000001;  // 1m -> 278 hours

  double MAX_TIME = 168. * 3600.;

  // 1. lepes: csomoponti atlagkor meghatarozasa
  double szaml, nevezo, c, m;
  vector<int> temp;
  // int length_be, length_ki;
  for (unsigned int i = 0; i < nodes.size(); i++) {
    // Csak akkor piszkaljuk a csomoponti eletkort, ha nincs betap.
    // Legalabb 1cm3/h betap legyen
    if (nodes.at(i)->Get_fogy() < (-1.e-6 * 1000. / 3600.)) {
      if (transp_debug)
        cout << endl
             << nodes.at(i)->Get_nev() << ": \n\tbetap miatt adott vizkor: "
             << nodes.at(i)->Get_dprop("tt") / 60. << "min";
      // cin.get();
    } else {
      // Tomegarammal sulyozott atlag
      szaml = 0.0;
      nevezo = 0.0;

      // Bemeno agak
      if (transp_debug)
        cout << endl << nodes.at(i)->Get_nev() << ": \n\tbemeno agak: (";
      temp.clear();
      temp = nodes.at(i)->ag_be;
      // length_be = temp.size();
      if (transp_debug) cout << temp.size() << "db)";

      if (temp.size() > 0) {
        for (unsigned int j = 0; j < temp.size(); j++) {
          if (edges.at(temp.at(j))->Get_v() >= 0) {
            c = edges.at(temp.at(j))->Get_tt_end();
            double ce = edges.at(temp.at(j))->Get_tt_start();
            m = edges.at(temp.at(j))->Get_mp();
            if (transp_debug)
              cout << " mp= " << m << " t=" << (c / 60)
                   << " min (tt_start = " << (ce / 60) << "min)";

            // 0 tomegaram eseten 0-val szorozzuk a szamlalot
            if (fabs(m) < TINY_MASS_FLOW_RATE) m = TINY_MASS_FLOW_RATE;
            szaml += fabs(m) * c;
            nevezo += fabs(m);
          }
        }
      }

      // Kimeno agak
      if (transp_debug) cout << endl << "\n\tkimeno agak: (";
      temp.clear();
      temp = nodes.at(i)->ag_ki;
      // length_ki = temp.size();
      if (transp_debug) cout << temp.size() << "db)";

      if (temp.size() > 0) {
        for (unsigned int j = 0; j < temp.size(); j++) {
          if (edges.at(temp.at(j))->Get_v() <= 0) {
            c = edges.at(temp.at(j))->Get_tt_start();
            double ce = edges.at(temp.at(j))->Get_tt_end();
            m = edges.at(temp.at(j))->Get_mp();
            if (transp_debug)
              cout << " mp= " << m << " t=" << (c / 60)
                   << " min (tt_end = " << (ce / 60) << "min)";

            // 0 tomegaram eseten 0-val szorozzuk a szamlalot
            if (fabs(m) < TINY_MASS_FLOW_RATE) m = TINY_MASS_FLOW_RATE;
            szaml += fabs(m) * c;
            nevezo += fabs(m);
          }
        }
      }
      // A virtualics csomopontokban siman lehet nevezo=0
      if (fabs(nevezo) < TINY_MASS_FLOW_RATE) nevezo = TINY_MASS_FLOW_RATE;

      nodes.at(i)->Set_dprop("tt", szaml / nevezo);
      sum = sum + szaml / nevezo;

      if (transp_debug)
        cout << "\n\t atlagos vizkor= " << nodes.at(i)->Get_dprop("tt") / 60
             << "min";
    }
  }

  for (unsigned int i = 0; i < nodes.size(); i++) {
    if ((nodes.at(i)->Get_dprop("tt")) > MAX_TIME)
      nodes.at(i)->Set_dprop("tt", MAX_TIME);
  }

  mean_VAL = sum / (double(nodes.size()));

  // 2. lepes: csoelemek leptetese

  double v, L, tt_s, tt_e;
  for (int i = 0; i < edges.size(); i++) {
    string type = edges.at(i)->GetType();
    int cspe_id = edges.at(i)->Get_Cspe_Index();
    int cspv_id = edges.at(i)->Get_Cspv_Index();

    if ((type.compare("Cso") == 0) || (type.compare("Csatorna") == 0)) {
      L = edges.at(i)->Get_dprop("L");
      v = edges.at(i)->Get_v();
      if (v >= 0) {
        // cout<<"\n v>=0!!!";
        if (v < TINY_VEL) {
          tt_s = nodes.at(cspe_id)->Get_dprop("tt");
          tt_e = nodes.at(cspv_id)->Get_dprop("tt");
          v = TINY_VEL;
          if (tt_s > tt_e)
            tt_e = tt_s + L / v;
          else
            tt_s = tt_e + L / v;

          if (tt_s > MAX_TIME) tt_s = MAX_TIME;

          if (tt_e > MAX_TIME) tt_e = MAX_TIME;
        } else {
          int cspe_id = edges.at(i)->Get_Cspe_Index();
          tt_s = nodes.at(cspe_id)->Get_dprop("tt");
          tt_e = tt_s + L / v;
          if (tt_e > MAX_TIME) tt_e = MAX_TIME;
        }
        // Get maximum value
        if (tt_e > mv) {
          max_VAL = tt_e;
          mv = tt_e;
          max_ID = edges.at(i)->Get_nev();
        }
        // }

        if (tt_s < MAX_TIME)
          edges.at(i)->Set_tt_start(tt_s);
        else
          edges.at(i)->Set_tt_start(MAX_TIME);
        if (tt_e < MAX_TIME)
          edges.at(i)->Set_tt_end(tt_e);
        else
          edges.at(i)->Set_tt_end(MAX_TIME);

        if (transp_debug)
          cout << endl
               << edges.at(i)->Get_nev() << ": v=" << v << "m/s, L=" << L
               << "m, tt_start=" << (tt_s / 60.)
               << "min, tt_end=" << (tt_e / 60.) << "min";
      } else {
        if (v > -TINY_VEL) {
          tt_s = nodes.at(cspe_id)->Get_dprop("tt");
          tt_e = nodes.at(cspv_id)->Get_dprop("tt");
          v = -TINY_VEL;  // Ki volt kommentelve
          if (tt_s > tt_e)
            tt_e = tt_s - L / v;
          else
            tt_s = tt_e - L / v;
          if (tt_s > MAX_TIME) tt_s = MAX_TIME;

          if (tt_e > MAX_TIME) tt_e = MAX_TIME;
        } else {
          int cspv_id = edges.at(i)->Get_Cspv_Index();
          tt_e = nodes.at(cspv_id)->Get_dprop("tt");
          tt_s = tt_e - L / v;
          if (tt_s > MAX_TIME) tt_s = MAX_TIME;

          if (tt_s > mv) {
            max_VAL = tt_s;
            mv = tt_s;
            max_ID = edges.at(i)->Get_nev();
          }
        }

        if (tt_s < MAX_TIME)
          edges.at(i)->Set_tt_start(tt_s);
        else
          edges.at(i)->Set_tt_start(MAX_TIME);
        if (tt_e < MAX_TIME)
          edges.at(i)->Set_tt_end(tt_e);
        else
          edges.at(i)->Set_tt_end(MAX_TIME);
      }
    } else {
      double v = edges.at(i)->Get_v();
      double mp = edges.at(i)->Get_mp();

      // cout << endl << "!!!!! "<<edges.at(i)->Get_nev()<< " type:
      // "<<type<<endl;

      if ((type.compare("KonstNyomas") == 0) ||
          (type.compare("Vegakna") == 0)) {
        if (mp < 0) {
          // Ez esetben a rendszerbe befele aramlik a kozeg, nem bantjuk a
          // vizkort
          tt_s = edges.at(i)->Get_tt_start();
          // tt_e = edges.at(i)->Get_tt_end();

          if (transp_debug) {
            cout << endl
                 << edges.at(i)->Get_nev()
                 << ": \n\t flow TO the system FROM the element (v=" << v
                 << "m/s) -> prescribed age: tt_start = " << tt_s / 60.
                 << "min";
            //<< ", tt_end = " << tte / 60. << "min";
            // cin.get();
          }
        } else {
          if (transp_debug) {
            tt_s = nodes.at(cspe_id)->Get_dprop("tt");
            edges.at(i)->Set_tt_start(tt_s);
            cout << endl
                 << edges.at(i)->Get_nev()
                 << ": \n\t flow FROM the system TO the element (v=" << v
                 << "m/s) -> age: tt_start = " << tt_s / 60. << "min";
            // cin.get();
          }
        }
      } else {
        double tt;

        if (fabs(v) < TINY_VEL) {
          double tt1 = nodes.at(cspe_id)->Get_dprop("tt");
          double tt2 = nodes.at(cspv_id)->Get_dprop("tt");
          if (tt1 > tt2)
            tt = tt1;
          else
            tt = tt2;
        } else {
          if (v > 0)
            tt = nodes.at(cspe_id)->Get_dprop("tt");
          else {
            if (cspv_id > -1)
              tt = nodes.at(cspv_id)->Get_dprop("tt");
            else
              tt = nodes.at(cspe_id)->Get_dprop("tt");
          }
        }

        if (tt > MAX_TIME) tt = MAX_TIME;
        edges.at(i)->Set_tt_start(tt);
        edges.at(i)->Set_tt_end(tt);
        if (transp_debug) {
          cout << endl
               << edges.at(i)->Get_nev() << ": \n\tvizkor: " << tt / 60.
               << "min";
          // cin.get();
        }
      }
    }
  }
}

string Transport::convert_to_hr_min(double input_seconds) {
  stringstream str;
  int seconds = round(input_seconds);
  int days = (int)seconds / 60 / 60 / 24;
  int hours = (int)(seconds / 60 / 60) % 24;
  int minutes = (int)(seconds / 60) % 60;

  str << days << " d, " << hours << " h, " << minutes << " m";
  return str.str();
}