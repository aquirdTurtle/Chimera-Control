#include "stdafx.h"
#include "CppUnitTest.h"
#include "../Chimera/MasterManager.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace TestExperimentManager
{
	void handleInitDio( DioSystem& dio, MasterThreadManager& testMan )
	{
		ScriptStream stream( "t += 50 on: A5 t += 1 pulseon: C13 0.1" );
		std::vector<std::pair<UINT, UINT>>  shades;
		dio.initializeDataObjects( 1, 0 );
		std::string scope = NO_PARAMETER_SCOPE;
		std::string word;
		auto emptyvars = std::vector<parameterType>( );
		timeType operationTime;
		// hard-wiring this sequence based on the stream text above.
		stream >> word;
		testMan.handleTimeCommands( word, stream, emptyvars, scope, operationTime);
		stream >> word;
		testMan.handleDioCommands( word, stream, emptyvars, dio, shades, 0, scope, operationTime );
		stream >> word;
		testMan.handleTimeCommands( word, stream, emptyvars, scope, operationTime);
		stream >> word;
		testMan.handleDioCommands( word, stream, emptyvars, dio, shades, 0, scope, operationTime);
		std::vector<std::vector<parameterType>> emptyVars2( 1, emptyvars );
		dio.interpretKey( emptyVars2 );
		dio.restructureCommands ( );
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
			MasterThreadManager testMan;
			DioSystem dio( true, true, true );
			handleInitDio( dio, testMan );
			dio.convertToFtdiSnaps( 0, 0 );
			
			// assert that the important structures are filled correctly.
			auto snaps = dio.getFtdiSnaps( );
			Assert::AreEqual(UINT ( 1 ), snaps.getNumSequences() );
			Assert::AreEqual(UINT ( 1 ), snaps.getNumVariations(0) );
			Assert::AreEqual( size_t( 2048 ), snaps(0,0).size( ) );
			// first command is at t=5, so first snapshot at t=0 should be all off.
			for ( auto pt : snaps ( 0, 0 )[0].pts )
			{
				Assert::AreEqual( 0, int(pt) );
			}
			Assert::AreEqual( 32, int(snaps ( 0, 0 )[1].pts[0]) );
			Assert::AreEqual( 32, int(snaps ( 0, 0 )[2].pts[0]) );
			Assert::AreEqual( 32, int( snaps ( 0, 0 )[3].pts[0] ) );
			Assert::AreEqual( 0, int( snaps ( 0, 0 )[1].pts[5] ) );
			Assert::AreEqual( 32, int( snaps ( 0, 0 )[2].pts[5] ) );
			Assert::AreEqual( 0, int( snaps ( 0, 0 )[3].pts[5] ) );
			for ( auto pt : snaps ( 0, 0 )[5].pts )
			{
				Assert::AreEqual( 0, int( pt ) );
			}
			double time = dio.getFtdiTotalTime( 0, 0 );
			//Assert::AreEqual( 51.1*1e5, time );
			// there is an extra microsecond offset in the ftdi system in order to prevent issues with zero ttls 
			// at zero time which would register as the end of the run (all zeros).
			Assert::AreEqual (51.101, time);
		}
		TEST_METHOD( FTDI_DioFinalFormatting )
		{
			MasterThreadManager testMan;
			DioSystem dio( true, true, true );
			handleInitDio( dio, testMan );
			dio.convertToFtdiSnaps( 0, 0 );
			dio.convertToFinalFtdiFormat( 0, 0 );
			auto fin = dio.getFinalFtdiData( )(0,0);
			// check first snapshot. See ftdiStructures.h and finBufInfo definition for more info on the structure here. 
			auto firstSnap = std::vector<unsigned char> (fin.pts.begin (), fin.pts.begin () + 21);
			auto secondSnap = std::vector<unsigned char> (fin.pts.begin ()+21, fin.pts.begin () + 2*21);
			Assert::IsTrue (firstSnap == std::vector<unsigned char>({ 161,8,0,0,0,0,100, // This 100 value is the initial 1us time offset of the ftdi system
				161,16,0,0,0,0,0,161,24,0,0,0,0,0 }));
			Assert::IsTrue (secondSnap == std::vector<unsigned char> ({ 161,8,1,0,76,75,164,161,16,1,32,0,0,0,161,24,1,0,0,0,0 }));
		};
	};
	TEST_CLASS( Test_Manager_Viewpoint_DIO )
	{
		TEST_METHOD(DEPRECATED_DioBasicHandling_viewpoint )
		{
			// script reading and checking snapshots
			MasterThreadManager testMan;
			DioSystem dio( true, true, true );
			handleInitDio( dio, testMan );
			dio.convertToFinalViewpointFormat( 0, 0 );
			// run a couple checks.
			dio.checkNotTooManyTimes( 0, 0 );
			dio.checkFinalFormatTimes( 0, 0 );
			// assert that the important structures are filled correctly.
			auto snaps = dio.getTtlSnapshots( );
			Assert::AreEqual( UINT ( 1 ), snaps.getNumSequences( ) );
			Assert::AreEqual( UINT ( 1 ), snaps.getNumVariations(0) );
			Assert::AreEqual( size_t( 4 ), snaps(0,0).size( ) );
			// first command is at t=5, so first snapshot at t=0 should be all off.
			for ( auto row : snaps ( 0, 0 )[0].ttlStatus )
			{
				for ( auto elem : row )
				{
					Assert::AreEqual( false, elem );
				}
			}
			Assert::AreEqual( true, snaps ( 0, 0 )[1].ttlStatus[0][5] );
			double time = dio.getTotalTime( 0, 0 );
			Assert::AreEqual( 51.1, time );
		}
		TEST_METHOD(DEPRECATED_DioFinalFormatting_Viewpoint )
		{
			MasterThreadManager testMan;
			DioSystem dio( true, true, true );
			handleInitDio( dio, testMan );
			dio.convertToFinalViewpointFormat( 0, 0 );
			// run a couple checks.
			dio.checkNotTooManyTimes( 0, 0 );
			dio.checkFinalFormatTimes( 0, 0 );
			auto fin = dio.getFinalViewpointData( );
			// again, first snapshot is all zeros should be empty on the final data as well.
			for ( auto inc : range( 6 ) )
			{
				Assert::IsTrue( fin ( 0, 0 )[inc] == WORD( 0 ) );
			}
			UINT offSet = 1;
			// check actual flip and time calculations
			// first element is loword of time, second his hiword. The units of this number are "clock cycles". 
			// We use a 10MHz clock. If this ever changed, I'd need to change the time calculations here.
			Assert::IsTrue( fin ( 0, 0 )[offSet * 6] == WORD( 41255 ) );
			Assert::IsTrue( fin ( 0, 0 )[offSet * 6 + 1] == WORD( 7 ) );
			// 32 because 2^5 for flipping 5th bit.
			Assert::IsTrue( fin ( 0, 0 )[offSet * 6 + 2] == WORD( 32 ) );
			Assert::IsTrue( fin ( 0, 0 )[offSet * 6 + 3] == WORD( 0 ) );
			Assert::IsTrue( fin ( 0, 0 )[offSet * 6 + 4] == WORD( 0 ) );
			Assert::IsTrue( fin ( 0, 0 )[offSet * 6 + 5] == WORD( 0 ) );
			offSet = 2;
			Assert::IsTrue( fin ( 0, 0 )[offSet * 6] == WORD( 51255 ) );
			Assert::IsTrue( fin ( 0, 0 )[offSet * 6 + 1] == WORD( 7 ) );
			Assert::IsTrue( fin ( 0, 0 )[offSet * 6 + 2] == WORD( 32 ) );
			Assert::IsTrue( fin ( 0, 0 )[offSet * 6 + 3] == WORD( 0 ) );
			Assert::IsTrue( fin ( 0, 0 )[offSet * 6 + 4] == WORD( 8192 ) );
			Assert::IsTrue( fin ( 0, 0 )[offSet * 6 + 5] == WORD( 0 ) );
			offSet = 3;
			Assert::IsTrue( fin ( 0, 0 )[offSet * 6] == WORD( 52255 ) );
			Assert::IsTrue( fin ( 0, 0 )[offSet * 6 + 1] == WORD( 7 ) );
			Assert::IsTrue( fin ( 0, 0 )[offSet * 6 + 2] == WORD( 32 ) );
			Assert::IsTrue( fin ( 0, 0 )[offSet * 6 + 3] == WORD( 0 ) );
			Assert::IsTrue( fin ( 0, 0 )[offSet * 6 + 4] == WORD( 0 ) );
			Assert::IsTrue( fin ( 0, 0 )[offSet * 6 + 5] == WORD( 0 ) );
		};
	};
	TEST_CLASS( Test_Manager_AO )
	{
		TEST_METHOD( AoBasicHandling )
		{
			MasterThreadManager testMan;
			AoSystem ao( true );
			DioSystem dio( true, true, true );
			ScriptStream stream( "t += 5 dac: dac2 1" );
			timeType opTime;
			std::string scope = NO_PARAMETER_SCOPE;
			ao.initializeDataObjects( 1, 0 );
			dio.initializeDataObjects( 1, 0 );

			std::string word;
			auto emptyvars = std::vector<parameterType>( );
			std::vector<UINT> shades;
			stream >> word;
			testMan.handleTimeCommands( word, stream, emptyvars, scope, opTime);
			stream >> word;
			testMan.handleAoCommands( word, stream, emptyvars, ao, shades, dio, 0, scope, opTime);
			auto emptyvars2 = std::vector<std::vector<parameterType>>( 1 );
			ao.interpretKey( emptyvars2, std::string( ) );
			ao.organizeDacCommands( 0, 0 );
			auto snaps = ao.getSnapshots( );
			Assert::AreEqual( size_t( 1 ), size_t (snaps.getNumSequences() ));
			Assert::AreEqual( size_t( 1 ), size_t (snaps.getNumVariations(0) ));
			Assert::AreEqual( size_t( 2 ), snaps ( 0, 0 ).size( ) );
			Assert::AreEqual( 1.0, snaps(0,0)[1].dacValues[2] );
			for ( auto dac : snaps(0,0)[0].dacValues )
			{
				Assert::IsTrue( 0 == dac );
			}
		}
		TEST_METHOD( AoFinalHandling )
		{
			MasterThreadManager testMan;
			AoSystem ao( true );
			DioSystem dio( true, true, true );
			ScriptStream stream( "t += 5 dac: dac2 1" );
			timeType opTime;
			std::string scope = NO_PARAMETER_SCOPE;
			ao.initializeDataObjects( 1, 0 );
			dio.initializeDataObjects( 1, 0 );

			std::string word;
			auto emptyvars = std::vector<parameterType>( );
			std::vector<UINT> shades;
			stream >> word;
			testMan.handleTimeCommands( word, stream, emptyvars, scope, opTime);
			stream >> word;
			testMan.handleAoCommands( word, stream, emptyvars, ao, shades, dio, 0, scope, opTime);
			auto emptyvars2 = std::vector<std::vector<parameterType>>( 1 );
			dio.sizeDataStructures( 1, 1 );
			ao.interpretKey( emptyvars2, std::string( ) );
			ao.organizeDacCommands( 0, 0 );
			ao.setDacTriggerEvents( dio, 0, 0, 1 );
			ao.findLoadSkipSnapshots( 0, emptyvars, 0, 0 );
			ao.makeFinalDataFormat( 0, 0 );
			auto res = ao.getFinData( );
			Assert::AreEqual( size_t( 1 ), size_t (res.getNumSequences()) );
			Assert::AreEqual( size_t( 1 ), size_t (res.getNumVariations(0)) );
			Assert::AreEqual( size_t( 3 ), res(0,0).size( ) );
			for ( auto finDat : res ( 0, 0 ) )
			{
				Assert::AreEqual( size_t( 16 ), res ( 0, 0 )[0].size( ) );
			}
			UINT count = 0;
			for ( auto finDat : res ( 0, 0 )[0] )
			{
				if ( count++ == 10 )
				{
					Assert::AreEqual( 1.0, finDat );
					continue;
				}
				Assert::AreEqual( 0.0, finDat );
			}
			for ( auto finDat : res ( 0, 0 )[1] )
			{
				Assert::AreEqual( 0.0, finDat );
			}
			for ( auto finDat : res ( 0, 0 )[2] )
			{
				Assert::AreEqual( 0.0, finDat );
			}
		}
		TEST_METHOD( AoRampLinspace )
		{
			MasterThreadManager testMan;
			AoSystem ao( true );
			DioSystem dio( true, true, true );
			ScriptStream stream( "t += 5 daclinspace: dac2 0 10 0.1 20" );
			timeType opTime;
			std::string scope = NO_PARAMETER_SCOPE;
			ao.initializeDataObjects( 1, 0 );
			dio.initializeDataObjects( 1, 0 );

			std::string word;
			auto emptyvars = std::vector<parameterType>( );
			std::vector<UINT> shades;
			stream >> word;
			testMan.handleTimeCommands( word, stream, emptyvars, scope, opTime);
			stream >> word;
			testMan.handleAoCommands( word, stream, emptyvars, ao, shades, dio, 0, scope, opTime);
			auto emptyvars2 = std::vector<std::vector<parameterType>>( 1 );
			dio.sizeDataStructures( 1, 1 );
			ao.interpretKey( emptyvars2, std::string( ) );
			ao.organizeDacCommands( 0, 0 );
			ao.setDacTriggerEvents( dio, 0, 0, 1 );
			ao.findLoadSkipSnapshots( 0, emptyvars, 0, 0 );
			ao.makeFinalDataFormat( 0, 0 );
			auto snaps = ao.getSnapshots( );
			Assert::AreEqual( UINT( 1 ), snaps.getNumSequences() );
			Assert::AreEqual( UINT( 1 ), snaps.getNumVariations(0) );
			// 20 steps means 21 points created from the ramp.
			Assert::AreEqual(size_t ( 22 ), snaps ( 0, 0 ).size( ) );
			Assert::AreEqual( 0.0, snaps ( 0, 0 )[0].time );
			Assert::AreEqual( 5.0, snaps ( 0, 0 )[1].time );
			Assert::AreEqual( 5.1, snaps ( 0, 0 ).back( ).time );
			auto res = ao.getFinData( );
			Assert::AreEqual( UINT ( 1 ), res.getNumSequences() );
			Assert::AreEqual( UINT ( 1 ), res.getNumVariations(0) );
			Assert::AreEqual( size_t( 3 ), res ( 0, 0 ).size( ) );
			for ( auto board : res ( 0, 0 ) )
			{
				Assert::AreEqual( size_t( 22 * 8 ), board.size( ) );
			}
		}
		TEST_METHOD( Ao_Ramp_Arange )
		{
			MasterThreadManager testMan;
			AoSystem ao( true );
			DioSystem dio( true, true, true );
			ScriptStream stream( "t += 5 dacarange: dac2 0 10 0.1 1" );
			timeType opTime;
			std::string scope = NO_PARAMETER_SCOPE;
			ao.initializeDataObjects( 1, 0 );
			dio.initializeDataObjects( 1, 0 );
			std::string word;
			auto emptyvars = std::vector<parameterType>( );
			std::vector<UINT> shades;
			stream >> word;
			testMan.handleTimeCommands( word, stream, emptyvars, scope, opTime);
			stream >> word;
			testMan.handleAoCommands( word, stream, emptyvars, ao, shades, dio, 0, scope, opTime);
			auto emptyvars2 = std::vector<std::vector<parameterType>>( 1 );
			dio.sizeDataStructures( 1, 1 );
			ao.interpretKey( emptyvars2, std::string( ) );
			ao.organizeDacCommands( 0, 0 );
			ao.setDacTriggerEvents( dio, 0, 0, 1 );
			ao.findLoadSkipSnapshots( 0, emptyvars, 0, 0 );
			ao.makeFinalDataFormat( 0, 0 );
			auto snaps = ao.getSnapshots( );
			Assert::AreEqual( UINT ( 1 ), snaps.getNumSequences() );
			Assert::AreEqual( UINT ( 1 ), snaps.getNumVariations(0) );
			// spacing of 1 means 11 pts in the ramp.
			Assert::AreEqual( size_t( 12 ), snaps ( 0, 0 ).size( ) );
			Assert::AreEqual( 0.0, snaps ( 0, 0 )[0].time );
			Assert::AreEqual( 5.0, snaps ( 0, 0 )[1].time );
			Assert::AreEqual( 5.1, snaps ( 0, 0 ).back( ).time );
			auto count = 0;
			double last_time = -1;
			double last_val = -1;
			for ( auto snap : snaps ( 0, 0 ) )
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
			Assert::AreEqual( UINT ( 1 ), res.getNumSequences() );
			Assert::AreEqual( UINT ( 1 ), res.getNumVariations(0) );
			Assert::AreEqual( size_t( 3 ), res ( 0, 0 ).size( ) );
			for ( auto board : res ( 0, 0 ) )
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
			MasterThreadManager testMan;
			Assert::AreEqual( false, testMan.runningStatus( ));
			Assert::AreEqual( false, testMan.getAbortStatus( ));
			Assert::AreEqual( false, testMan.getIsPaused( ));
			Assert::ExpectException<Error>( [this, &testMan] {testMan.abort( ); } );
			Assert::ExpectException<Error>( [this, &testMan] {testMan.unPause( ); } );
			Assert::ExpectException<Error>( [this, &testMan] {testMan.pause( ); } );
		}
		TEST_METHOD( TimeCalc )
		{
			MasterThreadManager testMan;
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
			MasterThreadManager testMan;
			DioSystem dio( true, true, true );
			handleInitDio( dio, testMan );
		}
	};
	TEST_CLASS( Test_Script_Testing )
	{
		// combines ttls, dacs, timing, function calls, repeat calls, etc. 
		void analyzeScript (std::string script, AoSystem& ao, DioSystem& dio, std::vector<parameterType>& vars, 
			bool expectEvalFail=false)
		{
			// script reading and checking snapshots
			MasterThreadManager testMan;
			RohdeSchwarz rsg;
			ScriptStream stream (script);
			timeType opTime;
			std::vector<timeType> loadSkipTimes (1);
			std::string warns;
			std::vector<std::pair<UINT, UINT>>  ttlShades;
			std::vector<UINT> dacShades;
			ao.initializeDataObjects (1, 0);
			dio.initializeDataObjects (1, 0);
			std::string scope = NO_PARAMETER_SCOPE;
			std::string word;
			try
			{
				MasterThreadManager::analyzeMasterScript (dio, ao, ttlShades, dacShades, rsg, vars, stream, 0, false, warns,
					opTime, loadSkipTimes);
				std::vector<std::vector<parameterType>> emptyVars2 (1, vars);
				dio.interpretKey (emptyVars2);
				dio.restructureCommands ();
				ao.interpretKey (emptyVars2, std::string ());
				ao.organizeDacCommands (0, 0);
				ao.setDacTriggerEvents (dio, 0, 0, 1);
				dio.organizeTtlCommands (0, 0);
				dio.findLoadSkipSnapshots (0, vars, 0, 0);
				dio.checkNotTooManyTimes (0, 0);
				dio.checkFinalFormatTimes (0, 0);
				dio.convertToFtdiSnaps (0, 0);
				ao.findLoadSkipSnapshots (0, vars, 0, 0);
				ao.makeFinalDataFormat (0, 0);
			}
			catch (Error & err)
			{
				if (expectEvalFail)
				{
					throwNested("Failed as expected.");
				}
				auto errMsg = err.trace ();
				Assert::Fail ((L"Excepted on standard script analysis. Error: " 
					+ std::wstring (errMsg.begin (), errMsg.end ())).c_str ());
			}
			if (expectEvalFail)
			{
				Assert::Fail (L"Expected to fail on analyzing script but didn't!");
			}
		}
		void standardVerifications (std::vector<DioSnapshot>& dioSnaps, std::vector<AoSnapshot>& aoSnaps, 
			std::array< std::array<bool, 16>, 4 >& testStat)
		{
			Assert::AreEqual (aoSnaps[0].time, double (0));
			Assert::IsTrue (aoSnaps[0].dacValues == std::array<double, 24> ({ 0 }));
			Assert::AreEqual (aoSnaps[1].time, double (1));
			Assert::AreEqual (aoSnaps[1].dacValues[0], double (0.5));
			// assert that the important structures are filled correctly.			
			Assert::AreEqual (double (0.001), dioSnaps[0].time, 1e-8);
			Assert::AreEqual (double (0.0015), dioSnaps[1].time, 1e-8);
			Assert::AreEqual (double (1.001), dioSnaps[2].time, 1e-8);
			Assert::AreEqual (double (1.0015), dioSnaps[3].time, 1e-8);
			
			// initial dac trigger
			testStat[3][15] = true;
			Assert::IsTrue (dioSnaps[0].ttlStatus == testStat);
			testStat[3][15] = false;
			Assert::IsTrue (dioSnaps[1].ttlStatus == testStat);
			testStat[0][0] = true;
			testStat[3][15] = true;
			Assert::IsTrue (dioSnaps[2].ttlStatus == testStat);
			testStat[3][15] = false;
			Assert::IsTrue (dioSnaps[3].ttlStatus == testStat);
		}
		TEST_METHOD( Simple_Script )
		{
			AoSystem ao (true);
			DioSystem dio (true, true, true);
			analyzeScript (simpleScript, ao, dio, std::vector<parameterType>());
			auto aoSnaps = ao.getSnapshots ()(0,0);
			auto dioSnaps = dio.getTtlSnapshots ()(0, 0);
			std::array< std::array<bool, 16>, 4 > testStat  = { 0 };
			standardVerifications (dioSnaps, aoSnaps, testStat);
			Assert::AreEqual (double (11.001), dioSnaps[4].time, 1e-8);
			Assert::AreEqual (aoSnaps.size (), size_t (2));
			Assert::AreEqual (size_t (5), dioSnaps.size ());
			testStat[0][0] = false;
			Assert::IsTrue (dioSnaps[4].ttlStatus == testStat);
		}
		TEST_METHOD( Use_A_Global_Variable )
		{
			AoSystem ao (true);
			DioSystem dio (true, true, true);
			std::vector<std::vector<parameterType>> params{ 1, { 1,{"timetowait", true, 30, false, false, 0, {}, {}, false, "global"} } };
			IndvRangeInfo info{ 2,true,true };
			ScanRangeInfo scans;
			scans.defaultInit ();
			ParameterSystem::generateKey (params, false, scans);

			analyzeScript (scriptBase + scriptEnd, ao, dio, params[0]);

			auto aoSnaps = ao.getSnapshots ()(0, 0);
			auto dioSnaps = dio.getTtlSnapshots ()(0, 0);
			auto testStat = std::array< std::array<bool, 16>, 4 > ({ 0 });
			standardVerifications (dioSnaps, aoSnaps, testStat);
			Assert::AreEqual (aoSnaps.size (), size_t (2));
			// assert that the important structures are filled correctly.
			Assert::AreEqual (size_t (5), dioSnaps.size ());
			Assert::AreEqual (double (31.001), dioSnaps[4].time, 1e-8);
			testStat[0][0] = false;
			Assert::IsTrue (dioSnaps[4].ttlStatus == testStat);
		}
		TEST_METHOD (Use_A_Config_Variable)
		{
			AoSystem ao (true);
			DioSystem dio (true, true, true);
			std::vector<std::vector<parameterType>> params{ 1, { 1,{"timetowait", true, 20, false, false, 0, {}, {}, false, "parent"} } };
			ScanRangeInfo scans;
			scans.defaultInit ();
			ParameterSystem::generateKey (params, false, scans);
			
			analyzeScript (scriptBase + scriptEnd, ao, dio, params[0]);
			auto aoSnaps = ao.getSnapshots ()(0, 0);
			auto dioSnaps = dio.getTtlSnapshots ()(0, 0);
			auto testStat = std::array< std::array<bool, 16>, 4 > ({ 0 });
			standardVerifications (dioSnaps, aoSnaps, testStat);
			Assert::AreEqual (aoSnaps.size (), size_t (2));
			Assert::AreEqual (size_t (5), dioSnaps.size ());
			Assert::AreEqual (double (21.001), dioSnaps[4].time, 1e-8);
			testStat[0][0] = false;
			Assert::IsTrue (dioSnaps[4].ttlStatus == testStat);
		}
		TEST_METHOD (UseSimpleFunction)
		{
			AoSystem ao (true);
			DioSystem dio (true, true, true);
			std::vector<std::vector<parameterType>> params{ 1, { 1,{"timetowait", true, 20, false, false, 0, {}, {}, false, "parent"} } };
			ScanRangeInfo scans;
			scans.defaultInit ();
			ParameterSystem::generateKey (params, false, scans);

			analyzeScript (scriptBase + "call ___testFunction()\n" + scriptEnd, ao, dio, params[0]);
			auto aoSnaps = ao.getSnapshots ()(0, 0);
			auto dioSnaps = dio.getTtlSnapshots ()(0, 0);
			auto testStat = std::array< std::array<bool, 16>, 4 > ({ 0 });
			standardVerifications (dioSnaps, aoSnaps, testStat);
			Assert::AreEqual (size_t (3), aoSnaps.size ());
			Assert::AreEqual (double (0), aoSnaps[1].dacValues[1]); 
			Assert::AreEqual (double (0.5), aoSnaps[2].dacValues[0]);
			Assert::AreEqual (double (-10), aoSnaps[2].dacValues[1]);
			Assert::AreEqual (size_t (7), dioSnaps.size ());
			Assert::AreEqual (double (22.001), dioSnaps[4].time, 1e-8);
			Assert::AreEqual (double (22.0015), dioSnaps[5].time, 1e-8);
			Assert::AreEqual (double (23.001), dioSnaps[6].time, 1e-8);
			testStat[0][0] = false;
			testStat[1][0] = true;
			testStat[3][15] = true;
			Assert::IsTrue (dioSnaps[4].ttlStatus == testStat);
			testStat[3][15] = false;
			Assert::IsTrue (dioSnaps[5].ttlStatus == testStat);
			Assert::IsTrue (dioSnaps[6].ttlStatus == testStat);
		}
		void stardardArgFunction ( std::vector<std::vector<parameterType>> params, std::string script, double expectedDac1Val,
								   bool expectEvalFail=false )
		{
			AoSystem ao (true);
			DioSystem dio (true, true, true);
			analyzeScript (script, ao, dio, params[0], expectEvalFail);
			auto aoSnaps = ao.getSnapshots ()(0, 0);
			auto dioSnaps = dio.getTtlSnapshots ()(0, 0);
			auto testStat = std::array< std::array<bool, 16>, 4 > ({ 0 });
			standardVerifications (dioSnaps, aoSnaps, testStat);
			Assert::AreEqual (size_t (3), aoSnaps.size ());
			Assert::AreEqual (double (0), aoSnaps[1].dacValues[1]);
			Assert::AreEqual (double (0.5), aoSnaps[2].dacValues[0]);
			Assert::AreEqual (expectedDac1Val, aoSnaps[2].dacValues[1]);
			// assert that the important structures are filled correctly.

			Assert::AreEqual (size_t (7), dioSnaps.size ());
			Assert::AreEqual (double (12.001), dioSnaps[4].time, 1e-8);
			Assert::AreEqual (double (12.0015), dioSnaps[5].time, 1e-8);
			Assert::AreEqual (double (13.001), dioSnaps[6].time, 1e-8);
			testStat[0][0] = false;
			testStat[1][0] = true;
			testStat[3][15] = true;
			Assert::IsTrue (dioSnaps[4].ttlStatus == testStat);
			testStat[3][15] = false;
			Assert::IsTrue (dioSnaps[5].ttlStatus == testStat);
			Assert::IsTrue (dioSnaps[6].ttlStatus == testStat);
		}
		TEST_METHOD (FunctionConstantArg)
		{
			std::string script = scriptBase + "call ___testFunction2(-5)\n" + scriptEnd;
			std::vector<std::vector<parameterType>> params{ 1 };
			stardardArgFunction (params, script, -5);
			// test a couple variations of expressions as arguments
			script = scriptBase + "call ___testFunction2(-6-1)\n" + scriptEnd;
			params = std::vector<std::vector<parameterType>> { 1 };
			stardardArgFunction (params, script, -7);
			script = scriptBase + "call ___testFunction2( - 6 - 1 )\n" + scriptEnd;
			params = std::vector<std::vector<parameterType>>{ 1 };
			stardardArgFunction (params, script, -7);
			script = scriptBase + "call ___testFunction2((-6-1))\n" + scriptEnd;
			params = std::vector<std::vector<parameterType>>{ 1 };
			stardardArgFunction (params, script, -7);
		}
		TEST_METHOD (FunctionLocalVarArg)
		{
			// should work
			std::string function2VarArgScript = scriptBase + "call ___testFunction2(timeToWait)\n" + scriptEnd;
			std::vector<std::vector<parameterType>> params{ 1 };
			stardardArgFunction (params, function2VarArgScript, 10);
		}
		TEST_METHOD (FunctionConfigVarArgProper)
		{
			// this should work.
			std::string function2VarArgScript = scriptBase + "call ___testFunction2(configArg)\n" + scriptEnd;
			std::vector<std::vector<parameterType>> params{ 1, { 1,{"configarg", true, 3, false, false, 0, {}, {}, false, "parent"} } };
			ScanRangeInfo scans;
			scans.defaultInit ();
			ParameterSystem::generateKey (params, false, scans);
			stardardArgFunction (params, function2VarArgScript, 3);
		}
		TEST_METHOD (FunctionConfigVarArgPotentialNameConflictProper)
		{
			// The name decorating should prevent any issues here. inside ___testfunction2 there is a local variable
			// called timeDelay. if the replacements just replaced with new name then the after the script 
			// would have two "timedelay"s in it where one should be the argument value and one should be the local
			// variable value. Thus, this would mess up either the value of the local var or the config var depending 
			// on which one took priority in the param list.
			std::string function2VarArgScript = scriptBase + "call ___testFunction2(timedelay)\n" + scriptEnd;
			std::vector<std::vector<parameterType>> params{ 1, { 1,{"timedelay", true, 3, false, false, 0, {}, {}, false, "parent"} } };
			ScanRangeInfo scans;
			scans.defaultInit ();
			ParameterSystem::generateKey (params, false, scans);
			stardardArgFunction (params, function2VarArgScript, 3);
		}

		TEST_METHOD (FunctionArgBadScope)
		{
			// this shouldn't work, the scope here should be considered wrong.
			std::string script = scriptBase + "call ___testFunction2(timeToWait)\n" + scriptEnd;
			std::vector<std::vector<parameterType>> params{ 1, { 1,{"timetowait", true, 3, false, false, 0, {}, {}, 
				false, "___testfunction2"} } };
			ScanRangeInfo scans;
			scans.defaultInit ();
			ParameterSystem::generateKey (params, false, scans);
			// the value should take the local variable value of 10 instead of the attempted hack value of 3 in the 
			// params above.
			stardardArgFunction (params, script, 10);

			script = scriptBase + "call ___testFunction2(arg)\n" + scriptEnd;
			params = std::vector<std::vector<parameterType>>{ 1, { 1,{"arg", true, 3, false, false, 0, {}, {},
				false, "___testfunction2"} } };
			scans = ScanRangeInfo();
			scans.defaultInit ();
			ParameterSystem::generateKey (params, false, scans);
			// It might be unexpected taht this actually works. since the function argument won't evaluate to a variable,
			// it won't get decorated, which means that this old hack will function. 
			stardardArgFunction (params, script, 3);

			script = scriptBase + "call ___testFunction2(arg)\n" + scriptEnd;
			params = std::vector<std::vector<parameterType>>{ 1, { 1,{"arg", true, 3, false, false, 0, {}, {},
				false, "___testfunction2"} } };
			params[0].push_back ({"arg", true, 6.5, false, false, 0, {}, {}, false, "parent"});
			scans = ScanRangeInfo ();
			scans.defaultInit ();
			ParameterSystem::generateKey (params, false, scans);
			// in this case the argument name gets decorated, so the parent level "arg" gets used instead of the 
			// function level "arg".
			stardardArgFunction (params, script, 6.5);


			params = std::vector<std::vector<parameterType>>{ 1, { 1,{"badname", true, 3, false, false, 0, {}, {},
				false, "___testfunction2"} } };
			scans = ScanRangeInfo ();
			scans.defaultInit ();
			ParameterSystem::generateKey (params, false, scans);
			// this should fail as it just comes out as an expression that can't evaluate.
			Assert::ExpectException<Error> ([this, params, script]() {stardardArgFunction (params, script, 0, true); });
		}

		TEST_METHOD( Use_A_Local_Variable )
		{
			AoSystem ao (true);
			DioSystem dio (true, true, true);
			analyzeScript (scriptBase + scriptEnd, ao, dio, std::vector<parameterType> ());
			auto aoSnaps = ao.getSnapshots ()(0, 0);
			auto dioSnaps = dio.getTtlSnapshots ()(0, 0);
			auto testStat = std::array< std::array<bool, 16>, 4 > ({ 0 });
			standardVerifications (dioSnaps, aoSnaps, testStat);
			Assert::AreEqual (aoSnaps.size (), size_t (2));
			// assert that the important structures are filled correctly.
			Assert::AreEqual (size_t (5), dioSnaps.size ());
			Assert::AreEqual (double (11.001), dioSnaps[4].time, 1e-8);
			testStat[0][0] = false;
			Assert::IsTrue (dioSnaps[4].ttlStatus == testStat);
		}
	private:
		std::string scriptBase =
			"t = 1\n"
			"var timeToWait 10\n"
			"on: a0\n"
			"dac: dac0 0.5\n"
			"t += timeToWait\n";
		std::string scriptEnd =
			"off: a0\n"
			"\n\n__END__";
		std::string simpleScript = 
			"t = 1\n"
			"on: a0\n"
			"dac: dac0 0.5\n"
			"t += 10\n"
			"off: a0\n"
			"\n\n__END__";
	};
}



