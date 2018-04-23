#include "stdafx.h"

#include <sstream>
#include <unordered_map>
#include <bitset>
#include "nidaqmx2.h"

#include "DioSystem.h"
#include "constants.h"
#include "AuxiliaryWindow.h"
#include "Thrower.h"
#include "range.h"

// I don't use this because I manually import dll functions.
// #include "Dio64.h"
DioSystem::DioSystem( bool ftSafemode, bool serialSafemode ) : 	ftFlume( ftSafemode ), 	winSerial( serialSafemode )
{
	connectType = ftdiConnectionOption::None;
	for ( auto& row : ttlStatus )
	{
		for ( auto& elem : row )
		{
			elem = 0;
		}
	}
}


void DioSystem::ftdi_connectasync( const char devSerial[] )
{
	if ( ftFlume.getNumDevices( ) <= 0 )
	{
		thrower( "No devices found." );
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
				thrower( "ERROR: bad value for dwNumberOfBytesWritten: " + str( bytesWritten ) );
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
		thrower( "No connection to close..." );
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
		auto& buf = loadSkip ? finFtdiBuffers_loadSkip[seqNum][variation] : finFtdiBuffers[seqNum][variation];
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
				unsigned long dwNumberOfBytesWritten;
				auto bytesWritten = winSerial.writeFile( dwNumberOfBytesSent, buf.pts );
				if ( bytesWritten > 0 )
				{
					++totalBytes;
				}
				else
				{
					thrower( "ERROR: bad value for dwNumberOfBytesWritten: " + str( bytesWritten ) );
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
		thrower( "No ftdi connection exists! Can't write without a connection." );
	}
	return 0;
}


void DioSystem::fillFtdiDataBuffer( std::vector<unsigned char>& dataBuffer, UINT offset, UINT count, ftdiPt pt )
{
	if ( offset + 20 >= dataBuffer.size( ) )
	{
		thrower( "ERROR: tried to write data buffer out of bounds!" );
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
	for ( int ttlRowInc = 0; ttlRowInc < getNumberOfTTLRows( ); ttlRowInc++ )
	{
		for ( int ttlNumberInc = 0; ttlNumberInc < getNumberOfTTLsPerRow( ); ttlNumberInc++ )
		{
			newFile << 0 << " ";
		}
		newFile << "\n";
	}
	newFile << "END_TTLS\n";
}


void DioSystem::handleSaveConfig(std::ofstream& saveFile)
{
	/// ttl settings
	saveFile << "TTLS\n";
	for (int ttlRowInc = 0; ttlRowInc < getNumberOfTTLRows(); ttlRowInc++)
	{
		for (int ttlNumberInc = 0; ttlNumberInc < getNumberOfTTLsPerRow(); ttlNumberInc++)
		{
			saveFile << getTtlStatus(ttlRowInc, ttlNumberInc) << " ";
		}
		saveFile << "\n";
	}
	saveFile << "END_TTLS\n";
}


void DioSystem::handleOpenConfig(std::ifstream& openFile, int versionMajor, int versionMinor )
{
	ProfileSystem::checkDelimiterLine(openFile, "TTLS");
	std::vector<std::vector<bool>> ttlStates;
	ttlStates.resize(getTtlBoardSize().first);
	UINT rowInc = 0;
	for (auto& row : ttlStates)
	{
		UINT colInc = 0;
		row.resize(getTtlBoardSize().second);
		for (auto& ttl : row)
		{
			std::string ttlString;
			openFile >> ttlString;
			try
			{
				ttl = std::stoi(ttlString);
				forceTtl(rowInc, colInc, ttl);
				updatePush( rowInc, colInc );
			}
			catch (std::invalid_argument&)
			{
				thrower("ERROR: the ttl status of \"" + ttlString + "\"failed to convert to a bool!");
			}
			colInc++;
		}
		rowInc++;
	}
	ProfileSystem::checkDelimiterLine(openFile, "END_TTLS");
}


ULONG DioSystem::countDacTriggers(UINT variation, UINT seqNum)
{
	ULONG triggerCount = 0;
	// D14
	std::pair<unsigned short, unsigned short> dacLine = { 3,15 };
	for (auto command : ttlCommandList[seqNum][variation])
	{
		// line each rising edge.
		if (command.line == dacLine && command.value == true)
		{
			triggerCount++;
		}
	}
	return triggerCount;
}


std::array< std::array<bool, 16>, 4 > DioSystem::getFinalSnapshot()
{
	return ttlSnapshots.back().back().back().ttlStatus;
}


void DioSystem::setTtlStatusNoForceOut(std::array< std::array<bool, 16>, 4 > status)
{
	ttlStatus = status;
	for (UINT rowInc = 0; rowInc < ttlStatus.size(); rowInc++)
	{
		for (UINT numberInc = 0; numberInc < ttlStatus[0].size(); numberInc++)
		{
			if (ttlStatus[rowInc][numberInc])
			{
				ttlPushControls[rowInc][numberInc].SetCheck(BST_CHECKED);
			}
			else
			{
				ttlPushControls[rowInc][numberInc].SetCheck(BST_UNCHECKED);
			}
			ttlPushControls[rowInc][numberInc].colorState = 0;
			ttlPushControls[rowInc][numberInc].RedrawWindow();
		}
	}
}




std::string DioSystem::getSystemInfo()
{
	DWORD answer = 1000;
	std::string info = "TTL System Info:\nInput Mode: ";
	vp_flume.dioGetAttr( 0, 0, answer);
	switch ( answer )
	{
		case 1100:
			// didn't change from start; no board or system connected.
			info += "no answer?\n";
			break;
			//return "";
		case 0:
			info += "Polled\n";
			break;
		case 1:
			info += "Interrupt\n";
			break;
		case 2:
			info += "Packet\n";
			break;
		case 3:
			info += "Demand\n";
			break;
		default:
			info += "UNKNOWN!\n";
	}
	vp_flume.dioGetAttr( 0, 1, answer );
	info += "Output Mode: ";
	switch ( answer )
	{
		case 1000:
			info += "no answer?\n";
			break;
		case 0:
			info += "Polled\n";
			break;
		case 1:
			info += "Interrupt\n";
			break;
		case 2:
			info += "Packet\n";
			break;
		case 3:
			info += "Demand\n";
			break;
		default:
			info += "UNKNOWN!\n";
	}
	vp_flume.dioGetAttr( 0, 2, answer );
	if (answer == 1000)
	{
		info += "Input Buffer Size: no answer?\n";
	}
	info += "Input Buffer Size: " + str( answer ) + "\n";
	vp_flume.dioGetAttr( 0, 3, answer );
	if (answer == 1000)
	{
		info += "Output Buffer Size: no answer?\n";
	}
	info += "Output Buffer Size: " + str( answer ) + "\n";
	vp_flume.dioGetAttr( 0, 4, answer );
	info += "Major Clock Source: ";
	switch ( answer )
	{
		case 1000:
			info += "no answer?\n";
			break;
		case 0:
			info += "Local 40 MHz Clock\n";
			break;
		case 1:
			info += "External Clock\n";
			break;
		case 2:
			info += "RTSI Clock / PXI chassis Clock\n";
			break;
		case 3:
			info += "10 MHz Clock\n";
			break;
		default:
			info += "UNKNOWN!";
	}
	// this is just a sampling... many more exist.
	return info;
}

std::array<std::array<std::string, 16>, 4> DioSystem::getAllNames()
{
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
		// shade it.
		ttlPushControls[row][col].SetCheck(BST_INDETERMINATE);
		ttlShadeStatus[row][col] = true;
		// a grey color is then used.
		ttlPushControls[row][col].colorState = 2;
		ttlPushControls[row][col].RedrawWindow();
	}
	for (auto& row : ttlPushControls)
	{
		for (auto& ctrl : row)
		{
			ctrl.EnableWindow(0);
		}
	}
}


void DioSystem::unshadeTtls()
{
	for (int rowInc = 0; rowInc < getNumberOfTTLRows(); rowInc++)
	{
		for (int numberInc = 0; numberInc < getNumberOfTTLsPerRow(); numberInc++)
		{
			auto& control = ttlPushControls[rowInc][numberInc];
			ttlShadeStatus[rowInc][numberInc] = false;
			if (control.colorState == 2)
			{
				control.colorState = 0;
				control.RedrawWindow();
			}
			if (ttlStatus[rowInc][numberInc])
			{
				control.SetCheck(BST_CHECKED);
			}
			else
			{
				control.SetCheck(BST_UNCHECKED);
			}
			control.EnableWindow();
		}
	}
}

void DioSystem::rearrange(UINT width, UINT height, fontMap fonts)
{
	ttlTitle.rearrange( width, height, fonts);
	ttlHold.rearrange( width, height, fonts);
	zeroTtls.rearrange( width, height, fonts);
	for (auto& row : ttlPushControls)
	{
		for (auto& control : row)
		{
			control.rearrange( width, height, fonts);
		}
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


void DioSystem::updatePush( UINT row, UINT number )
{
	ttlPushControls[row][number].SetCheck( ttlStatus[row][number] );
}


void DioSystem::handleInvert()
{
	for (UINT row = 0; row < ttlStatus.size(); row++)
	{
		for (UINT number = 0; number < ttlStatus[row].size(); number++)
		{
			if (ttlStatus[row][number])
			{
				forceTtl(row, number, 0);
				updatePush( row, number );
			}
			else
			{
				forceTtl(row, number, 1);
				updatePush( row, number );
			}
		}
	}

}


void DioSystem::updateDefaultTtl(UINT row, UINT column, bool state)
{
	defaultTtlState[row][column] = state;
}


bool DioSystem::getDefaultTtl(UINT row, UINT column)
{
	return defaultTtlState[row][column];
}


std::pair<UINT, UINT> DioSystem::getTtlBoardSize()
{
	if (ttlPushControls.size() == 0)
	{
		thrower("ttl push control is not 2D...");
	}
	return { ttlPushControls.size(), ttlPushControls.front().size() };
}


void DioSystem::initialize( POINT& loc, cToolTips& toolTips, AuxiliaryWindow* master, int& id, rgbMap rgbs )
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
	for (long row = 0; row < long(ttlPushControls.size()); row++)
	{
		ttlRowLabels[row].sPos = { loc.x, loc.y + row * 28, loc.x + 32, loc.y + (row + 1) * 28 };

		std::string rowName;
		switch (row)
		{
			case 0:
				rowName = "A";
				break;
			case 1:
				rowName = "B";
				break;
			case 2:
				rowName = "C";
				break;
			case 3:
				rowName = "D";
				break;
		}
		ttlRowLabels[row].Create( cstr(rowName), WS_CHILD | WS_VISIBLE | SS_CENTER,
								  ttlRowLabels[row].sPos, master, id++ );
	}
	// all push buttons
	UINT runningCount = 0;
	for (UINT row = 0; row < ttlPushControls.size(); row++)
	{
		for (UINT number = 0; number < ttlPushControls[row].size(); number++)
		{
			std::string name;
			switch (row)
			{
				case 0:
					name = "A";
					break;
				case 1:
					name = "B";
					break;
				case 2:
					name = "C";
					break;
				case 3:
					name = "D";
					break;
			}
			name += str( number );

			ttlPushControls[row][number].sPos = { long( loc.x + 32 + number * 28 ), long( loc.y + row * 28 ),
											long( loc.x + 32 + (number + 1) * 28 ), long( loc.y + (row + 1) * 28 ) };
			ttlPushControls[row][number].Create( "", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_RIGHT | BS_3STATE,
												 ttlPushControls[row][number].sPos, master, 
												 TTL_ID_BEGIN + runningCount++ );
			ttlPushControls[row][number].setToolTip(ttlNames[row][number], toolTips, master);
		}
	}
	loc.y += 28 * 4;
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
		thrower("ERROR: the name " + name + " is not the name of a ttl!");
	}
	timeType pulseEndTime = time;
	UINT row, collumn;
	int ttlLine = getNameIdentifier(name, row, collumn);
	ttlShadeLocations.push_back({ row, collumn });
	if (command == "on:")
	{
		ttlOn(row, collumn, time, seqNum );
	}
	else if (command == "off:")
	{
		ttlOff(row, collumn, time, seqNum );
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
			ttlOn( row, collumn, time, seqNum );
			ttlOff( row, collumn, pulseEndTime, seqNum );
		}
		if (command == "pulseoff:")
		{
			ttlOff( row, collumn, time, seqNum );
			ttlOn( row, collumn, pulseEndTime, seqNum );
		}
	}
}

// simple...
int DioSystem::getNumberOfTTLRows()
{
	return ttlPushControls.size();
}

int DioSystem::getNumberOfTTLsPerRow()
{
	if (ttlPushControls.size() > 0)
	{
		return ttlPushControls[0].size();
	}
	else
	{
		// shouldn't happen. always have ttls. 
		return -1;
	}
}

void DioSystem::handleTTLPress(int id)
{
	if (id >= ttlPushControls.front().front().GetDlgCtrlID() && id <= ttlPushControls.back().back().GetDlgCtrlID())
	{
		// figure out row #
		int row = (id - ttlPushControls.front().front().GetDlgCtrlID()) / ttlPushControls[0].size();
		// figure out collumn #
		int number = (id - ttlPushControls.front().front().GetDlgCtrlID()) % ttlPushControls[0].size();
		// if indeterminante, you can't change it, but that's fine, return true.
		if (ttlShadeStatus[row][number])
		{
			return;
		}
		if (holdStatus == false)
		{
			forceTtl( row, number, !ttlStatus[row][number] );
			updatePush( row, number );
		}
		else
		{
			if (ttlHoldStatus[row][number])
			{
				ttlHoldStatus[row][number] = false;
				ttlPushControls[row][number].colorState = -1;
				ttlPushControls[row][number].RedrawWindow();
			}
			else
			{
				ttlHoldStatus[row][number] = true;
				ttlPushControls[row][number].colorState = 1;
				ttlPushControls[row][number].RedrawWindow();
			}
		}
	}
}

// this function handles when the hold button is pressed.
void DioSystem::handleHoldPress()
{
	if (holdStatus == true)
	{
		holdStatus = false;
		// make changes
		for (UINT rowInc = 0; rowInc < ttlHoldStatus.size(); rowInc++)
		{
			for (UINT numberInc = 0; numberInc < ttlHoldStatus[0].size(); numberInc++)
			{
				ttlStatus[rowInc][numberInc] = ttlHoldStatus[rowInc][numberInc];
				// actually change the ttl.
				forceTtl( rowInc, numberInc, ttlHoldStatus[rowInc][numberInc] );
				updatePush( rowInc, numberInc );
				ttlPushControls[rowInc][numberInc].colorState = 0;
				ttlPushControls[rowInc][numberInc].RedrawWindow();
			}
		}
	}
	else
	{
		holdStatus = true;
		ttlHoldStatus = ttlStatus;
	}
}

template<class T> using vec = std::vector<T>;

// prepares some structures for a simple force event. 
void DioSystem::prepareForce( )
{
	ttlCommandFormList.resize( 1 );
	ttlSnapshots = vec<vec<vec<DioSnapshot>>>(1, vec<vec<DioSnapshot>>(1));
	loadSkipTtlSnapshots = vec<vec<vec<DioSnapshot>>>( 1, vec<vec<DioSnapshot>>( 1 ) );
	ttlCommandList = vec<vec<vec<DioCommand>>>(1, vec<vec<DioCommand>>( 1) );
	formattedTtlSnapshots = vec<vec<vec<std::array<WORD, 6>>>>(1, vec<vec<std::array<WORD, 6>>>(1) );
	loadSkipFormattedTtlSnapshots = vec<vec<vec<std::array<WORD, 6>>>>( 1, vec<vec<std::array<WORD, 6>>>( 1 ) );
	finalFormatViewpointData = vec<vec<vec<WORD>>>(1, vec<vec<WORD>>( 1 ) );
	loadSkipFinalFormatViewpointData = vec<vec<vec<WORD>>>( 1, vec<vec<WORD>>( 1 ) );
}


void DioSystem::initTtlObjs( UINT totalSequenceNumber )
{
	ttlCommandFormList.resize( totalSequenceNumber );
	ttlSnapshots.resize( totalSequenceNumber );
	ttlCommandList.resize( totalSequenceNumber );
	formattedTtlSnapshots.resize( totalSequenceNumber );
	finalFormatViewpointData.resize( totalSequenceNumber );
	loadSkipTtlSnapshots.resize( totalSequenceNumber );
	loadSkipFormattedTtlSnapshots.resize( totalSequenceNumber );
	loadSkipFinalFormatViewpointData.resize( totalSequenceNumber );
	ftdiSnaps.resize( totalSequenceNumber );
	ftdiSnaps_loadSkip.resize( totalSequenceNumber );
	finFtdiBuffers.resize( totalSequenceNumber );
	finFtdiBuffers_loadSkip.resize( totalSequenceNumber );
}


void DioSystem::resetTtlEvents( )
{
	ttlCommandFormList.clear( );	
	ttlSnapshots.clear( );
	ttlCommandList.clear( );
	formattedTtlSnapshots.clear( );
	finalFormatViewpointData.clear( );
	loadSkipTtlSnapshots.clear( );
	loadSkipFormattedTtlSnapshots.clear( );
	loadSkipFinalFormatViewpointData.clear( );
	ftdiSnaps.clear( );
	ftdiSnaps_loadSkip.clear( );
	finFtdiBuffers.clear( );
	finFtdiBuffers_loadSkip.clear( );
}


HBRUSH DioSystem::handleColorMessage(CWnd* window, brushMap brushes, rgbMap rGBs, CDC* cDC)
{
	int controlID = window->GetDlgCtrlID();
	if (controlID >= ttlPushControls.front().front().GetDlgCtrlID() && controlID <= ttlPushControls.back().back().GetDlgCtrlID())
	{
		// figure out row #
		int row = (controlID - ttlPushControls.front().front().GetDlgCtrlID()) / ttlPushControls[0].size();
		// figure out collumn #
		int number = (controlID - ttlPushControls.front().front().GetDlgCtrlID()) % ttlPushControls[0].size();
		if (ttlPushControls[row][number].colorState == -1)
		{
			cDC->SetBkColor(rGBs["Red"]);
			return *brushes["Red"];
		}
		else if (ttlPushControls[row][number].colorState == 1)
		{
			cDC->SetBkColor(rGBs["Green"]);
			return *brushes["Green"];
		}
		else if (ttlPushControls[row][number].colorState == 2)
		{
			cDC->SetBkColor(rGBs["White"]);
			return *brushes["White"];
		}
		else
		{
			cDC->SetBkColor(rGBs["Medium Grey"]);
			return *brushes["Medium Grey"];
		}
	}
	else if (controlID >= ttlRowLabels.front().GetDlgCtrlID() && controlID <= ttlRowLabels.back().GetDlgCtrlID())
	{
		cDC->SetBkColor(rGBs["Medium Grey"]);
		cDC->SetTextColor(rGBs["Solarized Base1"]);
		return *brushes["Medium Grey"];
	}
	else if (controlID >= ttlNumberLabels.front().GetDlgCtrlID() && controlID <= ttlNumberLabels.back().GetDlgCtrlID())
	{
		cDC->SetBkColor(rGBs["Medium Grey"]);
		cDC->SetTextColor(rGBs["Solarized Base1"]);
		return *brushes["Medium Grey"];
	}
	else
	{
		return NULL;
	}
}


bool DioSystem::isValidTTLName( std::string name )
{
	for (int rowInc = 0; rowInc < getNumberOfTTLRows(); rowInc++)
	{
		std::string rowStr;
		switch (rowInc)
		{
			case 0: rowStr = "a"; break;
			case 1: rowStr = "b"; break;
			case 2: rowStr = "c"; break;
			case 3: rowStr = "d"; break;
		}
		for (int numberInc = 0; numberInc < getNumberOfTTLsPerRow(); numberInc++)
		{
			// check default names
			UINT row, number;
			if (name == rowStr + str( numberInc))
			{
				return true;
			}
			else if (getNameIdentifier( name, row, number ) != -1)
			{
				return true;
			}
		}
	}
	return false;
}


vec<vec<vec<DioSnapshot>>> DioSystem::getSnapshots( )
{
	return ttlSnapshots;
}


vec<vec<std::array<ftdiPt, 2048>>> DioSystem::getFtdiSnaps( )
{
	return ftdiSnaps;
}


vec<vec<finBufInfo>> DioSystem::getFinalFtdiData( )
{
	return finFtdiBuffers;
}


void DioSystem::ttlOn(UINT row, UINT column, timeType time, UINT seqNum )
{
	// make sure it's either a variable or a number that can be used.
	ttlCommandFormList[seqNum].push_back({ {row, column}, time, true });
}


void DioSystem::ttlOff(UINT row, UINT column, timeType time, UINT seqNum)
{
	// check to make sure either variable or actual value.
	ttlCommandFormList[seqNum].push_back({ {row, column}, time, false });
}


void DioSystem::ttlOnDirect( UINT row, UINT column, double time, UINT variation, UINT seqInc )
{
	DioCommand command;
	command.line = { row, column };
	command.time = time;
	command.value = true;
	ttlCommandList[seqInc][variation].push_back( command );
}


void DioSystem::ttlOffDirect( UINT row, UINT column, double time, UINT variation, UINT seqInc )
{
	DioCommand command;
	command.line = { row, column };
	command.time = time;
	command.value = false;
	ttlCommandList[seqInc][variation].push_back( command );
}


void DioSystem::stopBoard()
{
	vp_flume.dioOutStop( 0 );
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

		if ( DIO_SAFEMODE )
		{
			thrower( "!" );
		}
	}
	catch ( Error& err )
	{
		//std::string msg = err.what( );
		//errBox( msg );
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
	return ttlStatus;
}

// forceTtl forces the actual ttl to a given value and changes the checkbox status to reflect that.
void DioSystem::forceTtl(int row, int number, int state)
{
	// change the ttl checkbox.
	ttlPushControls[row][number].SetCheck( state );
	ttlStatus[row][number] = state;
	// change the output.
	int result = 0;
	std::array<std::bitset<16>, 4> ttlBits;
	for (int rowInc = 0; rowInc < 4; rowInc++)
	{
		for (int numberInc = 0; numberInc < 16; numberInc++)
		{
			if (ttlStatus[rowInc][numberInc])
			{
				// flip bit to 1.
				ttlBits[rowInc].set( numberInc, true );
			}
			else
			{
				// flip bit to 0.
				ttlBits[rowInc].set( numberInc, false );
			}
		}
	}
	std::array<unsigned short, 4> tempCommand;
	tempCommand[0] = static_cast <unsigned short>(ttlBits[0].to_ulong());
	tempCommand[1] = static_cast <unsigned short>(ttlBits[1].to_ulong());
	tempCommand[2] = static_cast <unsigned short>(ttlBits[2].to_ulong());
	tempCommand[3] = static_cast <unsigned short>(ttlBits[3].to_ulong());
	vp_flume.dioForceOutput( 0, tempCommand.data(), 15 );
}


void DioSystem::setName(UINT row, UINT number, std::string name, cToolTips& toolTips, AuxiliaryWindow* master)
{
	if (name == "")
	{
		// no empty names allowed.
		return;
	}
	ttlNames[row][number] = str(name, 12, false, true);
	ttlPushControls[row][number].setToolTip(name, toolTips, master);
}


int DioSystem::getNameIdentifier(std::string name, UINT& row, UINT& number)
{
	
	for (UINT rowInc = 0; rowInc < ttlNames.size(); rowInc++)
	{
		std::string rowName;
		switch (rowInc)
		{
			case 0: rowName = "a"; break;
			case 1: rowName = "b"; break;
			case 2: rowName = "c"; break;
			case 3: rowName = "d"; break;
		}
		for (UINT numberInc = 0; numberInc < ttlNames[rowInc].size(); numberInc++)
		{
			// check the names array.
			std::transform( ttlNames[rowInc][numberInc].begin(), ttlNames[rowInc][numberInc].end(),
							ttlNames[rowInc][numberInc].begin(), ::tolower );
			if (ttlNames[rowInc][numberInc] == name)
			{
				row = rowInc;
				number = numberInc;
				return rowInc * ttlNames[rowInc].size() + numberInc;
			}
			// check standard names which are always acceptable.
			if (name == rowName + str(numberInc))
			{
				row = rowInc;
				number = numberInc;
				return rowInc * ttlNames[rowInc].size() + numberInc;
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
		vp_flume.dioOutWrite( 0, loadSkipFinalFormatViewpointData[seqNum][variation].data( ),
					 loadSkipFormattedTtlSnapshots[seqNum][variation].size( ), status );
	}
	else
	{
		vp_flume.dioOutWrite( 0, finalFormatViewpointData[seqNum][variation].data( ), formattedTtlSnapshots[seqNum][variation].size( ), status );
	}
}


std::string DioSystem::getName(UINT row, UINT number)
{
	return ttlNames[row][number];
}


ULONG DioSystem::getNumberEvents(UINT variation, UINT seqNum )
{
	return ttlSnapshots[variation].size();
}


bool DioSystem::getTtlStatus(int row, int number)
{
	return ttlStatus[row][number];
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
		totalTime = ( loadSkipFormattedTtlSnapshots[seqNum][variation].back( )[0]
					  + 65535 * loadSkipFormattedTtlSnapshots[seqNum][variation].back( )[1]) / 10000.0 + 1;
	}
	else 
	{
		totalTime = (formattedTtlSnapshots[seqNum][variation].back( )[0]
					  + 65535 * formattedTtlSnapshots[seqNum][variation].back( )[1]) / 10000.0 + 1;
	}
	 
	wait(totalTime);
}


double DioSystem::getFtdiTotalTime( UINT variation, UINT seqNum )
{
	double time = -1;
	for ( auto snap : ftdiSnaps[seqNum][variation] )
	{
		if ( snap == ftdiPt({0, 0, 0, 0, 0, 0, 0, 0, 0}) && time != -1 )
		{
			return time;
		}
		time = snap.time;
	}
	thrower( "ERROR: failed to find final time for dio system?!?!" );
}


double DioSystem::getTotalTime(UINT variation, UINT seqNum )
{
	// ??? there used to be a +1 at the end of this...
	return (formattedTtlSnapshots[seqNum][variation].back()[0]
			 + 65535 * formattedTtlSnapshots[seqNum][variation].back()[1]) / 10000.0;
}

// an "alias template". effectively a local using std::vector; declaration. makes these declarations much more
// readable. I very rarely use things like this.

template<class T> using vec = std::vector<T>;

void DioSystem::sizeDataStructures( UINT sequenceLength, UINT variations )
{
	/// imporantly, this sizes the relevant structures.
	ttlCommandList = vec<vec<vec<DioCommand>>>( sequenceLength, vec<vec<DioCommand>>( variations ) );
	ttlSnapshots = vec<vec<vec<DioSnapshot>>>( sequenceLength, vec<vec<DioSnapshot>>( variations ) );
	loadSkipTtlSnapshots = vec<vec<vec<DioSnapshot>>>( sequenceLength, vec<vec<DioSnapshot>>( variations ) );
	formattedTtlSnapshots = vec<vec<vec<std::array<WORD, 6>>>>( sequenceLength,
																vec<vec<std::array<WORD, 6>>>( variations ) );
	loadSkipFormattedTtlSnapshots = vec<vec<vec<std::array<WORD, 6>>>>( sequenceLength,
																		vec<vec<std::array<WORD, 6>>>( variations ) );
	finalFormatViewpointData = vec<vec<vec<WORD>>>( sequenceLength, vec<vec<WORD>>( variations ) );
	loadSkipFinalFormatViewpointData = vec<vec<vec<WORD>>>( sequenceLength, vec<vec<WORD>>( variations ) );

	ftdiSnaps = vec<vec<std::array<ftdiPt, 2048>>>( sequenceLength, vec<std::array<ftdiPt, 2048>>( variations ) );
	finFtdiBuffers = vec<vec<finBufInfo>>( sequenceLength, vec<finBufInfo>( variations ) );
	ftdiSnaps_loadSkip = vec<vec<std::array<ftdiPt, 2048>>>( sequenceLength, vec<std::array<ftdiPt, 2048>>( variations ) );
	finFtdiBuffers_loadSkip = vec<vec<finBufInfo>>( sequenceLength, vec<finBufInfo>( variations ) );
}


/*
 * Read key values from variables and convert command form to the final commands.
 */
void DioSystem::interpretKey( vec<vec<parameterType>>& variables )
{
	UINT sequenceLength = variables.size( );
	UINT variations = variables.front( ).size() == 0 ? 1 : variables.front().front( ).keyValues.size( );
	if (variations == 0)
	{
		variations = 1; 
	}
	sizeDataStructures( sequenceLength, variations );
	// and interpret the command list for each variation.
	for (auto seqInc : range( sequenceLength ) )
	{
		for (UINT variationNum = 0; variationNum < variations; variationNum++)
		{
			for (auto& formList : ttlCommandFormList[seqInc])
			{
				DioCommand tempCommand;
				tempCommand.line = formList.line;
				tempCommand.value = formList.value;
				double variableTime = 0;
				// add together current values for all variable times.
				if ( formList.time.first.size() != 0)
				{
					for (auto varTime : formList.time.first)
					{
						variableTime += varTime.evaluate(variables[seqInc], variationNum);
					}
				}
				tempCommand.time = variableTime + formList.time.second;
				ttlCommandList[seqInc][variationNum].push_back(tempCommand);
			}
		}
	}
}


vec<vec<vec<WORD>>> DioSystem::getFinalViewpointData( )
{
	return finalFormatViewpointData;
}


void DioSystem::organizeTtlCommands(UINT variation, UINT seqNum )
{
	// each element of this is a different time (the double), and associated with each time is a vector which locates 
	// which commands were on at this time, for ease of retrieving all of the values in a moment.
	std::vector<std::pair<double, std::vector<unsigned short>>> timeOrganizer;
	std::vector<DioCommand> orderedList(ttlCommandList[seqNum][variation]);
	// sort using a lambda. std::sort is effectively a quicksort algorithm.
	std::sort(orderedList.begin(), orderedList.end(), [](DioCommand a, DioCommand b) {return a.time < b.time; });
	/// organize all of the commands.
	for (USHORT commandInc = 0; commandInc < ttlCommandList[seqNum][variation].size(); commandInc++)
	{
		// because the events are sorted by time, the time organizer will already be sorted by time, and therefore I 
		// just need to check the back value's time.
		if (commandInc == 0 || fabs(orderedList[commandInc].time - timeOrganizer.back().first) > 2 * DBL_EPSILON)
		{
			// new time
			timeOrganizer.push_back({ orderedList[commandInc].time, std::vector<USHORT>({ commandInc }) });
		}
		else
		{
			// old time
			timeOrganizer.back().second.push_back(commandInc);
		}
	}
	/// now figure out the state of the system at each time.
	if (timeOrganizer.size() == 0)
	{
		thrower("ERROR: No ttl commands! The Ttl system is the master behind everything in a repetition, and so it "
				 "must contain something.\r\n");
	}
	auto& snaps = ttlSnapshots[seqNum][variation];
	snaps.clear();
	// start with the initial status.
	snaps.push_back({ 0, ttlStatus });
	if (timeOrganizer[0].first != 0)
	{
		// then there were no commands at time 0, so just set the initial state to be exactly the original state before
		// the experiment started. I don't need to modify the first snapshot in this case, it's already set. Add a snapshot
		// here so that the thing modified is the second snapshot.
		snaps.push_back({ 0, ttlStatus });
	}

	// handle the zero case specially. This may or may not be the literal first snapshot.
	snaps.back().time = timeOrganizer[0].first;
	for (UINT zeroInc = 0; zeroInc < timeOrganizer[0].second.size(); zeroInc++)
	{
		// make sure to address he correct ttl. the ttl location is located in individuaTTL_CommandList but you need 
		// to make sure you access the correct command.
		UINT cmdNum = timeOrganizer[0].second[zeroInc];
		UINT row = orderedList[cmdNum].line.first;
		UINT column = orderedList[cmdNum].line.second;
		snaps.back().ttlStatus[row][column]	= orderedList[cmdNum].value;
	}

	// already handled the first case.
	for (UINT commandInc = 1; commandInc < timeOrganizer.size(); commandInc++)
	{
		// first copy the last set so that things that weren't changed remain unchanged.
		snaps.push_back( snaps.back());
		//
		snaps.back().time = timeOrganizer[commandInc].first;
		for (auto cmdNum : timeOrganizer[commandInc].second)
		{
			// see description of this command above... update everything that changed at this time.
			UINT row = orderedList[cmdNum].line.first;
			UINT column = orderedList[cmdNum].line.second;
			snaps.back().ttlStatus[row][column] = orderedList[cmdNum].value;
		}
	}
	// phew. Check for good input by user:
	for (auto snapshot : snaps )
	{
		if (snapshot.time < 0)
		{
			thrower("ERROR: The code tried to set a ttl event at a negative time value! This is clearly not allowed."
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
	USHORT temp = time * 10000;
	hiwordTime = ULONGLONG( time * 10000 ) / 65535;
	if ( ULONGLONG( time * 10000 ) / 65535 > 65535 )
	{
		thrower( "ERROR: DIO system was asked to calculate at ime that was too long! this is limited by the card." );
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
		auto ttlSnaps = loadSkip ? loadSkipTtlSnapshots[seqNum][variation] : ttlSnapshots[seqNum][variation];
		auto& ftSnaps = loadSkip ? ftdiSnaps_loadSkip[seqNum][variation] : ftdiSnaps[seqNum][variation];
		for ( auto snapshot : ttlSnaps )
		{
			ftdiPt pt;
			fpgaBankCtr = 0;
			for ( auto bank : snapshot.ttlStatus )
			{
				// currently this is split an awkward because the viewpoint organization was organized in sets of 16, not 8.
				// convert first 8 of snap shot to int
				val1 = 0;
				for ( int i = 0; i < 8; i++ )
				{
					val1 = val1 + pow( 2, i )*bank[i];
				}
				// convert next 8 of snap shot to int
				val2 = 0;
				for ( int j = 0; j < 8; j++ )
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
		auto& snaps = loadSkip ? ftdiSnaps_loadSkip[seqNum][variation] : ftdiSnaps[seqNum][variation];
		// please note that Serial mode has not been thoroughly tested (by me, MOB at least)!
		auto& buf = loadSkip ? finFtdiBuffers_loadSkip[seqNum][variation] : finFtdiBuffers[seqNum][variation];
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
				thrower( "RC028.cpp: Non-Terminated table, data was filled all the way to end of data array... "
						 "Unit will not work right..., last element of data should be all zeros." );
			}
			number++;
			count++;
			buf.bytesToWrite += DIO_WRITESPERDATAPT * DIO_MSGLENGTH;
		}
	}
}


DWORD DioSystem::ftdi_ForceOutput( int row, int number, int state )
{
	// change the ttl checkbox.
	ttlPushControls[row][number].SetCheck( state );
	ttlStatus[row][number] = state;
	resetTtlEvents( );
	initTtlObjs( 1 );
	sizeDataStructures( 1, 1 );
	ttlSnapshots[0][0].push_back( { 0.1, ttlStatus } );
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
	auto& formattedSnaps = formattedTtlSnapshots[seqNum][variation];
	auto& loadSkipFormattedSnaps = loadSkipFormattedTtlSnapshots[seqNum][variation];
	auto& finalNormal = finalFormatViewpointData[seqNum][variation];
	auto& finalLoadSkip = loadSkipFinalFormatViewpointData[seqNum][variation];
	formattedSnaps.clear();
	loadSkipFormattedSnaps.clear( );
	finalNormal.clear( );
	finalLoadSkip.clear( );

	// do bit arithmetic.
	for ( auto& snapshot : ttlSnapshots[seqNum][variation])
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
		formattedTtlSnapshots[seqNum][variation].push_back(tempCommand);
	}
	// same loop with the loadSkipSnapshots.
	for ( auto& snapshot : loadSkipTtlSnapshots[seqNum][variation] )
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
		loadSkipFormattedTtlSnapshots[seqNum][variation].push_back( tempCommand );
	}

	/// flatten the data.
	finalFormatViewpointData[seqNum][variation].resize( formattedTtlSnapshots[seqNum][variation].size( ) * 6 );
	int count = 0;
	for ( auto& element : finalFormatViewpointData[seqNum][variation] )
	{
		// concatenate
		element = formattedTtlSnapshots[seqNum][variation][count / 6][count % 6];
		count++;
	}
	// the arrays are usually not the same length and need to be dealt with separately.
	loadSkipFinalFormatViewpointData[seqNum][variation].resize( loadSkipFormattedTtlSnapshots[seqNum][variation].size( ) * 6 );
	count = 0;
	for ( auto& element : loadSkipFinalFormatViewpointData[seqNum][variation] )
	{
		// concatenate
		element = loadSkipFormattedTtlSnapshots[seqNum][variation][count / 6][count % 6];
		count++;
	}
}


void DioSystem::findLoadSkipSnapshots( double time, std::vector<parameterType>& variables, UINT variation, UINT seqNum )
{
	// find the splitting time and set the loadSkip snapshots to have everything after that time.
	auto& snaps = ttlSnapshots[seqNum][variation];
	auto& loadSkipSnaps = loadSkipTtlSnapshots[seqNum][variation];
	for ( auto snapshotInc : range(ttlSnapshots[seqNum][variation].size() - 1) )
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
UINT DioSystem::countTriggers( UINT row, UINT number, UINT variation, UINT seqNum )
{
	auto& snaps = ttlSnapshots[seqNum][variation];
	UINT count = 0;
	if ( snaps.size( ) == 0 )
	{
		thrower( "ERROR: no ttl events in countTriggers?" );
	}
	for ( auto eventInc : range(ttlSnapshots[seqNum][variation].size()-1) )
	{
		if ( snaps[eventInc].ttlStatus[row][number] == false && snaps[eventInc+1].ttlStatus[row][number] == true )
		{
			count++;
		}
	}
	return count;
}


void DioSystem::checkNotTooManyTimes( UINT variation, UINT seqNum )
{
	if ( formattedTtlSnapshots[seqNum][variation].size( ) > 512 )
	{
		thrower( "ERROR: DIO Data has more than 512 individual timestamps, which is larger than the DIO64 FIFO Buffer"
				 ". The DIO64 card can only support 512 individual time-stamps. If you need more, you need to configure"
				 " this code to create a thread to continuously write to the DIO64 card as it outputs." );
	}
}


void DioSystem::checkFinalFormatTimes( UINT variation, UINT seqNum )
{
	// loop through all the commands and make sure that no two events have the same time-stamp. Was a common symptom
	// of a bug when code first created.
	for ( int dioEventInc = 0; dioEventInc < formattedTtlSnapshots[seqNum][variation].size( ); dioEventInc++ )
	{
		auto& snapOuter0 = formattedTtlSnapshots[seqNum][variation][dioEventInc][0];
		auto& snapOuter1 = formattedTtlSnapshots[seqNum][variation][dioEventInc][1];
		for ( int dioEventInc2 = 0; dioEventInc2 < dioEventInc; dioEventInc2++ )
		{
			auto& snapInner0 = formattedTtlSnapshots[seqNum][variation][dioEventInc2][0];
			auto& snapInner1 = formattedTtlSnapshots[seqNum][variation][dioEventInc2][1];
			if ( snapOuter0 == snapInner0 && snapOuter1 == snapInner1 )
			{
				thrower( "ERROR: Dio system somehow created two events with the same time stamp! This might be caused by"
						 " ttl events being spaced to close to each other." );
			}
		}
	}
}


void DioSystem::zeroBoard( )
{
	for ( UINT row = 0; row < ttlStatus.size( ); row++ )
	{
		for ( UINT number = 0; number < ttlStatus[row].size( ); number++ )
		{
			forceTtl( row, number, 0 );
			updatePush( row, number );
		}
	}
}


std::vector<std::vector<double>> DioSystem::getFinalTimes( )
{
	std::vector<std::vector<double>> finTimes(ttlSnapshots.size());
	UINT seqInc = 0;
	for ( auto& seqSnaps : ttlSnapshots )
	{
		UINT variationInc = 0;
		finTimes[seqInc].resize( seqSnaps.size( ) );
		for ( auto& variationSnaps : seqSnaps )
		{
			finTimes[seqInc][variationInc] = variationSnaps.back( ).time;
			variationInc++;
		}
		seqInc++;
	}
	return finTimes;
}


void DioSystem::fillPlotData( UINT variation, std::vector<std::vector<pPlotDataVec>> ttlData )
{
	std::string message;
	for ( auto& seqInfo : ttlSnapshots )
	{
		if ( seqInfo.size( ) <= variation )
		{
			thrower( "ERROR: Attempted to retrieve ttl data from variation " + str( variation ) + ", which does not "
					 "exist in the dio code object!" );
		}
	}
	// each element of ttlData should be one ttl line.
	UINT linesPerPlot = 64 / ttlData.size( );
	for ( auto line : range( 64 ) )
	{
		auto& data = ttlData[line / linesPerPlot][line % linesPerPlot];
		data->clear( );
		UINT runningSeqTime = 0;
		for ( auto& ttlSeqData : ttlSnapshots )
		{
			for ( auto& snap : ttlSeqData[variation] )
			{
				data->push_back( { runningSeqTime + snap.time, double( snap.ttlStatus[line / 16][line % 16] ), 0 } );
			}
			runningSeqTime += ttlSeqData[variation].back( ).time;
		}
	}
}


/// DIO64 Wrapper functions that I actually use
std::string DioSystem::getTtlSequenceMessage(UINT variation, UINT seqNum )
{
	std::string message;
	if ( ttlSnapshots[seqNum].size( ) <= variation )
	{
		thrower( "ERROR: Attempted to retrieve ttl sequence message from snapshot " + str( variation ) + ", which does not "
				 "exist!" );
	}
	for (auto snap : ttlSnapshots[seqNum][variation])
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
