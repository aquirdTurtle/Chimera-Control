#pragma once

#include "VariableStructures.h"
#include "windows.h"
#include <string>

class Expression
{
	public:
		Expression( );
		Expression( std::string expressionString );
		void assertValid( std::vector<variableType>& variables );
		// default values are empty objects and variation #-1.
		double evaluate( std::vector<variableType>& variables = std::vector<variableType>( ), UINT variation=-1 );
		static std::vector<std::string> splitString( std::string workingString );
		std::string expressionStr;
		bool varies( );
	private:
		void doMultAndDiv( std::vector<std::string>& terms );
		void doAddAndSub( std::vector<std::string>& terms );
		double reduce( std::vector<std::string> terms );
		void evaluateFunctions( std::vector<std::string>& terms );
		bool expressionVaries = false;
};

