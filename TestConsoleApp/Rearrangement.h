//Code for Rearranging Atoms in a lattice, using a Minimal Cost Bipartite Matching Algorithm
//Author: Kai-Niklas Schymik
//Version: June 2017

#pragma once
#include <vector>
#include <string>

struct parallelMove
{
	parallelMove( ) {}
	parallelMove( std::string rowColumn, int dimNum, int dir )
	{
		rowOrColumn = rowColumn;
		whichRowOrColumn = dimNum;
		upOrDown = dir;
	}
	std::string rowOrColumn;
	int whichRowOrColumn;
	int upOrDown;
	std::vector<int> whichAtoms;
};


class Rearrangement
{

public:

	// returns sign of x. Haven't found it in STL
	int sign(int);
	// returns cost, which is total travel distance. Algorithm from: http://cs.stanford.edu/group/acm/SLPC/notebook.pdf
	//You have to give it the cost matrix, and to empti vectors, in which it will write
	double MinCostMatching(const std::vector<std::vector<double>> & cost, std::vector<int> & Lmate, 
							std::vector<int> & Rmate);
	// returns a list of single elementary (left,right,up,down) moves. Size is 4 x n_moves: Initialx,Initialy,Finalx,Finaly
	double rearrangement(const std::vector<std::vector<int>> &sourcematrix, const std::vector<std::vector<int>> &targetmatrix, 
						  std::vector<std::vector<int>> &operationsmatrix);
	// From the single moves operationsmatrix, this function calculates parallel moves (rows and columns)
	double parallelmoves(std::vector<std::vector<int>> operationsmatrix, std::vector<std::vector<int>> C, 
						  double N, std::vector<parallelMove> &output);
	// returns maximal number of moves given a targetmatrix.
	int nofmovesmax(const std::vector<std::vector<int>> targetmatrix);
	
	Rearrangement();
	~Rearrangement();
};

