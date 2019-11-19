#include "stdafx.h"
#include "CppUnitTest.h"
#include "../Chimera/DioSystem.h"
#include "TestMacros.h"
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace TestDioSystem
{
	TEST_CLASS( TestDioSystem )
	{
		public:
		TEST_METHOD( InitDioTest )
		{
			DioSystem dio(true, true, true);
			auto status = dio.getCurrentStatus( );
			for ( auto row : status )
			{
				for ( auto elem : row )
				{
					Assert::AreEqual(false, elem);
				}
			}
		}
		TEST_METHOD( InitStructures )
		{
			DioSystem dio( true, true, true );
			dio.initializeDataObjects( 1, 1 );
			dio.interpretKey( std::vector<std::vector<parameterType>>( 1 ) );
			// Raises exception because no ttl commands are seen. 
			Assert::ExpectException<Error> ( [ this, &dio ] { dio.restructureCommands ( ); } );
			Assert::IsTrue ( dio.countTriggers ( { DioRows::which::A, 0 }, 0, 0 ) == 0 );
			auto size = dio.getTtlBoardSize( );
			Assert::IsTrue( size.first == 4 );
			Assert::IsTrue( size.second == 16 );
		}
		TEST_METHOD( DioTimeCalc )
		{
			DioSystem dio( true, true, true );
			std::vector<double> times = { 1e-4,10.5,100 };
			std::vector<std::pair<UINT, UINT>> diotimes = { {1,0},{39465,1},{16975,15} };
			for ( auto count : range( times.size( ) ))
			{
				auto dioTime = dio.calcDoubleShortTime( times[count] );
				Assert::IsTrue( dioTime.first == diotimes[count].first );
				Assert::IsTrue( dioTime.second == diotimes[count].second );
			}
		}

		TEST_METHOD( ForceOut )
		{
			DioSystem dio( true, true, true );
			dio.forceTtl( DioRows::which::A, 0, 1 );
			Assert::AreEqual( true, dio.getTtlStatus( DioRows::which::A, 0 ) );
		}
		TEST_METHOD( Ftdi_Force_Out_Disconnected )
		{
			DioSystem dio( true, true, true );
			auto res = dio.ftdi_ForceOutput( DioRows::which::A, 0, 1 );
			Assert::AreEqual( true, dio.getTtlStatus( DioRows::which::A, 0 ) );
			// this number doesn't mean much in debug mode; it's the size of the buffer which is always 
			// the same no matter how much the buffer is filled.
			Assert::AreEqual( DWORD(43008), res );
		}
		CONNECTED_TEST( c_Ftdi_Force_Out_Connected )
		{
			/*
			DioSystem dio( false, true, true );
			auto res = dio.ftdi_ForceOutput( DioRows::which::A, 0, 1 );
			Assert::AreEqual( true, dio.getTtlStatus( DioRows::which::A, 0 ) );
			Assert::AreEqual( DWORD( 1 ), res );
			res = dio.ftdi_ForceOutput( DioRows::which::A, 0, 0 );
			Assert::AreEqual( false, dio.getTtlStatus( DioRows::which::A, 0 ) );
			Assert::AreEqual( DWORD( 1 ), res );
			*/
		}

		TEST_METHOD( ZeroDio )
		{
			DioSystem dio( true, true, true );
			// set a bunch to non-zero to make sure it can flip them.
			dio.forceTtl( DioRows::which::A, 1, 1 );
			dio.forceTtl( DioRows::which::B, 0, 1 );
			dio.forceTtl( DioRows::which::A, 0, 1 );
			dio.forceTtl( DioRows::which::B, 11, 1 );
			dio.forceTtl( DioRows::which::C, 15, 1 );
			dio.forceTtl( DioRows::which::B, 8, 1 );
			dio.forceTtl ( DioRows::which::D, 8, 1 );
			dio.zeroBoard( );
			auto status = dio.getCurrentStatus( );
			for ( auto row : status )
			{
				for ( auto elem : row )
				{
					Assert::AreEqual( false, elem );
				}
			}
		}
		TEST_METHOD ( FtdiZeroDio )
		{
			DioSystem dio ( true, true, true );
			// set a bunch to non-zero to make sure it can flip them.
			dio.ftdi_ForceOutput ( DioRows::which::A, 1, 1 );
			dio.ftdi_ForceOutput ( DioRows::which::B, 0, 1 );
			dio.ftdi_ForceOutput ( DioRows::which::A, 0, 1 );
			dio.ftdi_ForceOutput ( DioRows::which::B, 11, 1 );
			dio.ftdi_ForceOutput ( DioRows::which::C, 15, 1 );
			dio.ftdi_ForceOutput ( DioRows::which::B, 8, 1 );
			dio.ftdi_ForceOutput ( DioRows::which::D, 8, 1 );
			dio.ftdiZeroBoard ( );
			auto status = dio.getCurrentStatus ( );
			for ( auto row : status )
			{
				for ( auto elem : row )
				{
					Assert::AreEqual ( false, elem );
				}
			}
		}
		CONNECTED_TEST( c_Connect_To_Ftdi )
		{
			DioSystem dio( false, false, true );
			try
			{
				dio.ftdi_connectasync( "FT2E722BB" );
				dio.ftdi_disconnect();
			}
			catch ( Error& err )
			{
				std::wstring errString( w_str( err.what( ) ) );
				try
				{
					dio.ftdi_disconnect( );
				}
				catch ( Error& err2 )
				{
					errString += L"Attempt to disconnect after initial fail also failed:" 
						+ w_str( err2.what( ) );
				}
				Assert::Fail(errString.c_str());
			}
		}
	};
}
