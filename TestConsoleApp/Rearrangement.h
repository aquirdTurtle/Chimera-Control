//Code for Rearranging Atoms in a lattice, using a Minimal Cost Bipartite Matching Algorithm
//Author: Kai-Niklas Schymik
//Version: June 2017

#pragma once
#include <vector>
#include <string>
using namespace std;

typedef vector<double> VD;
typedef vector<VD> VVD;
typedef vector<int> VI;
typedef vector<VI> VVI;

struct parallelMovesContainer {

	string rowOrColumn;
	int which_rowOrColumn;
	int upOrDown;
	vector<int> whichAtoms;

};

class Rearrangement
{

public:

	// returns sign of x. Haven't found it in STL
	int sign(int);

	// returns cost, which is total travel distance. Algorithm from: http://cs.stanford.edu/group/acm/SLPC/notebook.pdf
	//You have to give it the cost matrix, and to empti vectors, in which it will write
	double MinCostMatching(const VVD & cost, VI & Lmate, VI & Rmate);

	// returns a list of single elementary (left,right,up,down) moves. Size is 4 x n_moves: Initialx,Initialy,Finalx,Finaly
	double rearrangement(const VVI &sourcematrix, const VVI &targetmatrix, VVI &operationsmatrix);

	// From the single moves operationsmatrix, this function calculates parallel moves (rows and columns)
	double parallelmoves(VVI operationsmatrix, VVI C, double N, vector<parallelMovesContainer> &output);

	// returns maximal number of moves given a targetmatrix.
	int nofmovesmax(const VVI targetmatrix);
	
	Rearrangement();
	~Rearrangement();
};

