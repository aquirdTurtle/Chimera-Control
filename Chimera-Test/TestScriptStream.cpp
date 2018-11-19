#include "stdafx.h"
#include "CppUnitTest.h"
#include "../Chimera/ScriptStream.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace TestMatrix
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
			std::string testString( "0 1\t2\n3\r\n4\t\r\n5" );
			testStream.str( testString );
			for ( auto i : { 0,1,2,3,4,5 } )
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
									"1 %comment \tfollowing real"
									"\n"
									"\n2"
									"\r\n"
									"5"
									"\r6");
			testStream.str( testString );
			Assert::AreEqual( testStream.str( ), testString );
			std::string test;
			for ( auto i : { 0,1,2,5 } )
			{
				testStream >> test;
				Assert::AreEqual( test, std::to_string( i ) );
			}
			testStream >> test;
			Assert::IsTrue ( "\r6" == test );
			testStream.str( testString );
			testStream.clear( );
			testStream.seekg( 0 );
			Assert::AreEqual( testStream.getline( ).c_str( ), "0" );
		}
	};
}
