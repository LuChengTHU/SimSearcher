#include "SimSearcher.h"
#include <iostream>
using namespace std;

int main(int argc, char **argv)
{
	SimSearcher searcher;

	vector<pair<unsigned, unsigned> > resultED;
	vector<pair<unsigned, double> > resultJaccard;

	unsigned q = 3, edThreshold = 2;
	double jaccardThreshold = 0.1;

    searcher.createIndex(argv[1], q);
    cout << "Build Index success!" << endl;
    searcher.searchJaccard("hello world", jaccardThreshold, resultJaccard);
    cout << "Jaccard Success!" << endl;
    int jac_size = resultJaccard.size();
    cout << "Result_size: " << jac_size << endl;
    for (int i = 0; i < jac_size; i++)
        cout << resultJaccard[i].first << " " << resultJaccard[i].second << endl;
    searcher.searchED("TIm JHankss", edThreshold, resultED);
    cout << "ED Success!" << endl;
    int ed_size = resultED.size();
    cout << "Result_size: " << ed_size << endl;
    for (int i = 0; i < ed_size; i++)
        cout << resultED[i].first << " " << resultED[i].second << endl;

    return 0;
}

