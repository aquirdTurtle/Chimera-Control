#pragma once

#include "PiezoFlume.h"
#include "Expression.h"
#include "Version.h"

/* a simple wrapper for parameters for which there is one value for each channel, e.g. a double or an expression.*/
template <typename type> struct piezoChan
{
	type x;
	type y;
	type z;
};

class PiezoCore
{
	public:
		PiezoCore (bool safemode, std::string sn, std::string delim );
		void initialize (  );
		std::string getDeviceInfo ( );
		std::string getDeviceList ( );
		void exprProgramPiezo ( UINT sequenceNumber, UINT variationNumber );
		static piezoChan<std::string> getPiezoValsFromConfig ( std::ifstream& file, Version ver );
		void programAll ( piezoChan<double> vals );
		double getCurrentXVolt ( );
		void programXNow ( double val );
		double getCurrentYVolt ( );
		void programYNow ( double val );
		double getCurrentZVolt ( );
		void programZNow ( double val );
		void updateExprVals ( std::vector<piezoChan<Expression>> newVals );
		void evaluateVariations ( std::vector<std::vector<parameterType>>& params, UINT totalVariations );
		const std::string configDelim;
	private:
		PiezoFlume flume;
		std::vector<piezoChan<Expression>> experimentVals;

};