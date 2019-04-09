#include "graph.h"

graph::graph(vector<word_vec_t> words_vec, shown_vertice question, string additive){
   float dist = 0;
   this->question = question;
   for (unsigned long long i = 0; i < words_vec.size(); i++) {
     vertices.push_back(make_tuple(to_string(i) + additive, get<0>(words_vec[i]), new float[max_size]));
     for(long long j = 0; j < size; ++j) get<2>(vertices[vertices.size() - 1])[j] =  get<1>(words_vec[i])[j];
        long long best_word = 0;
        float best_dist = -1;
        for (unsigned long long j = 0; j < words_vec.size(); j++) {
          if(i != j){
            dist = 0;
            for(long long k = 0; k < size; k++){
              dist += get<1>(words_vec[i])[k] * get<1>(words_vec[j])[k];
            }
            if(dist > best_dist){
              best_dist = dist;
              best_word = j;
            }
          }
        }
      if(
        best_dist >= threshold_closeness_inside_graph){
        edges.push_back(make_tuple(get<0>(vertices[vertices.size() - 1]), to_string(best_word) + additive, best_dist));
        edges.push_back(make_tuple(to_string(best_word) + additive, get<0>(vertices[vertices.size() - 1]), best_dist));
      }
    }
}

graph::graph(graph graph_base, graph graph_input_phrase){
      float dist = 0;
      vector <tuple<int, int, dist_t>> relationships;
      for (unsigned long long i = 0; i < graph_input_phrase.vertices.size(); i++) {
        for (unsigned long long j = 0; j < graph_base.vertices.size(); j++) {
          dist = 0;
          for(long long k = 0; k < size; k++){
            dist += get<2>(graph_input_phrase.vertices[i])[k] * get<2>(graph_base.vertices[j])[k];
          }
          relationships.push_back(make_tuple(i, j, dist));
        }
      }
      std::sort(relationships.begin(), relationships.end(),
                                 [](tuple<int, int, float> first, tuple<int, int, float> second) {
                                     return (get<2>(first) > get<2>(second));
                                     }
                                  );
      vector<int> marked_index_input_phrase;
      vector<int> marked_index_base;

      for(auto relationship : relationships){
        bool is_marked_input_phrase = find(marked_index_input_phrase.begin(), marked_index_input_phrase.end(), get<0>(relationship)) != marked_index_input_phrase.end();
        bool is_marked_base = find(marked_index_base.begin(), marked_index_base.end(), get<1>(relationship)) != marked_index_base.end();

        if((!is_marked_base && !is_marked_input_phrase)){ //если слова не были помечены
         if(get<2>(relationship) >= threshold_closeness_between_graphs){
            edges.push_back(make_tuple(get<0>(graph_input_phrase.vertices[get<0>(relationship)]), get<0>(graph_base.vertices[get<1>(relationship)]), get<2>(relationship)));
            edges.push_back(make_tuple(get<0>(graph_base.vertices[get<1>(relationship)]), get<0>(graph_input_phrase.vertices[get<0>(relationship)]), get<2>(relationship)));

            marked_index_base.push_back(get<1>(relationship));
            marked_index_input_phrase.push_back(get<0>(relationship));

            edges_between_graphs.push_back(make_tuple(&graph_input_phrase.vertices[get<0>(relationship)], &graph_base.vertices[get<1>(relationship)], get<2>(relationship)));

            edges_two_words_between_graphs.push_back(make_tuple(&graph_input_phrase.vertices[get<0>(relationship)], &graph_base.vertices[get<1>(relationship)]));
#ifdef MYDEBUG
            cout << get<1>(graph_input_phrase.vertices[get<0>(relationship)]) << " " << get<2>(relationship) << ", ";
#endif
          }
        }
      }
#ifdef MYDEBUG
      cout << endl;
#endif
   calc_vectors_relationship();
   calc_dist_between_vectors_relationship();
}

graph::~graph(){
  edges_two_words_between_graphs.clear();
    edges_between_graphs.clear();
    dists_between_two_words_between_graphs.clear();
    edges.clear();
    vertices.clear();
    vectors_relationship_input_phrase.clear();
    vectors_relationship_base.clear();
}

void graph::free_ptr (graph& my_graph){
    for(unsigned int i = 0; i<my_graph.vertices.size(); ++i){
      delete[](get<2>(my_graph.vertices[i]));
    }
    for(unsigned int i = 0; i<my_graph.vectors_relationship_input_phrase.size(); ++i){
      delete[](get<2>(my_graph.vectors_relationship_input_phrase[i]));
    }
    for(unsigned int i = 0; i<my_graph.vectors_relationship_base.size(); ++i){
      delete[](get<2>(my_graph.vectors_relationship_base[i]));
    }
    my_graph.edges_between_graphs.clear();
    my_graph.dists_between_two_words_between_graphs.clear();
    my_graph.edges.clear();

    my_graph.vertices.clear();
    my_graph.vectors_relationship_input_phrase.clear();
    my_graph.vectors_relationship_base.clear();
}

void graph::free_ptr_from_vec (vector<graph>& my_vec){
  for (auto& my_graph : my_vec){
    graph::free_ptr(my_graph);
   }
}

void graph::calc_vectors_relationship(){
#ifdef MYDEBUG
  cout << edges_between_graphs.size() << endl;
#endif
  float len = 0;
  if(edges_between_graphs.size() > 1){
    for(unsigned long long i = 0; i < edges_between_graphs.size() - 1; ++i){
      for(unsigned long long j = 1; j < edges_between_graphs.size(); ++j){
        float vec[max_size];
        for (long long k = 0; k < size; ++k){
          vec[k] = get<2>(*get<0>(edges_between_graphs[i]))[k] + get<2>(*get<0>(edges_between_graphs[j]))[k];
        }
        len = 0;
        for (long long k = 0; k < size; ++k) len += vec[k] * vec[k];
        len = sqrt(len);
        for (long long k = 0; k < size; ++k) vec[k] /= len;
        vectors_relationship_input_phrase.push_back(make_tuple(get<2>(edges_between_graphs[i]), get<2>(edges_between_graphs[j]), new float[max_size]));
        for (long long k = 0; k < size; ++k){
          get<2>(vectors_relationship_input_phrase[vectors_relationship_input_phrase.size() - 1])[k] = vec[k];
        }
      }
    }
    for(unsigned long long i = 0; i < edges_between_graphs.size() - 1; ++i){
      for(unsigned long long j = 1; j < edges_between_graphs.size(); ++j){
        float vec[max_size];
        for (long long k = 0; k < size; ++k){
          vec[k] = get<2>(*get<1>(edges_between_graphs[i]))[k] + get<2>(*get<1>(edges_between_graphs[j]))[k];
        }
        len = 0;
        for (long long k = 0; k < size; ++k) len += vec[k] * vec[k];
        len = sqrt(len);
        for (long long k = 0; k < size; ++k) vec[k] /= len;
#ifdef MYDEBUG
        cout << get<2>(edges_between_graphs[i]) << " " << get<2>(edges_between_graphs[j]) << endl;
#endif
        vectors_relationship_base.push_back(make_tuple(get<2>(edges_between_graphs[i]), get<2>(edges_between_graphs[j]), new float[max_size]));
        for (long long k = 0; k < size; ++k){
          get<2>(vectors_relationship_base[vectors_relationship_base.size() - 1])[k] = vec[k];
        }
      }
    }
  }
}

void graph::calc_dist_between_vectors_relationship(){
#ifdef MYDEBUG
  cout << vectors_relationship_input_phrase.size() << endl;
#endif
  float dist = 0;
  for(unsigned long long i = 0; i < vectors_relationship_input_phrase.size(); ++i){
    dist = 0;
    for (long long k = 0; k < size; ++k){
      dist += get<2>(vectors_relationship_input_phrase[i])[k] * get<2>(vectors_relationship_base[i])[k];
    }
#ifdef MYDEBUG
    cout << dist <<endl;
#endif
    dists_between_two_words_between_graphs.push_back(make_tuple(get<0>(vectors_relationship_input_phrase[i]), get<1>(vectors_relationship_base[i]), dist));
  }
#ifdef MYDEBUG
  cout << "dist: " << dist<< endl;
#endif
}

float graph::get_closeness_graphs_2(graph relationship){
  float closeness = 0;
#ifdef MYDEBUG
  cout << "Веса: " << endl;
#endif
  for(auto dist : relationship.dists_between_two_words_between_graphs){
#ifdef MYDEBUG
    printf("(%f + %f) * %f\n", get<0>(dist), get<1>(dist), get<2>(dist));
#endif
    closeness += (get<0>(dist)+get<1>(dist)) * get<2>(dist);
  }
  if(relationship.dists_between_two_words_between_graphs.size() > 0){
    closeness = get_f_logistic_function(closeness);
  }
  else{
    if(relationship.edges_between_graphs.size() > 0){
      closeness = get_f_logistic_function(get<2>(relationship.edges_between_graphs[0]));
    }
    else{
      closeness = -1.0;
    }
  }
#ifdef MYDEBUG
  cout << closeness << endl;
#endif
  return closeness;
}

float graph::get_closeness_graphs(graph graph1, graph graph2, graph relationship){
  float closeness = 0;
  float closeness_vertice = 1;
  int count = 0;
  int count_sum = 0;
  int count_relationships = 0;
  cout << endl;
  for(auto vertice : graph1.vertices){
    closeness_vertice = 1;
    count = 0;
    for(auto edge : graph1.edges){
      if(get<0>(edge) == get<0>(vertice)){
        closeness_vertice *= closeness_vertice < 0 ? (-abs(get<2>(edge))) : get<2>(edge);
        ++count;
      }
    }
    for(auto edge : relationship.edges){
      if(get<0>(edge) == get<0>(vertice)){
        closeness_vertice *= 2.0 * closeness_vertice < 0 ? (-abs(get<2>(edge))) : get<2>(edge);
        ++count_relationships;
        ////////////////
        for(auto vertice2 : graph2.vertices){
          if(get<1>(edge) == get<0>(vertice2)){
            cout << get<1>(vertice) << ":" << get<1>(vertice2) << "=" << get<2>(edge) << endl;
          }
        }

        ////////////////
        ++count;
      }
    }
    ++count_sum;
    if(count != 0){
      closeness += 1.0/(pow(count, 2)*closeness_vertice);
    }
    cout << closeness << endl;
  }

  for(auto vertice : graph2.vertices){
    count = 0;
    closeness_vertice = 1;
    for(auto edge : graph2.edges){
      if(get<0>(edge) == get<0>(vertice)){
        closeness_vertice *= closeness_vertice < 0 ? (-abs(get<2>(edge))) : get<2>(edge);
        ++count;
      }
    }
    for(auto edge : relationship.edges){
      if(get<0>(edge) == get<0>(vertice)){
        closeness_vertice *= 2.0 * closeness_vertice < 0 ? (-abs(get<2>(edge))) : get<2>(edge);
        ++count;
        ++count_relationships;
      }
    }
    ++count_sum;
    if(count != 0){
      closeness += 1.0/(pow(count, 2)*closeness_vertice);
    }
  }
  return 1.0/(closeness) * ((count_relationships > 0) ? 1.0 : 0.0) * ((closeness == 0) ? 0.0 : 1.0);
}
