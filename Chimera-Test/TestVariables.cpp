#include "stdafx.h"
#include "CppUnitTest.h"
#include "afxwin.h"
#include "../Chimera/VariableSystem.h"
#include "../Chimera/resource.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;


namespace TestVariables
{
	TEST_CLASS( TestVariableSystem )	
	{
		public:
		
		TEST_METHOD( InitVariableSystem )
		{
			VariableSystem testSys;
			Assert::AreEqual(testSys.getAllConstants( ).size( ), size_t(0));
			Assert::AreEqual( testSys.getAllVariables( ).size( ), size_t( 0 ) );
			Assert::AreEqual( testSys.getEverything( ).size( ), size_t( 0 ) );
			Assert::AreEqual( testSys.getCurrentNumberOfVariables( ), UINT( 0 ) );
		}
		TEST_METHOD( AddConstant )
		{
			variableType var;
			var.name = "testvariable";
			var.constant = true;
			var.constantValue = 0.0;
			var.ranges.push_back( { 1,10,10,0,0 } );
			VariableSystem testSys;
			testSys.addConfigVariable( var, 0 );
			Assert::AreEqual( UINT( 1 ), testSys.getCurrentNumberOfVariables( ) );
			Assert::AreEqual( size_t( 1 ), testSys.getAllConstants( ).size() );
			Assert::AreEqual( size_t( 0 ), testSys.getAllVariables( ).size() );
			Assert::AreEqual( size_t( 1 ), testSys.getEverything( ).size( ) );
			auto var2 = testSys.getAllConstants( )[0];
			Assert::IsTrue( var2 == var );
		}
		TEST_METHOD( GenerateKey )
		{
			variableType var;
			var.name = "testvariable";
			var.constant = false;
			var.constantValue = 0.0;
			variationRangeInfo info;
			std::vector<double> expectedresult = { 1,2,3,4,5,6,7,8,9,10 };
			var.ranges.push_back( { 0,11,10,0,0 } );
			VariableSystem testSys;
			testSys.addConfigVariable( var, 0 );
			std::vector<std::vector<variableType>> vars;
			vars.push_back(testSys.getAllVariables( ));
			testSys.generateKey( vars, false );
			Assert::IsTrue( vars[0][0].keyValues.size( ) == 10 );
			auto vals = vars[0][0].keyValues;
			for ( auto count : range( expectedresult.size( ) ))
			{
				Assert::AreEqual( expectedresult[count], vals[count] );
			}

			testSys.clearVariables( );
			var.ranges[0] = { 11,21,10,true,false };
			testSys.addConfigVariable( var, 0 );
			vars[0] = testSys.getAllVariables( );
			testSys.generateKey( vars, false );
			Assert::IsTrue( vars[0][0].keyValues.size( ) == 10 );
			vals = vars[0][0].keyValues;
			for ( auto count : range( expectedresult.size( ) ) )
			{
				Assert::AreEqual( expectedresult[count] + 10, vals[count] );
			}

			testSys.clearVariables( );
			var.ranges[0] = { 31, 40, 10, true, true };
			testSys.addConfigVariable( var, 0 );
			vars[0] = testSys.getAllVariables( );
			testSys.generateKey( vars, false );
			Assert::IsTrue( vars[0][0].keyValues.size( ) == 10 );
			vals = vars[0][0].keyValues;
			for ( auto count : range( expectedresult.size( ) ) )
			{
				Assert::AreEqual( 30+expectedresult[count], vals[count] );
			}
		}

	};
}

