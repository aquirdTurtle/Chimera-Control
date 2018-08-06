// TestConsoleApp2.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "CodeTimer.h"
#include "range.h"
#include "afxwin.h"
#include <iostream>
#include <memory>
#include <string>
#include <vector>

int main()
{
	CodeTimer timer;
	std::vector<BYTE> dataArray;
	for ( auto i : range ( 512 * 512 ) )
	{
		dataArray.push_back(i);
	}
	timer.tick ( "Init" );
	for ( auto i : range ( 100 ) )
	{
		std::vector<BYTE> finalDataArray ( 512 * 512 * 4, 255 );
		for ( int dataInc = 0; dataInc < 512 * 512; dataInc++ )
		{
			int data = dataArray[ dataInc ];
			finalDataArray[ 4 * dataInc ] = data;
			finalDataArray[ 4 * dataInc + 1 ] = data;
			finalDataArray[ 4 * dataInc + 2 ] = data;
			finalDataArray[ 4 * dataInc + 3 ] = data;
		}
	}
	timer.tick ( "End-1" );
	for ( auto i : range ( 100 ) )
	{
		std::vector<BYTE> arr2;
		for ( int dataInc = 0; dataInc < 512 * 512; dataInc++ )
		{
			int data = dataArray[ dataInc ];
			arr2.push_back ( data );
			arr2.push_back ( data );
			arr2.push_back ( data );
			arr2.push_back ( data );
		}
	}
	timer.tick ( "End-2" );
	for ( auto i : range ( 100 ) )
	{
		std::vector<BYTE> arr3;
		for ( int dataInc = 0; dataInc < 512 * 512; dataInc++ )
		{
			std::vector<BYTE> data = { dataArray[ dataInc ],dataArray[ dataInc ],dataArray[ dataInc ],dataArray[ dataInc ] };
			arr3.insert ( arr3.end ( ), data.begin ( ), data.end ( ) );
		}
	}
	timer.tick ( "End-3" );
	for ( auto i : range ( 100 ) )
{
		std::vector<BYTE> finalDataArray ( 512 * 512 * 4, 255 );
		for ( auto d : range(finalDataArray.size() ))
		{
			finalDataArray[d] = dataArray[ int( d / 4) ];
		}
	}
	timer.tick ( "End-4" );
	std::cout << timer.getTimingMessage ( ); 
	std::cin.get ( );
}

