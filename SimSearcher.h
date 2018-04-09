#pragma once
#include <vector>
#include <utility>
#include <string>
#include <algorithm>
#include <map>

using namespace std;
const int SUCCESS = 0;
const int FAILURE = 1;

class SimSearcher
{
public:
  vector<string> context_jac;
  vector<string> context_ed;

  int line_num;
  int q_gram;

  int min_line_size;
  vector<int> **lines_indexes;
  vector<int> **jaccard_list;
  string *jaccard_hash;

  int min_line_gram_size;
  vector<int> **ed_list;
  string *ed_hash;

  SimSearcher();
  ~SimSearcher();

  void initIndex();
  int my_hash(string str);
  int findHashTableIndex(string str, string *table, bool adding, bool getAll);

  void str2HashIndex(string str, vector<int> &word, bool adding, bool getAll);
  int get_jacc_threshold(double threshold, int num);
  double compute_jaccard(vector<int> &l1, vector<int> &l2, double threshold);

  void str2QGramHashIndex(string str, int q, vector<int> &word, bool adding, bool getAll);
  int get_ed_threshold(double threshold, int query_len, int q);
  unsigned compute_ed(const string &s1, const string &s2, double threshold, int q);

  int createIndex(const char *filename, unsigned q);
  int searchJaccard(const char *query, double threshold, std::vector<std::pair<unsigned, double>> &result);
  int searchED(const char *query, unsigned threshold, std::vector<std::pair<unsigned, unsigned>> &result);
};

