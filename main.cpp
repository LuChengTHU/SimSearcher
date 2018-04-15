#include "SimSearcher.h"
#include <iostream>

using namespace std;

int main(int argc, char** argv)
{
	SimSearcher searcher;

	vector<pair<unsigned, unsigned> > resultED;
	vector<pair<unsigned, double> > resultJaccard;

	unsigned q = 3, edThreshold = 7;
	double jaccardThreshold = 0.2;

	cout << "build index:" << searcher.createIndex(argv[1], q)<<endl;
    searcher.searchJaccard("query", jaccardThreshold, resultJaccard);
    cout << "jaccard result" << endl;
	for (int i = 0; i < resultJaccard.size(); i++)
		cout << resultJaccard[i].first << " " << resultJaccard[i].second << endl;
	searcher.searchED("query", edThreshold, resultED);
	cout << "ed result" << endl;
	for (int i = 0; i < resultED.size(); i++)
		cout << resultED[i].first << " " << resultED[i].second << endl;

	return 0;
}

