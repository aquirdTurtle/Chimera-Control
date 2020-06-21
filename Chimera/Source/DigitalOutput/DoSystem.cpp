// created by Mark O. Brown
#include "stdafx.h"

#include "DoSystem.h"

#include "ConfigurationSystems/Version.h"
#include "LowLevel/constants.h"
#include "PrimaryWindows/QtAuxiliaryWindow.h"
#include "GeneralUtilityFunctions/range.h"

#include <sstream>
#include <unordered_map>
#include <bitset>
#include "nidaqmx2.h"
#include <boost/lexical_cast.hpp>

// I don't use this because I manually import dll functions.
// #include "Dio64.h"
DoSystem::DoSystem( bool ftSafemode, bool serialSafemode ) : core(ftSafemode, serialSafemode)
{
	for (auto& out : outputs) { out.set(0); }
}
DoSystem::~DoSystem() { }


void DoSystem::handleSaveConfig(ConfigStream& saveFile)
{
	/// ttl settings
	saveFile << "TTLS\n";
	// nothing at the moment.
	saveFile << "END_TTLS\n";
}


void DoSystem::handleOpenConfig(ConfigStream& openFile)
{
	if ( openFile.ver < Version ( "3.7" ) )
	{
		for ( auto i : range ( 64 ) )
		{
			// used to store an initial ttl config in the config file.
			std::string trash;
			openFile >> trash;
		}
	}
}


void DoSystem::setTtlStatusNoForceOut(std::array< std::array<bool, 16>, 4 > status)
{
	for ( auto rowInc : range(status.size()) )
	{
		for ( auto numInc : range(status[rowInc].size()) )
		{
			outputs ( numInc, DoRows::which ( rowInc ) ).set ( status[ rowInc ][ numInc ] );
		}
	}
}


Matrix<std::string> DoSystem::getAllNames()
{
	return core.getAllNames ();
}

void DoSystem::rearrange(UINT width, UINT height, fontMap fonts)
{
	/*
	ttlTitle.rearrange( width, height, fonts);
	ttlHold.rearrange( width, height, fonts);
	zeroTtls.rearrange( width, height, fonts);
	for ( auto& out : outputs )
	{
		out.rearrange ( width, height, fonts );
	}
	for (auto& control : ttlNumberLabels)
	{
		control.rearrange( width, height, fonts);
	}
	for (auto& control : ttlRowLabels)
	{
		control.rearrange( width, height, fonts);
	}*/
}


void DoSystem::updatePush( DoRows::which row, UINT number )
{
	outputs ( number, row ).updateStatus ( );
}


void DoSystem::handleInvert()
{
	for ( auto& out : outputs )
	{
		// seems like I need a ! here...
		out.set (out.getStatus ()); 
		core.ftdi_ForceOutput (out.getPosition ().first, out.getPosition ().second, out.getStatus (), getCurrentStatus ());
	}
}


void DoSystem::updateDefaultTtl( DoRows::which row, UINT column, bool state)
{
	outputs ( column, row ).defaultStatus = state;
}


bool DoSystem::getDefaultTtl( DoRows::which row, UINT column)
{
	return outputs ( column, row ).defaultStatus;
}


std::pair<UINT, UINT> DoSystem::getTtlBoardSize()
{
	return { outputs.numRows, outputs.numColumns };
}


void DoSystem::initialize( POINT& loc, IChimeraWindowWidget* parent )
{
	// title
	ttlTitle = new QLabel ("TTLS", parent);
	ttlTitle->setGeometry (loc.x, loc.y, 480, 25);
	loc.y += 25;
	// all number numberLabels
	ttlHold = new CQPushButton ("Hold Current Values", parent);
	ttlHold->setGeometry (loc.x, loc.y, 240, 20);
	ttlHold->setToolTip ("Press this button to change multiple TTLs simultaneously. Press the button, then change the "
		"ttls, then press the button again to release it. Upon releasing the button, the TTLs will "
		"change.");
	parent->connect (ttlHold, &QPushButton::released, [parent, this]() {
		try
		{
			parent->configUpdated ();
			handleHoldPress ();
		}
		catch (Error& exception)
		{
			parent->reportErr ("TTL Hold Handler Failed: " + exception.trace () + "\r\n");
		}
	});
	ttlHold->setCheckable (true);

	zeroTtls = new CQPushButton ("Zero TTLs", parent);
	zeroTtls->setGeometry (loc.x + 240, loc.y, 240, 20);
	zeroTtls->setToolTip( "Press this button to set all ttls to their zero (false) state." );
	parent->connect (zeroTtls, &QPushButton::released, [parent, this]() {
		try	{
			zeroBoard ();
			parent->configUpdated();
			parent->reportStatus ("Zero'd TTLs.\r\n");
		}
		catch (Error& exception) {
			parent->reportStatus ("Failed to Zero TTLs!!!\r\n");
			parent->reportErr (exception.trace ());
		}
	});
	loc.y += 20;

	for (long ttlNumberInc : range (ttlNumberLabels.size ())) {
		ttlNumberLabels[ttlNumberInc] = new QLabel (cstr (ttlNumberInc), parent);
		ttlNumberLabels[ttlNumberInc]->setGeometry ({ QPoint (loc.x + 32 + ttlNumberInc * 28, loc.y),
													  QPoint (loc.x + 32 + (ttlNumberInc + 1) * 28, loc.y + 20) });
	}
	loc.y += 20;
	// all row numberLabels
	for ( auto row : DoRows::allRows )
	{
		ttlRowLabels[int (row)] = new QLabel ((DoRows::toStr (row)).c_str(), parent);
		ttlRowLabels[int (row)]->setGeometry (loc.x, loc.y + int (row) * 28, 32, 28);
	}
	// all push buttons
	UINT runningCount = 0;
	auto startX = loc.x + 32;
	for (auto row : DoRows::allRows )
	{
		loc.x = startX;
		for (UINT number = 0; number < outputs.numColumns; number++)
		{
			auto& out = outputs (number, row);
			out.initialize ( loc, parent );
			parent->connect ( out.check, &QCheckBox::stateChanged, [this, &out, parent]() {
				try {
					handleTTLPress (out);
					parent->configUpdated ();
				}
				catch (Error& exception)
				{
					parent->reportErr ("TTL Press Handler Failed.\n" + exception.trace () + "\r\n");
				}
			});
			loc.x += 28;
		}
		loc.y += 28;
	}
	loc.x = startX - 32;
}


int DoSystem::getNumberOfTTLRows()
{
	return outputs.numRows;
}


int DoSystem::getNumberOfTTLsPerRow()
{
	return outputs.numColumns;
}


void DoSystem::handleTTLPress(DigitalOutput& out)
{
	if ( holdStatus == false )
	{
		//out.set (!out.getStatus ());
		out.set (out.check->isChecked ());
		core.ftdi_ForceOutput(out.getPosition().first, out.getPosition().second, !out.getStatus(), getCurrentStatus ());
	}
	else
	{
		out.setHoldStatus ( !out.holdStatus );
	}
}

// this function handles when the hold button is pressed.
void DoSystem::handleHoldPress()
{
	if (holdStatus == true)
	{
		// set all the holds.
		holdStatus = false;
		// make changes
		for ( auto& out : outputs )
		{
			out.set ( out.holdStatus );
			core.ftdi_ForceOutput (out.getPosition ().first, out.getPosition ().second, out.getStatus (), getCurrentStatus());
		}
	}
	else
	{
		holdStatus = true;
		for ( auto& out : outputs )
		{
			out.setHoldStatus ( out.getStatus ( ) );
		}
	}
}


std::array< std::array<bool, 16>, 4 > DoSystem::getCurrentStatus( )
{
	std::array< std::array<bool, 16>, 4 > currentStatus;
	for ( auto& out : outputs )
	{
		currentStatus[ int ( out.getPosition ( ).first ) ][ out.getPosition ( ).second ] = out.getStatus();
	}
	return currentStatus;
}


void DoSystem::setName( DoRows::which row, UINT number, std::string name)
{
	if (name == "")
	{
		// no empty names allowed.
		return;
	}
	outputs ( number, row ).setName ( name );
	auto names = core.getAllNames ();
	names (UINT(row), number) = name;
	core.setNames (names);
}


std::string DoSystem::getName( DoRows::which row, UINT number)
{
	return core.getAllNames ()(UINT (row), number);
}


bool DoSystem::getTtlStatus(DoRows::which row, int number)
{
	return outputs ( number, row ).getStatus ( );
}


allDigitalOutputs& DoSystem::getDigitalOutputs ( )
{
	return outputs;
}



std::pair<USHORT, USHORT> DoSystem::calcDoubleShortTime( double time )
{
	USHORT lowordTime, hiwordTime;
	// convert to system clock ticks. Assume that the crate is running on a 10 MHz signal, so multiply by
	// 10,000,000, but then my time is in milliseconds, so divide that by 1,000, ending with multiply by 10,000
	lowordTime = ULONGLONG( time * 10000 ) % 65535;
	hiwordTime = ULONGLONG( time * 10000 ) / 65535;
	if ( ULONGLONG( time * 10000 ) / 65535 > 65535 )
	{
		thrower ( "DIO system was asked to calculate a time that was too long! this is limited by the card." );
	}
	return { lowordTime, hiwordTime };
}

std::string DoSystem::getDoSystemInfo () {	return core.getDoSystemInfo (); }
bool DoSystem::getFtFlumeSafemode () { return core.getFtFlumeSafemode (); }

void DoSystem::zeroBoard( )
{
	for ( auto& out : outputs )
	{
		out.set (0); 
		core.ftdi_ForceOutput (out.getPosition ().first, out.getPosition ().second, 0, getCurrentStatus ());	
	}
}


DoCore& DoSystem::getCore ()
{
	return core;
}

