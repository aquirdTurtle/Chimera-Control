// created by Mark O. Brown
#include "stdafx.h"
#include "Version.h"

#include "DioSystem.h"
#include "constants.h"
#include "AuxiliaryWindow.h"
#include "Thrower.h"
#include "range.h"

#include <sstream>
#include <unordered_map>
#include <bitset>
#include "nidaqmx2.h"
#include <boost/lexical_cast.hpp>


// I don't use this because I manually import dll functions.
// #include "Dio64.h"
DioSystem::DioSystem( bool ftSafemode, bool serialSafemode, bool viewpointSafemode ) : 	ftFlume( ftSafemode ), 	
winSerial( serialSafemode ),
vp_flume(viewpointSafemode)
{
	connectType = ftdiConnectionOption::None;
	for ( auto& out : outputs )
	{
		out.set ( 0 );
	}
}


void DioSystem::ftdi_connectasync( const char devSerial[] )
{
	if ( ftFlume.getNumDevices( ) <= 0 )
	{
		thrower ( "No devices found." );
	}
	ftFlume.open( devSerial );
	ftFlume.setUsbParams( );
	connectType = ftdiConnectionOption::Async;
}

/*
* is this a software trigger? is it the "start" command?
*/
DWORD DioSystem::ftdi_trigger( )
{
	std::vector<unsigned char> dataBuffer = { 161, 0, 0, 0, 0, 0, 1 };
	if ( connectType == ftdiConnectionOption::Serial )
	{
		unsigned long totalBytesSent = 0;
		while ( totalBytesSent < 7 )
		{
			auto bytesWritten = winSerial.writeFile( totalBytesSent, dataBuffer );
			if ( bytesWritten > 0 )
			{
				++totalBytesSent;
			}
			else
			{
				thrower ( "bad value for dwNumberOfBytesWritten: " + str( bytesWritten ) );
			}
		}
		return totalBytesSent;
	}
	else if ( connectType == ftdiConnectionOption::Async )
	{
		return ftFlume.write( dataBuffer );
	}
	return 0;
}


void DioSystem::ftdi_disconnect( )
{
	if ( connectType == ftdiConnectionOption::Serial )
	{
		winSerial.close( );
	}
	else if ( connectType == ftdiConnectionOption::Async )
	{
		ftFlume.close( );
	}
	else
	{
		thrower ( "No connection to close..." );
	}
	connectType = ftdiConnectionOption::None;
}


/*
* Takes data from "mem" structure and writes to the dio board.
*/
DWORD DioSystem::ftdi_write( UINT seqNum, UINT variation, bool loadSkip )
{
	if ( connectType == ftdiConnectionOption::Serial || connectType == ftdiConnectionOption::Async )
	{
		auto& buf = loadSkip ? finFtdiBuffers_loadSkip(seqNum,variation) : finFtdiBuffers(seqNum,variation);
		// please note that Serial mode has not been thoroughly tested (by me, MOB at least)!
		bool proceed = true;
		int count = 0;
		int idx = 0;
		unsigned int totalBytes = 0;
		unsigned int number = 0;
		unsigned long dwNumberOfBytesSent = 0;
		if ( connectType == ftdiConnectionOption::Serial )
		{
			while ( dwNumberOfBytesSent < buf.bytesToWrite )
			{
				auto bytesWritten = winSerial.writeFile( dwNumberOfBytesSent, buf.pts );
				if ( bytesWritten > 0 )
				{
					++totalBytes;
				}
				else
				{
					thrower ( "bad value for dwNumberOfBytesWritten: " + str( bytesWritten ) );
				}
			}
			totalBytes += dwNumberOfBytesSent;
		}
		else
		{
			totalBytes += ftFlume.write( buf.pts, buf.bytesToWrite );
		}
		return totalBytes;
	}
	else
	{
		thrower ( "No ftdi connection exists! Can't write without a connection." );
	}
	return 0;
}


void DioSystem::fillFtdiDataBuffer( std::vector<unsigned char>& dataBuffer, UINT offset, UINT count, ftdiPt pt )
{
	if ( offset + 20 >= dataBuffer.size( ) )
	{
		thrower ( "tried to write data buffer out of bounds!" );
	}
	dataBuffer[offset] = WBWRITE;
	dataBuffer[offset + 1] = ((TIMEOFFS + count) >> 8) & 0xFF;
	dataBuffer[offset + 2] = (TIMEOFFS + count) & 0xFF;
	dataBuffer[offset + 3] = ((pt.time) >> 24) & 0xFF;
	dataBuffer[offset + 4] = ((pt.time) >> 16) & 0xFF;
	dataBuffer[offset + 5] = ((pt.time) >> 8) & 0xFF;
	dataBuffer[offset + 6] = (pt.time) & 0xFF;

	dataBuffer[offset + 7] = WBWRITE;
	dataBuffer[offset + 8] = ((BANKAOFFS + count) >> 8) & 0xFF;
	dataBuffer[offset + 9] = (BANKAOFFS + count) & 0xFF;
	dataBuffer[offset + 10] = pt.pts[0];
	dataBuffer[offset + 11] = pt.pts[1];
	dataBuffer[offset + 12] = pt.pts[2];
	dataBuffer[offset + 13] = pt.pts[3];

	dataBuffer[offset + 14] = WBWRITE;
	dataBuffer[offset + 15] = ((BANKBOFFS + count) >> 8) & 0xFF;
	dataBuffer[offset + 16] = (BANKBOFFS + count) & 0xFF;
	dataBuffer[offset + 17] = pt.pts[4];
	dataBuffer[offset + 18] = pt.pts[5];
	dataBuffer[offset + 19] = pt.pts[6];
	dataBuffer[offset + 20] = pt.pts[7];
}


void DioSystem::handleNewConfig( std::ofstream& newFile )
{
	newFile << "TTLS\n";
	// nothing at the moment.
	newFile << "END_TTLS\n";
}


void DioSystem::handleSaveConfig(std::ofstream& saveFile)
{
	/// ttl settings
	saveFile << "TTLS\n";
	// nothing at the moment.
	saveFile << "END_TTLS\n";
}


void DioSystem::handleOpenConfig(std::ifstream& openFile, Version ver )
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


std::array< std::array<bool, 16>, 4 > DioSystem::getFinalSnapshot()
{
	auto numSeq = ttlSnapshots.getNumSequences ( );
	if ( numSeq > 0 )
	{
		auto numVar = ttlSnapshots.getNumVariations ( numSeq-1 );
		if ( numVar > 0 )
		{
			if ( ttlSnapshots ( numSeq - 1, numVar - 1 ).size ( ) > 0 )
			{
				return ttlSnapshots ( numSeq - 1, numVar - 1 ).back ( ).ttlStatus;
			}
		}
	}
	thrower ( "Attempted to get final snapshot from dio system but no snapshots!" );
}


void DioSystem::setTtlStatusNoForceOut(std::array< std::array<bool, 16>, 4 > status)
{
	for ( auto rowInc : range(status.size()) )
	{
		for ( auto numInc : range(status[rowInc].size()) )
		{
			outputs ( numInc, DioRows::which ( rowInc ) ).set ( status[ rowInc ][ numInc ] );
		}
	}
}




std::string DioSystem::getSystemInfo()
{
	DWORD answer = 1000;
	std::string info = "Input Mode: ";
	vp_flume.dioGetAttr( 0, 0, answer);
	switch ( answer )
	{
		case 1100:
			// didn't change from start; no board or system connected.
			info += "no answer?";
			break;
			//return "";
		case 0:
			info += "Polled";
			break;
		case 1:
			info += "Interrupt";
			break;
		case 2:
			info += "Packet";
			break;
		case 3:
			info += "Demand";
			break;
		default:
			info += "UNKNOWN!";
	}
	vp_flume.dioGetAttr( 0, 1, answer );
	info += "\n\tOutput Mode: ";
	switch ( answer )
	{
		case 1000:
			info += "no answer?";
			break;
		case 0:
			info += "Polled";
			break;
		case 1:
			info += "Interrupt";
			break;
		case 2:
			info += "Packet";
			break;
		case 3:
			info += "Demand";
			break;
		default:
			info += "UNKNOWN!";
	}
	vp_flume.dioGetAttr( 0, 2, answer );
	if (answer == 1000)
	{
		info += "\n\tInput Buffer Size: no answer?";
	}
	info += "\n\tInput Buffer Size: " + str( answer );
	vp_flume.dioGetAttr( 0, 3, answer );
	if (answer == 1000)
	{
		info += "\n\tOutput Buffer Size: no answer?";
	}
	info += "\n\tOutput Buffer Size: " + str( answer );
	vp_flume.dioGetAttr( 0, 4, answer );
	info += "\n\tMajor Clock Source: ";
	switch ( answer )
	{
		case 1000:
			info += "no answer?";
			break;
		case 0:
			info += "Local 40 MHz Clock";
			break;
		case 1:
			info += "External Clock";
			break;
		case 2:
			info += "RTSI Clock / PXI chassis Clock";
			break;
		case 3:
			info += "10 MHz Clock";
			break;
		default:
			info += "UNKNOWN!";
	}
	// this is just a sampling... many more exist.
	return info;
}

std::array<std::array<std::string, 16>, 4> DioSystem::getAllNames()
{
	std::array<std::array<std::string, 16>, 4> ttlNames;
	for ( auto& out : outputs )
	{
		ttlNames[ int ( out.getPosition ( ).first ) ][ out.getPosition ( ).second ] = out.getName ( );
	}
	return ttlNames;
}


void DioSystem::startBoard()
{
	DIO64STAT status;
	DWORD scansAvailable;
	vp_flume.dioOutStatus( 0, scansAvailable, status );
	// start the dio board!
	vp_flume.dioOutStart( 0 );
}


void DioSystem::shadeTTLs(std::vector<std::pair<UINT, UINT>> shadeList)
{
	for (UINT shadeInc = 0; shadeInc < shadeList.size(); shadeInc++)
	{
		auto& row = shadeList[shadeInc].first;
		auto& col = shadeList[shadeInc].second;
		outputs ( col, DioRows::which ( row ) ).shade ( true );
	}
	for (auto& out : outputs)
	{
		out.enable ( 0 );
	}
}


void DioSystem::unshadeTtls()
{
	for ( auto& out : outputs )
	{
		out.shade ( false );
	}
}

void DioSystem::rearrange(UINT width, UINT height, fontMap fonts)
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


void DioSystem::updatePush( DioRows::which row, UINT number )
{
	outputs ( number, row ).updateStatus ( );
}


void DioSystem::handleInvert()
{
	for ( auto& out : outputs )
	{
		forceTtl ( out.getPosition ( ).first, out.getPosition ( ).second, !out.getStatus ( ) );
		out.updateStatus ( );
	}
}


void DioSystem::updateDefaultTtl( DioRows::which row, UINT column, bool state)
{
	outputs ( column, row ).defaultStatus = state;
}


bool DioSystem::getDefaultTtl( DioRows::which row, UINT column)
{
	return outputs ( column, row ).defaultStatus;
}


std::pair<UINT, UINT> DioSystem::getTtlBoardSize()
{
	return { outputs.numRows, outputs.numColumns };
}


void DioSystem::initialize( POINT& loc, cToolTips& toolTips, AuxiliaryWindow* master, int& id )
{
	// title
	ttlTitle.sPos = { loc.x, loc.y, loc.x + 480, loc.y + 25 };
	ttlTitle.Create( "TTLS", WS_CHILD | WS_VISIBLE | SS_CENTER, ttlTitle.sPos, master, id++ );
	ttlTitle.fontType = fontTypes::HeadingFont;
	// all number numberLabels
	loc.y += 25;
	ttlHold.sPos = { loc.x, loc.y, loc.x + 240, loc.y + 20 };
	ttlHold.Create( "Hold Current Values", WS_TABSTOP | WS_VISIBLE | BS_AUTOCHECKBOX | WS_CHILD | BS_PUSHLIKE,
					ttlHold.sPos, master, TTL_HOLD );
	ttlHold.setToolTip("Press this button to change multiple TTLs simultaneously. Press the button, then change the "
					   "ttls, then press the button again to release it. Upon releasing the button, the TTLs will "
					   "change.", toolTips, master);

	zeroTtls.sPos = { loc.x + 240, loc.y, loc.x + 480, loc.y + 20 };
	zeroTtls.Create( "Zero TTLs", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, zeroTtls.sPos, master, 
					 IDC_ZERO_TTLS );
	zeroTtls.setToolTip( "Pres this button to set all ttls to their zero (false) state.", toolTips, master );
	loc.y += 20;

	for (long ttlNumberInc = 0; ttlNumberInc < long(ttlNumberLabels.size()); ttlNumberInc++)
	{
		ttlNumberLabels[ttlNumberInc].sPos = { loc.x + 32 + ttlNumberInc * 28, loc.y,
			loc.x + 32 + (ttlNumberInc + 1) * 28, loc.y + 20 };
		ttlNumberLabels[ttlNumberInc].Create( cstr( ttlNumberInc ), WS_CHILD | WS_VISIBLE | SS_CENTER,
											  ttlNumberLabels[ttlNumberInc].sPos, master, id++ );
	}
	loc.y += 20;
	// all row numberLabels
	for ( auto row : DioRows::allRows )
	{
		ttlRowLabels[ int(row) ].sPos = { loc.x, loc.y + int(row) * 28, loc.x + 32, loc.y + ( int(row) + 1 ) * 28 };
		ttlRowLabels[ int(row) ].Create ( cstr ( DioRows::toStr(row) ), WS_CHILD | WS_VISIBLE | SS_CENTER,
									 ttlRowLabels[ int(row) ].sPos, master, id++ );
	}
	// all push buttons
	UINT runningCount = 0;
	auto startX = loc.x + 32;
	for (auto row : DioRows::allRows )
	{
		loc.x = startX;
		for (UINT number = 0; number < outputs.numColumns; number++)
		{
			outputs ( number, row ).initialize ( loc, master, TTL_ID_BEGIN + runningCount++, toolTips );
			loc.x += 28;
		}
		loc.y += 28;
	}
	loc.x = startX - 32;
}


void DioSystem::handleTtlScriptCommand( std::string command, timeType time, std::string name,
										std::vector<std::pair<UINT, UINT>>& ttlShadeLocations, 
										std::vector<parameterType>& vars, UINT seqNum, std::string scope )
{
	// use an empty expression.
	handleTtlScriptCommand( command, time, name, Expression(), ttlShadeLocations, vars, seqNum, scope );
}


void DioSystem::handleTtlScriptCommand( std::string command, timeType time, std::string name, Expression pulseLength, 
									    std::vector<std::pair<UINT, UINT>>& ttlShadeLocations, 
										std::vector<parameterType>& vars, UINT seqNum, std::string scope )
{
	if (!isValidTTLName(name))
	{
		thrower ("the name " + name + " is not the name of a ttl!");
	}
	timeType pulseEndTime = time;
	UINT collumn;
	DioRows::which row;
	getNameIdentifier(name, row, collumn);
	ttlShadeLocations.push_back({ int(row), collumn });
	if (command == "on:")
	{
		ttlOn(int(row), collumn, time, seqNum );
	}
	else if (command == "off:")
	{
		ttlOff(int(row), collumn, time, seqNum );
	}
	else if (command == "pulseon:" || command == "pulseoff:")
	{
		try
		{
			pulseEndTime.second += pulseLength.evaluate();
		}
		catch (Error&)
		{
			pulseLength.assertValid( vars, scope );
			pulseEndTime.first.push_back(pulseLength);
		}
		if (command == "pulseon:")
		{
			ttlOn( int(row), collumn, time, seqNum );
			ttlOff( int(row), collumn, pulseEndTime, seqNum );
		}
		if (command == "pulseoff:")
		{
			ttlOff( int(row), collumn, time, seqNum );
			ttlOn( int(row), collumn, pulseEndTime, seqNum );
		}
	}
}


void DioSystem::standardNonExperimentStartDioSequence( )
{
	organizeTtlCommands( 0, 0 );
	convertToFinalViewpointFormat( 0, 0 );
	writeTtlData( 0, false, 0 );
	startBoard( );
	waitTillFinished( 0, false, 0 );
}


int DioSystem::getNumberOfTTLRows()
{
	return outputs.numRows;
}


int DioSystem::getNumberOfTTLsPerRow()
{
	return outputs.numColumns;
}


void DioSystem::handleTTLPress(int id)
{
	for ( auto& out : outputs )
	{
		if ( out.getCheckID ( ) == id )
		{
			if ( out.getShadeStatus() )
			{
				// if indeterminante (i.e. shaded), you can't change it, but that's fine, just return.
				return;
			}
			if ( holdStatus == false )
			{
				forceTtl ( out.getPosition().first, out.getPosition ( ).second, !out.getStatus() );
				out.updateStatus ( );
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
void DioSystem::handleHoldPress()
{
	if (holdStatus == true)
	{
		// set all the holds.
		holdStatus = false;
		// make changes
		for ( auto& out : outputs )
		{
			out.set ( out.holdStatus );
			forceTtl ( out.getPosition ( ).first, out.getPosition ( ).second, out.getStatus ( ) );
			out.updateStatus ( );
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

template<class T> using vec = std::vector<T>;

// prepares some structures for a simple force event. 
void DioSystem::prepareForce( )
{
	initializeDataObjects( 1, 1 );
}


void DioSystem::initializeDataObjects( UINT seqNum, UINT variationNum )
{
	ttlCommandFormList.resize( seqNum );
	ttlCommandList.uniformSizeReset ( seqNum, variationNum );
	ttlSnapshots.uniformSizeReset ( seqNum, variationNum );
	loadSkipTtlSnapshots.uniformSizeReset ( seqNum, variationNum );
	formattedTtlSnapshots.uniformSizeReset ( seqNum, variationNum );
	loadSkipFormattedTtlSnapshots.uniformSizeReset ( seqNum, variationNum );
	finalFormatViewpointData.uniformSizeReset ( seqNum, variationNum );
	loadSkipFinalFormatViewpointData.uniformSizeReset ( seqNum, variationNum );
	ftdiSnaps.uniformSizeReset ( seqNum, variationNum );
	ftdiSnaps_loadSkip.uniformSizeReset ( seqNum, variationNum );
	finFtdiBuffers.uniformSizeReset ( seqNum, variationNum );
	finFtdiBuffers_loadSkip.uniformSizeReset ( seqNum, variationNum );
}


void DioSystem::resetTtlEvents( )
{
	initializeDataObjects( 0, 0 );
}


HBRUSH DioSystem::handleColorMessage(CWnd* window, CDC* cDC)
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


bool DioSystem::isValidTTLName( std::string name )
{
	DioRows::which row;
	UINT number;
	return getNameIdentifier ( name, row, number ) != -1;
}


void DioSystem::ttlOn(UINT row, UINT column, timeType time, UINT seqNum )
{
	ttlCommandFormList[ seqNum ].push_back ( { {row, column}, time, {}, true } );
}


void DioSystem::ttlOff(UINT row, UINT column, timeType time, UINT seqNum)
{
	ttlCommandFormList[ seqNum ].push_back ( { {row, column}, time, {}, false } );
}


void DioSystem::ttlOnDirect( UINT row, UINT column, double timev, UINT variation, UINT seqInc, UINT totalVariations )
{
	DioCommand command;
	command.line = { row, column };
	//command.timeVals.resize ( totalVariations );
	//command.timeVals[ variation ] = time;
	command.time = timev;
	command.value = true;
	ttlCommandList(seqInc, variation).push_back( command );
}


void DioSystem::ttlOffDirect( UINT row, UINT column, double timev, UINT variation, UINT seqInc, UINT totalVariations )
{
	DioCommand command;
	command.line = { row, column };
	command.time = timev;
	command.value = false;
	ttlCommandList(seqInc, variation).push_back( command );
}


void DioSystem::stopBoard()
{
	vp_flume.dioOutStop( 0 );
}

bool DioSystem::getViewpointSafemode ( )
{
	return vp_flume.getSafemodeSetting ( );
}


double DioSystem::getClockStatus()
{
	// initialize to zero so that in safemode goes directly to getting tick line.
	int result = 0;
	DIO64STAT stat;
	DWORD availableScans;
	try
	{
		vp_flume.dioOutStatus( 0, availableScans, stat );

		if ( vp_flume.getSafemodeSetting ( ) )
		{
			thrower ( "!" );
		}
	}
	catch ( Error&)
	{
		// get current time in ms...
		// ***NOT SURE*** if this is what I want. The vb6 code used...
		// return = Now * 24 * 60 * 60 * 1000
		return GetTickCount();
	}
	double timeInSeconds = stat.time[0] + stat.time[1] * 65535;
	return timeInSeconds / 10000.0;
	// assuming the clock runs at 10 MHz, return in ms.
}

std::array< std::array<bool, 16>, 4 > DioSystem::getCurrentStatus( )
{
	std::array< std::array<bool, 16>, 4 > currentStatus;
	for ( auto& out : outputs )
	{
		currentStatus[ int ( out.getPosition ( ).first ) ][ out.getPosition ( ).second ] = out.getStatus();
	}
	return currentStatus;
}

// forceTtl forces the actual ttl to a given value and changes the checkbox status to reflect that.
void DioSystem::forceTtl( DioRows::which row, int number, bool state)
{
	outputs ( number, row ).set ( state );
	// change the output.
	int result = 0;
	std::array<std::bitset<16>, 4> ttlBits;
	for ( auto& out : outputs )
	{
		ttlBits[int(out.getPosition().first)].set(out.getPosition().second, out.getStatus ( ));
	}
	std::array<unsigned short, 4> tempCommand;
	tempCommand[0] = static_cast <unsigned short>(ttlBits[0].to_ulong());
	tempCommand[1] = static_cast <unsigned short>(ttlBits[1].to_ulong());
	tempCommand[2] = static_cast <unsigned short>(ttlBits[2].to_ulong());
	tempCommand[3] = static_cast <unsigned short>(ttlBits[3].to_ulong());
	vp_flume.dioForceOutput( 0, tempCommand.data(), 15 );
}


void DioSystem::setName( DioRows::which row, UINT number, std::string name, cToolTips& toolTips, AuxiliaryWindow* master)
{
	if (name == "")
	{
		// no empty names allowed.
		return;
	}
	outputs ( number, row ).setName ( name, toolTips, master );
}


/*
Returns a single number which corresponds to the dio control with the name 
*/
int DioSystem::getNameIdentifier(std::string name, DioRows::which& row, UINT& number)
{
	for ( auto rowInc : DioRows::allRows )
	{
		for (auto numberInc : range( outputs.numColumns ) )
		{
			std::string DioName = str(outputs ( numberInc, rowInc ).getName ( ), 13, false, true);
			// second of the || is standard name which is always acceptable.
			if ( DioName == name || name == DioRows::toStr ( rowInc ) + str ( numberInc ) )
			{
				row = rowInc;
				number = numberInc;
				return int( rowInc ) * outputs.numColumns + numberInc;
			}
		}
	}
	// else not a name.
	return -1;
}


void DioSystem::writeTtlData(UINT variation, UINT seqNum, bool loadSkip)
{
	// all 64 outputs are used, so every bit in this should be 1. 
	WORD outputMask[4] = { WORD_MAX, WORD_MAX, WORD_MAX, WORD_MAX };
	double scanRate = 10000000;
	DWORD availableScans = 0;
	DIO64STAT status;
	status.AIControl = 0;
	try
	{
		vp_flume.dioOutStop( 0 );
	}
	catch ( Error& ) { /* if fails it probably just wasn't running before */ } 

	vp_flume.dioOutConfig( 0, 0, outputMask, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, scanRate );
	vp_flume.dioOutStatus( 0, availableScans, status );
	if ( loadSkip )
	{
		vp_flume.dioOutWrite( 0, loadSkipFinalFormatViewpointData(seqNum,variation).data( ),
					 loadSkipFormattedTtlSnapshots(seqNum,variation).size( ), status );
	}
	else
	{
		vp_flume.dioOutWrite( 0, finalFormatViewpointData(seqNum,variation).data( ), formattedTtlSnapshots(seqNum,variation).size( ), status );
	}
}


std::string DioSystem::getName( DioRows::which row, UINT number)
{
	return outputs(number, row).getName();
}


ULONG DioSystem::getNumberEvents(UINT variation, UINT seqNum )
{
	return ttlSnapshots(seqNum,variation).size();
}


bool DioSystem::getTtlStatus(DioRows::which row, int number)
{
	return outputs ( number, row ).getStatus ( );
}


// waits a time in ms, using the DIO clock
void DioSystem::wait(double time)
{
	double startTime;
	//clockstatus function reads the DIO clock, units are ms
	startTime = getClockStatus();
	//errBox( "start time = " + str( startTime ) );
	while (time - abs(getClockStatus() - startTime) > 110 /*&& getClockStatus() - startTime*/ != 0)
	{
		Sleep(100);
	}
	// check faster closer to the stop.
	while (time - abs(getClockStatus() - startTime) > 0.1)
	{
		/*immediately check again*/
	}
	double finTime = getClockStatus();
}


// uses the last time of the ttl trigger to wait until the experiment is finished.
void DioSystem::waitTillFinished(UINT variation, UINT seqNum, bool skipOption)
{
	double totalTime;
	if ( skipOption )
	{
		totalTime = ( loadSkipFormattedTtlSnapshots(seqNum,variation).back( )[0]
					  + 65535 * loadSkipFormattedTtlSnapshots(seqNum,variation).back( )[1]) / 10000.0 + 1;
	}
	else 
	{
		totalTime = (formattedTtlSnapshots(seqNum,variation).back( )[0]
					  + 65535 * formattedTtlSnapshots(seqNum,variation).back( )[1]) / 10000.0 + 1;
	}
	 
	wait(totalTime);
}


double DioSystem::getFtdiTotalTime( UINT variation, UINT seqNum )
{
	double time = -1;
	for ( auto snap : ftdiSnaps(seqNum,variation) )
	{
		if ( snap == ftdiPt({0, 0, 0, 0, 0, 0, 0, 0, 0}) && time != -1 )
		{
			return time;
		}
		time = snap.time;
	}
	thrower ( "failed to find final time for dio system?!?!" );
}


double DioSystem::getTotalTime(UINT variation, UINT seqNum )
{
	// ??? there used to be a +1 at the end of this...
	return (formattedTtlSnapshots(seqNum,variation).back()[0]
			 + 65535 * formattedTtlSnapshots(seqNum,variation).back()[1]) / 10000.0;
}

// an "alias template". effectively a local using std::vector; declaration. makes these declarations much more
// readable. I very rarely use things like this.

template<class T> using vec = std::vector<T>;

void DioSystem::sizeDataStructures( UINT sequenceLength, UINT variations )
{
	/// imporantly, this sizes the relevant structures.
	ttlSnapshots.uniformSizeReset ( sequenceLength, variations );
	loadSkipTtlSnapshots.uniformSizeReset ( sequenceLength, variations );
	formattedTtlSnapshots.uniformSizeReset ( sequenceLength, variations );
	loadSkipFormattedTtlSnapshots.uniformSizeReset ( sequenceLength, variations );
	finalFormatViewpointData.uniformSizeReset ( sequenceLength, variations );
	loadSkipFinalFormatViewpointData.uniformSizeReset ( sequenceLength, variations );

	ftdiSnaps.uniformSizeReset ( sequenceLength, variations );
	finFtdiBuffers.uniformSizeReset ( sequenceLength, variations );
	ftdiSnaps_loadSkip.uniformSizeReset ( sequenceLength, variations );
	finFtdiBuffers_loadSkip.uniformSizeReset ( sequenceLength, variations );
}

/*
PLAN:
interpret key gives me effectively a 3D list of all the FIXED dio commands. then I add commands to it after using the 
AO system, but these commands should only be added to the relevant variation / sequence instead of making a ton of
redundant commands at time t=0 (which I think were causing issues with the number of trigs...) So I need to break the 
list of dio commands out of the original structure which is [seq][cmd][variation] so that I can do it 
as [seq][variation][cmd].
*/


void DioSystem::restructureCommands ( )
{
	/* this is to be done after key interpretation. */
	ttlCommandFormList;
	ttlCommandList;
	ttlCommandList.resizeSeq ( ttlCommandFormList.size ( ) );
	for ( auto seqInc : range ( ttlCommandFormList.size ( ) ) )
	{
		if ( ttlCommandFormList[ seqInc ].size ( ) == 0 )
		{
			thrower ( "No TTL Commands???" );
		}
		ttlCommandList.resizeVariations ( seqInc, ttlCommandFormList[seqInc][0].timeVals.size());
		for ( auto varInc : range(ttlCommandList.getNumVariations ( seqInc ) ) )
		{
			for ( auto& cmd : ttlCommandFormList[ seqInc ] )
			{
				DioCommand nCmd;
				nCmd.line = cmd.line;
				nCmd.time = cmd.timeVals[ varInc ];
				nCmd.value = cmd.value;
				ttlCommandList ( seqInc, varInc ).push_back ( nCmd );
			}
		}
	}
}


/*
 * Read key values from variables and convert command form to the final commands.
 */
void DioSystem::interpretKey( vec<vec<parameterType>>& params )
{
	UINT sequenceLength = params.size( );
	UINT variations = params.front( ).size() == 0 ? 1 : params.front().front( ).keyValues.size( );
	if (variations == 0)
	{
		variations = 1;
	}
	sizeDataStructures( sequenceLength, variations );
	// and interpret the command list for each variation.
	
	for ( auto seqInc : range ( sequenceLength ) )
	{
		for ( auto& dioCommandForm : ttlCommandFormList[ seqInc ] )
		{
			dioCommandForm.timeVals.resize ( variations );
		}
		for (auto variationNum : range(variations))
		{
			for (auto& dioCommandForm : ttlCommandFormList[seqInc])
			{
				double variableTime = 0;
				// add together current values for all variable times.
				if ( dioCommandForm.time.first.size() != 0)
				{
					for (auto varTime : dioCommandForm.time.first)
					{
						variableTime += varTime.evaluate(params[seqInc], variationNum);
					}
				}
				dioCommandForm.timeVals[variationNum] = variableTime + dioCommandForm.time.second;
			}
		}
	}
}


allDigitalOutputs& DioSystem::getDigitalOutputs ( )
{
	return outputs;
}

ExpWrap<vec<DioSnapshot>> DioSystem::getTtlSnapshots ( )
{
	/* used in the unit testing suite */
	return ttlSnapshots;
}

ExpWrap<finBufInfo> DioSystem::getFinalFtdiData ( )
{
	return finFtdiBuffers;
}


ExpWrap<std::array<ftdiPt, 2048>> DioSystem::getFtdiSnaps ( )
{
	return ftdiSnaps;
}

ExpWrap<vec<WORD>> DioSystem::getFinalViewpointData ( )
{
	return finalFormatViewpointData;
}


void DioSystem::organizeTtlCommands(UINT variation, UINT seqNum )
{
	// each element of this is a different time (the double), and associated with each time is a vector which locates 
	// which commands were on at this time, for ease of retrieving all of the values in a moment.
	std::vector<std::pair<double, std::vector<unsigned short>>> timeOrganizer;
	std::vector<DioCommand> orderedCommandList ( ttlCommandList( seqNum , variation) );
	// sort using a lambda. std::sort is effectively a quicksort algorithm.
	std::sort(orderedCommandList.begin(), orderedCommandList.end(), 
			   [variation](DioCommand a, DioCommand b) {return a.time < b.time; });
	/// organize all of the commands.
	for (auto commandInc : range( ttlCommandList(seqNum, variation).size ( ) ) )
	{
		// because the events are sorted by time, the time organizer will already be sorted by time, and therefore I 
		// just need to check the back value's time. DIO64 uses a 10MHz clock, can do 100ns spacing, check diff 
		// threshold to extra room. If dt<1ns, probably just some floating point issue. 
		// If 1ns<dt<100ns I want to actually complain to the user since it seems likely that  this was intentional and 
		// not a floating error.
		if (commandInc == 0 || fabs(orderedCommandList[commandInc].time - timeOrganizer.back().first) > 1e-6)
		{
			// new time
			std::vector<USHORT> testVec =  { USHORT(commandInc) };
			timeOrganizer.push_back({ orderedCommandList[commandInc].time, testVec });
		}
		else
		{
			// old time
			timeOrganizer.back().second.push_back(commandInc);
		}
	}
	if (timeOrganizer.size() == 0)
	{
		thrower ("No ttl commands! The Ttl system is the master behind everything in a repetition, and so it "
				 "must contain something.\r\n");
	}
	/// now figure out the state of the system at each time.
	auto& snaps = ttlSnapshots(seqNum,variation);
	snaps.clear();
	// start with the initial status.
	snaps.push_back({ 0, getCurrentStatus() });
	if (timeOrganizer[0].first != 0)
	{
		// then there were no commands at time 0, so just set the initial state to be exactly the original state before
		// the experiment started. I don't need to modify the first snapshot in this case, it's already set. Add a snapshot
		// here so that the thing modified is the second snapshot.
		snaps.push_back({ 0, getCurrentStatus ( ) });
	}

	// handle the zero case specially. This may or may not be the literal first snapshot.
	snaps.back().time = timeOrganizer[0].first;
	for (auto zeroInc : range( timeOrganizer[ 0 ].second.size ( ) ) )
	{
		// make sure to address he correct ttl. the ttl location is located in individuaTTL_CommandList but you need 
		// to make sure you access the correct command.
		UINT cmdNum = timeOrganizer[0].second[zeroInc];
		UINT row = orderedCommandList[cmdNum].line.first;
		UINT column = orderedCommandList[cmdNum].line.second;
		snaps.back().ttlStatus[row][column]	= orderedCommandList[cmdNum].value;
	}

	// already handled the first case.
	for (UINT commandInc = 1; commandInc < timeOrganizer.size(); commandInc++)
	{
		// first copy the last set so that things that weren't changed remain unchanged.
		snaps.push_back( snaps.back());
		snaps.back().time = timeOrganizer[commandInc].first;
		for (auto cmdNum : timeOrganizer[commandInc].second)
		{
			// see description of this command above... update everything that changed at this time.
			UINT row = orderedCommandList[cmdNum].line.first;
			UINT column = orderedCommandList[cmdNum].line.second;
			snaps.back().ttlStatus[row][column] = orderedCommandList[cmdNum].value;
		}
	}
	// phew. Check for good input by user:
	for (auto snapshot : snaps )
	{
		if (snapshot.time < 0)
		{
			thrower ("The code tried to set a ttl event at a negative time value! This is clearly not allowed."
					" Aborting.");
		}
	}
}


std::pair<USHORT, USHORT> DioSystem::calcDoubleShortTime( double time )
{
	USHORT lowordTime, hiwordTime;
	// convert to system clock ticks. Assume that the crate is running on a 10 MHz signal, so multiply by
	// 10,000,000, but then my time is in milliseconds, so divide that by 1,000, ending with multiply by 10,000
	lowordTime = ULONGLONG( time * 10000 ) % 65535;
	hiwordTime = ULONGLONG( time * 10000 ) / 65535;
	if ( ULONGLONG( time * 10000 ) / 65535 > 65535 )
	{
		thrower ( "DIO system was asked to calculate at ime that was too long! this is limited by the card." );
	}
	return { lowordTime, hiwordTime };
}


void DioSystem::convertToFtdiSnaps( UINT variation, UINT seqNum )
{
	// formatting of these snaps is similar to the word formatting of the viewpoint dio64 card; the ttl on/off 
	int snapIndex = 0;
	int val1, val2, fpgaBankCtr;
	ULONG timeConv = 100000;
	for ( auto loadSkip : { false, true } )
	{
		auto ttlSnaps = loadSkip ? loadSkipTtlSnapshots(seqNum,variation) : ttlSnapshots(seqNum,variation);
		auto& ftSnaps = loadSkip ? ftdiSnaps_loadSkip(seqNum,variation) : ftdiSnaps(seqNum,variation);
		for ( auto snapshot : ttlSnaps )
		{
			ftdiPt pt;
			fpgaBankCtr = 0;
			for ( auto bank : snapshot.ttlStatus )
			{
				// currently this is split an awkward because the viewpoint organization was organized in sets of 16, not 8.
				// convert first 8 of snap shot to int
				val1 = 0;
				for (auto i : range(8) )
				{
					val1 = val1 + pow( 2, i )*bank[i];
				}
				// convert next 8 of snap shot to int
				val2 = 0;
				for (auto j : range(8))
				{
					val2 = val2 + pow( 2, j )*bank[j + 8];
				}
				pt.pts[fpgaBankCtr++] = val1;
				pt.pts[fpgaBankCtr++] = val2;
			}
			pt.time = snapshot.time * timeConv;
			ftSnaps[snapIndex] = pt;
			snapIndex++;
		}
		ftSnaps[snapIndex] = { 0,0,0,0,0,0,0,0,0 };
	}
}


void DioSystem::convertToFinalFtdiFormat( UINT variation, UINT seqNum )
{
	for ( auto loadSkip : { false, true } )
	{
		// first convert from diosnapshot to ftdi snapshot
		auto& snaps = loadSkip ? ftdiSnaps_loadSkip(seqNum,variation) : ftdiSnaps(seqNum,variation);
		auto& buf = loadSkip ? finFtdiBuffers_loadSkip(seqNum,variation) : finFtdiBuffers(seqNum,variation);
		// please note that Serial mode has not been thoroughly tested (by me, MOB at least)!
		buf.pts = std::vector<unsigned char>( (connectType == ftdiConnectionOption::Serial ?
								DIO_BUFFERSIZESER : DIO_BUFFERSIZEASYNC) * DIO_MSGLENGTH * DIO_WRITESPERDATAPT, 0 );
		bool proceed = true;
		int count = 0;
		unsigned int totalBytes = 0;
		buf.bytesToWrite = 0;
		unsigned int number = 0;
		while ( (number < DIO_BUFFERSIZESER) && proceed )
		{
			UINT offset = DIO_WRITESPERDATAPT * number * DIO_MSGLENGTH;
			fillFtdiDataBuffer( buf.pts, offset, count, snaps[count] );
			if ( snaps[count] == ftdiPt( { 0,0,0,0,0,0,0,0,0 } ) && number != 0 )
			{
				proceed = false;
			}
			if ( count == NUMPOINTS )
			{
				thrower ( "RC028.cpp: Non-Terminated table, data was filled all the way to end of data array... "
						 "Unit will not work right..., last element of data should be all zeros." );
			}
			number++;
			count++;
			buf.bytesToWrite += DIO_WRITESPERDATAPT * DIO_MSGLENGTH;
		}
	}
}


DWORD DioSystem::ftdi_ForceOutput( DioRows::which row, int number, int state )
{
	outputs ( number, row ).set ( state );
	resetTtlEvents( );
	initializeDataObjects( 1, 0 );
	sizeDataStructures( 1, 1 );
	ttlSnapshots(0,0).push_back( { 0.1, getCurrentStatus ( ) } );
	convertToFtdiSnaps( 0, 0 );
	convertToFinalFtdiFormat( 0, 0 );	
	ftdi_connectasync( "FT1VAHJPB" );
	auto bytesWritten = ftdi_write( 0, 0, false);
	ftdi_trigger( );
	ftdi_disconnect( );
	return bytesWritten;
}



void DioSystem::convertToFinalViewpointFormat(UINT variation, UINT seqNum )
{
	// excessive but just in case.
	auto& formattedSnaps = formattedTtlSnapshots(seqNum,variation);
	auto& loadSkipFormattedSnaps = loadSkipFormattedTtlSnapshots(seqNum,variation);
	auto& finalNormal = finalFormatViewpointData(seqNum,variation);
	auto& finalLoadSkip = loadSkipFinalFormatViewpointData(seqNum,variation);
	formattedSnaps.clear();
	loadSkipFormattedSnaps.clear( );
	finalNormal.clear( );
	finalLoadSkip.clear( );

	// do bit arithmetic.
	for ( auto& snapshot : ttlSnapshots(seqNum,variation))
	{
		// each major index is a row (A, B, C, D), each minor index is a ttl state (0, 1) in that row.
		std::array<std::bitset<16>, 4> ttlBits;
		for (UINT rowInc : range( 4 ) )
		{
			for (UINT numberInc : range( 16 ) )
			{
				ttlBits[rowInc].set( numberInc, snapshot.ttlStatus[rowInc][numberInc] );
			}
		}
		// I need to put it as an int (just because I'm not actually sure how the bitset gets stored... it'd probably 
		// work just passing the address of the bitsets, but I'm sure this will work so whatever.)
		std::array<USHORT, 6> tempCommand;
		tempCommand[0] = calcDoubleShortTime( snapshot.time ).first;
		tempCommand[1] = calcDoubleShortTime( snapshot.time ).second;
		tempCommand[2] = static_cast <unsigned short>(ttlBits[0].to_ulong());
		tempCommand[3] = static_cast <unsigned short>(ttlBits[1].to_ulong());
		tempCommand[4] = static_cast <unsigned short>(ttlBits[2].to_ulong());
		tempCommand[5] = static_cast <unsigned short>(ttlBits[3].to_ulong());
		formattedTtlSnapshots(seqNum,variation).push_back(tempCommand);
	}
	// same loop with the loadSkipSnapshots.
	for ( auto& snapshot : loadSkipTtlSnapshots(seqNum,variation) )
	{
		// each major index is a row (A, B, C, D), each minor index is a ttl state (0, 1) in that row.
		std::array<std::bitset<16>, 4> ttlBits;
		for ( UINT rowInc : range( 4 ) )
		{
			for ( UINT numberInc : range( 16 ) )
			{
				ttlBits[rowInc].set( numberInc, snapshot.ttlStatus[rowInc][numberInc] );
			}
		}
		// I need to put it as an int (just because I'm not actually sure how the bitset gets stored... it'd probably 
		// work just passing the address of the bitsets, but I'm sure this will work so whatever.)
		std::array<USHORT, 6> tempCommand;
		tempCommand[0] = calcDoubleShortTime( snapshot.time ).first;
		tempCommand[1] = calcDoubleShortTime( snapshot.time ).second;
		tempCommand[2] = static_cast <unsigned short>(ttlBits[0].to_ulong( ));
		tempCommand[3] = static_cast <unsigned short>(ttlBits[1].to_ulong( ));
		tempCommand[4] = static_cast <unsigned short>(ttlBits[2].to_ulong( ));
		tempCommand[5] = static_cast <unsigned short>(ttlBits[3].to_ulong( ));
		loadSkipFormattedTtlSnapshots(seqNum,variation).push_back( tempCommand );
	}

	/// flatten the data.
	finalFormatViewpointData(seqNum,variation).resize( formattedTtlSnapshots(seqNum,variation).size( ) * 6 );
	int count = 0;
	for ( auto& element : finalFormatViewpointData(seqNum,variation) )
	{
		// concatenate
		element = formattedTtlSnapshots(seqNum,variation)[count / 6][count % 6];
		count++;
	}
	// the arrays are usually not the same length and need to be dealt with separately.
	loadSkipFinalFormatViewpointData(seqNum,variation).resize( loadSkipFormattedTtlSnapshots(seqNum,variation).size( ) * 6 );
	count = 0;
	for ( auto& element : loadSkipFinalFormatViewpointData(seqNum,variation) )
	{
		// concatenate
		element = loadSkipFormattedTtlSnapshots(seqNum,variation)[count / 6][count % 6];
		count++;
	}
}


void DioSystem::findLoadSkipSnapshots( double time, std::vector<parameterType>& variables, UINT variation, UINT seqNum )
{
	// find the splitting time and set the loadSkip snapshots to have everything after that time.
	auto& snaps = ttlSnapshots(seqNum,variation);
	auto& loadSkipSnaps = loadSkipTtlSnapshots(seqNum, variation);
	for ( auto snapshotInc : range(ttlSnapshots(seqNum, variation).size() - 1) )
	{
		if ( snaps[snapshotInc].time < time && snaps[snapshotInc+1].time >= time )
		{
			loadSkipSnaps = std::vector<DioSnapshot>( snaps.begin( ) + snapshotInc + 1, snaps.end( ) );
			break;
		}
	}
	// need to zero the times.
	for ( auto& snapshot : loadSkipSnaps )
	{
		snapshot.time -= time;
	}
}


// counts the number of triggers on a given line.
// which.first = row, which.second = number.
UINT DioSystem::countTriggers( std::pair<DioRows::which, UINT> which, UINT variation, UINT seqNum )
{
	auto& snaps = ttlSnapshots(seqNum, variation);
	UINT count = 0;
	if ( snaps.size( ) == 0 )
	{
		return 0;
		//thrower ( "No ttl events to examine in countTriggers?" );
	}
	for ( auto snapshotInc : range(ttlSnapshots(seqNum,variation).size()-1) )
	{
		// count each rising edge. Also count if the first snapshot is high. 
		if (	(snaps[snapshotInc].ttlStatus[int(which.first)][which.second] == false 
			 &&  snaps[snapshotInc+1].ttlStatus[int(which.first)][which.second] == true) 
			 || (snaps[ snapshotInc ].ttlStatus[ int ( which.first ) ][ which.second ] == true 
			 && snapshotInc == 0))
		{
			count++;
		}
	}
	return count;
}


void DioSystem::checkNotTooManyTimes( UINT variation, UINT seqNum )
{
	if ( formattedTtlSnapshots(seqNum,variation).size( ) > 512 )
	{
		thrower ( "DIO Data has more than 512 individual timestamps, which is larger than the DIO64 FIFO Buffer"
				 ". The DIO64 card can only support 512 individual time-stamps. If you need more, you need to configure"
				 " this code to create a thread to continuously write to the DIO64 card as it outputs." );
	}
}


void DioSystem::checkFinalFormatTimes( UINT variation, UINT seqNum )
{
	// loop through all the commands and make sure that no two events have the same time-stamp. Was a common symptom
	// of a bug when code first created.
	for (UINT dioEventInc : range( formattedTtlSnapshots(seqNum , variation ).size ( ) ) )
	{
		auto& snapOuter0 = formattedTtlSnapshots(seqNum,variation)[dioEventInc][0];
		auto& snapOuter1 = formattedTtlSnapshots(seqNum,variation)[dioEventInc][1];
		for (UINT dioEventInc2 : range( dioEventInc ) )
		{
			auto& snapInner0 = formattedTtlSnapshots(seqNum,variation)[dioEventInc2][0];
			auto& snapInner1 = formattedTtlSnapshots(seqNum,variation)[dioEventInc2][1];
			if ( snapOuter0 == snapInner0 && snapOuter1 == snapInner1 )
			{
				thrower ( "Dio system somehow created two events with the same time stamp! This might be caused by"
						 " ttl events being spaced to close to each other." );
			}
		}
	}
}

void DioSystem::ftdiZeroBoard ( )
{
	for ( auto& out : outputs )
	{
		ftdi_ForceOutput ( out.getPosition ( ).first, out.getPosition ( ).second, 0 );
		out.updateStatus ( );
	}
}


void DioSystem::zeroBoard( )
{
	for ( auto& out : outputs )
	{
		forceTtl ( out.getPosition ( ).first, out.getPosition ( ).second, 0 );
		out.updateStatus ( );
	}
}


std::vector<std::vector<double>> DioSystem::getFinalTimes( )
{
	std::vector<std::vector<double>> finTimes(ttlSnapshots.getNumSequences());
	for (auto seqNum : range(ttlSnapshots.getNumSequences()))
	{
		finTimes[seqNum].resize( ttlSnapshots.getNumVariations(seqNum) );
		for ( auto varNum : range(ttlSnapshots.getNumVariations ( seqNum ) ) )
		{
			finTimes[ seqNum ][ varNum ] = ttlSnapshots(seqNum, varNum).back ( ).time;
		}
	}
	return finTimes;
}


void DioSystem::fillPlotData( UINT variation, std::vector<std::vector<pPlotDataVec>> ttlData )
{
	std::string message;
	for ( auto seqNum : range(ttlSnapshots.getNumSequences()) )
	{
		if ( ttlSnapshots.getNumVariations(seqNum) <= variation )
		{
			thrower ( "Attempted to retrieve ttl data from variation " + str( variation ) + ", which does not "
					 "exist in the dio code object!" );
		}
	}
	// each element of ttlData should be one ttl line.
	UINT linesPerPlot = 64 / ttlData.size( );
	for ( auto line : range( 64 ) )
	{
		auto& data = ttlData[line / linesPerPlot][line % linesPerPlot];
		data->clear( );
		double runningSeqTime = 0;
		for ( auto seqNum : range(ttlSnapshots.getNumSequences()))
		{
			for ( auto& snap : ttlSnapshots(seqNum, variation) )
			{
				data->push_back( { runningSeqTime + snap.time, double( snap.ttlStatus[line / 16][line % 16] ), 0 } );
			}
			runningSeqTime += ttlSnapshots(seqNum, variation).back( ).time;
		}
	}
}


/// DIO64 Wrapper functions that I actually use
std::string DioSystem::getTtlSequenceMessage(UINT variation, UINT seqNum )
{
	std::string message;
	
	if ( ttlSnapshots.getNumVariations ( seqNum ) <= variation )
	{
		thrower ( "Attempted to retrieve ttl sequence message from snapshot " + str( variation ) + ", which does not "
				 "exist!" );
	}
	for (auto snap : ttlSnapshots(seqNum, variation))
	{
		message += str(snap.time) + ":\n";
		int rowInc = 0;
		for (auto row : snap.ttlStatus)
		{
			switch (rowInc)
			{
				case 0:
					message += "A: ";
					break;
				case 1:
					message += "B: ";
					break;
				case 2:
					message += "C: ";
					break;
				case 3:
					message += "D: ";
					break;
			}
			rowInc++;
			for (auto num : row)
			{
				message += str(num) + ", ";
			}
			message += "\r\n";
		}
		message += "\r\n---\r\n";
	}
	return message;
}
