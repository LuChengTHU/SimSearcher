#include "SimSearcher.h"
#include <cstdlib>
#include <fstream>
#include <cmath>
#include <cstring>
#include <functional>
#include <iostream>
using namespace std;

#define HASH_SIZE 1000011
const double u = 0.0085;
double M1 = 0;
double M2 = 0;


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

void split(string strtem, vector<string>& strvec, char a)
{
    strvec.clear();
    string::size_type pos1, pos2;
    pos2 = strtem.find(a);
    pos1 = 0;
    while (string::npos != pos2)
    {
            strvec.push_back(strtem.substr(pos1, pos2 - pos1));
            pos1 = pos2 + 1;
            pos2 = strtem.find(a, pos1);
    }
    strvec.push_back(strtem.substr(pos1));
}

SimSearcher::SimSearcher()
{
    jaccard_hash = new string[HASH_SIZE];
    jaccard_list = new vector<int> *[HASH_SIZE];
    ed_hash = new string[HASH_SIZE];
    ed_list = new vector<int> *[HASH_SIZE];
}

SimSearcher::~SimSearcher()
{
}

void SimSearcher::initIndex()
{
    context_jac.clear();
    context_ed.clear();
    //memset(jaccard_hash, 0, sizeof(string) * HASH_SIZE);
    //memset(ed_hash, 0, sizeof(string) * HASH_SIZE);
    for (int i = 0; i < HASH_SIZE; i++)
    {
        jaccard_hash[i] = "";
        ed_hash[i] = "";
    }
    for (int i = 0; i < HASH_SIZE; i++)
    {
        jaccard_list[i] = NULL;
        ed_list[i] = NULL;
    }
    min_line_size = 1 << 30;
    min_line_gram_size = 1 << 30;
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

int SimSearcher::my_hash(string str) {
    std::hash<std::string> hash_fn;
    size_t str_hash = hash_fn(str);
    return str_hash % HASH_SIZE;
}

int SimSearcher::findHashTableIndex(string str, string* table, bool adding)
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
        else
            return -1;
    }
    return hash_value;
}

void SimSearcher::str2HashIndex(string str, vector<int>& word, bool adding=true)
{
    word.clear();
    vector<string> wordstr;
    split(str, wordstr, ' ');
    int wordstr_size = wordstr.size();
    for (int i = 0; i < wordstr_size; i++)
    {
        int index = findHashTableIndex(wordstr[i], jaccard_hash, adding);
        if (index != -1)
            word.push_back(index);
    }
    sort(word.begin(), word.end());
    auto iter = unique(word.begin(), word.end());
    word.resize(distance(word.begin(), iter));
}

void SimSearcher::str2QGramHashIndex(string str, int q, vector<int>& word, bool adding=true)
{
    word.clear();
    int r = str.length() - q + 1;
    for (int j = 0; j < r; j++)
    {
        int index = findHashTableIndex(str.substr(j, q), ed_hash, adding);
        if (index != -1)
            word.push_back(index);
    }
    sort(word.begin(), word.end());
    auto iter = unique(word.begin(), word.end());
    word.resize(distance(word.begin(), iter));
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
    int dp[m+1][n+1];
    for (int i = 0; i <= m; i++)
    {
        dp[i][0] = i;
    }
    for (int j = 0; j <= n; j++)
    {
        dp[0][j] = j;
    }
    for (int i = 1; i <= m; i++)
    {
        for (int j = 1; j <= n; j++)
        {
            int t = !(str1[i] == str2[j]);
            dp[i][j] = min_3(
                dp[i - 1][j] + 1,
                dp[i][j - 1] + 1,
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
        vector<int> line_gram_indexes;
        str2QGramHashIndex(context_ed[i], q, line_gram_indexes);
        int word_num = line_gram_indexes.size();
        if (min_line_gram_size > word_num)
            min_line_gram_size = word_num;
        for (int j = 0; j < word_num; j++)
        {
            int index = line_gram_indexes[j];
            if (ed_list[index] == NULL)
            {
                ed_list[index] = new vector<int>;
                ed_list[index]->clear();
            }
            ed_list[index]->push_back(i);
            if (ed_list[index]->size() > M2)
                M2 = ed_list[index]->size();
        }
    }
    return SUCCESS;
}

int SimSearcher::searchJaccard(const char *query, double threshold, vector<pair<unsigned, double> > &result)
{
	result.clear();

    /*
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

    int num_long_list = (double)thres / (u * log(M1) + 1.0);
    num_long_list = num_long_list > thres - 1 ? (thres - 1) : num_long_list;
    int num_short_list = word_num - num_long_list;

    if (thres <= 0)
    {
        for (int i = 0; i < line_num; i++)
            cand_lines.push_back(i);
    }
    else if (num_short_list <= 0 || thres <= 0)
    {
        for (auto index : query_indexes)
        {
            for (auto line : *(jaccard_list[index]))
            {
                lines_count[line]++;
            }
        }
        for (int i = 0; i < line_num; i++)
        {
            if(lines_count[i] >= thres)
            {
                cand_lines.push_back(i);
            }
        }
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
        double jaccard = compute_jaccard(query_indexes, *(lines_indexes[line]), threshold);
        if (jaccard >= threshold)
        {
            result.push_back(make_pair(line, jaccard));
        }
    }

    delete[] lines_count;
    */

    for (int i = 0; i < line_num; i++)
    {
        vector<int> query_indexes;
        string q_str(query);
        str2HashIndex(q_str, query_indexes, false);
        vector<int> r;
        str2HashIndex(context_jac[i], r, false);
        int jac = compute_jaccard(query_indexes, r, threshold);
        // cout << "Line: " << line << " , ed: " << ed << endl;
        if (jac >= threshold)
        {
            result.push_back(make_pair(i, jac));
        }
    }
    return SUCCESS;
}

int SimSearcher::searchED(const char *query, unsigned threshold, vector<pair<unsigned, unsigned> > &result)
{
	result.clear();
    string q_str(query);
    /*
    vector<pair<int, int>> cand_list; // index, list_length
    vector<int> cand_lines;
    vector<int> short_cand_lines;
    int *lines_count = new int[line_num];
    memset(lines_count, 0, sizeof(int) * line_num);
    vector<int> query_indexes;

    str2QGramHashIndex(q_str, q_gram, query_indexes, false);
    int word_num = query_indexes.size();
    int thres = get_ed_threshold(threshold, q_str.length(), q_gram);
    for (auto index : query_indexes)
    {
        cand_list.push_back(make_pair(index, ed_list[index]->size()));
    }

    int num_long_list = (double)thres / (u * log(M2) + 1.0);
    num_long_list = min(num_long_list, thres - 1);
    int num_short_list = word_num - num_long_list;

    // cout << "Thres: " << thres << endl;
    // cout << "Num short list: " << num_short_list << endl;
    // cout << "Word Num: " << word_num << endl;
    // for (auto index : query_indexes)
    // {
    //     cout << "Index: " << index << ", str: " << ed_hash[index] << endl;
    //     cout << "Lines: " << endl;
    //     for (auto line : *(ed_list[index]))
    //     {
    //         cout << line << endl;
    //     }
    // }

    if (thres <= 0)
    {
        for (int i = 0; i < line_num; i++)
            cand_lines.push_back(i);
    }
    else if (num_short_list <= 0)
    {
        for (auto index : query_indexes)
        {
            for (auto line : *(ed_list[index]))
            {
                lines_count[line]++;
            }
        }
        for (int i = 0; i < line_num; i++)
        {
            if(lines_count[i] >= thres)
            {
                cand_lines.push_back(i);
            }
        }
    }
    else
    {
        nth_element(cand_list.begin(), cand_list.begin() + num_short_list - 1, cand_list.end());
        for (int i = 0; i < num_short_list; i++)
        {
            int index = cand_list[i].first;
            for (auto line : *(ed_list[index]))
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
                int index = cand_list[i].first;
                if (binary_search(ed_list[index]->begin(), ed_list[index]->end(), line))
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
        // cout << "Cand: " << line << endl;
        int ed = compute_ed(query, context_ed[line], threshold, q_gram);
        // cout << "Line: " << line << " , ed: " << ed << endl;
        if (ed <= threshold)
        {
            result.push_back(make_pair(line, ed));
        }
    }

    delete[] lines_count;
    */
    for (int i = 0; i < line_num; i++)
    {
        int ed = compute_ed(query, context_ed[i], threshold, q_gram);
        // cout << "Line: " << line << " , ed: " << ed << endl;
        if (ed <= threshold)
        {
            result.push_back(make_pair(i, ed));
        }
    }
    return SUCCESS;
}

