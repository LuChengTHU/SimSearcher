#include "SimSearcher.h"
#include <cstdlib>
#include <fstream>
#include <cmath>
#include <cstring>
#include <functional>
#include <iostream>
#include <set>
using namespace std;

#define HASH_SIZE 1000011
#define MAX_INT 0xffffff
const double u = 0.0085;
double M1 = 0;
double M2 = 0;

struct index_len
{
    int index;
    int len;

    bool operator< (const index_len& idl)
    {
        return len < idl.len;
    }
};

inline bool comp(pair<int, int> a, pair<int, int> b)
{
    return a.second < b.second;
}

inline int my_abs(int a)
{
    if (a > 0)
        return a;
    else
        return -a;
}

inline int my_min(int a, int b)
{
    return a < b ? a : b;
}

inline int my_max(int a, int b)
{
    return a > b ? a : b;
}

inline int min_3(int x, int y, int z)
{
    return min(min(x, y), z);
}

void split(string strtem, set<string>& strvec, char a)
{
    strvec.clear();
    int len = strtem.length();
    int pos = 0;
    for (int i = 0; i < len; i++)
    {
        if(strtem[i] == ' ')
        {
            if(pos < i)
                strvec.insert(strtem.substr(pos, i - pos));
            pos = i + 1;
        }
    }
    if (pos < len)
    {
        strvec.insert(strtem.substr(pos, len - pos));
    }
}

SimSearcher::SimSearcher()
{
}

SimSearcher::~SimSearcher()
{
}

void SimSearcher::initIndex()
{
    context.clear();
    ed_list.clear();
    jaccard_list.clear();
    lines_indexes.clear();
    min_line_size = MAX_INT;
}

int SimSearcher::get_jacc_threshold(double threshold, int num)
{
    double t1 = threshold * num;
    double t2 = (num + min_line_size) * threshold / (1 + threshold);
    return ceil(t1) > ceil(t2) ? ceil(t1) : ceil(t2);
}

int SimSearcher::get_ed_threshold(double threshold, int query_len, int q)
{
    double t1 = query_len - q + 1 - threshold * q;
    return ceil(t1) > 0 ? ceil(t1) : 0;
}

double SimSearcher::compute_jaccard(set<string> &l1, set<string> &l2, double threshold)
{
    int cnt = 0;
    for (auto w : l1)
    {
        if (l2.find(w) != l2.end())
            cnt++;
    }
    return ((double)cnt / (double)(l1.size() + l2.size() - cnt));

    /*
    int len1 = l1.size();
    int h1 = 0;
    int len2 = l2.size();
    int h2 = 0;
    int cnt = 0;
    while (h1 < len1 && h2 < len2)
    {
        if (l1[h1] == l2[h2])
        {
            h1++;
            h2++;
            cnt++;
        }
        else if (l1[h1] > l2[h2])
        {
            h2++;
        }
        else
        {
            h1++;
        }
    }
    return ((double)cnt / (double)(len1 + len2 - cnt));
    */
}



unsigned SimSearcher::compute_ed(const string &str1, const string &str2, double threshold, int q)
{
    int m = str1.length();
    int n = str2.length();
    if (my_abs(m - n) > threshold)
        return MAX_INT;

    int dp[m+1][n+1];
    for (int i = 0; i <= my_min(threshold, m); i++)
    {
        dp[i][0] = i;
    }
    for (int j = 0; j <= my_min(threshold, n); j++)
    {
        dp[0][j] = j;
    }
    for (int i = 1; i <= m; i++)
    {
        int begin = my_max(i - threshold, 1);
        int end = my_min(i + threshold, n);
        if (begin > end)
            break;
        for (int j = begin; j <= end; j++)
        {
            int t = !(str1[i - 1] == str2[j - 1]);
            int d1 = my_abs(i - 1 - j) > threshold ? MAX_INT : dp[i - 1][j];
            int d2 = my_abs(i - j + 1) > threshold ? MAX_INT : dp[i][j - 1];
            dp[i][j] = min_3(
                d1 + 1,
                d2 + 1,
                dp[i - 1][j - 1] + t);
        }
    }
    return dp[m][n];
}

int SimSearcher::createIndex(const char *filename, unsigned q)
{
    initIndex();
    ifstream fin(filename);
    string str;
    while (getline(fin, str))
    {
        context.push_back(str);
    }
    line_num = context.size();

    // jacc inverse list
    for (int i = 0; i < line_num; i++)
    {
        string linestr = context[i];
        int len = linestr.length();
        int pos = 0;
        set<string> linewords;
        linewords.clear();
        for (int j = 0; j < len; j++)
        {
            if(linestr[j] == ' ')
            {
                if(pos < j)
                {
                    string word = linestr.substr(pos, j - pos);
                    linewords.insert(word);
                }
                pos = j + 1;
            }
        }
        if (pos < len)
        {
            string word = linestr.substr(pos, len - pos);
            linewords.insert(word);
        }
        if (min_line_size > linewords.size())
            min_line_size = linewords.size();
        lines_indexes.insert(make_pair(i, linewords));
        for (auto word : linewords)
        {
            if (jaccard_list.find(word) != jaccard_list.end())
            {
                jaccard_list[word].push_back(i);
            }
            else
            {
                vector<int> wlist;
                wlist.clear();
                wlist.push_back(i);
                jaccard_list.insert(make_pair(word, wlist));
            }
        }
    }

    // ed inverse list
    q_gram = q;
    for (int i = 0; i < line_num; i++)
    {
        string linestr = context[i];
        int r = linestr.length() - q + 1;
        set<string> linewords;
        for (int j = 0; j < r; j++)
        {
            string word = linestr.substr(j, q);
            linewords.insert(word);
        }
        for (auto word : linewords)
        {
            if (ed_list.find(word) != ed_list.end())
            {
                ed_list[word].push_back(i);
            }
            else
            {
                vector<int> wlist;
                wlist.clear();
                wlist.push_back(i);
                ed_list.insert(make_pair(word, wlist));
            }
        }
    }
    return SUCCESS;
}

int SimSearcher::searchJaccard(const char *query, double threshold, vector<pair<unsigned, double> > &result)
{
	result.clear();
    string q_str(query);

    vector<int> cand_lines;
    vector<int> short_cand_lines;
    int *lines_count = new int[line_num];
    memset(lines_count, 0, sizeof(int) * line_num);

    vector<vector<int>> inverted_lists;
    vector<index_len> vec_index;
    int len = q_str.length();
    int pos = 0;
    set<string> querywords;
    for (int j = 0; j < len; j++)
    {
        if(q_str[j] == ' ')
        {
            if(pos < j)
            {
                string word = q_str.substr(pos, j - pos);
                querywords.insert(word);
            }
            pos = j + 1;
        }
    }
    if (pos < len)
    {
        string word = q_str.substr(pos, len - pos);
        querywords.insert(word);
    }
    for (auto word : querywords)
    {
        if (jaccard_list.find(word) != jaccard_list.end())
        {
            inverted_lists.push_back(jaccard_list[word]);
        }
    }
    int thres = get_jacc_threshold(threshold, querywords.size());
    int word_num = inverted_lists.size();
    int num_long_list = thres - 1;
    int num_short_list = word_num - num_long_list;
    for (int i = 0; i < word_num; i++)
    {
        index_len idl;
        idl.len = inverted_lists[i].size();
        idl.index = i;
        vec_index.push_back(idl);
    }

    if (thres <= 0)
    {
        for (int i = 0; i < line_num; i++)
            cand_lines.push_back(i);
    }
    else
    {
        sort(vec_index.begin(), vec_index.end());
        for (int i = 0; i < num_short_list; i++)
        {
            int index = vec_index[i].index;
            for (auto line : inverted_lists[index])
            {
                if (lines_count[line] == 0)
                {
                    short_cand_lines.push_back(line);
                }
                lines_count[line]++;
            }
        }
        for (auto line : short_cand_lines)
        {
            int cnt = lines_count[line];
            for (int i = num_short_list; i < word_num; i++)
            {
                int num_unsearch_long_list = word_num - i;
                if (cnt >= thres || cnt + num_unsearch_long_list < thres)
                    break;
                int index = vec_index[i].index;
                if (binary_search(inverted_lists[index].begin(), inverted_lists[index].end(), line))
                {
                    cnt++;
                }
            }
            if (cnt >= thres)
            {
                cand_lines.push_back(line);
            }
        }
    }
    sort(cand_lines.begin(), cand_lines.end());
    for (auto line : cand_lines)
    {
        double jaccard = compute_jaccard(querywords, lines_indexes[line], threshold);
        if (jaccard >= threshold)
        {
            result.push_back(make_pair(line, jaccard));
        }
    }

    delete[] lines_count;
    return SUCCESS;
}

int SimSearcher::searchED(const char *query, unsigned threshold, vector<pair<unsigned, unsigned> > &result)
{
	result.clear();
    string q_str(query);

    vector<int> cand_lines;
    vector<int> short_cand_lines;
    int *lines_count = new int[line_num];
    memset(lines_count, 0, sizeof(int) * line_num);

    vector<vector<int>> inverted_lists;
    vector<index_len> vec_index;
    int r = q_str.length() - q_gram + 1;
    for (int j = 0; j < r; j++)
    {
        string word = q_str.substr(j, q_gram);
        if (ed_list.find(word) != ed_list.end())
        {
            inverted_lists.push_back(ed_list[word]);
        }
    }
    int thres = get_ed_threshold(threshold, q_str.length(), q_gram);
    int word_num = inverted_lists.size();
    int num_long_list = thres - 1;
    int num_short_list = word_num - num_long_list;
    for (int i = 0; i < word_num; i++)
    {
        index_len idl;
        idl.len = inverted_lists[i].size();
        idl.index = i;
        vec_index.push_back(idl);
    }

    if (thres <= 0)
    {
        for (int i = 0; i < line_num; i++)
            cand_lines.push_back(i);
    }
    else
    {
        sort(vec_index.begin(), vec_index.end());
        for (int i = 0; i < num_short_list; i++)
        {
            int index = vec_index[i].index;
            for (auto line : inverted_lists[index])
            {
                if (lines_count[line] == 0)
                {
                    short_cand_lines.push_back(line);
                }
                lines_count[line]++;
            }
        }
        for (auto line : short_cand_lines)
        {
            if (my_abs(q_str.length() - context[line].length()) > threshold)
                continue;
            int cnt = lines_count[line];
            for (int i = num_short_list; i < word_num; i++)
            {
                int num_unsearch_long_list = word_num - i;
                if (cnt >= thres || cnt + num_unsearch_long_list < thres)
                    break;
                int index = vec_index[i].index;
                if (binary_search(inverted_lists[index].begin(), inverted_lists[index].end(), line))
                {
                    cnt++;
                }
            }
            if (cnt >= thres)
            {
                cand_lines.push_back(line);
            }
        }
    }
    sort(cand_lines.begin(), cand_lines.end());
    for (auto line : cand_lines)
    {
        int ed = compute_ed(q_str, context[line], threshold, q_gram);
        if (ed <= threshold)
        {
            result.push_back(make_pair(line, ed));
        }
    }
    delete[] lines_count;
    return SUCCESS;
}

