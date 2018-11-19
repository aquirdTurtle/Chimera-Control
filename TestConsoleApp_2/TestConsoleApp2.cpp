// Rearranging moves as a function of loading rate and target size
//

#include "stdafx.h"
#include "Thrower.h"
#include "Matrix.h"
#include <fstream>
#include <vector>
#include <array>
enum class dir
{
	up,
	down,
	left,
	right
};

struct int_coordinate
{
	int_coordinate::int_coordinate ( int r, int c )
	{
		row = r;
		column = c;
	}

	int_coordinate::int_coordinate ( )
	{
		row = 0;
		column = 0;
	}

	bool int_coordinate::operator==( const int_coordinate& coordinate2 )
	{
		return row == coordinate2.row && column == coordinate2.column;
	}

	int row;
	int column;
};


struct coordinate
{
	coordinate::coordinate ( ULONG r, ULONG c )
	{
		row = r;
		column = c;
	}

	coordinate::coordinate ( )
	{
		row = 0;
		column = 0;
	}

	bool coordinate::operator==( const coordinate& coordinate2 )
	{
		return row == coordinate2.row && column == coordinate2.column;
	}

	ULONG row;
	ULONG column;
};

struct double_coordinate
{
	double_coordinate::double_coordinate ( double r, double c )
	{
		row = r;
		column = c;
	}

	double_coordinate::double_coordinate ( )
	{
		row = 0;
		column = 0;
	}

	bool operator==( const double_coordinate& coordinate2 )
	{
		return row == coordinate2.row && column == coordinate2.column;
	}

	double row;
	double column;
};



//#include "niawgStructures.h"
struct niawgWavePower
{
	enum class mode
	{
		// power is held constant.
		constant,
		// power can float beneath cap.
		capped,
		// no restrictions or modifications.
		unrestricted
	};
	static const std::array<mode, 4> allModes;
	static std::string toStr ( mode m );
	static mode fromStr ( std::string txt );
	static const mode defaultMode;
};


struct niawgLibOption
{
	enum class mode
	{
		// i.e. used if available
		allowed,
		// always calculate the wave
		banned,
		// forces use of library wave, throws if can't. Mostly used for testing.
		forced
	};
	static const std::array<mode, 4> allModes;
	static std::string toStr ( mode m );
	static mode fromStr ( std::string txt );
	static const mode defaultMode;
};



struct niawgWaveCalcOptions
{
	niawgWavePower::mode powerOpt = niawgWavePower::defaultMode;
	niawgLibOption::mode libOpt = niawgLibOption::defaultMode;
};


// order here matches the literal channel number on the 5451. Vertical is actually channel0 and Horizontal is actually 
// channel1. putting the enum in the struct here is a trick that makes you have to use the Axes:: scope but allows 
// automatic int conversion unlike enum class, which is useful for this.
struct Axes
{
	enum type { Vertical = 0, Horizontal = 1 };
};
// used to pair together info for each channel of the niawg in an easy, iterable way.
template<typename type> using niawgPair = std::array<type, 2>;
// used for pre-writing rearrangement moves...
struct rerngMove
{
	UINT row;
	UINT col;
	dir direction;
	double initFreq = 0;
	double finFreq = 0;
	/// all of the following can (in principle) be individually calibrated.
	double moveBias;
	// in ms
	double moveTime = 60e-3;
	// in MHz
	double flashingFreq = 1e6;
	// in ns
	double deadTime = 0;
	// the static / moving ratio.
	double staticMovingRatio = 1;
	// the actual wave, ready for pushing to the niawg.
	std::vector<double> waveVals;
};


// abstracted moves used by the min cost matching algorithm and the 
struct complexMove
{
	complexMove ( ) { }
	complexMove ( dir direction )
	{
		moveDir = direction;
		locationsToMove.clear ( );
	}
	bool isInlineParallel = false;
	dir moveDir;
	std::vector<int_coordinate> locationsToMove;
	bool needsFlash;
	int dirInt ( )
	{
		if ( moveDir == dir::right || moveDir == dir::left )
		{
			return ( moveDir == dir::right ) ? 1 : -1;
		}
		else
		{
			return ( moveDir == dir::up ) ? 1 : -1;
		}
	}
};;


// should be a one-dimensional move, only change in row or column. Could probably improve the struct to reflect that.
struct simpleMove
{
	simpleMove ( int irow, int icol, int frow, int fcol )
	{
		initRow = irow;
		initCol = icol;
		finRow = frow;
		finCol = fcol;
		distanceToTarget = -1;
	}
	simpleMove ( int irow, int icol, int frow, int fcol, double d )
	{
		initRow = irow;
		initCol = icol;
		finRow = frow;
		finCol = fcol;
		distanceToTarget = d;
	}
	bool operator == ( const simpleMove & other ) const
	{
		// two moves are equal if all members are equal.
		return( initRow == other.initRow &&
				initCol == other.initCol &&
				finRow == other.finRow  &&
				finCol == other.finCol );
	}
	ULONG initRow;
	ULONG initCol;
	ULONG finRow;
	ULONG finCol;
	double distanceToTarget;
	dir dir ( )
	{
		if ( finCol != initCol )
		{
			return ( finCol > initCol ) ? dir::right : dir::left;
		}
		else
		{
			return ( finRow > initRow ) ? dir::up : dir::down;
		}
	}
	int dirInt ( )
	{
		if ( finCol != initCol )
		{
			return ( finCol > initCol ) ? 1 : -1;
		}
		else
		{
			return ( finRow > initRow ) ? 1 : -1;
		}
	}

	int movingIndex ( )
	{
		if ( dir ( ) == dir::up || dir ( ) == dir::down )
		{
			return initRow;
		}
		else
		{
			return initCol;
		}
	}
	int staticIndex ( )
	{
		if ( dir ( ) == dir::up || dir ( ) == dir::down )
		{
			return initCol;
		}
		else
		{
			return initRow;
		}
	}
};
#include "range.h"
#include <random>
#include <iostream>

// these functions were pulled from niawgController. I pull them instead of just including niawgcontroller because 
// of the baggage that comes with including niawgController, would require working out all the dependencies of related 
// libraries
int sign_( int x )
{
	if ( x > 0 )
	{
		return 1;
	}
	else if ( x < 0 )
	{
		return -1;
	}
	else
	{
		return 0;
	}
}


double minCostMatching ( Matrix<double> cost, std::vector<int> &sourceMates, std::vector<int> &targetMates )
{
	/// 
	UINT numSources = cost.getRows ( );
	UINT numTargets = cost.getCols ( );
	// construct dual feasible solution

	// each element of u represents one of the sources, and the value of that element is the distance of that source to 
	// the closest target.
	std::vector<double> u ( numSources );
	// v is more complicated.
	std::vector<double> v ( numTargets );
	// 
	for ( int sourceInc = 0; sourceInc < numSources; sourceInc++ )
	{
		u[ sourceInc ] = cost ( sourceInc, 0 );
		for ( int targetInc = 1; targetInc < numTargets; targetInc++ )
		{
			u[ sourceInc ] = min ( u[ sourceInc ], cost ( sourceInc, targetInc ) );
		}
	}

	for ( int targetInc = 0; targetInc < numTargets; targetInc++ )
	{
		v[ targetInc ] = cost ( 0, targetInc ) - u[ 0 ];
		for ( int sourceInc = 1; sourceInc < numSources; sourceInc++ )
		{
			v[ targetInc ] = min ( v[ targetInc ], cost ( sourceInc, targetInc ) - u[ sourceInc ] );
		}
	}

	// construct primal solution satisfying complementary slackness
	// -1 indicates unmatched.
	sourceMates = std::vector<int> ( numSources, -1 );
	targetMates = std::vector<int> ( numTargets, -1 );
	int numberMated = 0;

	for ( int sourceInc = 0; sourceInc < numSources; sourceInc++ )
	{
		for ( int targetInc = 0; targetInc < numTargets; targetInc++ )
		{
			if ( targetMates[ targetInc ] != -1 )
			{
				// already matched.
				continue;
			}
			if ( fabs ( cost ( sourceInc, targetInc ) - u[ sourceInc ] - v[ targetInc ] ) < 1e-10 )
			{
				sourceMates[ sourceInc ] = targetInc;
				targetMates[ targetInc ] = sourceInc;
				numberMated++;
				break;
			}
		}
	}

	std::vector<double> dist ( numTargets );
	std::vector<int> dad ( numSources );
	std::vector<bool> seen ( numTargets, false );

	// repeat until primal solution is feasible
	while ( numberMated < numSources )
	{
		// find an unmatched left node
		int currentUnmatchedSource = 0;
		// I think there must be at least one because numberMated < rows.
		while ( sourceMates[ currentUnmatchedSource ] != -1 )
		{
			currentUnmatchedSource++;
			if ( currentUnmatchedSource >= sourceMates.size ( ) )
			{
				thrower ( "rearrangement Error! all mateColumn are matched but numberMated < rows!  "
						  "(A low level bug, this shouldn't happen)" );
			}
		}

		// initialize Dijkstra ...?
		fill ( dad.begin ( ), dad.end ( ), -1 );
		fill ( seen.begin ( ), seen.end ( ), false );
		for ( auto targetInc : range ( numTargets ) )
		{
			dist[ targetInc ] = cost ( currentUnmatchedSource, targetInc ) - u[ currentUnmatchedSource ] - v[ targetInc ];
		}

		int closestTarget = 0;
		while ( true )
		{
			// find closest target
			closestTarget = -1;
			for ( int targetInc = 0; targetInc < numTargets; targetInc++ )
			{
				if ( seen[ targetInc ] )
				{
					continue;
				}
				if ( closestTarget == -1 || dist[ targetInc ] < dist[ closestTarget ] )
				{
					closestTarget = targetInc;
				}
			}
			seen[ closestTarget ] = true;

			// termination condition
			if ( targetMates[ closestTarget ] == -1 )
			{
				break;
			}

			// relax neighbors
			const int closestTargetMate = targetMates[ closestTarget ];

			for ( int targetInc = 0; targetInc < numTargets; targetInc++ )
			{
				if ( seen[ targetInc ] )
				{
					continue;
				}

				const double new_dist = dist[ closestTarget ] + cost ( closestTargetMate, targetInc )
					- u[ closestTargetMate ] - v[ targetInc ];
				if ( dist[ targetInc ] > new_dist )
				{
					dist[ targetInc ] = new_dist;
					dad[ targetInc ] = closestTarget;
				}
			}
		}

		// update dual params
		for ( auto targetInc : range ( numTargets ) )
		{
			if ( targetInc == closestTarget || !seen[ targetInc ] )
			{
				continue;
			}

			const int closestTargetMate = targetMates[ targetInc ];
			v[ targetInc ] += dist[ targetInc ] - dist[ closestTarget ];
			u[ closestTargetMate ] -= dist[ targetInc ] - dist[ closestTarget ];
		}

		u[ currentUnmatchedSource ] += dist[ closestTarget ];
		// augment along path
		while ( dad[ closestTarget ] >= 0 )
		{
			const int d = dad[ closestTarget ];
			targetMates[ closestTarget ] = targetMates[ d ];
			sourceMates[ targetMates[ closestTarget ] ] = closestTarget;
			closestTarget = d;
		}
		targetMates[ closestTarget ] = currentUnmatchedSource;
		sourceMates[ currentUnmatchedSource ] = closestTarget;
		numberMated++;
	}

	double value = 0;
	for ( auto sourceInc : range ( numSources ) )
	{
		value += cost ( sourceInc, sourceMates[ sourceInc ] );
	}
	return value;
}



/*
this part was written by Mark O Brown. The other stuff in the rearrangment handling was written by Kai Niklas.
*/
void orderMoves ( std::vector<simpleMove> operationsList, std::vector<simpleMove>& moveSequence, 
				  Matrix<bool> sourceMatrix )
{

	// systemState keeps track of the state of the system after each move. It's important so that the algorithm can
	// avoid making atoms overlap.
	Matrix<bool> systemState = sourceMatrix;
	UINT moveNum = 0;
	while ( operationsList.size ( ) != 0 )
	{
		if ( moveNum >= operationsList.size ( ) )
		{
			// it's reached the end, reset this.
			moveNum = 0;
		}
		// make sure that the initial location IS populated and the final location ISN'T.
		bool initIsOpen = systemState ( operationsList[ moveNum ].initRow, operationsList[ moveNum ].initCol ) == false;
		bool finIsOccupied = systemState ( operationsList[ moveNum ].finRow, operationsList[ moveNum ].finCol ) == true;
		if ( initIsOpen || finIsOccupied )
		{
			moveNum++;
			continue;
		}
		// else it's okay. add this to the list of moves.
		moveSequence.push_back ( operationsList[ moveNum ] );
		// update the system state after this move.
		systemState ( operationsList[ moveNum ].initRow, operationsList[ moveNum ].initCol ) = false;
		systemState ( operationsList[ moveNum ].finRow, operationsList[ moveNum ].finCol ) = true;
		// remove the move from the list of moves.
		operationsList.erase ( operationsList.begin ( ) + moveNum );
	}
	// at this point moveList should be zero size and moveSequence should be full of the moves in a sequence that
	// works. return the travelled distance.
}

// runs the sample function above and prints the caught exception
double rearrangement ( Matrix<bool> & sourceMatrix, Matrix<bool> & targetMatrix, std::vector<simpleMove>& moveList )
{
	// I am sure this might be also included directly after evaluating the image, but for safety I also included it 
	// here.
	int numberTargets = 0;
	int numberSources = 0;
	std::string sourceStr = sourceMatrix.print ( );
	std::string targStr = targetMatrix.print ( );
	for ( UINT rowInc = 0; rowInc < sourceMatrix.getRows ( ); rowInc++ )
	{
		for ( UINT colInc = 0; colInc < sourceMatrix.getCols ( ); colInc++ )
		{
			if ( targetMatrix ( rowInc, colInc ) )
			{
				numberTargets++;
			}
			if ( sourceMatrix ( rowInc, colInc ) )
			{
				numberSources++;
			}
		}
	}
	// Throw, if  less atoms than targets!
	if ( numberSources < numberTargets )
	{
		thrower ( "Less atoms than targets!\nN source: " + str ( numberSources ) + ", N target: " + str ( numberTargets ) );
	}

	/// calculate cost matrix from Source and Targetmatrix
	// Cost matrix. Stores path length for each source atom to each target position
	Matrix<double> costMatrix ( numberSources, numberSources, 0 );
	// Indices of atoms in initial config
	std::vector<std::vector<int> > sourceCoordinates ( numberSources, std::vector<int> ( 2, 0 ) );
	// Indices of atoms in final config
	std::vector<std::vector<int> > targetCoordinates ( numberTargets, std::vector<int> ( 2, 0 ) );
	// Find out the indice
	int sourceCounter = 0;
	int targetCounter = 0;

	for ( UINT rowInc = 0; rowInc < sourceMatrix.getRows ( ); rowInc++ )
	{
		for ( UINT columnInc = 0; columnInc < sourceMatrix.getCols ( ); columnInc++ )
		{
			if ( sourceMatrix ( rowInc, columnInc ) == 1 )
			{
				sourceCoordinates[ sourceCounter ][ 0 ] = rowInc;
				sourceCoordinates[ sourceCounter ][ 1 ] = columnInc;
				sourceCounter++;
			}
			if ( targetMatrix ( rowInc, columnInc ) == 1 )
			{
				targetCoordinates[ targetCounter ][ 0 ] = rowInc;
				targetCoordinates[ targetCounter ][ 1 ] = columnInc;
				targetCounter++;
			}
		}
	}

	// Now compute the pathlengths
	for ( int sourceInc = 0; sourceInc < numberSources; sourceInc++ )
	{
		for ( int targetInc = 0; targetInc < numberTargets; targetInc++ )
		{
			costMatrix ( sourceInc, targetInc ) = abs ( sourceCoordinates[ sourceInc ][ 0 ] - targetCoordinates[ targetInc ][ 0 ] )
				+ abs ( sourceCoordinates[ sourceInc ][ 1 ] - targetCoordinates[ targetInc ][ 1 ] );
		}
	}

	/// Use MinCostMatching algorithm
	// input for bipartite matching algorithm, Algorithm writes into these vectors
	std::vector<int> left;
	std::vector<int> right;

	// The returned cost is the travelled distance
	double cost = minCostMatching ( costMatrix, left, right );

	/// calculate the move list

	// std::vector<simpleMove> moveList;
	moveList.resize ( cost, { 0,0,0,0 } );

	std::vector<std::vector<int> > matching ( numberTargets, std::vector<int> ( 4, 0 ) );

	// matching matrix, numberTargets x 4, Source and Target indice in each row
	for ( int targetInc = 0; targetInc < numberTargets; targetInc++ )
	{
		matching[ targetInc ][ 0 ] = sourceCoordinates[ right[ targetInc ] ][ 0 ];
		matching[ targetInc ][ 1 ] = sourceCoordinates[ right[ targetInc ] ][ 1 ];
		matching[ targetInc ][ 2 ] = targetCoordinates[ targetInc ][ 0 ];
		matching[ targetInc ][ 3 ] = targetCoordinates[ targetInc ][ 1 ];
	}

	int step_x, step_y, init_x, init_y;
	int counter = 0;

	// Setting up the moveSequence (only elementary steps) from the matching matrix (source - target)
	for ( int targetInc = 0; targetInc < numberTargets; targetInc++ )
	{
		step_x = matching[ targetInc ][ 2 ] - matching[ targetInc ][ 0 ];
		step_y = matching[ targetInc ][ 3 ] - matching[ targetInc ][ 1 ];
		init_x = matching[ targetInc ][ 0 ];
		init_y = matching[ targetInc ][ 1 ];
		for ( int xStepInc = 0; xStepInc < abs ( step_x ); xStepInc++ )
		{
			moveList[ counter ].initRow = init_x;
			moveList[ counter ].initCol = init_y;
			moveList[ counter ].finRow = init_x + sign_ ( step_x );
			moveList[ counter ].finCol = init_y;
			init_x = init_x + sign_ ( step_x );
			counter++;
		}
		for ( int yStepInc = 0; yStepInc < abs ( step_y ); yStepInc++ )
		{
			moveList[ counter ].initRow = init_x;
			moveList[ counter ].initCol = init_y;
			moveList[ counter ].finRow = init_x;
			moveList[ counter ].finCol = init_y + sign_ ( step_y );
			init_y = init_y + sign_ ( step_y );
			counter++;
		}
	}
	return cost;
}

void smartTargettingRearrangement ( Matrix<bool> source, Matrix<bool> target, niawgPair<ULONG>& finTargetPos,
									niawgPair<ULONG> finalPos, std::vector<simpleMove> &moveSequence )
{
	std::vector<simpleMove> moveList;
	Matrix<bool> finTarget ( source.getRows ( ), source.getCols ( ), 0 );

	try
	{
		// dimensions match, no flexibility.
		rearrangement ( source, target, moveList );
		finTargetPos = { 0,0 };
		finTarget = target;
	}
	catch ( Error& err )
	{
		std::string tmpStr = err.whatBare ( ).substr ( 0, 10 );
		if ( tmpStr == "Less atoms" )
		{
			if ( moveList.size ( ) == 0 )
			{
				// flip one atom and try again with less atoms.
				bool found = false;
				for ( auto& atom : target )
				{
					if ( atom )
					{
						atom = false;
						found = true;
						break;
					}
				}
				if ( found )
				{
					return;
				}
			}
		}
		else
		{
			throw;
		}
	}
	/// now order the operations.
	// can randomize first, otherwise the previous algorith always ends up filling the bottom left of the array first.
	/// not necessarily required for the simulation, but for sanity.
	orderMoves ( moveList, moveSequence, source );
}

int main ( )
{
	// use large base array so that should basically never be sampling edges of field.
	int n = 50;
	double lr = 0.4;
	int sn = 100;
	std::ofstream resFile ( ("lr_"+str(lr) + "_" + str(sn) + ".csv").c_str() );
	if ( !resFile.is_open ( ) )
	{
		std::cout << "failed to open result file!";
		std::cin.get ( );
		return 0;
	}

	std::random_device rd;
	std::mt19937 e2 ( rd ( ) );
	std::uniform_real_distribution<double> dist ( 0, 1 );
	/// loop through target sizes
	for ( auto m : range ( 25 ) )
	{
		m = m + 1;
		std::wcout << m << " ";
		Matrix<bool> targ ( n, n, 0 );
		for ( auto i : range ( m ) )
		{
			for ( auto j : range ( m ) )
			{
				targ ( i + std::round ( n / 2 - m / 2 ), j + std::round ( n / 2 - m / 2 ) ) = true;
			}
		}
		/// statistics loop
		double totalMoves=0;
		UINT countNum=0;
		resFile << m;
		for ( auto s : range ( sn ) )
		{
			std::cout << ".";
			Matrix<bool> init ( n, n, 0 );
			for ( auto& p : init )
			{
				p = dist ( e2 ) > 1 - lr;
			}
			niawgPair<ULONG> finPos = { 0,0 };
			std::vector<simpleMove> moves;
			smartTargettingRearrangement ( init, targ, finPos, { 0,0 }, moves );
			resFile << "," << moves.size ( );
		}
		resFile << "\n";
		std::wcout << "\n";
	}
	// for loading rate
	// for target size
	/// 
	std::wcout << "fin";
	std::cin.get ( );
	return 0;
}
