#include "stdafx.h"

#include <sstream>
#include <unordered_map>
#include <bitset>
#include "nidaqmx2.h"

#include "DioSystem.h"
#include "constants.h"
#include "AuxiliaryWindow.h"

// I don't use this because I manually import dll functions.
// #include "Dio64.h"


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


void DioSystem::handleOpenConfig(std::ifstream& openFile, double version)
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


ULONG DioSystem::countDacTriggers(UINT variation)
{
	ULONG triggerCount = 0;
	// D14
	std::pair<unsigned short, unsigned short> dacLine = { 3,15 };
	for (auto command : ttlCommandList[variation])
	{
		// count each rising edge.
		if (command.line == dacLine && command.value == true)
		{
			triggerCount++;
		}
	}
	return triggerCount;
}


std::array< std::array<bool, 16>, 4 > DioSystem::getFinalSnapshot()
{
	return ttlSnapshots.back().back().ttlStatus;
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


DioSystem::DioSystem()
{
	if (DIO_SAFEMODE)
	{
		// don't try to load.
		return;
	}
	/// load modules
	// this first module is required for the second module which I actually load functions from.
	HMODULE dio = LoadLibrary( "DIO64_Visa32.dll" );
	if (!dio)
	{
		int err = GetLastError();
		errBox( "Failed to load dio64_32.dll! Windows Error Code: " + str( err ) );
	}
	
	// initialize function pointers. This only requires the DLLs to be loaded (which requires them to be present on the machine...) 
	// so it's not in a safemode block.
	raw_DIO64_OpenResource = (DIO64_OpenResource)GetProcAddress(dio, "DIO64_OpenResource");
	raw_DIO64_Open = (DIO64_Open)GetProcAddress(dio, "DIO64_Open");
	raw_DIO64_Load = (DIO64_Load)GetProcAddress(dio, "DIO64_Load");
	raw_DIO64_Close = (DIO64_Close)GetProcAddress(dio, "DIO64_Close");
	raw_DIO64_Mode = (DIO64_Mode)GetProcAddress(dio, "DIO64_Mode");
	raw_DIO64_GetAttr = (DIO64_GetAttr)GetProcAddress(dio, "DIO64_GetAttr");
	raw_DIO64_SetAttr = (DIO64_SetAttr)GetProcAddress(dio, "DIO64_SetAttr");

	raw_DIO64_In_Read = (DIO64_In_Read)GetProcAddress(dio, "DIO64_In_Read");
	raw_DIO64_In_Start = (DIO64_In_Start)GetProcAddress(dio, "DIO64_In_Start");
	raw_DIO64_In_Read = (DIO64_In_Read)GetProcAddress(dio, "DIO64_In_Read");
	raw_DIO64_In_Status = (DIO64_In_Status)GetProcAddress(dio, "DIO64_In_Status");
	raw_DIO64_In_Stop = (DIO64_In_Stop)GetProcAddress(dio, "DIO64_In_Stop");
	
	raw_DIO64_Out_Config = (DIO64_Out_Config)GetProcAddress(dio, "DIO64_Out_Config");
	raw_DIO64_Out_ForceOutput = (DIO64_Out_ForceOutput)GetProcAddress(dio, "DIO64_Out_ForceOutput");
	raw_DIO64_Out_GetInput = (DIO64_Out_GetInput)GetProcAddress(dio, "DIO64_Out_GetInput");
	raw_DIO64_Out_Start = (DIO64_Out_Start)GetProcAddress(dio, "DIO64_Out_Start");
	raw_DIO64_Out_Status = (DIO64_Out_Status)GetProcAddress(dio, "DIO64_Out_Status");
	raw_DIO64_Out_Stop = (DIO64_Out_Stop)GetProcAddress(dio, "DIO64_Out_Stop");

	raw_DIO64_Out_Write = (DIO64_Out_Write)GetProcAddress(dio, "DIO64_Out_Write");

	// Open and Load DIO64
	try
	{
		int result;
		char* filename = "C:\\DIO64Visa\\DIO64Visa_Release Beta 2\\DIO64.CAT";
		char* resourceName = "PXI18::11::INSTR";
		WORD temp[4] = { -1, -1, -1, -1 };
		double tempd = 10000000;
		dioOpenResource(resourceName, 0, 0);
		//dioOpen( 0, 0 );
		dioLoad(0, filename, 0, 4);
		dioOutConfig(0, 0, temp, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, tempd);
		// done initializing.
	}
	catch (Error& exception)
	{
		errBox( exception.what() );
	}
}

std::string DioSystem::getSystemInfo()
{
	DWORD answer = 1000;
	std::string info = "TTL System Info:\nInput Mode: ";
	dioGetAttr( 0, 0, answer);
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
	dioGetAttr( 0, 1, answer );
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
	dioGetAttr( 0, 2, answer );
	if (answer == 1000)
	{
		info += "Input Buffer Size: no answer?\n";
	}
	info += "Input Buffer Size: " + str( answer ) + "\n";
	dioGetAttr( 0, 3, answer );
	if (answer == 1000)
	{
		info += "Output Buffer Size: no answer?\n";
	}
	info += "Output Buffer Size: " + str( answer ) + "\n";
	dioGetAttr( 0, 4, answer );
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
	dioOutStatus( 0, scansAvailable, status );
	// start the dio board!
	dioOutStart( 0 );
}


void DioSystem::shadeTTLs(std::vector<std::pair<UINT, UINT>> shadeList)
{
	for (UINT shadeInc = 0; shadeInc < shadeList.size(); shadeInc++)
	{
		// shade it.
		ttlPushControls[shadeList[shadeInc].first][shadeList[shadeInc].second].SetCheck(BST_INDETERMINATE);
		ttlShadeStatus[shadeList[shadeInc].first][shadeList[shadeInc].second] = true;
		// a grey color is then used.
		ttlPushControls[shadeList[shadeInc].first][shadeList[shadeInc].second].colorState = 2;
		ttlPushControls[shadeList[shadeInc].first][shadeList[shadeInc].second].RedrawWindow();
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
			ttlShadeStatus[rowInc][numberInc] = false;
			if (ttlPushControls[rowInc][numberInc].colorState == 2)
			{
				ttlPushControls[rowInc][numberInc].colorState = 0;
				ttlPushControls[rowInc][numberInc].RedrawWindow();
			}
			if (ttlStatus[rowInc][numberInc])
			{
				ttlPushControls[rowInc][numberInc].SetCheck(BST_CHECKED);
			}
			else
			{
				ttlPushControls[rowInc][numberInc].SetCheck(BST_UNCHECKED);
			}
			ttlPushControls[rowInc][numberInc].EnableWindow();
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


void DioSystem::handleInvert()
{
	for (UINT row = 0; row < ttlStatus.size(); row++)
	{
		for (UINT number = 0; number < ttlStatus[row].size(); number++)
		{
			if (ttlStatus[row][number])
			{
				forceTtl(row, number, 0);
			}
			else
			{
				forceTtl(row, number, 1);
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


void DioSystem::initialize( POINT& loc, cToolTips& toolTips, AuxiliaryWindow* master, int& id )
{
	// title
	ttlTitle.sPos = { loc.x, loc.y, loc.x + 480, loc.y + 25 };
	ttlTitle.Create( "TTLS", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER, ttlTitle.sPos, master, id++ );
	ttlTitle.fontType = HeadingFont;
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
		ttlNumberLabels[ttlNumberInc].Create( cstr( ttlNumberInc ), WS_CHILD | WS_VISIBLE | SS_SUNKEN,
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
		ttlRowLabels[row].Create( cstr(rowName), WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER,
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

			//ttlNames[row][number] = name;
			ttlPushControls[row][number].sPos = { long( loc.x + 32 + number * 28 ), long( loc.y + row * 28 ),
											long( loc.x + 32 + (number + 1) * 28 ), long( loc.y + (row + 1) * 28 ) };
			ttlPushControls[row][number].Create( "", WS_CHILD | WS_VISIBLE | BS_RIGHT | BS_3STATE,
												 ttlPushControls[row][number].sPos, master, 
												 TTL_ID_BEGIN + runningCount++ );
			ttlPushControls[row][number].setToolTip(ttlNames[row][number], toolTips, master);
		}
	}
	loc.y += 28 * 4;
}


void DioSystem::handleTtlScriptCommand( std::string command, timeType time, std::string name,
										std::vector<std::pair<UINT, UINT>>& ttlShadeLocations, 
										std::vector<variableType>& vars )
{
	// use an empty expression.
	handleTtlScriptCommand( command, time, name, Expression(), ttlShadeLocations, vars );
}


void DioSystem::handleTtlScriptCommand(std::string command, timeType time, std::string name, Expression pulseLength, 
									   std::vector<std::pair<UINT, UINT>>& ttlShadeLocations, 
										std::vector<variableType>& vars )
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
		ttlOn(row, collumn, time);
	}
	else if (command == "off:")
	{
		ttlOff(row, collumn, time);
	}
	else if (command == "pulseon:" || command == "pulseoff:")
	{
		try
		{
			pulseEndTime.second += pulseLength.evaluate();
		}
		catch (Error&)
		{
			pulseLength.assertValid( vars );
			pulseEndTime.first.push_back(pulseLength);
		}
		if (command == "pulseon:")
		{
			ttlOn( row, collumn, time );
			ttlOff( row, collumn, pulseEndTime );
		}
		if (command == "pulseoff:")
		{
			ttlOff( row, collumn, time );
			ttlOn( row, collumn, pulseEndTime );
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
			if (ttlStatus[row][number])
			{
				forceTtl(row, number, 0);
			}
			else
			{
				forceTtl(row, number, 1);
			}
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
				if (ttlHoldStatus[rowInc][numberInc])
				{
					ttlPushControls[rowInc][numberInc].SetCheck(BST_CHECKED);					
					ttlStatus[rowInc][numberInc] = true;
					// actually change the ttl.
					forceTtl(rowInc, numberInc, 1);
				}
				else
				{
					ttlPushControls[rowInc][numberInc].SetCheck(BST_UNCHECKED);
					ttlStatus[rowInc][numberInc] = false;
					// actually change the ttl.
					forceTtl(rowInc, numberInc, 0);
				}
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


// prepares some structures for a simple force event. 
void DioSystem::prepareForce( )
{
	ttlSnapshots.resize( 1 );
	ttlCommandList.resize( 1 );
	formattedTtlSnapshots.resize( 1 );
	finalFormatTtlData.resize( 1 );
}


void DioSystem::resetTtlEvents( )
{
	ttlCommandFormList.clear( );
	ttlSnapshots.clear( );
	ttlCommandList.clear( );
	formattedTtlSnapshots.clear( );
	finalFormatTtlData.clear( );
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


void DioSystem::ttlOn(UINT row, UINT column, timeType time)
{
	// make sure it's either a variable or a number that can be used.
	ttlCommandFormList.push_back({ {row, column}, time, true });
}


void DioSystem::ttlOff(UINT row, UINT column, timeType time)
{
	// check to make sure either variable or actual value.
	ttlCommandFormList.push_back({ {row, column}, time, false });
}


void DioSystem::ttlOnDirect( UINT row, UINT column, double time, UINT variation )
{
	DioCommand command;
	command.line = { row, column };
	command.time = time;
	command.value = true;
	ttlCommandList[variation].push_back( command );
}


void DioSystem::ttlOffDirect( UINT row, UINT column, double time, UINT variation)
{
	DioCommand command;
	command.line = { row, column };
	command.time = time;
	command.value = false;
	ttlCommandList[variation].push_back( command );
}


void DioSystem::stopBoard()
{
	dioOutStop( 0 );
}

double DioSystem::getClockStatus()
{
	// initialize to zero so that in safemode goes directly to getting tick count.
	int result = 0;
	DIO64STAT stat;
	DWORD availableScans;
	try
	{
		dioOutStatus( 0, availableScans, stat );

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

// forceTtl forces the actual ttl to a given value and changes the checkbox status to reflect that.
void DioSystem::forceTtl(int row, int number, int state)
{
	// change the ttl checkbox.
	if (state == 0)
	{
		ttlPushControls[row][number].SetCheck(BST_UNCHECKED);
		ttlStatus[row][number] = false;
	}
	else
	{
		ttlPushControls[row][number].SetCheck(BST_CHECKED);
		ttlStatus[row][number] = true;
	}
	
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
	dioForceOutput( 0, tempCommand.data(), 15 );
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


void DioSystem::writeTtlData(UINT variation)
{
	WORD temp[4] = { WORD_MAX, WORD_MAX, WORD_MAX, WORD_MAX };
	double scanRate = 10000000;
	DWORD availableScans = 0;
	DIO64STAT status;
	status.AIControl = 0;
	try
	{
		dioOutStop( 0 );
	}
	catch ( Error& ) { /* if fails it probably just wasn't running before */ } 
	dioOutConfig( 0, 0, temp, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, scanRate );
	dioOutStatus( 0, availableScans, status );
	dioOutWrite( 0, finalFormatTtlData[variation].data(), formattedTtlSnapshots[variation].size(), status );
}


std::string DioSystem::getName(UINT row, UINT number)
{
	return ttlNames[row][number];
}


ULONG DioSystem::getNumberEvents(UINT variation)
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
void DioSystem::waitTillFinished(UINT variation)
{
	double totalTime = (formattedTtlSnapshots[variation].back()[0] 
						 + 65535 * formattedTtlSnapshots[variation].back()[1]) / 10000.0 + 1;
	wait(totalTime);
	stopBoard();
}


double DioSystem::getTotalTime(UINT variation)
{
	return (formattedTtlSnapshots[variation].back()[0]
			 + 65535 * formattedTtlSnapshots[variation].back()[1]) / 10000.0 + 1;
}


void DioSystem::interpretKey(key variationKey, std::vector<variableType>& vars)
{
	UINT variations;
	variations = variationKey[vars[0].name].first.size();
	if (variations == 0)
	{
		variations = 1; 
	}
	/// imporantly, this sizes the relevant structures.
	ttlCommandList.clear();
	ttlCommandList.resize(variations);
	ttlSnapshots.clear();
	ttlSnapshots.resize(variations);
	formattedTtlSnapshots.clear();
	formattedTtlSnapshots.resize(variations);
	finalFormatTtlData.clear( );
	finalFormatTtlData.resize( variations );
	// and interpret for each variation.
	for (UINT variationNum = 0; variationNum < variations; variationNum++)
	{
		for (UINT commandInc = 0; commandInc < ttlCommandFormList.size(); commandInc++)
		{
			DioCommand tempCommand;
			tempCommand.line = ttlCommandFormList[commandInc].line;
			tempCommand.value = ttlCommandFormList[commandInc].value;
			double variableTime = 0;
			// add together current values for all variable times.
			if (ttlCommandFormList[commandInc].time.first.size() != 0)
			{
				for (auto varTime : ttlCommandFormList[commandInc].time.first)
				{
					variableTime += varTime.evaluate(variationKey, variationNum, vars);
					// this assumed no expressions.
					//variableTime += variationKey[varTime].first[variationNum];
				}
			}
			tempCommand.time = variableTime + ttlCommandFormList[commandInc].time.second;
			ttlCommandList[variationNum].push_back(tempCommand);
		}
	}
}


void DioSystem::organizeTtlCommands(UINT variation)
{
	// each element of this is a different time (the double), and associated with each time is a vector which locates 
	// which commands were on at this time, for ease of retrieving all of the values in a moment.
	std::vector<std::pair<double, std::vector<unsigned short>>> timeOrganizer;
	std::vector<DioCommand> orderedList(ttlCommandList[variation]);
	// sort using a lambda. std::sort is effectively a quicksort algorithm.
	std::sort(orderedList.begin(), orderedList.end(), [](DioCommand a, DioCommand b) {return a.time < b.time; });
	/// organize all of the commands.
	for (USHORT commandInc = 0; commandInc < ttlCommandList[variation].size(); commandInc++)
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

	ttlSnapshots[variation].clear();
	// start with the initial status.
	ttlSnapshots[variation].push_back({ 0, ttlStatus });
	if (timeOrganizer[0].first != 0)
	{
		// then there were no commands at time 0, so just set the initial state to be exactly the original state before
		// the experiment started. I don't need to modify the first snapshot in this case, it's already set. Add a snapshot
		// here so that the thing modified is the second snapshot.
		ttlSnapshots[variation].push_back({ 0, ttlStatus });
	}

	// handle the zero case specially. This may or may not be the literal first snapshot.
	ttlSnapshots[variation].back().time = timeOrganizer[0].first;
	for (UINT zeroInc = 0; zeroInc < timeOrganizer[0].second.size(); zeroInc++)
	{
		// make sure to address he correct ttl. the ttl location is located in individuaTTL_CommandList but you need 
		// to make sure you access the correct command.
		UINT row = orderedList[timeOrganizer[0].second[zeroInc]].line.first;
		UINT column = orderedList[timeOrganizer[0].second[zeroInc]].line.second;
		ttlSnapshots[variation].back().ttlStatus[row][column]	= orderedList[timeOrganizer[0].second[zeroInc]].value;
	}
	
	// already handled the first case.
	for (UINT commandInc = 1; commandInc < timeOrganizer.size(); commandInc++)
	{
		// first copy the last set so that things that weren't changed remain unchanged.
		ttlSnapshots[variation].push_back(ttlSnapshots[variation].back());
		//
		ttlSnapshots[variation].back().time = timeOrganizer[commandInc].first;
		for (UINT zeroInc = 0; zeroInc < timeOrganizer[commandInc].second.size(); zeroInc++)
		{
			// see description of this command above... update everything that changed at this time.
			UINT row = orderedList[timeOrganizer[commandInc].second[zeroInc]].line.first;
			UINT column = orderedList[timeOrganizer[commandInc].second[zeroInc]].line.second;
			ttlSnapshots[variation].back().ttlStatus[row][column] = orderedList[timeOrganizer[commandInc].second[zeroInc]].value;
		}
	}
	// phew. Check for good input by user:
	for (auto snapshot : ttlSnapshots[variation])
	{
		if (snapshot.time < 0)
		{
			thrower("ERROR: The code tried to set a ttl event at a negative time value! This is clearly not allowed."
					" Aborting.");
		}
	}
}


void DioSystem::convertToFinalFormat(UINT variation)
{
	// excessive but just in case.
	formattedTtlSnapshots[variation].clear();
	finalFormatTtlData[variation].clear( );
	// do bit arithmetic.
	for (UINT timeInc = 0; timeInc < ttlSnapshots[variation].size(); timeInc++)
	{
		USHORT lowordTime;
		USHORT hiwordTime;
		// convert to system clock ticks. Assume that the crate is running on a 10 MHz signal, so multiply by
		// 10,000,000, but then my time is in milliseconds, so divide that by 1,000, ending with multiply by 10,000
		lowordTime = ULONGLONG(ttlSnapshots[variation][timeInc].time * 10000) % 65535;
		USHORT temp = ttlSnapshots[variation][timeInc].time * 10000;
		hiwordTime = ULONGLONG(ttlSnapshots[variation][timeInc].time * 10000) / 65535;
		// each major index is a row (A, B, C, D), each minor index is a ttl state (0, 1) in that row.
		std::array<std::bitset<16>, 4> ttlBits;
		for (UINT rowInc = 0; rowInc < 4; rowInc++)
		{
			for (UINT numberInc = 0; numberInc < 16; numberInc++)
			{
				if (ttlSnapshots[variation][timeInc].ttlStatus[rowInc][numberInc])
				{
					// flip bit to 1.
					ttlBits[rowInc].set(numberInc, true);
				}
				else
				{
					// flip bit to 0.
					ttlBits[rowInc].set(numberInc, false);
				}
			}
		}
		// I need to put it as an int (just because I'm not actually sure how the bitset gets stored... it'd probably 
		// work just passing the address of the bitsets, but I'm sure this will work so whatever.)
		std::array<USHORT, 6> tempCommand;
		tempCommand[0] = lowordTime;
		tempCommand[1] = hiwordTime;
		tempCommand[2] = static_cast <unsigned short>(ttlBits[0].to_ulong());
		tempCommand[3] = static_cast <unsigned short>(ttlBits[1].to_ulong());
		tempCommand[4] = static_cast <unsigned short>(ttlBits[2].to_ulong());
		tempCommand[5] = static_cast <unsigned short>(ttlBits[3].to_ulong());
		formattedTtlSnapshots[variation].push_back(tempCommand);
	}
	// flatten the array.
	finalFormatTtlData.resize( formattedTtlSnapshots[variation].size( ) * 6 );
	int count = 0;
	for ( auto& element : finalFormatTtlData[variation] )
	{
		// concatenate all the data at once.
		element = formattedTtlSnapshots[variation][count / 6][count % 6];
		count++;
	}
}


void DioSystem::checkNotTooManyTimes( UINT variation )
{
	if ( formattedTtlSnapshots[variation].size( ) > 512 )
	{
		thrower( "ERROR: DIO Data has more than 512 individual timestamps, which is larger than the DIO64 FIFO Buffer"
				 ". The DIO64 card can only support 512 individual time-stamps. If you need more, you need to configure"
				 " this code to create a thread to continuously write to the DIO64 card as it outputs." );
	}
}


void DioSystem::checkFinalFormatTimes( UINT variation )
{
	// loop through all the commands and make sure that no two events have the same time-stamp. Was a common symptom
	// of a bug when code first created.
	for ( int dioEventInc = 0; dioEventInc < formattedTtlSnapshots[variation].size( ); dioEventInc++ )
	{
		for ( int dioEventInc2 = 0; dioEventInc2 < dioEventInc; dioEventInc2++ )
		{
			if ( formattedTtlSnapshots[variation][dioEventInc][0] == formattedTtlSnapshots[variation][dioEventInc2][0]
				 && formattedTtlSnapshots[variation][dioEventInc][1] == formattedTtlSnapshots[variation][dioEventInc2][1] )
			{
				thrower( "ERROR: Dio system somehow created two events with the same time stamp! This might be caused by"
						 " ttl events being spaced to close to each other." );
			}
		}
	}
}


std::string DioSystem::getErrorMessage(int errorCode)
{
	switch (errorCode)
	{
		case -8:
			return "Illegal board number - the board number must be between 0 and 7.";
		case -9:
			return "The requested board number has not been opened.";
		case -10:
			return "The buffers have over or under run.";
		case -12:
			return "Invalid parameter.";
		case -13:
			return "No Driver Interface.";
		case -14:
			return "Board does not have the OCXO option installed.";
		case -15:
			return "Only available on PXI.";
		case -16:
			return "Stop trigger source is invalid.";
		case -17:
			return "Port number conflicts. Check the hints used in DIO64_Load().";
		case -18:
			return "Missing DIO64.cat file.";
		case -19:
			return "Not enough system resources available.";
		case -20:
			return "Invalid DIO64.cat file.";
		case -21:
			return "Required image not found.";
		case -22:
			return "Error programming the FPGA.";
		case -23:
			return "File not found.";
		case -24:
			return "Board error.";
		case -25:
			return "Function call invalid at this time.";
		case -26:
			return "Not enough transitions specified for operation.";
		default:
			return "Unrecognized DIO64 error code!";
	}
}


void DioSystem::zeroBoard()
{
	for (UINT row = 0; row < ttlStatus.size(); row++)
	{
		for (UINT number = 0; number < ttlStatus[row].size(); number++)
		{
			forceTtl( row, number, 0 );
		}
	}
}


/// DIO64 Wrapper functions that I actually use
std::string DioSystem::getTtlSequenceMessage(UINT variation)
{
	std::string message;
	if ( ttlSnapshots.size( ) <= variation )
	{
		thrower( "ERROR: Attemted to retrieve ttl sequence message from snapshot " + str( variation ) + ", which does not "
				 "exist!" );
	}
	for (auto snap : ttlSnapshots[variation])
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

void DioSystem::dioOpen(WORD board, WORD baseio)
{
	if (!DIO_SAFEMODE)
	{
		int result = raw_DIO64_Open(board, baseio);
		if (result)
		{
			thrower("dioOpen failed! : (" + str(result) + "): " + getErrorMessage(result));
		}
	}
}


void DioSystem::dioMode(WORD board, WORD mode)
{
	if (!DIO_SAFEMODE)
	{
		int result = raw_DIO64_Mode(board, mode);
		if (result)
		{
			thrower("dioMode failed! : (" + str(result) + "): " + getErrorMessage(result));
		}
	}
}


void DioSystem::dioLoad(WORD board, char *rbfFile, int inputHint, int outputHint)
{
	if (!DIO_SAFEMODE)
	{
		int result = raw_DIO64_Load(board, rbfFile, inputHint, outputHint);
		if (result)
		{
			thrower("dioLoad failed! : (" + str(result) + "): " + getErrorMessage(result));
		}
	}
}


void DioSystem::dioClose(WORD board)
{
	if (!DIO_SAFEMODE)
	{
		int result = raw_DIO64_Close(board);
		if (result)
		{
			thrower("dioClose failed! : (" + str(result) + "): " + getErrorMessage(result));
		}
	}
}


void DioSystem::dioInStart(WORD board, DWORD ticks, WORD& mask, WORD maskLength, WORD flags, WORD clkControl,
						   WORD startType, WORD startSource, WORD stopType, WORD stopSource, DWORD AIControl,
						   double& scanRate)
{
	if (!DIO_SAFEMODE)
	{
		int result = raw_DIO64_In_Start(board, ticks, &mask, maskLength, flags, clkControl, startType, startSource,
										stopType, stopSource, AIControl, &scanRate);
		if (result)
		{
			thrower("dioInStart failed! : (" + str(result) + "): " + getErrorMessage(result));
		}
	}
}


void DioSystem::dioInStatus(WORD board, DWORD& scansAvail, DIO64STAT& status)
{
	if (!DIO_SAFEMODE)
	{
		int result = raw_DIO64_In_Status(board, &scansAvail, &status);
		if (result)
		{
			thrower("dioInStatus failed! : (" + str(result) + "): " + getErrorMessage(result));
		}
	}
}


void DioSystem::dioInRead(WORD board, WORD& buffer, DWORD scansToRead, DIO64STAT& status)
{
	if (!DIO_SAFEMODE)
	{
		int result = raw_DIO64_In_Read(board, &buffer, scansToRead, &status);
		if (result)
		{
			thrower("dioInRead failed! : (" + str(result) + "): " + getErrorMessage(result));
		}
	}
}


void DioSystem::dioInStop(WORD board)
{
	if (!DIO_SAFEMODE)
	{
		int result = raw_DIO64_In_Stop(board);
		if (result)
		{
			thrower("dioInStop failed! : (" + str(result) + "): " + getErrorMessage(result));
		}
	}
}


void DioSystem::dioForceOutput(WORD board, WORD* buffer, DWORD mask)
{
	if (!DIO_SAFEMODE)
	{
		int result = raw_DIO64_Out_ForceOutput(board, buffer, mask);
		if (result)
		{
			thrower("dioForceOutput failed! : (" + str(result) + "): " + getErrorMessage(result));
		}
	}
}


void DioSystem::dioOutGetInput(WORD board, WORD& buffer)
{
	if (!DIO_SAFEMODE)
	{
		int result = raw_DIO64_Out_GetInput(board, &buffer);
		if (result)
		{
			thrower("dioOutGetInput failed! : (" + str(result) + "): " + getErrorMessage(result));
		}
	}
}


void DioSystem::dioOutConfig(WORD board, DWORD ticks, WORD* mask, WORD maskLength, WORD flags, WORD clkControl,
							 WORD startType, WORD startSource, WORD stopType, WORD stopSource, DWORD AIControl,
							 DWORD reps, WORD ntrans, double& scanRate)
{
	if (!DIO_SAFEMODE)
	{
		int result = raw_DIO64_Out_Config(board, ticks, mask, maskLength, flags, clkControl,
										  startType, startSource, stopType, stopSource, AIControl,
										  reps, ntrans, &scanRate);
		if (result)
		{
			thrower("dioOutConfig failed! : (" + str(result) + "): " + getErrorMessage(result));
		}
	}
}


void DioSystem::dioOutStart(WORD board)
{
	if (!DIO_SAFEMODE)
	{
		int result = raw_DIO64_Out_Start(board);
		if (result)
		{
			thrower("dioOutStart failed! : (" + str(result) + "): " + getErrorMessage(result));
		}
	}
}


void DioSystem::dioOutStatus(WORD board, DWORD& scansAvail, DIO64STAT& status)
{
	if (!DIO_SAFEMODE)
	{
		int result = raw_DIO64_Out_Status(board, &scansAvail, &status);
		if (result)
		{
			thrower("dioOutStatus failed! : (" + str(result) + "): " + getErrorMessage(result) + "\r\n");
		}
	}
}


void DioSystem::dioOutWrite(WORD board, WORD* buffer, DWORD bufsize, DIO64STAT& status)
{
	if (!DIO_SAFEMODE)
	{
		int result = raw_DIO64_Out_Write(board, buffer, bufsize, &status);
		if (result)
		{
			thrower("dioOutWrite failed! : (" + str(result) + "): " + getErrorMessage(result));
		}
	}
}


void DioSystem::dioOutStop(WORD board)
{
	if (!DIO_SAFEMODE)
	{
		int result = raw_DIO64_Out_Stop(board);
		if (result)
		{
			thrower("dioOutStop failed! : (" + str(result) + "): " + getErrorMessage(result));
		}
	}
}


void DioSystem::dioSetAttr(WORD board, DWORD attrID, DWORD value)
{
	if (!DIO_SAFEMODE)
	{
		int result = raw_DIO64_SetAttr(board, attrID, value);
		if (result)
		{
			thrower("dioSetAttr failed! : (" + str(result) + "): " + getErrorMessage(result));
		}
	}
}


void DioSystem::dioGetAttr(WORD board, DWORD attrID, DWORD& value)
{
	if (!DIO_SAFEMODE)
	{
		int result = raw_DIO64_GetAttr(board, attrID, &value);
		if (result)
		{
			thrower("dioGetAttr failed! : (" + str(result) + "): " + getErrorMessage(result));
		}
	}
}


void DioSystem::dioOpenResource(char* resourceName, WORD board, WORD baseio)
{
	if (!DIO_SAFEMODE)
	{
		int result = raw_DIO64_OpenResource(resourceName, board, baseio);
		if (result)
		{
			thrower("dioOpenResource failed! : (" + str(result) + "): " + getErrorMessage(result));
		}
	}
}

