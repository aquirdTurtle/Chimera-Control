#include "stdafx.h"
#include "CppUnitTest.h"
#include "../Chimera/MasterManager.h"


using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace TestScript
{
	TEST_CLASS( TestManager )
	{
		public:
		TEST_METHOD( InitManager )
		{
			MasterManager testMan;
			Assert::AreEqual( false, testMan.runningStatus( ));
			Assert::AreEqual( false, testMan.getAbortStatus( ));
			Assert::AreEqual( false, testMan.getIsPaused( ));
			Assert::ExpectException<Error>( [this, &testMan] {testMan.abort( ); } );
			Assert::ExpectException<Error>( [this, &testMan] {testMan.unPause( ); } );
			Assert::ExpectException<Error>( [this, &testMan] {testMan.pause( ); } );
		}
		TEST_METHOD( TimeCalc )
		{
			MasterManager testMan;
			timeType time;
			time.second = 0;
			auto val = testMan.convertToTime(time, std::vector<variableType>(), 0);
			Assert::AreEqual( 0.0, val );
			time.second = 10.0;
			time.first.push_back( Expression("0.1") );
			val = testMan.convertToTime( time, std::vector<variableType>( ), 0 );
			Assert::AreEqual( 10.1, val );
			time.first.push_back( Expression( "(10+4)*2" ) );
			val = testMan.convertToTime( time, std::vector<variableType>( ), 0 );
			Assert::AreEqual( 38.1, val );
		}
		TEST_METHOD( ExperimentThread )
		{
			MasterManager testMan;
		}
	};
}

