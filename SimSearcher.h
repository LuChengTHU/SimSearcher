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
  vector<string> context;

  int line_num;
  int q_gram;

  int min_line_size;
  unordered_map<int, set<string>> lines_indexes;
  unordered_map<string, vector<int>> jaccard_list;

  unordered_map<string, vector<int>> ed_list;

  SimSearcher();
  ~SimSearcher();

  void initIndex();

  int get_jacc_threshold(double threshold, int num);
  double compute_jaccard(set<string> &l1, set<string> &l2, double threshold);

  int get_ed_threshold(double threshold, int query_len, int q);
  unsigned compute_ed(const string &s1, const string &s2, double threshold, int q);

  int createIndex(const char *filename, unsigned q);
  int searchJaccard(const char *query, double threshold, std::vector<std::pair<unsigned, double>> &result);
  int searchED(const char *query, unsigned threshold, std::vector<std::pair<unsigned, unsigned>> &result);
};

