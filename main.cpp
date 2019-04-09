#include "main.h"
#include "graph.h"

#define READ 0
#define WRITE 1

#define NUM_PIPES          2

#define PARENT_WRITE_PIPE  0
#define PARENT_READ_PIPE   1

int pipes[NUM_PIPES][2];

#define PARENT_READ  ( pipes[PARENT_READ_PIPE][READ]   )
#define PARENT_WRITE ( pipes[PARENT_WRITE_PIPE][WRITE] )

#define CHILD_READ   ( pipes[PARENT_WRITE_PIPE][READ]  )
#define CHILD_WRITE  ( pipes[PARENT_READ_PIPE][WRITE]  )

#define READ_FD  0
#define WRITE_FD 1

#define PARENT_READ_FD  ( pipes[PARENT_READ_PIPE][READ_FD]   )
#define PARENT_WRITE_FD ( pipes[PARENT_WRITE_PIPE][WRITE_FD] )

#define CHILD_READ_FD   ( pipes[PARENT_WRITE_PIPE][READ_FD]  )
#define CHILD_WRITE_FD  ( pipes[PARENT_READ_PIPE][WRITE_FD]  )

const long long max_size = 2000;         // max length of strings
const long long N = 40;                  // number of closest words that will be shown
const long long max_w = 50;              // max length of vocabulary entries

const char* path_questions = "questions.txt";
const char* path_answers = "answers.txt";
const char* path_id = "id.txt";
const char* path_vectors = "vectors.bin";
const char* path_log = "log.txt";
const char* path_prepositions = "prepositions.txt";

const char* command_mystem = "./mystem -indl ";

const char* str_load_questions = "QUESTIONS";

const string space = " ";

clock_t time_start = 0;
clock_t time_stop = 0;
clock_t time_load_questions = 0;

char *vocab;
long long words, size;
float *M;

pid_t my_fork;

string designation_introductory_word = "ADV,вводн";
vector<string> mas_forbidden_parts_of_speech =
                                               {designation_introductory_word,
                                               "SPRO",
                                               "APRO",
                                               "PR",
                                               "CONJ",
                                               "PART",
                                               "INTJ"
                                               };
const vector<string> forbidden_words =
                              {"телевизор"};

const float threshold_closeness = 0.018;
const float threshold_closeness_inside_graph = -1.0;
const float threshold_closeness_between_graphs = 0.1;

int GetDialVec(const char* st, vector_t vec);
void remove_big_spaces(char* str);
void remove_big_spaces_and_prepositions(char* str, const vector <string>& prepositions);
void open_mystem();
void send_to_mystem (const string str);
void receive_from_mystem (char* buf);
int get_count_spaces (const string& str );
bool is_found_number (const string& str);
string get_processed_question(const string& question, bool is_question = true);
void free_ptr_from_words_vec (vector<word_vec_t>& words_vec);

int main(int argc, char **argv) {
  FILE *f;
  char st1[max_size];
  char *bestw[N];
  char st[100][max_size];
  float len, bestd[N];
  long long  a, b, c, cn;
  char c_line[max_size];
  vector<graph> graphs_base;

  ofstream log_file;

  f = fopen(path_vectors, "rb");
  if (f == NULL) {
    printf("Input file not found\n");
    return -1;
  }
  fscanf(f, "%lld", &words);
  fscanf(f, "%lld", &size);
  vocab = (char *)malloc((long long)words * max_w * sizeof(char));
  for (a = 0; a < N; a++) bestw[a] = (char *)malloc(max_size * sizeof(char));
  M = (float *)malloc((long long)words * (long long)size * sizeof(float));
  if (M == NULL) {
    printf("Cannot allocate memory: %lld MB    %lld  %lld\n", (long long)words * size * sizeof(float) / 1048576, words, size);
    return -1;
  }
  for (b = 0; b < words; b++) {
    a = 0;
    while (1) {
      vocab[b * max_w + a] = fgetc(f);
      if (feof(f) || (vocab[b * max_w + a] == ' ')) break;
      if ((a < max_w) && (vocab[b * max_w + a] != '\n')) a++;
    }
    vocab[b * max_w + a] = 0;
    for (a = 0; a < size; a++) fread(&M[a + b * size], sizeof(float), 1, f);
    len = 0;
    for (a = 0; a < size; a++) len += M[a + b * size] * M[a + b * size];
    len = sqrt(len);
    for (a = 0; a < size; a++) M[a + b * size] /= len;
  }
  fclose(f);

  //////
  open_mystem();

  //////////////////////////////////////////////////////
  do{
    time_start = clock();
    vector<tuple<int, vector<tuple<string, float*>>>> mas_vec_question;
    vector<int> ids_questions;
    vector <string> questions_with_parts_of_speech;

    vector<string> questions;
    vector<string> questions_with_prepositions;
    vector<string> answers;

    map<int, vector<int>> questions_answers;

    /////////////////////
    std::ifstream input_prepositions (path_prepositions);
    std::string line;
    vector <string> prepositions;
    while (input_prepositions){
      std::getline(input_prepositions, line);
      if(line.length()!=0){
        prepositions.push_back(line);
      }
    }
    input_prepositions.close();
    ////////////////////

    std::ifstream input_open (path_questions);

    while (input_open){
      std::getline(input_open, line);
      if(line.length()!=0){
        int len_line = line.length();
        for(auto index = 0; index < len_line; ++index){
          c_line[index] = line[index];
        }
        c_line[len_line] = 0;
        remove_big_spaces(c_line);

        questions_with_prepositions.push_back(line);
        questions.push_back(string(c_line));
      }
    }

    if(my_fork > 0) {
      for (auto question : questions){
#ifdef MYDEBUG
        cout << question << endl;
#endif
        questions_with_parts_of_speech.push_back(get_processed_question(question));
      }
      questions = questions_with_parts_of_speech;
    }
#ifdef MYDEBUG
    if(my_fork > 0){
      cout << questions_with_parts_of_speech.size() << endl;
      for (int i = 0; i< questions_with_parts_of_speech.size(); ++i){
        cout << i << ": " << questions_with_parts_of_speech[i] << endl;
      }
    }
#endif
    input_open.close();

    std::ifstream input_open3 (path_id);
     while (input_open3){
      std::getline(input_open3, line);
	  	std::istringstream iss(line);

	  	int id_question = -1;
	  	int id_answer = -1;

	  	iss >> id_question;
	  	iss >> id_answer;

      ids_questions.push_back(id_question);

	  	questions_answers[id_question].push_back(id_answer);
    }
    input_open3.close();
    //вычисление для каждого слова в вопросе отдельного вектора
    //деление строки на слова
    for (int n_question=0;n_question<questions.size();++n_question){
       bool is_all_zero_code = true;
	     char* str = (char*)malloc(strlen(questions[n_question].c_str()) + 1);
	     memcpy(str, questions[n_question].c_str(), strlen(questions[n_question].c_str()) + 1);
	     str[strlen(questions[n_question].c_str())] = 0;

	     char* st2 = (char*)malloc(strlen(questions[n_question].c_str()) + 1);
	     memcpy(st2, questions[n_question].c_str(), strlen(questions[n_question].c_str()) + 1);
	     st2[strlen(questions[n_question].c_str())] = 0;
	     vector<string> str_list;
	     int ppos = 0;
	     int i;
	     for (i = 0; st2[i]; i++){
	     	if (st2[i] == ' ')
	     	{
	     		st2[i] = 0;
	     		if (ppos < i)
	     			str_list.push_back(st2 + ppos);

	     		ppos = i + 1;
	     	}
	     }
	     if (ppos < i){
	     	str_list.push_back(str + ppos);
	     }
	     //вычисление для каждого слова вектор
	     vector<word_vec_t> words_vec;
	     for(int i = 0; i < str_list.size();++i){
	     	char* str2 = (char*)malloc(strlen(str_list[i].c_str()) + 1);
	      memcpy(str2, str_list[i].c_str(), strlen(str_list[i].c_str()) + 1);
	      str2[strlen(str_list[i].c_str())] = 0;
        float p[max_size];
	     	int code = GetDialVec(str2, p);

        if (code == 0){
          words_vec.push_back(make_tuple(string(str2), new float[max_size]));
        for (int i = 0; i < size; i++) get<1>(words_vec[words_vec.size() - 1])[i] = p[i];
        }
        else {
          is_all_zero_code = false;
          break;
        }
	     }
       if(is_all_zero_code){
         mas_vec_question.push_back(make_tuple(ids_questions[n_question], words_vec));
         graphs_base.push_back(graph(words_vec, (shown_vertice)questions_with_prepositions[n_question], "a"));
       }
	     free(str);
	     free(st2);

       free_ptr_from_words_vec (words_vec);
    }

    std::ifstream input_open2 (path_answers);

    while (input_open2){
      std::getline(input_open2, line);
      answers.push_back(line);
    }
    input_open2.close();

    time_stop = clock();
    time_load_questions = time_stop - time_start;
    cout << "Загрузка базы вопросов заняло: " << time_load_questions / 1000.0 << " мс" << endl;

    ////////////////////////////////////////////////////////////////////////////////
    if(my_fork > 0) {
    while (1) {
      for (a = 0; a < N; a++) bestd[a] = 0;
      for (a = 0; a < N; a++) bestw[a][0] = 0;
      printf("Введите предложение: ");
      a = 0;
      while (1) {
        st1[a] = fgetc(stdin);
        if ((st1[a] == '\n') || (a >= max_size - 1)) {
          st1[a] = 0;
          break;
        }
        a++;
      }
      if (!strcmp(st1, "EXIT") || !strcmp(st1, str_load_questions)) break;
      remove_big_spaces(st1);
      string str_st1 = string(st1);
      string processed_str = get_processed_question(str_st1, false);
      for (int i = 0; i < processed_str.length(); ++i){
        st1[i] = processed_str[i];
      }
      st1[processed_str.length()] = 0;
#ifdef MYDEBUG
      cout << "processed_str: " << processed_str << endl;
#endif
      log_file.open (path_log, ios::app);
      log_file << str_st1 << "\n";
      log_file.close();
      cn = 0;
      b = 0;
      c = 0;
      while (1) {
        st[cn][b] = st1[c];
        b++;
        c++;
        st[cn][b] = 0;
        if (st1[c] == 0) break;
        if (st1[c] == ' ') {
          cn++;
          b = 0;
          c++;
        }
      }
      cn++; //cn - количество слов во входной фразе
      vector<tuple<string, float*>> words_vec;
      vector<tuple<int, float>> similarities_with_questions;
      for (a = 0; a < cn; a++) {
	     	float p[max_size];
	     	int code = GetDialVec(st[a], p);
        if(code == 0){
	     	  words_vec.push_back(make_tuple(string(st[a]), new float[max_size]));
          for (int i = 0; i < size; i++) get<1>(words_vec[words_vec.size() - 1])[i] = p[i];
        }
      }
      graph input_phrase = graph(words_vec, (shown_vertice)str_st1, "b");
      vector<graph> relationships;

      for(auto graph_base : graphs_base){
#ifdef MYDEBUG
        cout << graph_base.question << endl;
#endif
        relationships.push_back(graph(graph_base, input_phrase));
      }

      vector<tuple<int, float>> closenesses;
      for(long long index = 0; index < graphs_base.size(); ++index){
        closenesses.push_back(make_tuple(index, graph::get_closeness_graphs_2(relationships[index])));
      }

      std::sort(closenesses.begin(), closenesses.end(),
                                 [](tuple<int, float> first, tuple<int, float> second) {
                                     return (get<1>(first) < get<1>(second));
                                     }
                                  );

      for(long long index = 0; index < closenesses.size(); ++index){
        cout << graphs_base[get<0>(closenesses[index])].question << " ";
        cout << "= " << get<1>(closenesses[index]) << endl;
      }
      cout << string(10, '-') << endl;

      if (b == -1) continue;
#if FALSE
      for (a = 0; a < cn; a++) {
        string best_question;
        float best_dist = -1;
        for (int a2 = 0; a2 < cn; a2++) {
          if(a != a2){
            dist = 0;
            for(long long a3 = 0; a3 < size; a3++){
              dist += get<1>(words_vec[a])[a3] * get<1>(words_vec[a2])[a3];
            }
            if(dist > best_dist){
              best_dist = dist;
              best_question = get<0>(words_vec[a2]);
            }
          }
        }
      }

      float best_dist = -1;
      vector<vector<tuple<string, float>>> mas_dists;
      for(int i = 0; i < mas_vec_question.size(); ++i){
        vector<tuple<string, float>> temp_mas_dists;
        for(int k = 0; k < words_vec.size(); ++k){
          best_dist = -1;
          string best_question;
          for(int j = 0; j < get<1>(mas_vec_question[i]).size(); ++j){
            dist = 0;
            for (a = 0; a < size; a++) dist += get<1>(get<1>(mas_vec_question[i])[j])[a] * get<1>(words_vec[k])[a];
            ///////////////////
            if(dist > best_dist){
              best_dist = dist;
              best_question = get<0>(get<1>(mas_vec_question[i])[j]);
            }
          }
          temp_mas_dists.push_back(make_tuple(best_question, best_dist));
        }
        ///////////
      float expected_value = 0;
      float standard_deviation = 0;

      for (int q = 0; q < temp_mas_dists.size(); q++){
        expected_value+=get<1>(temp_mas_dists[q]);
      }
      expected_value /= temp_mas_dists.size();

      for (int q = 0; q < temp_mas_dists.size(); q++){
        standard_deviation+=pow(expected_value - get<1>(temp_mas_dists[q]), 2.0);
      }
      standard_deviation /= temp_mas_dists.size();
      standard_deviation = sqrt(standard_deviation);
      similarities_with_questions.push_back(make_tuple(i, 0));
      int count = 0;
      for (int q = 0; q < temp_mas_dists.size(); q++){
        if(get<1>(temp_mas_dists[q]) >= (expected_value + standard_deviation)){
          ++count;
          get<1>(similarities_with_questions[similarities_with_questions.size() - 1]) += (get<1>(temp_mas_dists[q])+float(1))/float(2);
        }
      }
      if(count == 0){
        get<1>(similarities_with_questions[similarities_with_questions.size() - 1]) = 0;
      }
      else{
        get<1>(similarities_with_questions[similarities_with_questions.size() - 1]) /= count;
      }
      //////////////////////
        mas_dists.push_back(temp_mas_dists);
      }
      std::sort(similarities_with_questions.begin(), similarities_with_questions.end(),
                                 [](tuple<int, float> first, tuple<int, float> second) {
                                     return (get<1>(first) < get<1>(second));
                                     }
                                  );
#endif
      graph::free_ptr_from_vec(relationships);
      free_ptr_from_words_vec (words_vec);
    }
  }
    graph::free_ptr_from_vec(graphs_base);
    graphs_base.clear();
  }while(strcmp(st1, "EXIT") != 0);

  return 0;
}

int GetDialVec(const char* st, float* vec){
  long long  a, b, bi;
  float len;

  for (b = 0; b < words; b++) if (!strcmp(&vocab[b * max_w], st)) break;
  if (b == words) b = -1;
  bi = b;
  if (b == -1) {
    return 1;
  }
  for (a = 0; a < size; a++) vec[a] = 0;
  for (a = 0; a < size; a++) vec[a] += M[a + bi * size];
  len = 0;
  for (a = 0; a < size; a++) len += vec[a] * vec[a];
  len = sqrt(len);
  for (a = 0; a < size; a++) vec[a] /= len;

  return 0;
}

void remove_big_spaces(char* str){
  string phrase = string(str);
  int j = 1;
  while(true){
    if(phrase.length() == 0){ break;}
    if(j == phrase.length()){break;}
    if(phrase[j] == ' ' && phrase[j - 1] == ' '){
      phrase.erase(phrase.begin() + j);
    }
    else{
      ++j;
    }
  }
  //убираем пробел в начале строки
  auto i = phrase.find(space);
  if(i != string::npos){
    if(i == 0){
      phrase.erase(phrase.begin() + i);
    }
  }
  //убираем пробел в конце строки
  i = phrase.find_last_of(space);
  if(i != string::npos){
    if(i == phrase.length() - 1){
      phrase.erase(phrase.begin() + phrase.length() - 1);
    }
  }

  for(int index = 0; index < phrase.length(); ++index){
    str[index] = phrase[index];
  }
  str[phrase.length()] = 0;
}

int get_count_spaces (const string& str ){
  int count = 0;
  for(auto ch : str){
    if(ch == ' '){
      ++count;
    }
  }
  return count;
}

bool is_found_number (const string& str){
  bool is_found = false;
  for(auto ch : str){
    if(ch >= '0' && ch <= '9'){
      is_found = true;
      break;
    }
  }
  return is_found;
}

void remove_prepositions(char* str, const vector <string>& prepositions){
  string phrase = string(str);
  for(auto preposition : prepositions){
    while(true){
      //удаляем предлог в начале строки
      auto index = phrase.find(preposition + " ");
      if(index != string::npos && index == 0){
        phrase.erase(phrase.begin(), phrase.begin() + preposition.length() + 1);
      }
      else {
        //удаляем предлог в конце строки
        index = phrase.rfind(" " + preposition);

        if(index != string::npos && index == (phrase.length() - (1 + preposition.length()))){
          phrase.erase(phrase.begin() + index, phrase.end());
        }
        else{
          //удаляем предлог в середине строки
          index = phrase.find(" " + preposition + " ");

          if(index != string::npos){
            phrase.erase(phrase.begin() + index, phrase.begin() + index + (1 + preposition.length()));
          }
          else{ break; }
        }
      }
    }
  }
  for(int index = 0; index < phrase.length(); ++index){
    str[index] = phrase[index];
  }
  str[phrase.length()] = 0;
}

void remove_big_spaces_and_prepositions(char* str, const vector <string>& prepositions){
  remove_big_spaces(str);
  remove_prepositions(str, prepositions);
}

int get_index_question_mark (const string& str){
  int index = 0;
  char question_mark = '?';
  index = str.find(question_mark);
  if (index == string::npos){
    index = -1;
  }
  return index;
}

void open_mystem(){
    // pipes for parent to write and read
    pipe(pipes[PARENT_READ_PIPE]);
    pipe(pipes[PARENT_WRITE_PIPE]);
    my_fork = fork();
    if(my_fork == 0) {
        char *argv[]={ (char *)"./mystem",  (char *)"-indl", 0 };

        dup2(CHILD_READ_FD, STDIN_FILENO);
        dup2(CHILD_WRITE_FD, STDOUT_FILENO);

        /* Close fds not required by child. Also, we don't
           want the exec'ed program to know these existed */
        close(CHILD_READ_FD);
        close(CHILD_WRITE_FD);
        close(PARENT_READ_FD);
        close(PARENT_WRITE_FD);

        execv(argv[0], argv);
    } else {
        /* close fds not required by parent */
        close(CHILD_READ_FD);
        close(CHILD_WRITE_FD);
    }
}

void send_to_mystem (const string str){
  write(PARENT_WRITE_FD, (str+"\n").c_str(), strlen((str+"\n").c_str()));
}

vector<string> get_mas_words_from_mystem(const int& count_spaces){
  int count_line_break = 0;
  int ret;
  vector <string> mas_words;
  char ch;
  string str;
	while ((count_line_break < (count_spaces + 1)) && (ret = read (PARENT_READ_FD, &ch, 1)) > 0)
	{
    if(int(ch) == 10){
      ++count_line_break;
      mas_words.push_back(str);
      str.clear();
#ifdef MYDEBUG
      cout << count_line_break << endl;
#endif
    }
    else{
      str += ch;
    }
	}
#ifdef MYDEBUG
  cout << endl;
#endif
  return mas_words;
}

bool is_found_forbidden_part_of_speech (const string& str){
  bool is_found = false;
  for (auto part_of_speech : mas_forbidden_parts_of_speech){
    if (str.find(part_of_speech) != string::npos){
      is_found = true;
      break;
    }
  }
  return is_found;
}

bool is_found_forbidden_word (const string& str){
  bool is_found = false;
  for (auto word : forbidden_words){
    if(str == word){
      is_found = true;
      break;
    }
  }
  return is_found;
}

string get_question_with_parts_of_speech (const vector<string>& mas_words, bool is_question){
  char equal_sign = '=';
  char comma_sign = ',';
  char space_sign = ' ';
  char underscore_sign = '_';

  string unknown_part_of_speech = "UNKN";

  string part_of_speech;
  string question_with_parts_of_speech;

  bool found_forbidden_part_of_speech = false;

  int count_words = 0;

  for (int i = 0; i < mas_words.size(); ++i){
    found_forbidden_part_of_speech = false;
    string word = mas_words[i];
    int index_question_mark = get_index_question_mark(word);
    if(index_question_mark == string::npos){
      if(!is_found_forbidden_part_of_speech (word)){
        int index_first_equal = word.find(equal_sign);
        int index_second_equal = word.find(equal_sign, index_first_equal + 1);
        int index_comma = word.find(comma_sign);

        int left_border = index_first_equal;
        int right_border =  index_comma == string::npos ? index_second_equal :  index_second_equal == string::npos ? word.length() : std::min(index_comma, index_second_equal);
        int index_introductory_word = word.find(designation_introductory_word);
        if(index_introductory_word != string::npos){
          part_of_speech = designation_introductory_word;
        }
        else{
          part_of_speech = word.substr(left_border + 1, right_border - (left_border + 1));
        }
        if(is_question || !is_found_forbidden_word (word.substr(0, left_border))){
          if(count_words > 0){
            question_with_parts_of_speech += space_sign;
          }

          question_with_parts_of_speech += word.substr(0, left_border);
          question_with_parts_of_speech += underscore_sign + part_of_speech;

          ++count_words;
        }
#ifdef MYDEBUG
        cout << word.substr(0, left_border) << " " << part_of_speech << endl;
#endif
      }
#ifdef MYDEBUG
      else{
        cout << "forbidden_part_of_speech: " << word << endl;
      }
#endif
    }
    else{
      if(count_words > 0){
        question_with_parts_of_speech += space_sign;
      }
      question_with_parts_of_speech += word.substr(0, index_question_mark);
      question_with_parts_of_speech += underscore_sign + unknown_part_of_speech;

      ++count_words;
    }
  }
  return question_with_parts_of_speech;
}

string get_str_tolower_case (const string& str){
  string new_str;
  for (auto c : str){
    new_str += tolower(c);
  }
  return new_str;
}

string get_string_without_extra_characters(const string& str){
  string new_str;
  int c_int = 0;
  for (int i = 0; i < str.length(); ++i){
    char c = str[i];
    if(c >= '!' && c <= '/' || c >= ':' && c <= '@' || c >= '[' && c <= '`'){
      new_str += ' ';
    }
    else{
      if((c >= 'A' && c <= 'z') || (c >= '0' && c <= '9')){
       new_str += c;
      }
      else{
        if(i < (str.length() - 1)){
          c_int=int((unsigned char)str[i+1]) + (int((unsigned char)c) << 8);
          if(!(c_int >= 'А' && c_int <= 'я' || c_int == 'Ё' || c_int == 'ё')){
            new_str += ' ';
          }
          else{
            new_str += str[i];
            ++i;
            new_str += str[i];
          }
        }
      }
    }
  }
  char c_new_str[new_str.length() + 1];
  for (int i = 0; i < new_str.length(); ++i){
    c_new_str[i] = new_str[i];
  }
  c_new_str[new_str.length()] = 0;
  remove_big_spaces(c_new_str);
  new_str = string(c_new_str);

  return new_str;
}

string get_processed_question(const string& question, bool is_question){
  string processed_question;
  string new_question;
  new_question = get_str_tolower_case (question);
  new_question = get_string_without_extra_characters (new_question);
  if(!is_found_number(new_question)){
    int count_spaces = get_count_spaces (new_question);
    send_to_mystem(new_question);
    vector<string> mas_words = get_mas_words_from_mystem(count_spaces);
    processed_question= get_question_with_parts_of_speech(mas_words, is_question);
  }
  else{
    processed_question = new_question;
  }
  return processed_question;
}

float get_f_logistic_function(const float& value){
  return value/(1.0 + abs(value));
}

void free_ptr_from_words_vec (vector<word_vec_t>& words_vec){
  for (auto& vec : words_vec){
    delete[](get<1>(vec));
  }
}
