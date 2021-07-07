// created by Mark O. Brown
#pragma once

#include "ParameterSystem/ParameterSystemStructures.h"
#include <string>
#include <vector>
#include <AnalogInput/calInfo.h>

class Expression {
	public:
		Expression( );
		Expression( std::string expressionString );
		void assertValid( std::vector<parameterType>& variables, std::string scope );
		// default values are empty objects and variation #-1.
		static std::vector<std::string> splitString( std::string workingString );
		std::string expressionStr;
		std::string calName = "";
		bool varies( );
		double evaluate (std::vector<std::reference_wrapper<parameterType>> & variables, unsigned variation = -1,
						 std::vector<calResult> calibrations = std::vector<calResult> ());
		double evaluate ( std::vector<parameterType>& variables = std::vector<parameterType> (), unsigned variation = -1,
						  std::vector<calResult> calibrations = std::vector<calResult>());
		void internalEvaluate ( std::vector<parameterType>& variables = std::vector<parameterType> ( ),
			unsigned totalVariationNumber = -1 );
		double handleCalibration (double val, std::vector<calResult> calibrations);
		double getValue ( unsigned variation ) const;	
		std::string stringRepr() const;
		std::string getScope();
	private:
		void doMultAndDiv( std::vector<std::string>& terms );
		void doAddAndSub( std::vector<std::string>& terms );
		double reduce( std::vector<std::string> terms );
		void evaluateFunctions( std::vector<std::string>& terms );
		bool expressionVaries = false;
		std::string expressionScope;
		std::vector<double> values;
};