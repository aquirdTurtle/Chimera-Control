
#include "stdafx.h" //for precompiled header
#include <vector>
#include <iostream>
#include <ctime>
#include "Rearrangement.h"

using namespace std;

typedef vector<double> VD;
typedef vector<VD> VVD;
typedef vector<int> VI;
typedef vector<VI> VVI;

int main( )
{
	try {
		//-----------------------------------------------------------------------------------------------------------------
		//First, make Source and Target Matrix. In experiment the source will come from the evaluation of the photos, but
		//the target has still to be made.
		//-----------------------------------------------------------------------------------------------------------------
		//const int matrixSize = 10; //Matrix size
		const int rows = 4;
		const int columns = 5;
		double loadingProb = 0.3; //loading probability
		vector<vector<int> >source( rows, vector<int>( columns, 0 ) ); //Source Matrix
		int atomsInSource = 0; //Number source atoms

		//initialize source matrix
		/*
		srand( time( NULL ) );
		for ( int i = 0; i < rows; i++ ) {
			for ( int j = 0; j < columns; j++ ) {
				//cout << double(rand())/RAND_MAX << endl;
				double random = double( rand( ) ) / RAND_MAX;
				if ( random < loadingProb ) {
					source[i][j] = 1;
					//Ns++;
				}
				else {
					source[i][j] = 0;
				}

			}
		}
		*/
		source[0] = { 0,0,0,0,1 };
		source[1] = { 0,0,0,0,0 };
		source[2] = { 0,0,0,0,1 };
		source[3] = { 0,0,0,0,0 };

		//initialize Target matrix
		vector<vector<int> > target( rows, vector<int>( columns, 0 ) ); //Target matrix

		//Square
		//for ( int i = ceil( rows*4.0 / 10.0 ) - 1; i < floor( rows*6.0 / 10.0 ); i++ ) {
		//	for ( int j = ceil( columns*4.0 / 10.0 ) - 1; j < floor( columns *6.0 / 10.0 ); j++ ) {
		//		target[i][j] = 1;
		//	}
		//}
		target[0] = { 0,0,0,0,0 };
		target[1] = { 0,0,1,0,0 };
		target[2] = { 0,0,0,0,0 };
		target[3] = { 0,0,0,0,0 };

		//------------------------------------------
		//Create an instance of Class Rearrangement
		//------------------------------------------
		Rearrangement *rearrangementobject = new Rearrangement( );


		//------------------------------------------------------------------------
		//Get the maximal number of moves which equals to the maximal travelled
		//distance, only given the Target matrix!
		//------------------------------------------------------------------------
		int maxmoves = rearrangementobject->nofmovesmax( target );
		cout << "The maximal Number of moves or the maximal travelled distance given the Target Matrix is:  " << maxmoves << endl;

		//--------------------------------------------------------------------------------------------------------------------
		//Use the rearrangement algorithm. You need to make an empty operationsmatrix first, in which the algorithm will write
		//All the single moves. Stop here, if only single moves of interest
		//The returned value is the travelled distance
		//--------------------------------------------------------------------------------------------------------------------
		vector<vector<int>	> operationsmatrix;
		double cost = rearrangementobject->rearrangement( source, target, operationsmatrix );

		cout << "The cost using the Matching Algorithm is: " << cost << endl;

		//----------------------------------------------------------
		cout << "This is how the operationsmatrix looks.init_row,init_col,fin_row,fin_col" << endl;
		for ( int i = 0; i < operationsmatrix.size( ); i++ ) {
			for ( int j = 0; j < operationsmatrix[0].size( ); j++ ) {
				cout << operationsmatrix[i][j] << " ";
			}
			cout << endl;
		}

		//--------------------------------------------------------------------------------------------------------------------
		//You're done if only considering single moves. Operationsmatrix is your list of moves.
		//Now for considering parallel movements, that means more than one atom in each row and column, use the parallelmoves
		//function after the rearrangement function
		//--------------------------------------------------------------------------------------------------------------------
		/*
		vector<parallelMovesContainer> giveMark; //this is the container that gives you all the parallel moves
												 //it looks like this:
												 //Is a row or a column being moved?
												 // What is the index of row or column?
												 // +1 for col->col+1 and -1 for col->col-1)
												 //vector of the column/row indice that are moved.
		
		double  nmovesparallel = rearrangementobject->parallelmoves( operationsmatrix, source, matrixSize, giveMark );

		cout << "--------------Parallel Moves--------------------" << endl;
		cout << "Number of parallel moves: " << nmovesparallel << endl;

		for ( int i = 0; i < giveMark.size( ); i++ ) {
			cout << giveMark[i].rowOrColumn << " " << giveMark[i].which_rowOrColumn << " upOrDown: " << giveMark[i].upOrDown;
			cout << "{ ";
			for ( int j = 0; j < giveMark[i].whichAtoms.size( ); j++ ) {
				cout << giveMark[i].whichAtoms[j] << ", ";
			}
			cout << "} " << endl;
		}
		*/
	}
	//catch this exception in case there are less atoms than targets!
	catch ( exception& e )
	{
		cout << e.what( ) << '\n';

	}

	std::cin.get( );

	return 0;
}
