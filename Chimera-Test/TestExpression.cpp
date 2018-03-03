#include "stdafx.h"
#include "CppUnitTest.h"

#include <cmath>
#include <string>
#include "../Chimera/miscellaneousCommonFunctions.h"
#include "../Chimera/Expression.h"
#include "../Chimera/Thrower.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace TestExpressions
{
	TEST_CLASS( TestExpressions )
	{
		public:
		TEST_METHOD( InitExpression )
		{
			Expression expr1( "4" );
			Assert::AreEqual( expr1.expressionStr, std::string("4") );
			Expression expr2( "\tsin( 4 )" );
			Assert::AreEqual( expr2.expressionStr, std::string( "\tsin( 4 )" ) );
		}
		TEST_METHOD( ExpressionFailures )
		{
			for ( auto tmpStr : { "", "3^2", "3**2", "sin 4", "sin{4}", "sin[4]", "4sin(2)", "4 * var", "((1+2)", 
				  "(2" } )
			{
				Expression expr1( tmpStr );
				std::wstring errString = L"Expected the following expression to cause an exception upon evaluation: " 
					+ w_str( tmpStr );
				Assert::ExpectException<Error>( [this, &expr1] {expr1.evaluate( ); }, errString.c_str() );
			}
		}
		TEST_METHOD( BasicExpressionMathTests )
		{
			// note that the precision on each operation varies due to the different floating point operations being 
			// done. All operations tested here are at least 1e-11 precise.
			std::vector<std::string> txt = { "4", "4+5", "4 \t+    5", "4*5", "sin(0)", "cos(0)", "exp(0)", "4*exp(0)",
											 "cos(0)*exp(0)", "sin(1)", "sin( 1 \t)", "sin(4/1.5)", "sin(sin(2))", 
											 "cos(1)", "(4.9)", "(1+2)*3", "((1+4)*3+2)*sin(1)*(4+5)"};
			std::vector<double> res = { 4, 9, 9, 20, 0, 1, 1, 4, 
										1, sin(1), sin(1), sin(4/1.5), sin(sin(2)), 
										cos(1), 4.9, 9, 17*sin(1)*9};
			for ( auto count : range( txt.size() ) )
			{
				Expression expr( txt[count] );
				std::wstring errString = L"Expected the expression " + w_str(txt[count].c_str()) 
					+ L" to evaluate to " + w_str( res[count] );
				Assert::AreEqual( res[count], expr.evaluate( ), 1e-11, errString.c_str() );
			}
		}
	};
}
