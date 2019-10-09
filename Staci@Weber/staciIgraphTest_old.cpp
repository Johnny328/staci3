#include <iostream>
#include "/home/rweber/Dropbox/___Phd_kutat/staci/Staci.h"
#include "/home/rweber/Eigen/Eigen/Eigen"
#include "/home/rweber/matplotlib-cpp-master/matplotlibcpp.h"
#include <igraph.h>

using namespace std;
using namespace Eigen;

namespace plt = matplotlibcpp;

double Average_igraph_vector(igraph_vector_t *v);
void Print_igraph_vector(igraph_vector_t *v);

Staci *wds;

int main(){
	//string case_folder = "/home/rweber/Dropbox/2017_2021_Wéber_Phd/halozatok/alap_halozatok/csillag/2/"; // name of containing folder of staci file
	string case_folder = "/home/rweber/Dropbox/2017_2021_Wéber_Phd/halozatok/nagy_valós_hálózatok/"; // name of containing folder of staci file
    string case_name = "soptest"; // name of staci file WITHOUT .spr
    string sens_par_name = "demand";

	wds = new Staci(case_folder + case_name + ".spr");
	wds->build_system();
	
    
	// Getting the edge vector from staci.cpp
	vector<int> edge_vector = wds->Get_edge();

	igraph_t graph;
    igraph_vector_t v;
    igraph_vector_t result;
    igraph_real_t edge[edge_vector.size()];

    // Copying from std::vector to igraph vector
    for(int i=0; i<edge_vector.size(); i++){
    	edge[i] = edge_vector[i];
    }
    
	cout << endl;
    cout << " =================================================" << endl;
    cout << " Calculation of graph properties with igraph 0.7.1" << endl;
    cout << " =================================================" << endl << endl;
    cout << " Number of nodes: " << wds->cspok.size() << endl;
    cout << " Number of edges: " << edge_vector.size()/2 << endl;
	
	igraph_vector_view(&v, edge, sizeof(edge)/sizeof(double));
    igraph_create(&graph, &v, 0, IGRAPH_UNDIRECTED);
    igraph_vector_init(&result, 0);

    igraph_degree(&graph, &result, igraph_vss_all(), IGRAPH_ALL, IGRAPH_LOOPS);
    vector<double> Vrank(wds->cspok.size());
    for(int i=0;i<igraph_vector_size(&result);i++)
        Vrank[i] = VECTOR(result)[i];
    plt::hist(Vrank);
    plt::show();

    printf(" Maximum degree\t \t %3i,\t \t at node %4i\n",(int)igraph_vector_max(&result), (int)igraph_vector_which_max(&result));

    igraph_closeness(&graph, &result, igraph_vss_all(), IGRAPH_ALL,0,0);
    printf(" Maximum closeness\t %1.3e,\t at node %4i\n",
             (double)igraph_vector_max(&result), (int)igraph_vector_which_max(&result));

    igraph_betweenness(&graph, &result, igraph_vss_all(),IGRAPH_UNDIRECTED, 0, 1);
    printf(" Maximum betweenness\t %1.3e,\t at node %4i\n",(double)igraph_vector_max(&result), (int)igraph_vector_which_max(&result));

	igraph_transitivity_local_undirected(&graph, &result, igraph_vss_all(), IGRAPH_TRANSITIVITY_ZERO);
    double trans = Average_igraph_vector(&result);
    printf(" Transitivity (cluster coef) %1.3e, \n", trans);

    cout << endl << " ================== END ================== " << endl << endl;

    igraph_vector_destroy(&result);
    igraph_destroy(&graph);
	return 0;
}

double Average_igraph_vector(igraph_vector_t *v){
	double sum=0;
	for (int i=0; i<igraph_vector_size(v);i++)
		sum += (double)VECTOR(*v)[i];
	return sum / (double)igraph_vector_size(v);
}

void Print_igraph_vector(igraph_vector_t *v){
    for (int i=0; i<igraph_vector_size(v);i++)
        printf("%10f \n", (double)VECTOR(*v)[i]);
}