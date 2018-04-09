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
    jaccard_hash = new string[HASH_SIZE];
    jaccard_list = new vector<int> *[HASH_SIZE];
}

SimSearcher::~SimSearcher()
{
}

void SimSearcher::initIndex()
{
    context_jac.clear();
    context_ed.clear();
    ed_list.clear();
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

int SimSearcher::my_hash(const string& str)
{
    return 1;
}

int SimSearcher::findHashTableIndex(const string& str, string* table, bool adding, bool getAll)
{
    int hash_value = my_hash(str);
    int inc = 1;
    while ((!table[hash_value].empty()) && table[hash_value].compare(str))
    {
        hash_value += inc;
        inc *= 2;
        if (hash_value >= HASH_SIZE)
            hash_value = hash_value % HASH_SIZE;
    }
    if (table[hash_value].empty()) {
        if (adding)
            table[hash_value] = str;
        else if (!getAll)
            return -1;
    }
    return hash_value;
}

void SimSearcher::str2HashIndex(const string& str, vector<int>& word, bool adding=true, bool getAll=false)
{
    word.clear();
    set<string> wordstrs;
    split(str, wordstrs, ' ');
    for (auto wordstr : wordstrs)
    {
        int index = findHashTableIndex(wordstr, jaccard_hash, adding, getAll);
        if (index != -1 || getAll)
            word.push_back(index);
    }
    sort(word.begin(), word.end());
    auto iter = unique(word.begin(), word.end());
    word.resize(distance(word.begin(), iter));
}

double jac_baoli(const string &s1, const string &s2)
{
    set<string> l1;
    set<string> l2;
    split(s1, l1, ' ');
    split(s2, l2, ' ');
    int cnt = 0;
    for (auto w : l1)
    {
        if(l2.find(w) != l2.end())
            cnt++;
    }
    return (double)cnt / (double)(l1.size() + l2.size() - cnt);
}

double SimSearcher::compute_jaccard(vector<int> &l1, vector<int> &l2, double threshold)
{
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
}

inline int min_3(int x, int y, int z)
{
    return min(min(x, y), z);
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
        context_jac.push_back(str);
        context_ed.push_back(str);
    }
    line_num = context_jac.size();
    // jacc inverse list
    lines_indexes = new vector<int> *[line_num];
    for (int i = 0; i < line_num; i++)
    {
        vector<int> line_word_indexes;
        str2HashIndex(context_jac[i], line_word_indexes);
        int word_num = line_word_indexes.size();
        lines_indexes[i] = new vector<int>;
        lines_indexes[i]->clear();
        *(lines_indexes[i]) = line_word_indexes;
        if (min_line_size > word_num)
            min_line_size = word_num;
		for (int j = 0; j < word_num; j++)
		{
			int index = line_word_indexes[j];
            if (jaccard_list[index] == NULL)
			{
				jaccard_list[index] = new vector<int>;
				jaccard_list[index]->clear();
			}
			jaccard_list[index]->push_back(i);
			if (jaccard_list[index]->size() > M1)
                M1 = jaccard_list[index]->size();
		}
	}

    // ed inverse list
    q_gram = q;
    for (int i = 0; i < line_num; i++)
    {
        string linestr = context_ed[i];
        int len = linestr.length();
        int pos = 0;
        for (int j = 0; j < len; j++)
        {
            if(linestr[j] == ' ')
            {
                if(pos < j)
                {
                    string word = linestr.substr(pos, j - pos);
                    if (ed_list.find(word) != ed_list.end())
                    {
                        ed_list[word].push_back(i);
                    }
                    else
                    {
                        vector<int> wlist;
                        wlist.clear();
                        ed_list.insert(make_pair(word, wlist));
                    }
                }
                pos = j + 1;
            }
        }
        if (pos < len)
        {
            string word = linestr.substr(pos, len - pos);
            if (ed_list.find(word) != ed_list.end())
            {
                ed_list[word].push_back(i);
            }
            else
            {
                vector<int> wlist;
                wlist.clear();
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
    vector<pair<int, int>> cand_list; // index, list_length
    vector<int> cand_lines;
    vector<int> short_cand_lines;
    int *lines_count = new int[line_num];
    memset(lines_count, 0, sizeof(int) * line_num);
    vector<int> query_indexes;

    str2HashIndex(q_str, query_indexes, false);
    int word_num = query_indexes.size();
    int thres = get_jacc_threshold(threshold, word_num);
    for (auto index : query_indexes)
    {
        cand_list.push_back(make_pair(index, jaccard_list[index]->size()));
    }

    int num_long_list = thres - 1;
    int num_short_list = word_num - num_long_list;

    if (thres <= 0)
    {
        for (int i = 0; i < line_num; i++)
            cand_lines.push_back(i);
    }
    else
    {
        nth_element(cand_list.begin(), cand_list.begin() + num_short_list - 1, cand_list.end());
        for (int i = 0; i < num_short_list; i++)
        {
            int index = cand_list[i].first;
            for (auto line : *(jaccard_list[index]))
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
                int index = cand_list[i].first;
                if (binary_search(jaccard_list[index]->begin(), jaccard_list[index]->end(), line))
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
        query_indexes.clear();
        str2HashIndex(q_str, query_indexes, false, true);
        double jaccard = compute_jaccard(query_indexes, *(lines_indexes[line]), threshold);
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
    int len = q_str.length();
    int pos = 0;
    for (int j = 0; j < len; j++)
    {
        if(q_str[j] == ' ')
        {
            if(pos < j)
            {
                string word = q_str.substr(pos, j - pos);
                if (ed_list.find(word) != ed_list.end())
                {
                    inverted_lists.push_back(ed_list[word]);
                }
            }
            pos = j + 1;
        }
    }
    if (pos < len)
    {
        string word = q_str.substr(pos, len - pos);
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
            if (my_abs(q_str.length() - context_ed[line].length()) > threshold)
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
        int ed = compute_ed(q_str, context_ed[line], threshold, q_gram);
        if (ed <= threshold)
        {
            result.push_back(make_pair(line, ed));
        }
    }
    delete[] lines_count;
    return SUCCESS;
}

