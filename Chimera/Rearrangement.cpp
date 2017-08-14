#include "stdafx.h"
#include "Rearrangement.h"
#include <iostream>
#include <algorithm>
#include <stdexcept>
#include <sstream> 

int Rearrangement::sign(int x) 
{
	if (x > 0)
	{
		return 1;
	}
	if (x < 0)
	{
		return -1;
	}
	if (x == 0)
	{
		return 0;
	}
}


double Rearrangement::MinCostMatching(const std::vector<std::vector<double>> &cost, std::vector<int> &Lmate, 
									   std::vector<int> &Rmate)
{
	int n = int(cost.size());
	// construct dual feasible solution
	std::vector<double> u(n);
	std::vector<double> v(n);
	for (int i = 0; i < n; i++) 
	{
		u[i] = cost[i][0];
		for (int j = 1; j < n; j++)
		{
			u[i] = min( u[i], cost[i][j] );
		}
	}

	for (int j = 0; j < n; j++) 
	{
		v[j] = cost[0][j] - u[0];
		for (int i = 1; i < n; i++)
		{
			v[j] = min( v[j], cost[i][j] - u[i] );
		}
	}

	// construct primal solution satisfying complementary slackness
	Lmate = std::vector<int>(n, -1);
	Rmate = std::vector<int>(n, -1);
	int mated = 0;

	for (int i = 0; i < n; i++) 
	{
		for (int j = 0; j < n; j++) 
		{
			if (Rmate[j] != -1)
			{
				continue;
			}
			if (fabs(cost[i][j] - u[i] - v[j]) < 1e-10) 
			{
				Lmate[i] = j;
				Rmate[j] = i;
				mated++;
				break;
			}
		}
	}

	std::vector<double> dist(n);
	std::vector<int> dad(n), seen(n);

	// repeat until primal solution is feasible
	while (mated < n) 
	{
		// find an unmatched left node
		int s = 0;
		while (Lmate[s] != -1)
		{
			s++;
		}

		// initialize Dijkstra
		fill(dad.begin(), dad.end(), -1);
		fill(seen.begin(), seen.end(), 0);
		for (int k = 0; k < n; k++)
		{
			dist[k] = cost[s][k] - u[s] - v[k];
		}

		int j = 0;
		while (true) 
		{
			// find closest
			j = -1;
			for (int k = 0; k < n; k++) 
			{
				if (seen[k])
				{
					continue;
				}
				if (j == -1 || dist[k] < dist[j])
				{
					j = k;
				}
			}
			seen[j] = 1;

			// termination condition
			if (Rmate[j] == -1)
			{
				break;
			}

			// relax neighbors
			const int i = Rmate[j];

			for (int k = 0; k < n; k++) 
			{
				if (seen[k])
				{
					continue;
				}

				const double new_dist = dist[j] + cost[i][k] - u[i] - v[k];

				if (dist[k] > new_dist) 
				{
					dist[k] = new_dist;
					dad[k] = j;
				}
			}
		}

		// update dual variables
		for (int k = 0; k < n; k++) 
		{
			if (k == j || !seen[k])
			{
				continue;
			}

			const int i = Rmate[k];
			v[k] += dist[k] - dist[j];
			u[i] -= dist[k] - dist[j];
		}

		u[s] += dist[j];
		// augment along path
		while (dad[j] >= 0) 
		{
			const int d = dad[j];
			Rmate[j] = Rmate[d];
			Lmate[Rmate[j]] = j;
			j = d;
		}
		Rmate[j] = s;
		Lmate[s] = j;
		mated++;
	}
	double value = 0;

	for (int i = 0; i < n; i++)
	{
		value += cost[i][Lmate[i]];
	}

	return value;
}


double Rearrangement::rearrangement( const std::vector<std::vector<bool>> & sourceMatrix, 
									 const std::vector<std::vector<bool>> & targetMatrix,
									 std::vector<std::vector<int>> & operationsMatrix)
{
	//For Error Handling: Algorithm does not work with less atoms than targets
	//I am sure this might be also included directly after evaluating the image, but for safety
	//I also included it here
	int numberTargets = 0;
	int numberSources = 0; 
	for (int i = 0; i < sourceMatrix.size(); i++)
	{
		for (int j = 0; j < sourceMatrix.size(); j++)
		{
			if (targetMatrix[i][j] == 1)
			{
				numberTargets++;
			}
			if (sourceMatrix[i][j] == 1)
			{
				numberSources++;
			}
		}
	}
	//Throw, if  less atoms than targets! myexception class defined above
	if (numberSources < numberTargets) 
	{
		thrower( "Less atoms than targets!\nN source: " + str(numberSources) + ", N target: " + str(numberTargets) );
	}

	//------------------------------------------------------------------------------------------
	//calculate cost matrix from Source and Targetmatrix
	//------------------------------------------------------------------------------------------
	//Cost matrix. Stores path length for each source atom to each target position
	std::vector<std::vector<double> > costMatrix(numberSources, std::vector<double>(numberSources, 0));	
	//Indices of atoms in initial config
	std::vector<std::vector<int> > SourceIndice(numberSources, std::vector<int>(2, 0));	
	//Indices of atoms in final config
	std::vector<std::vector<int> > TargetIndice(numberTargets, std::vector<int>(2, 0));	

	//Find out the indice
	int sourcecounter = 0;
	int targetcounter = 0;
	for (int i = 0; i < sourceMatrix.size(); i++)
	{
		for (int j = 0; j < sourceMatrix.size(); j++)
		{
			if (sourceMatrix[i][j] == 1) 
			{
				SourceIndice[sourcecounter][0] = i;
				SourceIndice[sourcecounter][1] = j;
				sourcecounter++;
			}
			if (targetMatrix[i][j] == 1) 
			{
				TargetIndice[targetcounter][0] = i;
				TargetIndice[targetcounter][1] = j;
				targetcounter++;
			}
		}
	}
	double pathlength = 0; 
	//Now compute the pathlengths
	for (int i = 0; i < numberSources; i++) 
	{
		for (int j = 0; j < numberTargets; j++) 
		{
			costMatrix[i][j] = abs(SourceIndice[i][0] - TargetIndice[j][0]) + abs(SourceIndice[i][1] - TargetIndice[j][1]);
			pathlength += costMatrix[i][j];
		}
	}

	//------------------------------------------------------------------------------
	//Use MinCostMatching algorithm
	//------------------------------------------------------------------------------
	std::vector<int> left; //input for bipartite matching algorithm, Algorithm writes into this vector
	std::vector<int> right; //input for bipartite matching algorithm, Algorithm writes into this vector

	double cost = MinCostMatching(costMatrix, left, right); //The returned cost is the travelled distance

	//------------------------------------------------------------------------------
	//calculate the operationsMatrix
	//------------------------------------------------------------------------------

	//First resize operationsMatrix, empty in code, but now we now how many entrys: cost!
	operationsMatrix.resize(cost, std::vector<int>(4, 0));

	std::vector<std::vector<int> > matching(numberTargets, std::vector<int>(4, 0));
	//matching matrix, numberTargets x 4, Source and Target indice in each row
	for (int i = 0; i < numberTargets; i++) 
	{
		matching[i][0] = SourceIndice[right[i]][0];
		matching[i][1] = SourceIndice[right[i]][1];
		matching[i][2] = TargetIndice[i][0];
		matching[i][3] = TargetIndice[i][1];
	}
	
	int step_x;
	int step_y;
	int init_x;
	int init_y;
	int counter = 0;

	// Setting up the operationsMatrix (only elementary steps) from the matching matrix (source - target)
	for (int i = 0; i < numberTargets; i++) 
	{
		step_x = matching[i][2] - matching[i][0];
		step_y = matching[i][3] - matching[i][1];
		init_x = matching[i][0];
		init_y = matching[i][1];

		for (int j = 0; j < abs(step_x); j++) 
		{
			operationsMatrix[counter][0] = init_x;
			operationsMatrix[counter][1] = init_y;
			operationsMatrix[counter][2] = init_x + sign(step_x);
			operationsMatrix[counter][3] = init_y;
			init_x = init_x + sign(step_x);
			counter++;
		}
		
		for (int j = 0; j < abs(step_y); j++) 
		{
			operationsMatrix[counter][0] = init_x;
			operationsMatrix[counter][1] = init_y;
			operationsMatrix[counter][2] = init_x;
			operationsMatrix[counter][3] = init_y + sign(step_y);
			init_y = init_y + sign(step_y);
			counter++;
		}
	}	
	return cost; //travelled distance
}


//How does this algorithm work? Simple Flow algorithm that makes sure that all moves will be done regardless of the order!
//while loop until the operationsMatrix (list of moves) is empty
//delete moves that will be done
//Look at all moves that go from row to row+1 (after that row->row-1,col->col+1,col->col-1)
//to find more than one atom in each row/column to move at the same time
//Be vary of duplicates, the move (init_row,init_col)->(fin_row,fin_col) might be done more than once during the whole 
//algorithm, but only move and delete it once at the same time
double Rearrangement::parallelMoves(std::vector<std::vector<int>> operationsMatrix, 
									 std::vector<std::vector<int>> C, double N, 
									 std::vector<parallelMovesContainer> &output)
{
	//Vector that should save indice of all the moves (operationmatrix) that are in a certain column/row
	std::vector<int> opM_ix;
	//vector that should save the moves, important to check for duplicates
	std::vector<std::vector<int> > selecteditems; 
    //bool for avoiding duplicates
	bool check = true; 
	// true if there is sth to move
	bool move; 

	//Saves the number of moves that the parallization takes. One count if several atoms move from row to row+1
	int parallelMoveNumber = 0; 
	while (operationsMatrix.size() != 0) 
	{
		//First moves from row to row+1. Fill opM_ix with indice of the moves you want to make
		//Because the other for loops are similarily structured, I included the comments only here
		for (int row = 0; row < N; row++) 
		{
			move = false;
			for (int i = 0; i < operationsMatrix.size(); i++) 
			{
				if (operationsMatrix[i][0] == row && operationsMatrix[i][2] == row + 1) 
				{
					check = true;
					//erase duplicates
					for (int k = 0; k != selecteditems.size(); k++) 
					{
						if (selecteditems[k] == operationsMatrix[i]) 
						{
							check = false;
							break;
						}
					}
					if (check) 
					{
						opM_ix.push_back(i);
						selecteditems.push_back(operationsMatrix[i]);
					}
				}
			}

			//From all the moves in operationsMatrix that go from row to row+1, select those that have a atom at the initial position
			//and have no atom at the final position!
			if (opM_ix.size() != 0) 
			{
				for (unsigned k = opM_ix.size(); k-- > 0; ) 
				{
					//only move if there is an atom to move and if target is free!
					if (C[operationsMatrix[opM_ix[k]][0]][operationsMatrix[opM_ix[k]][1]] != 0 
						 && C[operationsMatrix[opM_ix[k]][2]][operationsMatrix[opM_ix[k]][3]] == 0) 
					{
						move = true;
					}
					else 
					{
						opM_ix.erase(opM_ix.begin() + k);
						selecteditems.erase(selecteditems.begin() + k);
					}

				}
				if (move) 
				{
					parallelMoveNumber++;
					//Save the moves
					output.push_back(parallelMovesContainer());
					output.back().rowOrColumn = "row";
					output.back().upOrDown = 1;
					output.back().which_rowOrColumn = row;

					for (unsigned k = opM_ix.size(); k-- > 0;) 
					{
						C[operationsMatrix[opM_ix[k]][0]][operationsMatrix[opM_ix[k]][1]] = 0;
						C[operationsMatrix[opM_ix[k]][2]][operationsMatrix[opM_ix[k]][3]] = 1;

						output.back().whichAtoms.push_back(operationsMatrix[opM_ix[k]][1]);
						
						operationsMatrix[opM_ix[k]] = operationsMatrix.back();
						operationsMatrix.pop_back();
					}
				}
			}
			opM_ix.clear();
			selecteditems.clear();
		}

		//reset some values
		check = true;
		opM_ix.clear();
		selecteditems.clear();
		//Second moves from row to row-1
		for (unsigned row = N; row-- > 0;) 
		{
			move = false;
			for (int i = 0; i < operationsMatrix.size(); i++) 
			{
				if (operationsMatrix[i][0] == row && operationsMatrix[i][2] == row - 1) 
				{
					check = true;
					//erase duplicates
					for (int k = 0; k != selecteditems.size(); k++) 
					{
						if (selecteditems[k] == operationsMatrix[i]) 
						{
							check = false;
							break;
						}
					}
					if (check) 
					{
						opM_ix.push_back(i);
						selecteditems.push_back(operationsMatrix[i]);
					}
				}
			}

			if (opM_ix.size() != 0) 
			{
				for (UINT k = opM_ix.size(); k-- > 0; ) 
				{
					if (C[operationsMatrix[opM_ix[k]][0]][operationsMatrix[opM_ix[k]][1]] != 0 
						 && C[operationsMatrix[opM_ix[k]][2]][operationsMatrix[opM_ix[k]][3]] == 0) 
					{
						move = true;
					}
					else 
					{
						opM_ix.erase(opM_ix.begin() + k);
						selecteditems.erase(selecteditems.begin() + k);	
					}
				}
				if (move) 
				{
					parallelMoveNumber++;
					output.push_back(parallelMovesContainer());
					output.back().rowOrColumn = "row";
					output.back().upOrDown = -1;
					output.back().which_rowOrColumn = row;					

					for (unsigned k = opM_ix.size(); k-- > 0;) 
					{
						C[operationsMatrix[opM_ix[k]][0]][operationsMatrix[opM_ix[k]][1]] = 0;
						C[operationsMatrix[opM_ix[k]][2]][operationsMatrix[opM_ix[k]][3]] = 1;

						output.back().whichAtoms.push_back(operationsMatrix[opM_ix[k]][1]);
						
						operationsMatrix[opM_ix[k]] = operationsMatrix.back();
						operationsMatrix.pop_back();
					}
				}
			}
			opM_ix.clear();
			selecteditems.clear();
		}

		//reset some values
		check = true;
		opM_ix.clear();
		selecteditems.clear();
		//Third moves from col to col+1
		for (int col = 0; col < N; col++) 
		{
			move = false;
			for (int i = 0; i < operationsMatrix.size(); i++) 
			{
				if (operationsMatrix[i][1] == col && operationsMatrix[i][3] == col + 1) 
				{
					check = true;
					//erase duplicates
					for (int k = 0; k != selecteditems.size(); k++) 
					{
						if (selecteditems[k] == operationsMatrix[i]) 
						{
							check = false;
							break;
						}
					}
					if (check) 
					{
						opM_ix.push_back(i);
						selecteditems.push_back(operationsMatrix[i]);
					}
				}
			}

			if (opM_ix.size() != 0) 
			{
				for (unsigned k = opM_ix.size(); k-- > 0; ) 
				{

					if (C[operationsMatrix[opM_ix[k]][0]][operationsMatrix[opM_ix[k]][1]] != 0 
						 && C[operationsMatrix[opM_ix[k]][2]][operationsMatrix[opM_ix[k]][3]] == 0) 
					{
						move = true;
					}
					else 
					{
						opM_ix.erase(opM_ix.begin() + k);
						selecteditems.erase(selecteditems.begin() + k);
					}

				}
				if (move) 
				{
					parallelMoveNumber++;
					output.push_back(parallelMovesContainer());
					output.back().rowOrColumn = "column";
					output.back().upOrDown = 1;
					output.back().which_rowOrColumn = col;

					for (unsigned k = opM_ix.size(); k-- > 0;) 
					{
						C[operationsMatrix[opM_ix[k]][0]][operationsMatrix[opM_ix[k]][1]] = 0;
						C[operationsMatrix[opM_ix[k]][2]][operationsMatrix[opM_ix[k]][3]] = 1;

						output.back().whichAtoms.push_back(operationsMatrix[opM_ix[k]][0]);
						operationsMatrix[opM_ix[k]] = operationsMatrix.back();
						operationsMatrix.pop_back();
					}
				}
			}
			opM_ix.clear();
			selecteditems.clear();
		}

		// reset some values
		check = true;
		opM_ix.clear();
		selecteditems.clear();
		//Fourth moves from col to col-1
		for (unsigned col = N; col-- > 0;) 
		{
			move = false;
			//get all elements in this row that move to row-1
			opM_ix.clear();
			selecteditems.clear();
			for (int i = 0; i < operationsMatrix.size(); i++) 
			{
				if (operationsMatrix[i][1] == col && operationsMatrix[i][3] == col - 1) {
					check = true;
					//erase duplicates
					for (int k = 0; k != selecteditems.size(); k++) 
					{
						if (selecteditems[k] == operationsMatrix[i]) 
						{
							check = false;
							break;
						}
					}
					if (check) 
					{
						opM_ix.push_back(i);
						selecteditems.push_back(operationsMatrix[i]);
					}
				}
			}

			if (opM_ix.size() != 0) 
			{
				for (unsigned k = opM_ix.size(); k-- > 0;) 
				{
					if (C[operationsMatrix[opM_ix[k]][0]][operationsMatrix[opM_ix[k]][1]] != 0 
						 && C[operationsMatrix[opM_ix[k]][2]][operationsMatrix[opM_ix[k]][3]] == 0) 
					{
						move = true;
					}
					else 
					{
						opM_ix.erase(opM_ix.begin() + k);
						selecteditems.erase(selecteditems.begin() + k);
					}
				}
				if (move) 
				{
					parallelMoveNumber++;
					output.push_back(parallelMovesContainer());
					output.back().rowOrColumn = "column";
					output.back().upOrDown = -1;
					output.back().which_rowOrColumn = col;

					for (unsigned k = opM_ix.size(); k-- > 0;) {
						//cout << "----------------------" << endl;
						C[operationsMatrix[opM_ix[k]][0]][operationsMatrix[opM_ix[k]][1]] = 0;
						C[operationsMatrix[opM_ix[k]][2]][operationsMatrix[opM_ix[k]][3]] = 1;

						output.back().whichAtoms.push_back(operationsMatrix[opM_ix[k]][0]);

						operationsMatrix[opM_ix[k]] = operationsMatrix.back();
						operationsMatrix.pop_back();
					}
				}
			}
		}
	}
	return parallelMoveNumber;
}


//Task was: Find out the maximum number of moves, by only knowing the Target Matrix configuration
//I added together the furthest distances from each target.
//Therefore it assumes there is no atom on each target
//This is really overestimating the number of moves, but it is a maximum
//Is overestimating the most if you have a very small target in a big lattice.
//If you wanted to scale it down, one idea might be to scale maxMoves with the filling fraction!
//Also: Not super fast because of nested for loops
UINT Rearrangement::maxMoves(const std::vector<std::vector<bool>> targetmatrix) 
{
	int targetNumber = 0;
	for (auto row : targetmatrix)
	{
		for (auto elem : row)
		{
			if (elem == 1)
			{
				targetNumber++;
			}
		}
	}

	std::vector<std::vector<int> >targetIndice(targetNumber, std::vector<int>(2, 0));
	UINT targetcounter = 0;
	for (int i = 0; i < targetmatrix.size(); i++) 
	{
		for (int j = 0; j < targetmatrix[0].size(); j++) 
		{
			if (targetmatrix[i][j] == 1) 
			{
				targetIndice[targetcounter][0] = i;
				targetIndice[targetcounter][1] = j;
				targetcounter++;
			}
		}
	}
	UINT maxlength = 0, sumlength = 0, length = 0;
	for (int k = 0; k < targetcounter; k++) 
	{
		for (int i = 0; i < targetmatrix.size(); i++) 
		{
			for (int j = 0; j < targetmatrix[0].size(); j++) 
			{
				length = abs(i - targetIndice[k][0]) + abs(j - targetIndice[k][1]);
				if (length > maxlength)
				{
					maxlength = length;
				}
			}
		}
		sumlength += maxlength;
	}
	return sumlength;
}
