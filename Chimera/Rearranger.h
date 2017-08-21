#pragma once
#include "Windows.h"
#include <atomic>
#include "rearrangementStructures.h"
#include "rearrangementThreadInput.h"

class Rearranger
{
	public:



	private:

};


/*
// RearrangementCode.cpp : Defines the entry point for the console application.
//
//This is an example of how to use the Rearrangement Code
//Author: Kai-Niklas Schymik
//Date: June 2017

int main()
{
	try {
		//-----------------------------------------------------------------------------------------------------------------
		//First, make Source and Target Matrix. In experiment the source will come from the evaluation of the photos, but
		//the target has still to be made.
		//-----------------------------------------------------------------------------------------------------------------
		const int N = 10; //Matrix size
		double p = 0.1; //loading probability
		vector<vector<int> >S(N, vector<int>(N, 0)); //Source Matrix
		int Ns = 0; //Number source atoms

		//initialize source matrix
		srand(time(NULL));
		for (int i = 0; i < N; i++) {
			for (int j = 0; j < N; j++) {
				//cout << double(rand())/RAND_MAX << endl;
				double random = double(rand()) / RAND_MAX;
				if (random < p) {
					S[i][j] = 1;
					//Ns++;
				}
				else {
					S[i][j] = 0;
				}

			}
		}

		//initialize Target matrix
		vector<vector<int> > T(N, vector<int>(N, 0)); //Target matrix

		//Square
		for (int i = ceil(N*4.0 / 10.0) - 1; i < floor(N*6.0 / 10.0); i++) {
			for (int j = ceil(N*4.0 / 10.0) - 1; j < floor(N*6.0 / 10.0); j++) {
				T[i][j] = 1;
			}
		}

		//------------------------------------------
		//Create an instance of Class Rearrangement
		//------------------------------------------
		Rearrangement *rearrangementobject = new Rearrangement();

		//------------------------------------------------------------------------
		//Get the maximal number of moves which equals to the maximal travelled
		//distance, only given the Target matrix!
		//------------------------------------------------------------------------
		int maxmoves = rearrangementobject->nofmovesmax(T);
		cout << "The maximal Number of moves or the maximal travelled distance given the Target Matrix is:  " << maxmoves << endl;
		
		//--------------------------------------------------------------------------------------------------------------------
		//Use the rearrangement algorithm. You need to make an empty operationsmatrix first, in which the algorithm will write
		//All the single moves. Stop here, if only single moves of interest
		//The returned value is the travelled distance
		//--------------------------------------------------------------------------------------------------------------------
		vector<vector<int>	> operationsmatrix;
		double cost = rearrangementobject->rearrangement(S, T, operationsmatrix);

		cout << "The cost using the Matching Algorithm is: " << cost << endl;

		
		//----------------------------------------------------------
		cout << "This is how the operationsmatrix looks.init_row,init_col,fin_row,fin_col" << endl;
		for (int i = 0; i < operationsmatrix.size(); i++) {
			for (int j = 0; j < operationsmatrix[0].size(); j++) {
				cout << operationsmatrix[i][j] << " ";
			}
			cout << endl;
		}

		//--------------------------------------------------------------------------------------------------------------------
		//You're done if only considering single moves. Operationsmatrix is your list of moves. 
		//Now for considering parallel movements, that means more than one atom in each row and column, use the parallelmoves
		//function after the rearrangement function
		//--------------------------------------------------------------------------------------------------------------------
		vector<parallelMovesContainer> giveMark; //this is the container that gives you all the parallel moves
												 //it looks like this:
												//Is a row or a column being moved?
												// What is the index of row or column?
												// +1 for col->col+1 and -1 for col->col-1)
												//vector of the column/row indice that are moved. 

		double  nmovesparallel = rearrangementobject->parallelmoves(operationsmatrix, S, N, giveMark);
		
		cout << "--------------Parallel Moves--------------------" << endl;
		cout << "Number of parallel moves: " << nmovesparallel << endl;
		
		for (int i = 0; i < giveMark.size(); i++) {
			cout << giveMark[i].rowOrColumn << " " << giveMark[i].which_rowOrColumn << " upOrDown: " << giveMark[i].upOrDown;
			cout << "{ ";
			for (int j = 0; j < giveMark[i].whichAtoms.size(); j++) {
				cout << giveMark[i].whichAtoms[j] << ", ";
			}
			cout << "} " << endl;

		}



	}
	//catch this exception in case there are less atoms than targets!
	catch (exception& e)
	{
		cout << e.what() << '\n';
		
	} 
	
	return 0;
}


*/