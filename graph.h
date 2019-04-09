#include "main.h"

class graph {
  public:
    vector <vertice_t> vertices;
    vector <edge_t> edges;
    vector<tuple<vertice_t*, vertice_t*>> edges_two_words_between_graphs;
    vector<tuple<vertice_t*, vertice_t*, dist_t>> edges_between_graphs;
    vector <tuple<dist_t, dist_t, vector_relationship_t>> vectors_relationship_input_phrase;
    vector <tuple<dist_t, dist_t, vector_relationship_t>> vectors_relationship_base;
    vector <tuple<dist_t, dist_t, dist_between_two_words_between_graphs_t>> dists_between_two_words_between_graphs;
    shown_vertice question;

  public:
    graph(vector<word_vec_t> words_vec, shown_vertice question, string additive = "");
    graph(graph graph1, graph graph2);
    virtual ~graph();
    static void free_ptr (graph& my_graph);
    static void free_ptr_from_vec (vector<graph>& my_vec);
    static float get_closeness_graphs(graph graph1, graph graph2, graph relationship);
    static float get_closeness_graphs_2(graph relationship);

  private:
    void calc_vectors_relationship();
    void calc_dist_between_vectors_relationship();
};
