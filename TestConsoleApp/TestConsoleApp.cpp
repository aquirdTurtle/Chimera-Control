#include "stdafx.h" 
#include <iostream>
#include <vector>
#include <fstream>
#include <string>

using namespace std;


int main( )
{
	std::ifstream biasFile;
	biasFile.open ("J:/Code-Files/Bias-Cal.txt");
	std::string name1, name2, name, row, col, val, trash, dir;
	while ( true )
	{

		std::getline ( biasFile, trash, '[' );
		std::getline ( biasFile, row, ',' );
		std::getline ( biasFile, col, ']' );
		std::getline ( biasFile, trash, ',' );
		std::getline ( biasFile, dir, ':' );
		std::getline ( biasFile, val );
		if ( biasFile.eof ( ) )
		{
			break;
		}
		cout << row << "," << col << ' ' << dir << " " << val << endl;
	}
	cin.get ( );
	return 0;
}

