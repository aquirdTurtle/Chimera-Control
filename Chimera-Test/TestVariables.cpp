#include "stdafx.h"
#include "CppUnitTest.h"
#include "afxwin.h"
#include "../Chimera/ParameterSystem.h"
#include "../Chimera/resource.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;


namespace TestVariables
{
	TEST_CLASS( TestVariableSystem )	
	{
		public:
		
		TEST_METHOD( InitVariableSystem )
		{
			ParameterSystem testSys( "CONFIG_PARAMETERS" );
			Assert::AreEqual(testSys.getAllConstants( ).size( ), size_t(0));
			Assert::AreEqual( testSys.getAllVariables( ).size( ), size_t( 0 ) );
			Assert::AreEqual( testSys.getAllParams( ).size( ), size_t( 0 ) );
			Assert::AreEqual( testSys.getCurrentNumberOfVariables( ), UINT( 0 ) );
		}
		TEST_METHOD( AddConstant )
		{
			parameterType var;
			var.name = "testvariable";
			var.constant = true;
			var.constantValue = 0.0;
			var.ranges.push_back( { 1,10 } );
			ParameterSystem testSys( "CONFIG_PARAMETERS" );
			testSys.addParameter( var );
			Assert::AreEqual( UINT( 1 ), testSys.getCurrentNumberOfVariables( ) );
			Assert::AreEqual( size_t( 1 ), testSys.getAllConstants( ).size() );
			Assert::AreEqual( size_t( 0 ), testSys.getAllVariables( ).size() );
			Assert::AreEqual( size_t( 1 ), testSys.getAllParams( ).size( ) );
			auto var2 = testSys.getAllConstants( )[0];
			Assert::IsTrue( var2 == var );
		}
		TEST_METHOD ( Generate2DimensionalKey )
		{
			ScanRangeInfo rangeInfo;
			rangeInfo.defaultInit ( );
			rangeInfo.setNumScanDimensions ( 2 );
			rangeInfo.setNumRanges ( 1, 1 );
			rangeInfo ( 0, 0 ) = { 10,false,false };
			rangeInfo ( 1, 0 ) = { 20,true,false };
			parameterType var, var2;
			var.name = "testvariable";
			var.constant = false;
			var.constantValue = 0.0;
			
			var2 = var;
			var2.name = "test2";
			var2.scanDimension = 1;
			var2.ranges.push_back ( {0,10} );
			std::vector<double> expectedresult_s1 = { 1,2,3,4,5,6,7,8,9,10 };
			std::vector<double> expectedresult_s2 = { 0,0.5,1,1.5,2,2.5,3,3.5,4,4.5,5,5.5,
				6,6.5,7,7.5,8,8.5,9,9.5 };
			std::vector<double> expectedresult_full1;
			std::vector<double> expectedresult_full2;
			for ( auto s1 : expectedresult_s1 )
			{
				for ( auto s2 : expectedresult_s2 )
				{
					expectedresult_full1.push_back ( s1 );
					expectedresult_full2.push_back ( s2 );
				}
			}
			var.ranges.push_back ( { 0,11 } );
			ParameterSystem testSys ( "CONFIG_PARAMETERS" );
			testSys.addParameter ( var );
			testSys.addParameter ( var2 );
			std::vector<std::vector<parameterType>> vars;
			vars.push_back ( testSys.getAllParams( ) );
			testSys.generateKey ( vars, false, rangeInfo );
			Assert::IsTrue ( vars.size ( ) == 1 );
			Assert::IsTrue ( vars[ 0 ].size ( ) == 2 );
			Assert::IsTrue ( vars[ 0 ][ 0 ].keyValues.size ( ) == expectedresult_full1.size() );
			Assert::IsTrue ( vars[ 0 ][ 1 ].keyValues.size ( ) == expectedresult_full2.size ( ) );
			Assert::IsTrue ( expectedresult_full1.size ( ) == expectedresult_full2.size ( ) );
			for ( auto count : range ( expectedresult_full1.size ( ) ) )
			{
				Assert::AreEqual ( expectedresult_full1[ count ], vars[ 0 ][ 0 ].keyValues[ count ] );
				Assert::AreEqual ( expectedresult_full2[ count ], vars[ 0 ][ 1 ].keyValues[ count ] );
			}
		}
		TEST_METHOD ( Generate5DimensionalKey )
		{
			ScanRangeInfo rangeInfo;
			rangeInfo.defaultInit ( );
			rangeInfo.setNumScanDimensions ( 5 );
			rangeInfo.setNumRanges ( 1, 1 );
			std::vector<UINT> numVariations = { 5,6,7,8,9 };
			for ( auto inc : range ( numVariations.size ( ) ) )
			{
				rangeInfo ( inc, 0 ) = { numVariations[inc], true, false };
			}
			std::vector<parameterType> vars_ ( 5 );
			vars_[ 0 ].name = "test";
			vars_[ 0 ].constant = false;
			vars_[ 0 ].constantValue = 0.0;
			vars_[ 0 ].ranges.push_back ( { 0,5 } );
			auto valCount = 0;
			std::vector<std::vector<double>> expectedResults_singles(5);
			for ( auto v_inc : range(vars_.size()) )
			{
				auto& v = vars_[ v_inc ];
				v = vars_[ 0 ];
				v.name += str ( v_inc );
				v.ranges[ 0 ].initialValue = valCount;
				valCount += numVariations[ v_inc ];
				v.ranges[ 0 ].finalValue = valCount;
				v.scanDimension = v_inc;
				for ( auto valInc : range ( numVariations[ v_inc ] ))
				{
					expectedResults_singles[ v_inc ].push_back ( v.ranges[ 0 ].initialValue + valInc );
				}
			}
			std::vector<std::vector<double>> expectedResultsFull(5);
			for ( auto val1 : expectedResults_singles[ 0 ] )
			{
				for ( auto val2 : expectedResults_singles[ 1 ] )
				{
					for ( auto val3 : expectedResults_singles[ 2 ] )
					{
						for ( auto val4 : expectedResults_singles[ 3 ] )
						{
							for ( auto val5 : expectedResults_singles[ 4 ] )
							{
								expectedResultsFull[ 0 ].push_back ( val1 );
								expectedResultsFull[ 1 ].push_back ( val2 );
								expectedResultsFull[ 2 ].push_back ( val3 );
								expectedResultsFull[ 3 ].push_back ( val4 );
								expectedResultsFull[ 4 ].push_back ( val5 );
							}
						}
					}
				}
			}
			ParameterSystem testSys ( "CONFIG_PARAMETERS" );
			for ( auto var : vars_ )
			{
				testSys.addParameter ( var );
			}
			std::vector<std::vector<parameterType>> vars;
			vars.push_back ( testSys.getAllParams ( ) );
			testSys.generateKey ( vars, false, rangeInfo );
			Assert::IsTrue ( vars.size ( ) == 1 );
			Assert::IsTrue ( vars[ 0 ].size ( ) == 5 );
			Assert::IsTrue ( vars[ 0 ][ 0 ].keyValues.size ( ) == expectedResultsFull[ 0 ].size ( ) );
			Assert::IsTrue ( vars[ 0 ][ 1 ].keyValues.size ( ) == expectedResultsFull[ 1 ].size ( ) );
			Assert::IsTrue ( vars[ 0 ][ 2 ].keyValues.size ( ) == expectedResultsFull[ 2 ].size ( ) );
			Assert::IsTrue ( vars[ 0 ][ 3 ].keyValues.size ( ) == expectedResultsFull[ 3 ].size ( ) );
			Assert::IsTrue ( vars[ 0 ][ 4 ].keyValues.size ( ) == expectedResultsFull[ 4 ].size ( ) );
			Assert::IsTrue ( expectedResultsFull[0].size ( ) == expectedResultsFull[1].size ( ) );
			for ( auto count : range ( expectedResultsFull[0].size ( ) ) )
			{
				for ( auto varInc : range(expectedResultsFull.size ( )) )
				{
					Assert::AreEqual ( expectedResultsFull[varInc][ count ], vars[ 0 ][ varInc ].keyValues[ count ] );
				}
			}
		}
		TEST_METHOD( GenerateKey )
		{
			ScanRangeInfo rangeInfo;
			rangeInfo.defaultInit ( );
			rangeInfo ( 0, 0 ) = { 10,false,false };
			parameterType var;
			var.name = "testvariable";
			var.constant = false;
			var.constantValue = 0.0;
			std::vector<double> expectedresult = { 1,2,3,4,5,6,7,8,9,10 };
			var.ranges.push_back ( { 0,11 } );
			ParameterSystem testSys( "CONFIG_PARAMETERS" );
			testSys.addParameter( var );
			std::vector<std::vector<parameterType>> vars;
			vars.push_back(testSys.getAllVariables( ));
			testSys.generateKey( vars, false, rangeInfo );
			Assert::IsTrue ( vars.size ( ) == 1 );
			Assert::IsTrue ( vars[0].size ( ) == 1 );
			Assert::IsTrue( vars[0][0].keyValues.size( ) == 10 );
			auto vals = vars[0][0].keyValues;
			for ( auto count : range( expectedresult.size( ) ))
			{
				Assert::AreEqual( expectedresult[count], vals[count] );
			}
			testSys.clearParameters( );
			rangeInfo(0,0) = { 10,true,false };
			var.ranges[0] = { 11,21 };
			testSys.addParameter( var );
			vars[0] = testSys.getAllVariables( );
			testSys.generateKey( vars, false, rangeInfo );
			Assert::IsTrue ( vars.size ( ) == 1 );
			Assert::IsTrue ( vars[ 0 ].size ( ) == 1 );
			Assert::IsTrue( vars[0][0].keyValues.size( ) == 10 );
			vals = vars[0][0].keyValues;
			for ( auto count : range( expectedresult.size( ) ) )
			{
				Assert::AreEqual( expectedresult[count] + 10, vals[count] );
			}

			testSys.clearParameters ( );
			var.ranges[0] = { 31, 40 };
			rangeInfo(0, 0) = { 10,true,true };
			testSys.addParameter( var );
			vars[0] = testSys.getAllVariables( );
			testSys.generateKey( vars, false, rangeInfo );
			Assert::IsTrue ( vars.size ( ) == 1 );
			Assert::IsTrue ( vars[ 0 ].size ( ) == 1 );
			Assert::IsTrue( vars[0][0].keyValues.size( ) == 10 );
			vals = vars[0][0].keyValues;
			for ( auto count : range( expectedresult.size( ) ) )
			{
				Assert::AreEqual( 30+expectedresult[count], vals[count] );
			}
		}
	};
}

