#include "stdafx.h" 
#include <iostream>
#include <chrono>
#include <vector>
#include "afxwin.h"
#include <fstream>
#include <string>
#include "../Chimera/rerngStructures.h"

using namespace std;
using namespace chrono;


int main( )
{
	std::vector<simpleMove> moveList;
	moveList.push_back ( { 1,1,2,2, 3 } );
	moveList.push_back ( { 2,2,3,3, 2 } );
	moveList.push_back ( { 3,3,4,4, 1 } );
	moveList.push_back ( { 4,4,5,5, 2 } );
	moveList.push_back ( { 5,5,6,6, 3 } );
	std::sort ( moveList.begin ( ), moveList.end ( ),
				[] ( simpleMove const& a, simpleMove const& b ) { return a.distanceToTarget < b.distanceToTarget; } );
	for ( auto move : moveList )
	{
		cout << move.distanceToTarget << endl;
	}
	cin.get( );
	return 0;
}

