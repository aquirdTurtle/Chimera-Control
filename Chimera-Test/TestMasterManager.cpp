#include "stdafx.h"
#include "CppUnitTest.h"
#include "../Chimera/MasterManager.h"


using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace TestManager
{
	void handleInitDio( DioSystem& dio, MasterManager& testMan )
	{
		ScriptStream stream( "t += 50 on: A5 t += 1 pulseon: C13 0.1" );
		std::vector<std::pair<UINT, UINT>>  shades;
		dio.initializeDataObjects( 1, 0 );
		std::string scope = NO_PARAMETER_SCOPE;
		std::string word;
		auto emptyvars = std::vector<parameterType>( );
		stream >> word;
		testMan.handleTimeCommands( word, stream, emptyvars, scope );
		stream >> word;
		testMan.handleDioCommands( word, stream, emptyvars, &dio, shades, 0, scope );
		stream >> word;
		testMan.handleTimeCommands( word, stream, emptyvars, scope );
		stream >> word;
		testMan.handleDioCommands( word, stream, emptyvars, &dio, shades, 0, scope );
		std::vector<std::vector<parameterType>> emptyVars2( 1, emptyvars );
		dio.interpretKey( emptyVars2 );
		dio.organizeTtlCommands( 0, 0 );
		dio.findLoadSkipSnapshots( 0, emptyvars, 0, 0 );
		// run a couple checks.
		dio.checkNotTooManyTimes( 0, 0 );
		dio.checkFinalFormatTimes( 0, 0 );

	}
	TEST_CLASS( Test_Manager_FTDI_DIO )
	{
		TEST_METHOD( FTDI_basic_handling )
		{
			// script reading and checking snapshots
			MasterManager testMan;
			DioSystem dio( true, true );
			handleInitDio( dio, testMan );
			dio.convertToFtdiSnaps( 0, 0 );
			// assert that the important structures are filled correctly.
			auto snaps = dio.getFtdiSnaps( );
			Assert::AreEqual( size_t( 1 ), snaps.size( ) );
			Assert::AreEqual( size_t( 1 ), snaps[0].size( ) );
			Assert::AreEqual( size_t( 2048 ), snaps[0][0].size( ) );
			// first command is at t=5, so first snapshot at t=0 should be all off.
			for ( auto pt : snaps[0][0][0].pts )
			{
				Assert::AreEqual( 0, int(pt) );
			}
			Assert::AreEqual( 32, int(snaps[0][0][1].pts[0]) );
			Assert::AreEqual( 32, int(snaps[0][0][2].pts[0]) );
			Assert::AreEqual( 32, int( snaps[0][0][3].pts[0] ) );
			Assert::AreEqual( 0, int( snaps[0][0][1].pts[5] ) );
			Assert::AreEqual( 32, int( snaps[0][0][2].pts[5] ) );
			Assert::AreEqual( 0, int( snaps[0][0][3].pts[5] ) );
			for ( auto pt : snaps[0][0][5].pts )
			{
				Assert::AreEqual( 0, int( pt ) );
			}
			double time = dio.getFtdiTotalTime( 0, 0 );
			Assert::AreEqual( 51.1*1e5, time );
		}
		TEST_METHOD( FTDI_DioFinalFormatting )
		{
			MasterManager testMan;
			DioSystem dio( true, true );
			handleInitDio( dio, testMan );
			dio.convertToFtdiSnaps( 0, 0 );
			dio.convertToFinalFtdiFormat( 0, 0 );
			auto fin = dio.getFinalFtdiData( );
			// check first snapshot
			for ( auto inc : range( 20 ) )
			{
				if ( inc == 0 || inc == 7 || inc == 14 )
				{
					Assert::IsTrue( fin[0][0].pts[inc] == WORD( 161 ) );
				}
				else
				{
					Assert::IsTrue( fin[0][0].pts[inc] == WORD( 0 ) );
				}
			}
			// check (only) some points in the second.
			Assert::IsTrue( fin[0][0].pts[21] == WORD( 161 ) );
			Assert::IsTrue( fin[0][0].pts[22] == WORD( 0 ) );
			Assert::IsTrue( fin[0][0].pts[23] == WORD( 1 ) );
			Assert::IsTrue( fin[0][0].pts[24] == WORD( 0 ) );
			Assert::IsTrue( fin[0][0].pts[25] == WORD( 76 ) );
			Assert::IsTrue( fin[0][0].pts[26] == WORD( 75 ) );
			Assert::IsTrue( fin[0][0].pts[27] == WORD( 64 ) );
			Assert::IsTrue( fin[0][0].pts[28] == WORD( 161 ) );
			Assert::IsTrue( fin[0][0].pts[35] == WORD( 161 ) );

		};
	};
	TEST_CLASS( Test_Manager_Viewpoint_DIO )
	{
		TEST_METHOD( DioBasicHandling )
		{
			// script reading and checking snapshots
			MasterManager testMan;
			DioSystem dio( true, true );
			handleInitDio( dio, testMan );
			dio.convertToFinalViewpointFormat( 0, 0 );
			// run a couple checks.
			dio.checkNotTooManyTimes( 0, 0 );
			dio.checkFinalFormatTimes( 0, 0 );
			// assert that the important structures are filled correctly.
			auto snaps = dio.getSnapshots( );
			Assert::AreEqual( size_t( 1 ), snaps.size( ) );
			Assert::AreEqual( size_t( 1 ), snaps[0].size( ) );
			Assert::AreEqual( size_t( 4 ), snaps[0][0].size( ) );
			// first command is at t=5, so first snapshot at t=0 should be all off.
			for ( auto row : snaps[0][0][0].ttlStatus )
			{
				for ( auto elem : row )
				{
					Assert::AreEqual( false, elem );
				}
			}
			Assert::AreEqual( true, snaps[0][0][1].ttlStatus[0][5] );
			double time = dio.getTotalTime( 0, 0 );
			Assert::AreEqual( 51.1, time );
		}
		TEST_METHOD( DioFinalFormatting )
		{
			MasterManager testMan;
			DioSystem dio( true, true );
			handleInitDio( dio, testMan );
			dio.convertToFinalViewpointFormat( 0, 0 );
			// run a couple checks.
			dio.checkNotTooManyTimes( 0, 0 );
			dio.checkFinalFormatTimes( 0, 0 );
			auto fin = dio.getFinalViewpointData( );
			// again, first snapshot is all zeros should be empty on the final data as well.
			for ( auto inc : range( 6 ) )
			{
				Assert::IsTrue( fin[0][0][inc] == WORD( 0 ) );
			}
			UINT offSet = 1;
			// check actual flip and time calculations
			// first element is loword of time, second his hiword. The units of this number are "clock cycles". 
			// We use a 10MHz clock. If this ever changed, I'd need to change the time calculations here.
			Assert::IsTrue( fin[0][0][offSet * 6] == WORD( 41255 ) );
			Assert::IsTrue( fin[0][0][offSet * 6 + 1] == WORD( 7 ) );
			// 32 because 2^5 for flipping 5th bit.
			Assert::IsTrue( fin[0][0][offSet * 6 + 2] == WORD( 32 ) );
			Assert::IsTrue( fin[0][0][offSet * 6 + 3] == WORD( 0 ) );
			Assert::IsTrue( fin[0][0][offSet * 6 + 4] == WORD( 0 ) );
			Assert::IsTrue( fin[0][0][offSet * 6 + 5] == WORD( 0 ) );
			offSet = 2;
			Assert::IsTrue( fin[0][0][offSet * 6] == WORD( 51255 ) );
			Assert::IsTrue( fin[0][0][offSet * 6 + 1] == WORD( 7 ) );
			Assert::IsTrue( fin[0][0][offSet * 6 + 2] == WORD( 32 ) );
			Assert::IsTrue( fin[0][0][offSet * 6 + 3] == WORD( 0 ) );
			Assert::IsTrue( fin[0][0][offSet * 6 + 4] == WORD( 8192 ) );
			Assert::IsTrue( fin[0][0][offSet * 6 + 5] == WORD( 0 ) );
			offSet = 3;
			Assert::IsTrue( fin[0][0][offSet * 6] == WORD( 52255 ) );
			Assert::IsTrue( fin[0][0][offSet * 6 + 1] == WORD( 7 ) );
			Assert::IsTrue( fin[0][0][offSet * 6 + 2] == WORD( 32 ) );
			Assert::IsTrue( fin[0][0][offSet * 6 + 3] == WORD( 0 ) );
			Assert::IsTrue( fin[0][0][offSet * 6 + 4] == WORD( 0 ) );
			Assert::IsTrue( fin[0][0][offSet * 6 + 5] == WORD( 0 ) );
		};
	};
	TEST_CLASS( Test_Manager_AO )
	{
		TEST_METHOD( AoBasicHandling )
		{
			MasterManager testMan;
			AoSystem ao( true );
			DioSystem dio( true, true );
			ScriptStream stream( "t += 5 dac: dac2 1" );
			std::string scope = NO_PARAMETER_SCOPE;
			ao.initializeDataObjects( 1, 0 );
			dio.initializeDataObjects( 1, 0 );

			std::string word;
			auto emptyvars = std::vector<parameterType>( );
			std::vector<UINT> shades;
			stream >> word;
			testMan.handleTimeCommands( word, stream, emptyvars, scope );
			stream >> word;
			testMan.handleAoCommands( word, stream, emptyvars, &ao, shades, &dio, 0, scope );
			auto emptyvars2 = std::vector<std::vector<parameterType>>( 1 );
			ao.interpretKey( emptyvars2, std::string( ) );
			ao.organizeDacCommands( 0, 0 );
			auto snaps = ao.getSnapshots( );
			Assert::AreEqual( size_t( 1 ), snaps.size( ) );
			Assert::AreEqual( size_t( 1 ), snaps[0].size( ) );
			Assert::AreEqual( size_t( 2 ), snaps[0][0].size( ) );
			Assert::AreEqual( 1.0, snaps[0][0][1].dacValues[2] );
			for ( auto dac : snaps[0][0][0].dacValues )
			{
				Assert::IsTrue( 0 == dac );
			}
		}
		TEST_METHOD( AoFinalHandling )
		{
			MasterManager testMan;
			AoSystem ao( true );
			DioSystem dio( true, true );
			ScriptStream stream( "t += 5 dac: dac2 1" );
			std::string scope = NO_PARAMETER_SCOPE;
			ao.initializeDataObjects( 1, 0 );
			dio.initializeDataObjects( 1, 0 );

			std::string word;
			auto emptyvars = std::vector<parameterType>( );
			std::vector<UINT> shades;
			stream >> word;
			testMan.handleTimeCommands( word, stream, emptyvars, scope );
			stream >> word;
			testMan.handleAoCommands( word, stream, emptyvars, &ao, shades, &dio, 0, scope );
			auto emptyvars2 = std::vector<std::vector<parameterType>>( 1 );
			dio.sizeDataStructures( 1, 1 );
			ao.interpretKey( emptyvars2, std::string( ) );
			ao.organizeDacCommands( 0, 0 );
			ao.setDacTriggerEvents( &dio, 0, 0 );
			ao.findLoadSkipSnapshots( 0, emptyvars, 0, 0 );
			ao.makeFinalDataFormat( 0, 0 );
			auto res = ao.getFinData( );
			Assert::AreEqual( size_t( 1 ), res.size( ) );
			Assert::AreEqual( size_t( 1 ), res[0].size( ) );
			Assert::AreEqual( size_t( 3 ), res[0][0].size( ) );
			for ( auto finDat : res[0][0] )
			{
				Assert::AreEqual( size_t( 16 ), res[0][0][0].size( ) );
			}
			UINT count = 0;
			for ( auto finDat : res[0][0][0] )
			{
				if ( count++ == 10 )
				{
					Assert::AreEqual( 1.0, finDat );
					continue;
				}
				Assert::AreEqual( 0.0, finDat );
			}
			for ( auto finDat : res[0][0][1] )
			{
				Assert::AreEqual( 0.0, finDat );
			}
			for ( auto finDat : res[0][0][2] )
			{
				Assert::AreEqual( 0.0, finDat );
			}
		}
		TEST_METHOD( AoRampLinspace )
		{
			MasterManager testMan;
			AoSystem ao( true );
			DioSystem dio( true, true );
			ScriptStream stream( "t += 5 daclinspace: dac2 0 10 0.1 20" );
			std::string scope = NO_PARAMETER_SCOPE;
			ao.initializeDataObjects( 1, 0 );
			dio.initializeDataObjects( 1, 0 );

			std::string word;
			auto emptyvars = std::vector<parameterType>( );
			std::vector<UINT> shades;
			stream >> word;
			testMan.handleTimeCommands( word, stream, emptyvars, scope );
			stream >> word;
			testMan.handleAoCommands( word, stream, emptyvars, &ao, shades, &dio, 0, scope );
			auto emptyvars2 = std::vector<std::vector<parameterType>>( 1 );
			dio.sizeDataStructures( 1, 1 );
			ao.interpretKey( emptyvars2, std::string( ) );
			ao.organizeDacCommands( 0, 0 );
			ao.setDacTriggerEvents( &dio, 0, 0 );
			ao.findLoadSkipSnapshots( 0, emptyvars, 0, 0 );
			ao.makeFinalDataFormat( 0, 0 );
			auto snaps = ao.getSnapshots( );
			Assert::AreEqual( size_t( 1 ), snaps.size( ) );
			Assert::AreEqual( size_t( 1 ), snaps[0].size( ) );
			// 20 steps means 21 points created from the ramp.
			Assert::AreEqual( size_t( 22 ), snaps[0][0].size( ) );
			Assert::AreEqual( 0.0, snaps[0][0][0].time );
			Assert::AreEqual( 5.0, snaps[0][0][1].time );
			Assert::AreEqual( 5.1, snaps[0][0].back( ).time );
			auto res = ao.getFinData( );
			Assert::AreEqual( size_t( 1 ), res.size( ) );
			Assert::AreEqual( size_t( 1 ), res[0].size( ) );
			Assert::AreEqual( size_t( 3 ), res[0][0].size( ) );
			for ( auto board : res[0][0] )
			{
				Assert::AreEqual( size_t( 22 * 8 ), board.size( ) );
			}
		}
		TEST_METHOD( Ao_Ramp_Arange )
		{
			MasterManager testMan;
			AoSystem ao( true );
			DioSystem dio( true, true );
			ScriptStream stream( "t += 5 dacarange: dac2 0 10 0.1 1" );
			std::string scope = NO_PARAMETER_SCOPE;
			ao.initializeDataObjects( 1, 0 );
			dio.initializeDataObjects( 1, 0 );
			std::string word;
			auto emptyvars = std::vector<parameterType>( );
			std::vector<UINT> shades;
			stream >> word;
			testMan.handleTimeCommands( word, stream, emptyvars, scope );
			stream >> word;
			testMan.handleAoCommands( word, stream, emptyvars, &ao, shades, &dio, 0, scope );
			auto emptyvars2 = std::vector<std::vector<parameterType>>( 1 );
			dio.sizeDataStructures( 1, 1 );
			ao.interpretKey( emptyvars2, std::string( ) );
			ao.organizeDacCommands( 0, 0 );
			ao.setDacTriggerEvents( &dio, 0, 0 );
			ao.findLoadSkipSnapshots( 0, emptyvars, 0, 0 );
			ao.makeFinalDataFormat( 0, 0 );
			auto snaps = ao.getSnapshots( );
			Assert::AreEqual( size_t( 1 ), snaps.size( ) );
			Assert::AreEqual( size_t( 1 ), snaps[0].size( ) );
			// spacing of 1 means 11 pts in the ramp.
			Assert::AreEqual( size_t( 12 ), snaps[0][0].size( ) );
			Assert::AreEqual( 0.0, snaps[0][0][0].time );
			Assert::AreEqual( 5.0, snaps[0][0][1].time );
			Assert::AreEqual( 5.1, snaps[0][0].back( ).time );
			auto count = 0;
			double last_time = -1;
			double last_val = -1;
			for ( auto snap : snaps[0][0] )
			{
				if ( count++ == 0 )
				{
					continue;
				}
				if ( last_time == -1 )
				{
					last_time = snap.time;
					last_val = snap.dacValues[2];
					continue;
				}
				Assert::AreNotEqual( last_time, snap.time );
				Assert::AreEqual( 0.01, snap.time - last_time, 1e-13 );
				Assert::AreEqual( 1.0, snap.dacValues[2] - last_val );
				last_time = snap.time;
				last_val = snap.dacValues[2];
			}
			auto res = ao.getFinData( );
			Assert::AreEqual( size_t( 1 ), res.size( ) );
			Assert::AreEqual( size_t( 1 ), res[0].size( ) );
			Assert::AreEqual( size_t( 3 ), res[0][0].size( ) );
			for ( auto board : res[0][0] )
			{
				Assert::AreEqual( size_t( 12 * 8 ), board.size( ) );
			}
		}
	};
	TEST_CLASS( Test_Manager_Core )
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
			auto val = testMan.convertToTime(time, std::vector<parameterType>(), 0);
			Assert::AreEqual( 0.0, val );
			time.second = 10.0;
			time.first.push_back( Expression("0.1") );
			val = testMan.convertToTime( time, std::vector<parameterType>( ), 0 );
			Assert::AreEqual( 10.1, val );
			time.first.push_back( Expression( "(10+4)*2" ) );
			val = testMan.convertToTime( time, std::vector<parameterType>( ), 0 );
			Assert::AreEqual( 38.1, val );
		}
		TEST_METHOD( Run_DIO_Core )
		{
			MasterManager testMan;
			DioSystem dio( true, true );
			handleInitDio( dio, testMan );
		}
	};
	TEST_CLASS( Test_Script_Testing )
	{
		TEST_METHOD( Simple_Script )
		{

		}
		TEST_METHOD( Use_A_Global_Variable )
		{

		}
		TEST_METHOD( Use_A_Local_Variable )
		{

		}
	};
}



