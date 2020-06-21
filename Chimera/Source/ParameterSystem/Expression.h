// created by Mark O. Brown
#pragma once

#include "ParameterSystem/ParameterSystemStructures.h"
#include "afxwin.h"
#include <string>
#include <vector>
class ConfigStream;

class Expression
{
	public:
		Expression( );
		Expression( std::string expressionString );
		//Expression (const std::string&) = delete;
		void assertValid( std::vector<parameterType>& variables, std::string scope );
		// default values are empty objects and variation #-1.
		double evaluate( std::vector<parameterType>& variables = std::vector<parameterType>( ), UINT variation=-1 );
		static std::vector<std::string> splitString( std::string workingString );
		std::string expressionStr;
		bool varies( );
		void internalEvaluate ( std::vector<parameterType>& variables = std::vector<parameterType> ( ), 
			UINT totalVariationNumber = -1 );
		double getValue ( UINT variation );
		// not sure why, but need to use ofstream instead of stringstream or else I get very strange errors. 
		// stringstream inherits from both basic_ofstream and basic_istream, so it makes sense that this works, but
		// it doesn't make sense to me that using stringstream doesn't work.
	
		//friend ConfigStream& operator<<(ConfigStream& os, const Expression& expr);
	private:
		void doMultAndDiv( std::vector<std::string>& terms );
		void doAddAndSub( std::vector<std::string>& terms );
		double reduce( std::vector<std::string> terms );
		void evaluateFunctions( std::vector<std::string>& terms );
		bool expressionVaries = false;
		std::string expressionScope;
		std::vector<double> values;
};