#include "stdafx.h"
#include "CppUnitTest.h"
#include "../Chimera/AoSystem.h"
#include "../Chimera/MainWindow.h"
#include "TestMacros.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace TestAoSystem
{
	TEST_CLASS( TestAoSys )
	{
		public:
		TEST_METHOD( InitAoSys )
		{
			AoSystem aoSys(true);
			//auto stat = aoSys.getDacStatus( );
			auto stat = aoSys.getDacInfo ( );
			for ( auto dac : stat )
			{
				Assert::IsTrue( 0 == dac.currVal );
			}
			aoSys.initializeDataObjects( 1, 0 );
			aoSys.interpretKey( std::vector<std::vector<parameterType>>( 1 ), std::string( ) );
			Assert::IsTrue( aoSys.getNumberEvents( 0, 0 ) == 0 );
			Assert::IsTrue( aoSys.getNumberSnapshots( 0, 0 ) == 0 );
			Assert::IsTrue( aoSys.getNumberOfDacs( ) == 24 );
		}
		TEST_METHOD( ForceOut )
		{
			// initialize in safemode
			AoSystem aoSys( true );
			DioSystem dio(true, true, true);
			aoSys.resetDacEvents( );
			dio.resetTtlEvents( );
			aoSys.handleSetDacsButtonPress( &dio, true );
			aoSys.organizeDacCommands( 0, 0 );
			aoSys.makeFinalDataFormat( 0, 0 );
			// start the boards which actually sets the dac values.
			aoSys.stopDacs( );
			aoSys.configureClocks( 0, false, 0 );
			aoSys.writeDacs( 0, false, 0 );
			aoSys.startDacs( );
			dio.organizeTtlCommands( 0, 0 );
			dio.convertToFinalViewpointFormat( 0, 0 );
			dio.writeTtlData( 0, false, 0 );
			dio.startBoard( );
			dio.waitTillFinished( 0, false, 0 );
			auto vals = dio.getCurrentStatus( );
			// should test vals?
		}
		CONNECTED_TEST( c_Init_Connected_AoSystem)
		{
			AoSystem aoSys(false);
		}
	};
}
