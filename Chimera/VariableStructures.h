// created by Mark O. Brown
#pragma once
#include <string>
#include "afxwin.h"
#include <vector>
#include "range.h"
#include <unordered_map>
#include "Thrower.h"

#define GLOBAL_PARAMETER_SCOPE "global"
#define PARENT_PARAMETER_SCOPE "parent"
#define NO_PARAMETER_SCOPE "__NO__"

// individual to a given parameter.
struct indvParamRangeInfo
{
double initialValue;
double finalValue;
bool operator==( const indvParamRangeInfo& v )
{
	return ( initialValue == v.initialValue && finalValue == v.finalValue );
}
};

// shared between all parameters.
struct IndvRangeInfo
{
unsigned int variations;
bool leftInclusive;
bool rightInclusive;
bool operator==( const IndvRangeInfo& v )
{
	return ( variations == v.variations &&
			 leftInclusive == v.leftInclusive &&
			 rightInclusive == v.rightInclusive );
}
};


class ScanRangeInfo
{
	/*
	This class is basically a wrapper around a 2d vector
	*/
	private:
	std::vector<std::vector<IndvRangeInfo>> data;
	const IndvRangeInfo defaultRangeInfo = { 2,false,true };
	public:
		void removeDim ( UINT which )
		{
			data.erase ( data.begin ( ) + which );
		}
		ScanRangeInfo& operator=( const ScanRangeInfo& v )
		{
			data = v.data;
			return *this;
		}
		void reset ( )
		{
			data.clear ( );
		}
		void defaultInit ( )
		{
			setNumScanDimensions ( 1 );
			setNumRanges ( 0, 1 );
			( *this )( 0, 0 ) = defaultRangeInfo;
		}
		std::vector<IndvRangeInfo>& dimensionInfo ( UINT dimIndex )
		{
			if ( dimIndex >= data.size ( ) )
			{
				thrower ( "Attempted to access scan-range-info dimension which does not exist!" );
			}
			return data[ dimIndex ];
		}

		IndvRangeInfo operator()( UINT scanDimIndex, UINT rangeIndex ) const
		{
			if ( scanDimIndex >= data.size ( ) )
			{
				thrower ( "Tried to access scan dimension that does not exist." );
			}
			if ( rangeIndex >= data[ scanDimIndex ].size ( ) )
			{
				thrower ( "Tried to access range number that does not exist" );
			}
			return data[ scanDimIndex ][ rangeIndex ];
		}

		IndvRangeInfo & operator()( UINT scanDimIndex, UINT rangeIndex )
		{
			if ( scanDimIndex >= data.size ( ) )
			{
				thrower ( "Tried to access scan dimension that does not exist." );
			}
			if ( rangeIndex >= data[ scanDimIndex ].size ( ) )
			{
				thrower ( "Tried to access range number that does not exist" );
			}
			return data[ scanDimIndex ][ rangeIndex ];
		}
		UINT numScanDimensions ( )
		{
			return data.size ( );
		}
		UINT numRanges ( UINT scanDimIndex )
		{
			if ( scanDimIndex >= data.size ( ) )
			{
				thrower ( "Tried to access scan dimension that does not exist." );
			}
			return data[ scanDimIndex ].size ( );
		}
		void setNumScanDimensions ( UINT numDimensions )
		{
			UINT initSize = data.size ( );
			data.resize ( numDimensions );
			if ( data.size ( ) > initSize )
			{
				for ( auto newDim : range ( data.size ( ) - initSize ) )
				{
					data[ initSize + newDim ].push_back ( defaultRangeInfo );
				}
			}
		}
		void setNumRanges ( UINT scanDimIndex, UINT numRanges )
		{
			if ( numRanges > data[ scanDimIndex ].size ( ) )
			{
				while ( data[ scanDimIndex ].size ( ) < numRanges )
				{
					data[ scanDimIndex ].push_back ( defaultRangeInfo );
				}
			}
			else
			{
				data[ scanDimIndex ].resize ( numRanges );
			}
		}
};

struct parameterType
{
	std::string name;
	// whether this variable is constant or varies.
	bool constant;
	double constantValue = 0;
	bool active = false;
	bool overwritten = false;

	// records which scan dimension the variable is in.
	USHORT scanDimension=0;
	std::vector<indvParamRangeInfo> ranges;
	/// this stuff used to be in the keyHandler system.
	std::vector<double> keyValues;
	// this might just be redundant with constant above...
	bool valuesVary=false;
	std::string parameterScope = GLOBAL_PARAMETER_SCOPE;

	bool operator==( const parameterType& v )
	{
		bool rangeResult=true;
		if ( ranges.size( ) != v.ranges.size( ) )
		{
			rangeResult = false;
		}
		else
		{
			for ( auto rangeInc : range( ranges.size()) )
			{
				if ( !(ranges[rangeInc] == v.ranges[rangeInc] ))
				{
					rangeResult = false;
					break;
				}
			}
		}
		return ( name == v.name &&
				 constant == v.constant &&
				 constantValue == v.constantValue &&
				 active == v.active &&
				 overwritten == v.overwritten &&
				 scanDimension == v.scanDimension &&
				 rangeResult &&
				 keyValues == v.keyValues &&
				 valuesVary == v.valuesVary);
	}
};

typedef std::unordered_map<std::string, std::vector<parameterType>> funcVarMap;
