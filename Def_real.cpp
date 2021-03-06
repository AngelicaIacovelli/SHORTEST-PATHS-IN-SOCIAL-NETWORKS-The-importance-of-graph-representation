#include "headers.h"

// Funzione utilizzo memoria
void process_mem_usage(unsigned long& vm_usage, unsigned long& resident_set, bool diff);
unsigned long vm = 0;
unsigned long rss = 0;

long r [] = {0,0,0,0,0,0,0,0,0};

// Creo strutture
typedef std::pair<int, int> Edge; 
// This class couples together a pair of values, which may be of different types (T1 and T2). 
// The individual values can be accessed through its public members first and second.
// Pairs are a particular case of tuple.

// Creo strutture per Adjacency List
typedef adjacency_list<listS, vecS, directedS, no_property, property<edge_weight_t, int> > list;
typedef graph_traits<list>::vertex_descriptor vertex_list;

// Creo strutture per Adjacency Matrix
typedef adjacency_matrix<directedS,no_property, property<edge_weight_t, int> > matrix;    
typedef graph_traits<matrix>::vertex_descriptor vertex_matrix;

// Creo strutture per la CSR
struct CsrWeight{
    int w;
};
typedef compressed_sparse_row_graph <directedS, no_property, CsrWeight> csr;
typedef graph_traits<csr>::vertex_descriptor vertex_csr;

//MAIN

int main(int argc, char *argv[]) { 
/* Cosa passo al main:
argv[] è un vettore dove:
argv[1] = path del grafo di input 
argv[2] = separator char
argv[3] = D (directed) or U (undirected)
argv[4] = L (adj. list), M (adj. matrix) or C (csr) data stucture
argv[5] = seed
*/ 

    // flag per indicare se eseguire o meno Johnson
    bool all_pairs = 0;
    
    // flag per indicare se stampare o meno i risultati di Dijstra (e Johnson)
    bool verbose = 0;

    // flag per indicare se visualizzare o meno il grafo
    bool visualize = 0;

    // parsing degli argomenti
    std::string filename = argv[1];
    char sep = argv[2][0];
    char graph_type = argv[3][0];
    char datastruct = argv[4][0];
    const int seed = atoi(argv[5]); //atoi convert string to integer


    std::set<int> nodes;
    std::set<int> new_nodes;
    std::map<int, int> node_dict;

    std::vector<std::vector<int>> content;
	std::vector<int> row;
	std::string line, word;

    std::fstream file (filename, std::ios::in);
	if(file.is_open())
	{
		while(getline(file, line))
		{
			row.clear();
			std::stringstream str(line);
			while(getline(str, word, sep)){
				row.push_back(atoi(word.c_str()));
                nodes.insert(atoi(word.c_str()));
            }
			content.push_back(row);
		}
	}
	else{
		std::cout<<"Error: could not open the file\n";
        return -1;
    }

    // fisso numero di nodi 
    const int num_nodes = nodes.size();
    std::cout << "Number of nodes: " << num_nodes << std::endl;
   
    int i = 0;
    std::set<int>::iterator itr;
    // re-labeling dei nodi per avere indici incrementali
    for(itr = nodes.begin(); itr!=nodes.end(); itr++){
        node_dict[*itr] = i;
        new_nodes.insert(i);
        i++;
    }

    // fisso il seed 
    srand(seed); //srand initialize random number generator
    std::cout << "Seed: " << seed << std::endl;

    // Genero edges
    std::vector<Edge> edges_array;

    for(int i=0; i < content.size(); i++) {
        edges_array.push_back(Edge(node_dict[content[i][0]],node_dict[content[i][1]]));
        if (graph_type=='U')
            edges_array.push_back(Edge(node_dict[content[i][1]],node_dict[content[i][0]])); // Va rimossa quando ho grafo direzionato 
    }

    const int num_edges = edges_array.size();
    std::cout << "Number of edges: " << num_edges << std::endl;

    // Genero weights
    std::vector<CsrWeight> weights2;
    int *weights = (int*)malloc(sizeof(int)*num_edges);
    CsrWeight random;

    for(int i = 0; i < num_edges; i++){
        random.w = rand()%(10) + 1;     /*A: random è di tipo csr_weight. Se vedo csr_weight è una struct formata solo dal tipo w! quindi scrivendo random.w posso dire a quanto equivale il campo w*/
        weights[i] = random.w;                
        if (datastruct == 'C')
            weights2.push_back(random);
    }

    // Genero nodo radice
    int V = 0;
    do{
        V = rand()%(num_nodes-1);
    }while(!new_nodes.count(V));

    // Timers
    auto start = std::chrono::high_resolution_clock::now();
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);  

    switch(datastruct){

        ////// Adjaceny List    
        case 'L':{
            // Utilizzo Memoria 0  
            process_mem_usage(vm, rss, 0);

            // Creo Grafo (Adjacency List)
            list list_g(edges_array.begin(), edges_array.end(), weights, num_nodes);

            // Utilizzo Memoria 1
            process_mem_usage(vm, rss, 1);
            std::cout << std::fixed << "Adjacency List -> Memory usage: " << vm << " kB" << std::endl;
            r[0] = rss;
            
            std::vector< vertex_list > list_p(num_vertices(list_g));
            std::vector< int > list_d(num_vertices(list_g));
            vertex_list list_s = vertex(V, list_g);

            // dijkstra algorithm for list graph   
            
            start = std::chrono::high_resolution_clock::now();

            dijkstra_shortest_paths_no_color_map(list_g, list_s,
                predecessor_map(make_iterator_property_map(
                                    list_p.begin(), get(vertex_index, list_g)))
                .distance_map(make_iterator_property_map(
                        list_d.begin(), get(vertex_index, list_g))));

            stop = std::chrono::high_resolution_clock::now();

            duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);  
            std::cout << "Duration Dijkstra: " << duration.count() << "\u00B5s" << std::endl; // \u00B5s : Character 'MICRO SIGN'
            r[1] = duration.count();

            if (verbose){
                // Stampo i risultati
                std::cout << "Source vertex: " << V << std::endl;
                std::cout << "Distances and parents:" << std::endl;
                graph_traits < list >::vertex_iterator vi, vend;
                for (tie(vi, vend) = vertices(list_g); vi != vend; ++vi) {
                    if (list_d[*vi] == (std::numeric_limits<int>::max)())
                        std::cout << "distance(" << *vi << ") = " << std::setw(7) << "inf, ";
                    else
                        std::cout << "distance(" << *vi << ") = " << std::setw(5) << list_d[*vi] << ", ";
                    std::cout << "parent(" << *vi << ") = " << list_p[*vi] << std::endl;
                }
                std::cout << std::endl;
            }
            break;
        }

        ////// Adjaceny Matrix
        case 'M':{
            // Utilizzo Memoria 0  
            process_mem_usage(vm, rss, 0);

            // Creo Grafo (Adjacency Matrix)
            matrix matrix_g(edges_array.begin(), edges_array.end(), weights, num_nodes);

            // Utilizzo Memoria 1 
            process_mem_usage(vm, rss, 1);
            std::cout << std::fixed << "Adjacency Matrix -> Memory usage: " << vm << " kB" << std::endl;
            r[3] = rss;
            
            std::vector< vertex_matrix > matrix_p(num_vertices(matrix_g));
            std::vector< int > matrix_d(num_vertices(matrix_g));

            vertex_matrix matrix_s = vertex(V, matrix_g);

            // Dijkstra
            // Avvio misurazione tempo
            start = std::chrono::high_resolution_clock::now();    

            dijkstra_shortest_paths_no_color_map(matrix_g, matrix_s,
                predecessor_map(make_iterator_property_map(
                                    matrix_p.begin(), get(vertex_index, matrix_g)))
                .distance_map(make_iterator_property_map(
                        matrix_d.begin(), get(vertex_index, matrix_g))));

            // Stop Tempo
            stop = std::chrono::high_resolution_clock::now();

            duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);  
            std::cout << "Duration Dijkstra: " << duration.count() << "\u00B5s" << std::endl; // \u00B5s : Character 'MICRO SIGN'
            r[4] = duration.count();

            if (verbose){
                // Stampo i risultati
                std::cout << "Source vertex: " << V << std::endl;
                std::cout << "Distances and parents:" << std::endl;
                graph_traits < matrix >::vertex_iterator vi, vend;
                for (tie(vi, vend) = vertices(matrix_g); vi != vend; ++vi) {
                    if (matrix_d[*vi] == (std::numeric_limits<int>::max)())
                        std::cout << "distance(" << *vi << ") = " << std::setw(7) << "inf, ";
                    else
                        std::cout << "distance(" << *vi << ") = " << std::setw(5) << matrix_d[*vi] << ", ";
                    std::cout << "parent(" << *vi << ") = " << matrix_p[*vi] << std::endl;
                }
                std::cout << std::endl;
            }
            break;
        }
        ////// Compressed Sparse Row
        case 'C':{
            // Utilizzo Memoria 0  
            process_mem_usage(vm, rss, 0);

            // Creo Grafo (Csr)
            csr csr_g(edges_are_unsorted, edges_array.begin(), edges_array.end(), weights2.begin(), num_nodes);

            // Utilizzo memoria 1
            process_mem_usage(vm, rss, 1);
            std::cout << std::fixed << "Csr -> Memory usage: " << vm << " kB" << std::endl;
            r[6] = rss;

            std::vector< vertex_csr > csr_p(num_vertices(csr_g));
            std::vector< int > csr_d(num_vertices(csr_g));
            vertex_csr csr_s = vertex(V, csr_g);

            // Dijkstra 
            // Avvio misurazione tempo
            start = std::chrono::high_resolution_clock::now();    

            dijkstra_shortest_paths_no_color_map(csr_g, csr_s,
                predecessor_map(make_iterator_property_map(
                                    csr_p.begin(), get(boost::vertex_index, csr_g))).
                distance_map(make_iterator_property_map(
                        csr_d.begin(), get(vertex_index, csr_g))).
                weight_map(boost::get(&CsrWeight::w, csr_g)));

            // Stop tempo
            stop = std::chrono::high_resolution_clock::now();

            duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);  
            std::cout << "Duration Dijkstra: " << duration.count() << "\u00B5s" << std::endl; // \u00B5s : Character 'MICRO SIGN'
            r[7] = duration.count();
            
            if (verbose){
                // Stampo i risultati
                std::cout << "Source vertex: " << V << std::endl;
                std::cout << "Distances and parents:" << std::endl;
                graph_traits < csr >::vertex_iterator vi, vend;
                for (tie(vi, vend) = vertices(csr_g); vi != vend; ++vi) {
                    if (csr_d[*vi] == (std::numeric_limits<int>::max)())
                        std::cout << "distance(" << *vi << ") = " << std::setw(7) << "inf, ";
                    else
                        std::cout << "distance(" << *vi << ") = " << std::setw(5) << csr_d[*vi] << ", ";
                    std::cout << "parent(" << *vi << ") = " << csr_p[*vi] << std::endl;
                }
                std::cout << std::endl;
            }
            break;
        }

        default:
        std::cout << "Unsupported data structure " << datastruct << std::endl;
        std::cout << "Available options are L (adj. list), M (adj. matrix) and C (csr)" << std::endl;
        return -1;
        break;
    }    
    
    // // Johnson (tenuto alla fine per evitare interferenza su memoria rilevata)
    // if (all_pairs){
    //     // Alloco Distance Matrix
    //     int **D1 = (int**)malloc(sizeof(int*)*num_nodes);
    //     for (int i = 0; i < num_nodes; i++) 
    //         D1[i] = (int*)malloc(sizeof(int)*num_nodes);
        
    //     switch(datastruct){

    //         ////// Adjaceny List
    //         case 'L':
    //         // Avvio misurazione tempo
    //         start = std::chrono::high_resolution_clock::now();   
    //         // Eseguo Johnson
    //         johnson_all_pairs_shortest_paths(list_g, D1);
    //         // Stop tempo
    //         stop = std::chrono::high_resolution_clock::now();
    //         // Calcolo e stampo tempo di esecuzione
    //         duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);  
    //         std::cout << "Duration Johnson (List): " << duration.count() << "\u00B5s" << std::endl; // \u00B5s : Character 'MICRO SIGN'
    //         r[2] = duration.count();
            
    //         if (verbose){    
    //             // Stampo i risultati
    //             std::cout << "       ";
    //             for (int k = 0; k < num_nodes; ++k)
    //                 std::cout << std::setw(5) << k;
    //             std::cout << std::endl;
    //             for (int i = 0; i < num_nodes; ++i) {
    //                 std::cout << std::setw(3) << i << " -> ";
    //                 for (int j = 0; j < num_nodes; ++j) {
    //                     if (D1[i][j] == (std::numeric_limits<int>::max)())
    //                         std::cout << std::setw(5) << "inf";
    //                     else
    //                         std::cout << std::setw(5) << D1[i][j];
    //                 }
    //                 std::cout << std::endl;
    //             }
    //         }
    //         break;

    //         ////// Adjaceny Matrix
    //         case 'M':
    //         // Avvio misurazione tempo
    //         start = std::chrono::high_resolution_clock::now();   
    //         // Eseguo Johnson
    //         johnson_all_pairs_shortest_paths(matrix_g, D1);
    //         // Stop tempo
    //         stop = std::chrono::high_resolution_clock::now();
    //         // Calcolo e stampo tempo di esecuzione
    //         duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);  
    //         std::cout << "Duration Johnson (Matrix): " << duration.count() << "\u00B5s" << std::endl; // \u00B5s : Character 'MICRO SIGN'
    //         r[5] = duration.count();

    //         if (verbose){    
    //             // Stampo i risultati
    //             std::cout << "       ";
    //             for (int k = 0; k < num_nodes; ++k)
    //                 std::cout << std::setw(5) << k;
    //             std::cout << std::endl;
    //             for (int i = 0; i < num_nodes; ++i) {
    //                 std::cout << std::setw(3) << i << " -> ";
    //                 for (int j = 0; j < num_nodes; ++j) {
    //                     if (D1[i][j] == (std::numeric_limits<int>::max)())
    //                         std::cout << std::setw(5) << "inf";
    //                     else
    //                         std::cout << std::setw(5) << D1[i][j];
    //                 }
    //                 std::cout << std::endl;
    //             }
    //         }
    //         break;


    //         ////// CSR
    //         case 'C':
    //         // Avvio misurazione tempo
    //         start = std::chrono::high_resolution_clock::now();   
    //         // Eseguo Johnson
    //         johnson_all_pairs_shortest_paths(csr_g, D1, 
    //             predecessor_map(make_iterator_property_map(
    //                     csr_p.begin(), get(boost::vertex_index, csr_g))).
    //             distance_map(make_iterator_property_map(
    //                     csr_d.begin(), get(boost::vertex_index, csr_g))).
    //             weight_map(boost::get(&CsrWeight::w, csr_g)));
    //         // Stop tempo
    //         stop = std::chrono::high_resolution_clock::now();
    //         // Calcolo e stampo tempo di esecuzione
    //         duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);  
    //         std::cout << "Duration Johnson (CSR): " << duration.count() << "\u00B5s" << std::endl; // \u00B5s : Character 'MICRO SIGN'
    //         r[8] = duration.count();

    //         if (verbose){    
    //             // Stampo i risultati
    //             std::cout << "       ";
    //             for (int k = 0; k < num_nodes; ++k)
    //                 std::cout << std::setw(5) << k;
    //             std::cout << std::endl;
    //             for (int i = 0; i < num_nodes; ++i) {
    //                 std::cout << std::setw(3) << i << " -> ";
    //                 for (int j = 0; j < num_nodes; ++j) {
    //                     if (D1[i][j] == (std::numeric_limits<int>::max)())
    //                         std::cout << std::setw(5) << "inf";
    //                     else
    //                         std::cout << std::setw(5) << D1[i][j];
    //                 }
    //                 std::cout << std::endl;
    //             }
    //         }
    //         break;

    //         default:
    //         break;

    //     }

    //     // Deallocate D1
    //     for (int i = 0; i < num_nodes; i++)
    //         free(D1[i]); 
    //     free(D1);
        
    // }   

    free(weights);

    // Salvo i risultati su results_real.csv 
    std::ifstream infile;
    infile.open("results_real.csv");
    std::ofstream myfile;
    if(infile) {  // controllo se il csv già esiste: se esiste apro in appendice, altrimenti apro (senza appendice) semplicemente e stampo prima riga
        infile.close();
        myfile.open ("results_real.csv",std::ios_base::app);
    } else {
        infile.close();
        myfile.open ("results_real.csv");
        myfile << "Graph,Number of nodes,Number of edges,Seed,Data structure,Memory Usage (kB),Duration Dijkstra (µs),Duration Johnson (µs),\n";
    }
    switch(datastruct){
        case 'L':
        myfile << filename << "," << num_nodes << "," << num_edges << "," << seed << "," << "Adjacency List"   << "," << r[0] << "," << r[1] << "," << r[2] << "\n" ;
        break;

        case 'M':
        myfile << filename << "," << num_nodes << "," << num_edges << "," << seed << "," << "Adjacency Matrix" << "," << r[3] << "," << r[4] << "," << r[5] << "\n" ;
        break;
        
        case 'C':
        myfile << filename << "," << num_nodes << "," << num_edges << "," << seed << "," << "Csr"              << "," << r[6] << "," << r[7] << "," << r[8] << "\n" ;
        break;
        
        default:
        break;
    }
    myfile.close();
    
    // if (visualize){
    //     std::ofstream fout("figs/"+filename+".dot");
    //     fout << "digraph A {\n"
    //     << "  rankdir=LR\n"
    //     << "size=\"15,10\"\n"   // %immagine alta 5 e larga 3 pollici (con gdpi definirò densità pixel)
    //     << "ratio=\"fill\"\n"
    //     << "edge[style=\"bold\"]\n" << "node[shape=\"circle\"]\n";

    //     graph_traits < list >::edge_iterator ei, ei_end;
    //     for (boost::tie(ei, ei_end) = edges(list_g); ei != ei_end; ++ei)
    //         fout << source(*ei, list_g) << " -> " << target(*ei, list_g) 
    //         << "[slabel=" << get(edge_weight, list_g)[*ei] << "]\n";

    //     fout << "}\n";  
    // }

    return 0;    
}


void process_mem_usage(unsigned long& vm_usage, unsigned long& resident_set, bool diff)
{   
    unsigned long vsize;
    unsigned long rss;
    {
        std::string ignore;
        std::ifstream ifs("/proc/self/stat", std::ios_base::in);
        ifs >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore
                >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore
                >> ignore >> ignore >> vsize >> rss;
    }

    unsigned long page_size_kb = sysconf(_SC_PAGE_SIZE) / 1024; 
    if(diff == 0){  
        resident_set = rss * page_size_kb;
        vm_usage = vsize / 1024;
    }
    else{  
        resident_set = (rss * page_size_kb) - resident_set;
        vm_usage = (vsize / 1024) - vm_usage;
    }
}

