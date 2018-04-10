#pragma once
#include <vector>
#include <utility>
#include <string>
#include <algorithm>
#include <unordered_map>
#include <set>

const int SUCCESS = 0;
const int FAILURE = 1;

class SimSearcher
{
public:
  std::vector<std::string> context;

  int line_num;
  int global_time;
  int *lines_count;
  int *time_count;
  int q_gram;

  int min_line_size;
  std::unordered_map<int, std::set<std::string>> lines_indexes;
  std::unordered_map<std::string, std::vector<int>> jaccard_list;

  std::unordered_map<unsigned long long, std::vector<int>> ed_list;

  SimSearcher();
  ~SimSearcher();

  void initIndex();

  int get_jacc_threshold(double threshold, int num);
  double compute_jaccard(std::set<std::string> &l1, std::set<std::string> &l2, double threshold);

  int get_ed_threshold(double threshold, int query_len, int q);
  unsigned compute_ed(const std::string &s1, const std::string &s2, double threshold, int q);

  int createIndex(const char *filename, unsigned q);
  int searchJaccard(const char *query, double threshold, std::vector<std::pair<unsigned, double>> &result);
  int searchED(const char *query, unsigned threshold, std::vector<std::pair<unsigned, unsigned>> &result);
};

