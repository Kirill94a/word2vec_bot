#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#include <iostream>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <map>
#include <set>
#include <vector>
#include <cmath>
#include <algorithm>

#include <fstream>
#include <sstream>

//#define MYDEBUG

using namespace std;

typedef string shown_vertice;
typedef string word_t;
typedef string code_vertice_t;
typedef float dist_t;
typedef float dist_between_two_words_between_graphs_t;
typedef float* vector_t;
typedef tuple<word_t, vector_t> word_vec_t;
typedef vector_t vector_relationship_t;

typedef tuple<code_vertice_t, word_t, vector_t> vertice_t;
typedef tuple<code_vertice_t, code_vertice_t, dist_t> edge_t;

extern const long long max_size;         // max length of strings
extern long long size;
extern const float threshold_closeness_inside_graph;
extern const float threshold_closeness_between_graphs;

float get_f_logistic_function(const float& value);
