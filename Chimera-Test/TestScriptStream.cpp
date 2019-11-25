#include "stdafx.h"
#include "CppUnitTest.h"
#include "../Chimera/ScriptStream.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace TestScriptStream
{
	TEST_CLASS( TestScriptStream )
	{
		public:
		TEST_METHOD( InitScriptStream )
		{
			ScriptStream testStream;
			std::string testString( "Hello, world!" );
			testStream.str( testString );
			Assert::AreEqual( testStream.str( ), testString);
		}
		TEST_METHOD( ReadScriptStream )
		{
			ScriptStream testStream;
			std::string testString( "0 1\t"
									"2\n"
									"3\r\n"
									"4\t\r\n"
									"5\r\r\n"
									"6\r\n\r\n\r\n\t\r\n\n\r"
									"7\n\n\r"
									"8\n\n\r\r9");
			testStream.str( testString );
			for ( auto i : { 0,1,2,3,4,5,6,7,8,9 } )
			{
				std::string test;
				testStream >> test;
				Assert::AreEqual( test, std::to_string( i ) );
			}
			testStream.str( testString );
			testStream.clear( );
			testStream.seekg( 0 );			
			Assert::AreEqual( testStream.getline( ).c_str(), "0 1\t2" );
		}
		TEST_METHOD( ScriptStreamCommentEating_WhiteSpaceHandling )
		{
			ScriptStream testStream;
			std::string testString( "%hello, @#$() base 45 \n"
									"0\n"
									"1 %comment \tfollowing real\n\n"
									"2\r\n"
									"3 %commen following\r\n"
									"4 %asf following\r\n\r\r\n\r\n"
									"5\r"
									"6 \t %commen following\r\n"
									"7 %bad eol\r\r"
									"100 % actually still part of prev comment\r\n"
									"8 % c % c2\r\n"
									"9 %% comment \r\n"
									"10 %% comment \r\r"
									"100 %% actually still part of prev comment\r\n"
									"11 %% comment \n\n"
									"12");
			testStream.str( testString );
			Assert::AreEqual( testStream.str( ), testString );
			std::string test;
			for ( auto i : { 0,1,2,3,4,5,6,7,8,9,10,11,12 } )
			{
				testStream >> test;
				Assert::AreEqual( std::to_string ( i ), test );
			}
			testStream.str( testString );
			testStream.clear( );
			testStream.seekg( 0 );
			Assert::AreEqual( testStream.getline( ).c_str( ), "0" );
		}

		TEST_METHOD ( FunctionArgReplacements ) 
		{
			ScriptStream testStream;
			std::string testString ("t+= hello");
			testStream.str (testString);
			std::vector<std::pair<std::string, std::string>> repls;
			repls.push_back ({ "hello", "goodbye" });
			std::vector<parameterType> params;
			params.push_back ({ "goodbye", true, 0, false, false, 0, {}, {}, false, "GLOBAL_PARAMETER_SCOPE" });
			testStream.loadReplacements (repls, params, "test1", "test2", "test1" );
			std::string timeWord;
			Expression timeAmount;
			testStream >> timeWord >> timeAmount;
			Assert::AreEqual (timeWord, str("t+="));
			// this is the replacement.
			Assert::AreEqual (timeAmount.expressionStr, str ("goodbye"));
		}
	};
}
