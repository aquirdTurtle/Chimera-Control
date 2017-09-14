#pragma once

#include "keyHandler.h"
#include "VariableStructures.h"
#include "windows.h"
#include <string>
#include "KeyHandler.h"

class Expression
{
	public:
		Expression( );
		Expression( std::string expressionString );
		void assertValid( std::vector<variableType>& vars );
		double evaluate( key variationKey = {}, UINT variation = -1, 
						 std::vector<variableType>& vars = std::vector<variableType>( ) );
		static std::vector<std::string> splitString( std::string workingString );
		std::string expressionStr;
	private:
		void doMultAndDiv( std::vector<std::string>& terms );
		void doAddAndSub( std::vector<std::string>& terms );
		double reduce( std::vector<std::string> terms );
		void evaluateFunctions( std::vector<std::string>& terms );
};


// delayed setting of constant varible.
// don't want to make everything a pointer;
// 