// created by Mark O. Brown
#include "stdafx.h"

#include "DoSystem.h"

#include "ConfigurationSystems/Version.h"
#include "LowLevel/constants.h"
#include "PrimaryWindows/AuxiliaryWindow.h"
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


void DoSystem::handleNewConfig( std::ofstream& newFile )
{
	newFile << "TTLS\n";
	// nothing at the moment.
	newFile << "END_TTLS\n";
}


void DoSystem::handleSaveConfig(std::ofstream& saveFile)
{
	/// ttl settings
	saveFile << "TTLS\n";
	// nothing at the moment.
	saveFile << "END_TTLS\n";
}


void DoSystem::handleOpenConfig(std::ifstream& openFile, Version ver )
{
	if ( ver < Version ( "3.7" ) )
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
	}
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


void DoSystem::initialize( POINT& loc, cToolTips& toolTips, CWnd* parent, int& id )
{
	// title
	ttlTitle.sPos = { loc.x, loc.y, loc.x + 480, loc.y + 25 };
	ttlTitle.Create( "TTLS", WS_CHILD | WS_VISIBLE | SS_CENTER, ttlTitle.sPos, parent, id++ );
	ttlTitle.fontType = fontTypes::HeadingFont;
	// all number numberLabels
	loc.y += 25;
	ttlHold.sPos = { loc.x, loc.y, loc.x + 240, loc.y + 20 };
	ttlHold.Create( "Hold Current Values", WS_TABSTOP | WS_VISIBLE | BS_AUTOCHECKBOX | WS_CHILD | BS_PUSHLIKE,
					ttlHold.sPos, parent, TTL_HOLD );
	ttlHold.setToolTip("Press this button to change multiple TTLs simultaneously. Press the button, then change the "
					   "ttls, then press the button again to release it. Upon releasing the button, the TTLs will "
					   "change.", toolTips, parent);

	zeroTtls.sPos = { loc.x + 240, loc.y, loc.x + 480, loc.y + 20 };
	zeroTtls.Create( "Zero TTLs", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, zeroTtls.sPos, parent, 
					 IDC_ZERO_TTLS );
	zeroTtls.setToolTip( "Press this button to set all ttls to their zero (false) state.", toolTips, parent );
	loc.y += 20;

	for (long ttlNumberInc = 0; ttlNumberInc < long(ttlNumberLabels.size()); ttlNumberInc++)
	{
		ttlNumberLabels[ttlNumberInc].sPos = { loc.x + 32 + ttlNumberInc * 28, loc.y,
			loc.x + 32 + (ttlNumberInc + 1) * 28, loc.y + 20 };
		ttlNumberLabels[ttlNumberInc].Create( cstr( ttlNumberInc ), WS_CHILD | WS_VISIBLE | SS_CENTER,
											  ttlNumberLabels[ttlNumberInc].sPos, parent, id++ );
	}
	loc.y += 20;
	// all row numberLabels
	for ( auto row : DoRows::allRows )
	{
		ttlRowLabels[ int(row) ].sPos = { loc.x, loc.y + int(row) * 28, loc.x + 32, loc.y + ( int(row) + 1 ) * 28 };
		ttlRowLabels[ int(row) ].Create ( cstr ( DoRows::toStr(row) ), WS_CHILD | WS_VISIBLE | SS_CENTER,
									 ttlRowLabels[ int(row) ].sPos, parent, id++ );
	}
	// all push buttons
	UINT runningCount = 0;
	auto startX = loc.x + 32;
	for (auto row : DoRows::allRows )
	{
		loc.x = startX;
		for (UINT number = 0; number < outputs.numColumns; number++)
		{
			outputs ( number, row ).initialize ( loc, parent, TTL_ID_BEGIN + runningCount++, toolTips );
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


void DoSystem::handleTTLPress(int id)
{
	
	for ( auto& out : outputs )
	{		
		if ( out.getCheckID ( ) == id )
		{
			if ( holdStatus == false )
			{
				out.set (!out.getStatus ()); 
				core.ftdi_ForceOutput(out.getPosition().first, out.getPosition().second, !out.getStatus(), getCurrentStatus ());
			}
			else
			{
				out.setHoldStatus ( !out.holdStatus );
			}
			break;
		}
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


HBRUSH DoSystem::handleColorMessage(CWnd* window, CDC* cDC)
{
	int controlID = window->GetDlgCtrlID();
	for ( auto& out : outputs )
	{
		auto res = out.handleColorMessage ( controlID, window, cDC );
		if ( res != NULL )
		{
			return res;
		}
	}
	if (controlID >= ttlRowLabels.front().GetDlgCtrlID() && controlID <= ttlRowLabels.back().GetDlgCtrlID())
	{
		cDC->SetBkColor( _myRGBs["Static-Bkgd"]);
		cDC->SetTextColor( _myRGBs["Text"]);
		return *_myBrushes["Static-Bkgd"];
	}
	else if (controlID >= ttlNumberLabels.front().GetDlgCtrlID() && controlID <= ttlNumberLabels.back().GetDlgCtrlID())
	{
		cDC->SetBkColor( _myRGBs["Static-Bkgd"]);
		cDC->SetTextColor( _myRGBs["Text"]);
		return *_myBrushes["Static-Bkgd"];
	}
	else
	{
		return NULL;
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


void DoSystem::setName( DoRows::which row, UINT number, std::string name, cToolTips& toolTips, AuxiliaryWindow* master)
{
	if (name == "")
	{
		// no empty names allowed.
		return;
	}
	outputs ( number, row ).setName ( name, toolTips, master );
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

