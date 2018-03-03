#include "stdafx.h"
#include "CppUnitTest.h"
#include "afxwin.h"
#include "../Chimera/NiawgController.h"
#include "../Chimera/miscellaneousCommonFunctions.h"
#include <string>
//fsnamespace Microsoft{ namespace VisualStudio {namespace CppUnitTestFramework
using namespace Microsoft::VisualStudio::CppUnitTestFramework;


namespace TestNiawg
{
	TEST_CLASS( TestNiawg )
	{
		public:
		TEST_METHOD( InitNiawg )
		{
			UINT trigRow(0), trigNum( 0 );
			bool safemode( true );
			NiawgController niawg(trigRow, trigNum, safemode );
			std::pair<UINT, UINT> res = niawg.getTrigLines( );
			Assert::AreEqual( res.first, trigRow );
			Assert::AreEqual( res.second, trigNum );
			Assert::AreEqual( niawg.getCurrentScript( ), std::string("") );
			UINT numTrigs = niawg.getNumberTrigsInScript( );
			Assert::AreEqual( UINT( 0 ), numTrigs );
		}
		TEST_METHOD( SimpleScript_1_Waveform )
		{
			// assert that analyzeNiawgScript correctly creates a simple form, i.e. it reads the
			// script correctly and fills out the form correctly.
			UINT trigRow( 0 ), trigNum( 0 );
			bool safemode( true );
			NiawgController niawg( trigRow, trigNum, safemode );
			ScriptStream stream(simpleScript);
			NiawgOutput output;
			//
			niawg.analyzeNiawgScript( stream, output, profileSettings(), debugInfo(), std::string(), rerngOptions(), 
									  std::vector<variableType>() );
			// asserts
			Assert::AreEqual( size_t( 1 ), output.waveFormInfo.size( ));
			Assert::AreEqual( size_t( 1 ), output.waves.size( ) );
			Assert::AreEqual( false, bool(output.isDefault) );
			Assert::AreEqual( str("generate Waveform1\n"), output.niawgLanguageScript);
			auto& form = output.waveFormInfo[0];
			Assert::AreEqual( false, form.flash.isFlashing );
			Assert::AreEqual( false, form.rearrange.isRearrangement );
			Assert::AreEqual( false, form.isStreamed );
			auto& core=form.core;
			Assert::AreEqual( size_t( 2 ), core.chan.size( ) );
			Assert::AreEqual( std::string( "Waveform1" ), core.name );
			Assert::AreEqual( std::string( "0.01" ), core.time.expressionStr );
			Assert::AreEqual( false, core.varies );
			UINT count = 0;
			std::vector<std::string> freqs = { "70", "80" };
			for ( auto& axis : core.chan )
			{
				Assert::AreEqual( std::string( "#" ), axis.delim );
				Assert::AreEqual( 0, axis.phaseOption );
				Assert::AreEqual( 1, axis.initType );
				Assert::AreEqual( size_t( 1 ), axis.signals.size( ) );
				auto& signal = axis.signals[0];
				Assert::AreEqual( std::string( "1" ), signal.initPower.expressionStr );
				Assert::AreEqual( std::string( "1" ), signal.finPower.expressionStr );
				Assert::AreEqual( std::string( freqs[count] ), signal.freqInit.expressionStr );
				Assert::AreEqual( std::string( freqs[count] ), signal.freqFin.expressionStr );
				Assert::AreEqual( std::string( "nr" ), signal.freqRampType );
				Assert::AreEqual( std::string( "nr" ), signal.powerRampType );
				count++;
			}
		}
		TEST_METHOD( SimpleScript_2_Wave )
		{
			UINT trigRow( 0 ), trigNum( 0 );
			bool safemode( true );
			NiawgController niawg( trigRow, trigNum, safemode );
			ScriptStream stream( simpleScript );
			NiawgOutput output;
			//
			niawg.analyzeNiawgScript( stream, output, profileSettings( ), debugInfo( ), std::string( ), rerngOptions( ),
									  std::vector<variableType>( ) );
			niawg.writeStaticNiawg( output, debugInfo( ), std::vector<variableType>( ) );
			// asserts
			auto& wave = output.waves[0];
			Assert::AreEqual( false, wave.flash.isFlashing );
			Assert::AreEqual( false, wave.rearrange.isRearrangement );
			Assert::AreEqual( false, wave.isStreamed );
			auto& core = wave.core;
			Assert::AreEqual( str( "Waveform1" ), core.name );
			Assert::AreEqual( size_t( 2 ), core.chan.size( ) );
			Assert::AreEqual( 0.01e-3, core.time );
			Assert::AreEqual( false, core.varies );
			Assert::AreEqual( size_t( 0 ), core.waveVals.size( ) );
			Assert::AreEqual( long( NIAWG_SAMPLE_RATE * 0.01e-3 ), core.sampleNum );
			UINT count = 0;
			std::vector<double> freqs = { 70.0e6, 80.0e6 };
			for ( auto& axis : core.chan )
			{
				Assert::AreEqual( std::string( "#" ), axis.delim );
				Assert::AreEqual( 0, axis.phaseOption );
				Assert::AreEqual( 1, axis.initType );
				Assert::AreEqual( size_t( 1 ), axis.signals.size( ) );
				auto& signal = axis.signals[0];
				Assert::AreEqual( 1.0, signal.initPower);
				Assert::AreEqual( 1.0, signal.finPower );
				Assert::AreEqual( freqs[count], signal.freqInit );
				Assert::AreEqual( freqs[count], signal.freqFin );
				Assert::AreEqual( std::string( "nr" ), signal.freqRampType );
				Assert::AreEqual( std::string( "nr" ), signal.powerRampType );
				count++;
			}
		}
		TEST_METHOD( SimpleScript_3_Wavevals )
		{
			UINT trigRow( 0 ), trigNum( 0 );
			bool safemode( true );
			NiawgController niawg( trigRow, trigNum, safemode );
			ScriptStream stream( simpleScript );
			NiawgOutput output;
			//
			niawg.analyzeNiawgScript( stream, output, profileSettings( ), debugInfo( ), std::string( ), rerngOptions( ),
									  std::vector<variableType>( ) );
			niawg.writeStaticNiawg( output, debugInfo( ), std::vector<variableType>( ), false );
			//
			auto& vals = output.waves[0].core.waveVals;
			// should be long enough to sum close to zero.
			double sum = 0;
			double max = 0, min = 0;
			for ( auto val : vals )
			{
				sum += val;
				if ( val > max )
				{
					max = val;
				}
				else if ( val < min )
				{
					min = val;
				}
			}
			Assert::AreEqual( 0.0, sum, 1e-9 );
			Assert::AreEqual( 0.0, vals[0] );
			Assert::AreEqual( 0.0, vals[1] );
			Assert::AreNotEqual( vals[2], vals[3] );			
			Assert::IsTrue( max < 1.0 );
			Assert::IsTrue( min > -1.0 );
		}
		TEST_METHOD( SimpleRerng )
		{			
			UINT trigRow( 0 ), trigNum( 0 );
			bool safemode( true );
			NiawgController niawg( trigRow, trigNum, safemode );
			Matrix<bool> source( 6, 3 );
			Matrix<bool> target( 6, 3 );
			
			loadBools( source, std::vector<bool>(
							  { 0,1,0,
							    0,1,0,
							    0,1,0,
							    0,0,1,
							    0,1,0,
								0,1,0 } ) );
			loadBools( target, std::vector<bool>(
							   { 0,1,0,
								 0,1,0,
								 0,1,0,
								 0,1,0,
								 0,1,0,
								 0,1,0 } ) );
			std::vector<simpleMove> moves;
			niawgPair<ULONG> finPos;
			niawg.smartRearrangement( source, target, finPos, { 0,0 }, moves, rerngOptions() );
			Assert::AreEqual( size_t( 1 ), moves.size( ) );
			Assert::IsTrue( simpleMove({3,2,3,1}) == moves.front( ) );
		}
		TEST_METHOD( ComplexRerng )
		{
			UINT trigRow( 0 ), trigNum( 0 );
			bool safemode( true );
			NiawgController niawg( trigRow, trigNum, safemode );
			Matrix<bool> source( 6, 3 );
			Matrix<bool> target( 6, 3 );

			loadBools( source, std::vector<bool>(
			  { 0,1,1,
				0,0,0,
				0,0,0,
				1,0,1,
				1,1,0,
				0,1,1 } ) );
			loadBools( target, std::vector<bool>(
			{ 0,1,0,
				0,1,0,
				0,1,0,
				0,1,0,
				0,1,0,
				0,1,0 } ) );
			std::vector<simpleMove> moves;
			niawgPair<ULONG> finPos;
			niawg.smartRearrangement( source, target, finPos, { 0,0 }, moves, rerngOptions( ) );
			// the following moves were copied from a trial that I know rearranges correctly. Details of the moves
			// are hard to actualy predict.
			Assert::AreEqual( size_t( 5 ), moves.size( ) );
			Assert::IsTrue( simpleMove( { 0,2,1,2 } ) == moves[0] );
			Assert::IsTrue( simpleMove( { 1,2,1,1 } ) == moves[1] );
			Assert::IsTrue( simpleMove( { 3,0,2,0 } ) == moves[2] );
			Assert::IsTrue( simpleMove( { 2,0,2,1 } ) == moves[3] );
			Assert::IsTrue( simpleMove( { 3,2,3,1 } ) == moves[4] );
		}
		TEST_METHOD( NoFlashRerng )
		{
			// a predictable no-flash config.
			UINT trigRow( 0 ), trigNum( 0 );
			bool safemode( true );
			NiawgController niawg( trigRow, trigNum, safemode );
			Matrix<bool> source( 6, 3 );
			Matrix<bool> target( 6, 3 );

			loadBools( source, std::vector<bool>(
				{ 0,1,0,
				0,0,0,
				0,1,0,
				0,1,0,
				0,1,0,
				0,1,0} ) );
			loadBools( target, std::vector<bool>(
				{ 0,0,0,
				0,1,0,
				0,1,0,
				0,1,0,
				0,1,0,
				0,1,0 } ) );
			std::vector<simpleMove> smoves;
			std::vector<complexMove> moves;
			niawgPair<ULONG> finPos;
			niawg.smartRearrangement( source, target, finPos, { 0,0 }, smoves, rerngOptions( ) );
			niawg.optimizeMoves( smoves, source, moves, rerngOptions( ) );
			Assert::AreEqual( size_t(1), moves.size( ) );
			Assert::AreEqual( false, moves.front().needsFlash );
		}
		TEST_METHOD( ComplexOptimizeRerng )
		{
			UINT trigRow( 0 ), trigNum( 0 );
			bool safemode( true );
			NiawgController niawg( trigRow, trigNum, safemode );
			Matrix<bool> source( 6, 3 );
			Matrix<bool> target( 6, 3 );

			loadBools( source, std::vector<bool>(
			{ 0,1,1,
				0,0,0,
				0,0,0,
				1,0,1,
				1,1,0,
				0,1,1 } ) );
			loadBools( target, std::vector<bool>(
			{ 0,1,0,
				0,1,0,
				0,1,0,
				0,1,0,
				0,1,0,
				0,1,0 } ) );
			std::vector<simpleMove> smoves;
			std::vector<complexMove> moves;
			niawgPair<ULONG> finPos;
			niawg.smartRearrangement( source, target, finPos, { 0,0 }, smoves, rerngOptions( ) );
			niawg.optimizeMoves( smoves, source, moves, rerngOptions());
		}

		private:
			const std::string simpleScript = "gen1const HORIZONTAL 80 1 0 # gen1const VERTICAL 70 1 0 # 0.01 0";
			const std::string rampScript = "gen2ampramp HORIZONTAL 80 lin 0.3 0.7 0 80 lin 0.7 0.3 0 # "
										   "gen1freqramp VERTICAL lin 70 80 1 0 # 0.01 0";
			const std::string complexLogicScript = "repeattiltrig "
													"gen1const HORIZONTAL 80 1 0 # "
													"gen1const VERTICAL 70 1 0 # 0.01 0"
													"endrepeat "
													"gen1const HORIZONTAL 80 1 0 # "
													"gen1const VERTICAL 70 1 0 # 0.01 0";
	};
}
