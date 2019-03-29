// created by Mark O. Brown
#pragma once
#include <string>
#include "afxwin.h"
#include <vector>
#include "range.h"
#include <unordered_map>


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
		return (initialValue == v.initialValue && finalValue == v.finalValue);
	}
};

// shared between all parameters.
struct variationRangeInfo
{
	unsigned int variations;
	bool leftInclusive;
	bool rightInclusive;
	bool operator==( const variationRangeInfo& v )
	{
		return ( variations == v.variations &&
				 leftInclusive == v.leftInclusive &&
				 rightInclusive == v.rightInclusive);
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
	USHORT scanDimension=1;
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
