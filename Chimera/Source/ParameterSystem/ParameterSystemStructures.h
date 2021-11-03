// created by Mark O. Brown
#pragma once

#include "GeneralUtilityFunctions/range.h"
#include "GeneralUtilityFunctions/Thrower.h"
#include <string>
#include <vector>
#include <unordered_map>

#define GLOBAL_PARAMETER_SCOPE "global"
#define PARENT_PARAMETER_SCOPE "parent"
#define NO_PARAMETER_SCOPE "__NO__"


// individual to a given parameter.
struct indvParamRangeInfo{
	double initialValue;
	double finalValue;
	bool operator==( const indvParamRangeInfo& v ){
		return ( initialValue == v.initialValue && finalValue == v.finalValue );
	}
};

// shared between all parameters. not sure why the inclusivity needs to be shared. maybe something to do with the
// algorithm currently used to generate vals. 
struct IndvRangeInfo{
	unsigned int variations;
	bool leftInclusive;
	bool rightInclusive;
	bool operator==( const IndvRangeInfo& v ){
		return ( variations == v.variations &&
				 leftInclusive == v.leftInclusive &&
				 rightInclusive == v.rightInclusive );
	}
};


class ScanRangeInfo{
	/*This class is basically a wrapper around a 2d vector
	*/
	private:
	const IndvRangeInfo defaultRangeInfo = { 2,false,true };
	public:
		std::vector<std::vector<IndvRangeInfo>> data;
		void removeDim ( unsigned which )		{
			data.erase ( data.begin ( ) + which );
		}
		ScanRangeInfo& operator=( const ScanRangeInfo& v )		{
			data = v.data;
			return *this;
		}
		void reset ( ){
			data.clear ( );
		}
		void defaultInit ( ){
			setNumScanDimensions ( 1 );
			setNumRanges ( 0, 1 );
			( *this )( 0, 0 ) = defaultRangeInfo;
		}
		std::vector<IndvRangeInfo>& dimensionInfo ( unsigned dimIndex )	{
			if ( dimIndex >= data.size ( ) ){
				thrower ( "IndvRangeInfo: Attempted to access scan-range-info dimension which does not exist!" );
			}
			return data[ dimIndex ];
		}

		IndvRangeInfo operator()( unsigned scanDimIndex, unsigned rangeIndex ) const{
			if ( scanDimIndex >= data.size ( ) ){
				thrower ( "IndvRangeInfo: Tried to access scan dimension that does not exist." );
			}
			if ( rangeIndex >= data[ scanDimIndex ].size ( ) ){
				thrower ( "Tried to access range number that does not exist" );
			}
			return data[ scanDimIndex ][ rangeIndex ];
		}

		IndvRangeInfo & operator()( unsigned scanDimIndex, unsigned rangeIndex ){
			if ( scanDimIndex >= data.size ( ) ){
				thrower ( "IndvRangeInfo: Tried to access scan dimension that does not exist." );
			}
			if ( rangeIndex >= data[ scanDimIndex ].size ( ) ){
				thrower ( "IndvRangeInfo: Tried to access range number that does not exist" );
			}
			return data[ scanDimIndex ][ rangeIndex ];
		}
		size_t numScanDimensions ( ){
			return data.size ( );
		}
		size_t numRanges ( unsigned scanDimIndex ){
			if ( scanDimIndex >= data.size ( ) ){
				thrower ( "IndvRangeInfo: Tried to access scan dimension that does not exist." );
			}
			return data[ scanDimIndex ].size ( );
		}
		void setNumScanDimensions ( unsigned numDimensions ){
			auto initSize = data.size ( );
			data.resize ( numDimensions );
			if ( data.size ( ) > initSize ){
				for ( auto newDim : range ( data.size ( ) - initSize ) ){
					data[ initSize + newDim ].push_back ( defaultRangeInfo );
				}
			}
		}
		void setNumRanges ( unsigned scanDimIndex, unsigned numRanges ){
			if ( numRanges > data[ scanDimIndex ].size ( ) ){
				while ( data[ scanDimIndex ].size ( ) < numRanges ){
					data[ scanDimIndex ].push_back ( defaultRangeInfo );
				}
			}
			else{
				data[ scanDimIndex ].resize ( numRanges );
			}
		}
};


struct vectorizedNiawgVals{
	std::string name;
	std::vector<std::string> vals;
};


struct parameterType {
	parameterType () { };
	parameterType (int rangeSize){
		for (auto rangeVariations : range(rangeSize)){
			ranges.push_back ({ 0, 0 });
		}
	}
	std::string name="x";
	// whether this variable is constant or varies.
	bool constant=false;
	double constantValue = 0;
	bool active = false;
	bool overwritten = false;

	// records which scan dimension the variable is in.
	unsigned short scanDimension=0;
	std::vector<indvParamRangeInfo> ranges;
	/// this stuff used to be in the keyHandler system.
	std::vector<double> keyValues;
	// this might just be redundant with constant above...
	bool valuesVary=false;
	std::string parameterScope = GLOBAL_PARAMETER_SCOPE;

	bool operator==( const parameterType& v ){
		bool rangeResult=true;
		if ( ranges.size( ) != v.ranges.size( ) ){
			rangeResult = false;
		}
		else{
			for ( auto rangeInc : range( ranges.size()) ){
				if ( !(ranges[rangeInc] == v.ranges[rangeInc] )){
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

Q_DECLARE_METATYPE (parameterType)
Q_DECLARE_METATYPE (std::vector<parameterType>)

typedef std::unordered_map<std::string, std::vector<parameterType>> funcVarMap;
