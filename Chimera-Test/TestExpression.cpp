#include "stdafx.h"
#include "CppUnitTest.h"

#include <cmath>
#include <string>
#include "../Chimera/miscCommonFunctions.h"
#include "../Chimera/Expression.h"
#include "../Chimera/Thrower.h"
#include "../Chimera/ParameterSystem.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace TestExpressions
{
	TEST_CLASS( TestExpressions )
	{
		public:
		TEST_METHOD( Init_Expression )
		{
			Expression expr1( "4" );
			Assert::AreEqual( expr1.expressionStr, std::string("4") );
			Expression expr2( "\tsin( 4 )" );
			Assert::AreEqual( expr2.expressionStr, std::string( "\tsin( 4 )" ) );
		}
		TEST_METHOD( Evaluation_Failures )
		{
			for ( auto tmpStr : { "", "3^2", "3**2", "sin 4", "sin{4}", "sin[4]", "4sin(2)", "4 * var", "((1+2)", 
				  "(2", "1+-1", "*4-1", "SSS", "1s", "s1", "6e", "6e-", "e6" } )
			{
				Expression expr1( tmpStr );
				std::wstring errString = L"Expected the following expression to cause an exception upon evaluation: " 
					+ w_str( tmpStr );
				Assert::ExpectException<Error>( [this, &expr1] {expr1.evaluate( ); }, errString.c_str() );
			}
		}
		TEST_METHOD( Basic_Math_Tests )
		{
			// note that the precision on each operation varies due to the different floating point operations being 
			// done. All operations tested here are at least 1e-11 precise.
			std::vector<std::string> txt = { "6e3","6e0","6e03","6e-3", "6e-03", "4", "4+5", "4 \t+    5", "4*5", 
											 "sin(0)", "cos(0)", "exp(0)", "4*exp(0)", "cos(0)*exp(0)", "sin(1)", 
											 "sin( 1 \t)", "sin(4/1.5)", "sin(sin(2))", "cos(1)", "(4.9)", "(1+2)*3", 
											 "((1+4)*3+2)*sin(1)*(4+5)", "-1", "-1-3", "4*(-(1+3))", "cos(-1)","1+(-1)"};
			std::vector<double> res = { 6e3, 6e0, 6e03, 6e-3, 6e-03, 4, 9, 9, 20, 0, 1, 1, 4, 
										1, sin(1), sin(1), sin(4/1.5), sin(sin(2)), 
										cos(1), 4.9, 9, 17*sin(1)*9, -1, -4, 
									    -16, cos(-1), 0};
			for ( auto count : range( txt.size() ) )
			{
				Expression expr( txt[count] );
				std::wstring errString = L"Expected the expression " + w_str(txt[count].c_str()) 
					+ L" to evaluate to " + w_str( res[count] );
				Assert::AreEqual( res[count], expr.evaluate( ), 1e-11, errString.c_str() );
			}
		}
		TEST_METHOD( Check_Expression_Is_Valid )
		{
			for ( auto tmpStr : { "", "1+++4","3^2", "3**2", "sin 4", "sin{4}", "sin[4]", "4sin(2)", "4 * var", "((1+2)",
				  "(2", "1+-1", "*4+2" } )
			{
				Expression expr1( tmpStr );
				std::wstring errString = L"Expected the following expression to cause an exception upon assertValid: \""
					+ w_str( tmpStr ) + L"\"";
				Assert::ExpectException<Error>( [this, &expr1] {expr1.assertValid( std::vector<parameterType>(), NO_PARAMETER_SCOPE); },
												errString.c_str( ) );
			}
		}
		TEST_METHOD( Assert_Valid_With_Global_Constant )
		{
			ScanRangeInfo rangeInfo;
			rangeInfo.defaultInit ( );
			rangeInfo ( 0, 0 ) = { 1, false, false };
			std::vector<std::vector<parameterType>> params( 1 );
			parameterType param;
			param.name = "testVar";
			param.constant = true;
			param.constantValue = 1;
			param.ranges = std::vector<indvParamRangeInfo>( 1, { 1,2 } );
			param.parameterScope = "global";
			params[0].push_back( param );
			ParameterSystem::generateKey( params, false, rangeInfo );

			Expression expr( "testVar" );
			// make sure it fails without constant
			Assert::ExpectException<Error>( [this, &expr] {expr.assertValid( std::vector<parameterType>( ), NO_PARAMETER_SCOPE ); });
			// make sure succeeds with constant.
			expr.assertValid( params[0], NO_PARAMETER_SCOPE );
		}
		TEST_METHOD( Assert_Valid_With_Global_Variable )
		{
			ScanRangeInfo rangeInfo;
			rangeInfo.defaultInit ( );
			rangeInfo ( 0, 0 ) = { 3, false, false };
			std::vector<std::vector<parameterType>> params( 1 );
			parameterType param;
			param.name = "testVar";
			param.constant = false;
			param.constantValue = 1;
			param.ranges = std::vector<indvParamRangeInfo>( 1, { 1,2 } );
			param.parameterScope = "global";
			params[0].push_back( param );
			ParameterSystem::generateKey( params, false, rangeInfo );
			Expression expr( "testVar" );
			// make sure it fails without constant
			Assert::ExpectException<Error>( [this, &expr] {expr.assertValid( std::vector<parameterType>( ), NO_PARAMETER_SCOPE ); } );
			// make sure succeeds with variable.
			expr.assertValid( params[0], NO_PARAMETER_SCOPE );
		}
		TEST_METHOD( Assert_Valid_With_Local_Constant )
		{
			ScanRangeInfo rangeInfo;
			rangeInfo.defaultInit ( );
			rangeInfo ( 0, 0 ) = { 1, false, false };
			std::vector<std::vector<parameterType>> params( 1 );
			parameterType param;
			param.name = "testVar";
			param.constant = true;
			param.constantValue = 1;
			param.ranges = std::vector<indvParamRangeInfo>( 1, { 1,2 } );
			param.parameterScope = "TestScope";
			params[0].push_back( param );
			ParameterSystem::generateKey( params, false, rangeInfo );

			Expression expr( "testVar" );
			// make sure it fails without constant
			Assert::ExpectException<Error>( [this, &expr] {expr.assertValid( std::vector<parameterType>( ), "TestScope" ); } );
			Assert::ExpectException<Error>( [this, &expr, &params] {expr.assertValid( params[0], "OtherScope" ); } );
			// make sure succeeds with constant.
			expr.assertValid( params[0], "TestScope" );
		}
		TEST_METHOD( Assert_Valid_With_Local_Variable )
		{
			std::vector<std::vector<parameterType>> params( 1 );
			ScanRangeInfo rangeInfo;
			rangeInfo.defaultInit ( );
			rangeInfo ( 0, 0 ) = { 3, false, false };
			parameterType param;
			param.name = "testVar";
			param.constant = false;
			param.constantValue = 1;
			param.ranges = std::vector<indvParamRangeInfo>( 1, { 1,2 } );
			param.parameterScope = "TestScope";
			params[0].push_back( param );
			ParameterSystem::generateKey( params, false, rangeInfo );
			Expression expr( "testVar" );

			// make sure it fails without constant
			Assert::ExpectException<Error>( [this, &expr] {expr.assertValid( std::vector<parameterType>( ), "TestScope" ); } );
			Assert::ExpectException<Error>( [this, &expr, &params] {expr.assertValid( params[0], "OtherScope" ); } );
			// make sure succeeds with variable.
			expr.assertValid( params[0], "TestScope" );
		}
		TEST_METHOD( Evaluate_With_Global_Constant )
		{
			ScanRangeInfo rangeInfo;
			rangeInfo.defaultInit ( );
			rangeInfo ( 0, 0 ) = { 1, false, false };
			std::vector<std::vector<parameterType>> params( 1 );
			parameterType param;
			param.name = "testVar";
			param.constant = true;
			param.constantValue = 1;
			param.ranges = std::vector<indvParamRangeInfo>( 1, { 1,2 } );
			param.parameterScope = "global";
			params[0].push_back( param );
			ParameterSystem::generateKey( params, false, rangeInfo );

			Expression expr( "testVar" );
			// make sure it fails without constant
			Assert::ExpectException<Error>( [this, &expr] {expr.evaluate(); } );
			// make sure succeeds with constant.
			expr.evaluate( params[0], 0 );
		}
		TEST_METHOD( Evaluate_With_Global_Variable )
		{
			ScanRangeInfo rangeInfo;
			rangeInfo.defaultInit ( );
			rangeInfo ( 0, 0 ) = { 3, false, false };
			std::vector<std::vector<parameterType>> params( 1 );
			parameterType param;
			param.name = "testVar";
			param.constant = false;
			param.constantValue = 1;
			param.ranges = std::vector<indvParamRangeInfo>( 1, { 1,2 } );
			param.parameterScope = "global";
			params[0].push_back( param );
			ParameterSystem::generateKey( params, false, rangeInfo );
			Expression expr( "testVar" );
			// make sure it fails without variable
			Assert::ExpectException<Error>( [this, &expr] {expr.evaluate( ); } );
			// make sure succeeds with variable.
			expr.evaluate( params[0], 0 );
		}
		TEST_METHOD( Evaluate_With_Local_Constant )
		{
			std::vector<std::vector<parameterType>> params( 1 );
			ScanRangeInfo rangeInfo;
			rangeInfo.defaultInit ( );
			rangeInfo ( 0, 0 ) = { 1, false, false };
			parameterType param;
			param.name = "testVar";
			param.constant = true;
			param.constantValue = 4;
			param.ranges = std::vector<indvParamRangeInfo>( 1, { 1,2 } );
			param.parameterScope = "TestScope";
			params[0].push_back( param );
			ParameterSystem::generateKey( params, false, rangeInfo );

			Expression expr( "testVar" );
			expr.assertValid( params[0], "TestScope" );
			// make sure it fails without constant
			Assert::ExpectException<Error>( [this, &expr] {expr.evaluate( ); } );
			// make sure succeeds with constant.
			Assert::AreEqual(4.0, expr.evaluate( params[0], 0 ));
			// make sure fails if constant out of scope.
			params[0].front().parameterScope = "OtherScope";
			Assert::ExpectException<Error>( [this, &expr, &params] {expr.evaluate(params[0], 0 ); } );
		}
		TEST_METHOD( Evaluate_With_Local_Variable )
		{
			std::vector<std::vector<parameterType>> params( 1 );
			ScanRangeInfo rangeInfo;
			rangeInfo.defaultInit ( );
			rangeInfo ( 0, 0 ) = { 3, false, false };
			parameterType param;
			param.name = "testVar";
			param.constant = false;
			param.constantValue = 1;
			param.ranges = std::vector<indvParamRangeInfo>( 1, { 1,2 } );
			param.parameterScope = "TestScope";
			params[0].push_back( param );
			ParameterSystem::generateKey( params, false, rangeInfo );
			Expression expr( "testVar" );
			expr.assertValid( params[0], "TestScope" );
			// make sure it fails without Variable
			Assert::ExpectException<Error>( [this, &expr] {expr.evaluate( ); } );
			// make sure succeeds with variable.
			Assert::AreEqual( 1.25, expr.evaluate( params[0], 0 ) );
			Assert::AreEqual( 1.5, expr.evaluate( params[0], 1 ) );
			Assert::AreEqual( 1.75, expr.evaluate( params[0], 2 ) );
			// make sure fails if variable out of scope.
			params[0].front( ).parameterScope = "OtherScope";
			Assert::ExpectException<Error>( [this, &expr, &params] {expr.evaluate( params[0], 0 ); } );
		}
		TEST_METHOD ( Evaluate_ComplicatedExpressionWithVariables )
		{
			std::vector<std::vector<parameterType>> params ( 1 );
			ScanRangeInfo rangeInfo;
			rangeInfo.defaultInit ( );
			rangeInfo ( 0, 0 ) = { 3, false, false };
			parameterType param;
			param.name = "field";
			param.constant = false;

			param.constantValue = 2;
			param.parameterScope = "TestScope";
			param.ranges = std::vector<indvParamRangeInfo> ( 1, { 1,2 } );
			params[ 0 ].push_back ( param );
			param.name = "fieldtheta";
			param.constant = true;
			param.constantValue = 1;
			param.parameterScope = "TestScope";
			params[ 0 ].push_back ( param );
			ParameterSystem::generateKey ( params, false, rangeInfo );
			Expression expr ( "(2.049+2.1*field/(-0.716623)*cos(fieldtheta/2+3.1415926/4)/2)" );
			expr.assertValid ( params[ 0 ], "TestScope" );
			// make sure it fails without Variable
			Assert::ExpectException<Error> ( [ this, &expr ] { expr.evaluate ( ); } );
			// make sure succeeds with variable.
			Assert::AreEqual ( 2.049 + 2.1*1.25 / ( -0.716623 )* cos ( 1. / 2. + 3.1415926 / 4. ) / 2.,
							   expr.evaluate ( params[ 0 ], 0 ), 1e-8 );
			Assert::AreEqual ( ( 2.049 + 2.1*1.5 / ( -0.716623 )*cos ( 1. / 2. + 3.1415926 / 4. ) / 2. ), 
							   expr.evaluate ( params[ 0 ], 1 ), 1e-8 );
			Assert::AreEqual ( ( 2.049 + 2.1*1.75 / ( -0.716623 )*cos ( 1. / 2. + 3.1415926 / 4. ) / 2. ),
							   expr.evaluate ( params[ 0 ], 2 ), 1e-8 );
			// make sure fails if variable out of scope.
			params[ 0 ].front ( ).parameterScope = "OtherScope";
			Assert::ExpectException<Error> ( [ this, &expr, &params ] { expr.evaluate ( params[ 0 ], 0 ); } );
		}

	};
}
