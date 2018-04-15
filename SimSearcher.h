#pragma once
#include <vector>
#include <utility>
#include <string>
#include <algorithm>
#include <unordered_map>
#include <set>
#include <cstdlib>
#include <fstream>
#include <cmath>
#include <cstring>
#include <iostream>
#include <functional>

const int SUCCESS = 0;
const int FAILURE = 1;

struct TrieNode
{
    bool exist;
    TrieNode *child[128];
    std::vector<int> indexVec;

    TrieNode() : exist(false)
    {
        for (int i = 0; i < 128; i++)
            child[i] = NULL;
        indexVec.clear();
    }
};

struct Trie
{
    TrieNode* root;

    Trie() {
        root = new TrieNode();
    }

    void insert(const char* str, int len, int line) {
        TrieNode *node = root;
        for (int i = 0; i < len; i++) {
            if (!node->child[(int)str[i]]) {
                node->child[(int)str[i]] = new TrieNode();
            }
            node = node->child[(int)str[i]];
        }
        node->exist = true;
        if (node->indexVec.empty() || *(node->indexVec.end() - 1) != line)
            node->indexVec.push_back(line);
    }

    std::vector<int>* search(const char* str, int len) {
        TrieNode *node = root;
        for (int i = 0; i < len; i++) {
            if(!node->child[(int)str[i]])
                return NULL;
            node = node->child[(int)str[i]];
        }
        if (!node)
            return NULL;
        if (!node->exist)
            return NULL;
        return &(node->indexVec);
    }
};


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
  std::unordered_map<int, std::set<unsigned long long>> lines_indexes;

  Trie jaccTrie;
  Trie edTrie;

  SimSearcher();
  ~SimSearcher();

  void initIndex();

  int get_jacc_threshold(double threshold, int queryNum, int lineNum);
  double compute_jaccard(std::set<unsigned long long> &l1, std::set<unsigned long long> &l2, double threshold);

  int get_ed_threshold(double threshold, int query_len, int q);
  unsigned compute_ed(const char* s1, int len1, const char* s2, int len2, unsigned threshold, int q);

  int createIndex(const char *filename, unsigned q);
  int searchJaccard(const char *query, double threshold, std::vector<std::pair<unsigned, double>> &result);
  int searchED(const char *query, unsigned threshold, std::vector<std::pair<unsigned, unsigned>> &result);
};

