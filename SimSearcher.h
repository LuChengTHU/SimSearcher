#pragma once
#include <vector>
#include <utility>
#include <string>
#include <algorithm>
#include <unordered_map>
#include <set>

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
  unordered_map<string, vector<int>> ed_list;

  SimSearcher();
  ~SimSearcher();

  void initIndex();
  int my_hash(const string& str);
  int findHashTableIndex(const string& str, string *table, bool adding, bool getAll);

  void str2HashIndex(const string& str, vector<int> &word, bool adding, bool getAll);
  int get_jacc_threshold(double threshold, int num);
  double compute_jaccard(vector<int> &l1, vector<int> &l2, double threshold);

  int get_ed_threshold(double threshold, int query_len, int q);
  unsigned compute_ed(const string &s1, const string &s2, double threshold, int q);

  int createIndex(const char *filename, unsigned q);
  int searchJaccard(const char *query, double threshold, std::vector<std::pair<unsigned, double>> &result);
  int searchED(const char *query, unsigned threshold, std::vector<std::pair<unsigned, unsigned>> &result);
};

