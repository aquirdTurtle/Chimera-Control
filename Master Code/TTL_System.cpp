#include "stdafx.h"
#include "TTL_System.h"
#include "constants.h"
#include "MasterWindow.h"
#include "nidaqmx2.h"
#include <sstream>
#include "ExperimentManager.h"
#include <unordered_map>
#include <bitset>
// don't use this because I manually import dll functions.
//#include "Dio64.h"

void TtlSystem::dioOpen( WORD board, WORD baseio )
{
	if ( !DIO_SAFEMODE )
	{
		int result = raw_DIO64_Open( board, baseio );
		if ( result )
		{
			thrower( "dioOpen failed! : (" + std::to_string( result ) + "): " + this->getErrorMessage( result ) );
		}
	}
	return;
}


void TtlSystem::dioMode( WORD board, WORD mode )
{
	if ( !DIO_SAFEMODE )
	{
		int result = raw_DIO64_Mode( board, mode );
		if ( result )
		{
			thrower( "dioMode failed! : (" + std::to_string( result ) + "): " + this->getErrorMessage( result ) );
		}
	}
	return;
}


void TtlSystem::dioLoad( WORD board, char *rbfFile, int inputHint, int outputHint )
{
	if ( !DIO_SAFEMODE )
	{
		int result = raw_DIO64_Load( board, rbfFile, inputHint, outputHint );
		if ( result )
		{
			thrower( "dioLoad failed! : (" + std::to_string( result ) + "): " + this->getErrorMessage( result ) );
		}
	}
	return;
}


void TtlSystem::dioClose( WORD board )
{
	if ( !DIO_SAFEMODE )
	{
		int result = raw_DIO64_Close( board );
		if ( result )
		{
			thrower( "dioClose failed! : (" + std::to_string( result ) + "): " + this->getErrorMessage( result ) );
		}
	}
	return;
}


void TtlSystem::dioInStart( WORD board, DWORD ticks, WORD& mask, WORD maskLength, WORD flags, WORD clkControl,
							 WORD startType, WORD startSource, WORD stopType, WORD stopSource, DWORD AIControl,
							 double& scanRate )
{
	if ( !DIO_SAFEMODE )
	{
		int result = raw_DIO64_In_Start( board, ticks, &mask, maskLength, flags, clkControl, startType, startSource, 
										 stopType, stopSource, AIControl, &scanRate );
		if ( result )
		{
			thrower( "dioInStart failed! : (" + std::to_string( result ) + "): " + this->getErrorMessage( result ) );
		}
	}
	return;
}


void TtlSystem::dioInStatus( WORD board, DWORD& scansAvail, DIO64STAT& status )
{
	if ( !DIO_SAFEMODE )
	{
		int result = raw_DIO64_In_Status( board, &scansAvail, &status );
		if ( result )
		{
			thrower( "dioInStatus failed! : (" + std::to_string( result ) + "): " + this->getErrorMessage( result ) );
		}
	}
	return;
}


void TtlSystem::dioInRead( WORD board, WORD& buffer, DWORD scansToRead, DIO64STAT& status )
{
	if ( !DIO_SAFEMODE )
	{
		int result = raw_DIO64_In_Read( board, &buffer, scansToRead, &status );
		if ( result )
		{
			thrower( "dioInRead failed! : (" + std::to_string( result ) + "): " + this->getErrorMessage( result ) );
		}
	}
	return;
}


void TtlSystem::dioInStop( WORD board )
{
	if ( !DIO_SAFEMODE )
	{
		int result = raw_DIO64_In_Stop( board );
		if ( result )
		{
			thrower( "dioInStop failed! : (" + std::to_string( result ) + "): " + this->getErrorMessage( result ) );
		}
	}
	return;
}


void TtlSystem::dioForceOutput( WORD board, WORD* buffer, DWORD mask )
{
	if ( !DIO_SAFEMODE )
	{
		int result = raw_DIO64_Out_ForceOutput( board, buffer, mask );
		if ( result )
		{
			thrower( "dioForceOutput failed! : (" + std::to_string( result ) + "): " + this->getErrorMessage( result ) );
		}
	}
	return;
}


void TtlSystem::dioOutGetInput( WORD board, WORD& buffer )
{
	if ( !DIO_SAFEMODE )
	{
		int result = raw_DIO64_Out_GetInput( board, &buffer );
		if ( result )
		{
			thrower( "dioOutGetInput failed! : (" + std::to_string( result ) + "): " + this->getErrorMessage( result ) );
		}
	}
	return;
}


void TtlSystem::dioOutConfig( WORD board, DWORD ticks, WORD* mask, WORD maskLength, WORD flags, WORD clkControl,
							   WORD startType, WORD startSource, WORD stopType, WORD stopSource, DWORD AIControl,
							   DWORD reps, WORD ntrans, double& scanRate )
{
	if ( !DIO_SAFEMODE )
	{
		int result = raw_DIO64_Out_Config( board, ticks, mask, maskLength, flags, clkControl,
										   startType, startSource, stopType, stopSource, AIControl,
										   reps, ntrans, &scanRate );
		if ( result )
		{
			thrower( "dioOutConfig failed! : (" + std::to_string( result ) + "): " + this->getErrorMessage( result ) );
		}
	}
	return;
}


void TtlSystem::dioOutStart( WORD board )
{
	if ( !DIO_SAFEMODE )
	{
		int result = raw_DIO64_Out_Start( board );
		if ( result )
		{
			thrower( "dioOutStart failed! : (" + std::to_string( result ) + "): " + this->getErrorMessage( result ) );
		}
	}
	return;
}


void TtlSystem::dioOutStatus( WORD board, DWORD& scansAvail, DIO64STAT& status )
{
	if ( !DIO_SAFEMODE )
	{
		int result = raw_DIO64_Out_Status( board, &scansAvail, &status );
		if ( result )
		{
			thrower( "dioOutStatus failed! : (" + std::to_string( result ) + "): " + this->getErrorMessage( result ) );
		}
	}
	return;
}


void TtlSystem::dioOutWrite( WORD board, WORD* buffer, DWORD bufsize, DIO64STAT& status )
{
	if ( !DIO_SAFEMODE )
	{
		int result = raw_DIO64_Out_Write( board, buffer, bufsize, &status );
		if ( result )
		{
			thrower( "dioOutWrite failed! : (" + std::to_string( result ) + "): " + this->getErrorMessage( result ) );
		}
	}
	return;
}


void TtlSystem::dioOutStop( WORD board )
{
	if ( !DIO_SAFEMODE )
	{
		int result = raw_DIO64_Out_Stop( board );
		if ( result )
		{
			thrower( "dioOutStop failed! : (" + std::to_string( result ) + "): " + this->getErrorMessage( result ) );
		}
	}
	return;
}


void TtlSystem::dioSetAttr( WORD board, DWORD attrID, DWORD value )
{
	if ( !DIO_SAFEMODE )
	{
		int result = raw_DIO64_SetAttr( board, attrID, value );
		if ( result )
		{
			thrower( "dioSetAttr failed! : (" + std::to_string( result ) + "): " + this->getErrorMessage( result ) );
		}
	}
	return;
}


void TtlSystem::dioGetAttr( WORD board, DWORD& attrID, DWORD& value )
{
	if ( !DIO_SAFEMODE )
	{
		int result = raw_DIO64_GetAttr( board, &attrID, &value );
		if ( result )
		{
			thrower( "dioGetAttr failed! : (" + std::to_string( result ) + "): " + this->getErrorMessage( result ) );
		}
	}
	return;
}


TtlSystem::TtlSystem() {}

TtlSystem::TtlSystem(int& startID)
{
	ttlTitle.ID = startID;
	startID += 1;
	if (startID != TTL_ID_BEGIN)
	{
		MessageBox(0, "ERROR: TTL_ID_BEGIN doesn't match the actual ID.", 0, 0);
	}
	for (int ttlNumberInc = 0; ttlNumberInc < ttlNumberLabels.size(); ttlNumberInc++)
	{
		ttlNumberLabels[ttlNumberInc].ID = startID;
		startID += 1;
	}
	for (int row = 0; row < ttlPushControls.size(); row++)
	{
		ttlRowLabels[row].ID = startID;
		startID += 1;
	}
	for (int row = 0; row < ttlPushControls.size(); row++)
	{
		for (int number = 0; number < ttlPushControls[row].size(); number++)
		{
			ttlPushControls[row][number].ID = startID;
			startID += 1;
		}
	}
	if (startID != TTL_HOLD)
	{
		MessageBox(0, "ERROR: TTL_HOLD doesn't match button ID.", 0, 0);
	}
	ttlHold.ID = startID;
	startID += 1;
	/// load modules
	// this first module is required for the second module which I actually load functions from.
	//HMODULE dioNT2 = LoadLibrary( "Dio64_NT.dll" );
	HMODULE dioNT = LoadLibrary( "dio64_nt.dll" );
	//HMODULE dioNT = LoadLibrary("C:/Windows/SysWOW64/dio64_nt.dll");
	if (!dioNT)
	{
		int err = GetLastError();
		errBox( std::to_string(err) );
		errBox( "!" );
	}
	HMODULE dio = LoadLibrary( "dio64_32.dll" );
	//HMODULE dio = LoadLibrary("C:/Windows/SysWOW64/dio64_32.dll");
	if (!dio)
	{
		int err = GetLastError();
		errBox( std::to_string( err ) );
		errBox( "!" );
		errBox( "!!" );
	}
	
	// initialize function pointers. This only requires the DLLs to be loaded (which requires them to be present on the machine...) 
	// so it's not in a safemode block.

	this->raw_DIO64_Open = (DIO64_Open)GetProcAddress(dio, "DIO64_Open");
	this->raw_DIO64_Load = (DIO64_Load)GetProcAddress(dio, "DIO64_Load");
	this->raw_DIO64_Close = (DIO64_Close)GetProcAddress(dio, "DIO64_Close");
	this->raw_DIO64_Mode = (DIO64_Mode)GetProcAddress(dio, "DIO64_Mode");
	this->raw_DIO64_GetAttr = (DIO64_GetAttr)GetProcAddress(dio, "DIO64_GetAttr");
	this->raw_DIO64_SetAttr = (DIO64_SetAttr)GetProcAddress(dio, "DIO64_SetAttr");

	this->raw_DIO64_In_Read = (DIO64_In_Read)GetProcAddress(dio, "DIO64_In_Read");
	this->raw_DIO64_In_Start = (DIO64_In_Start)GetProcAddress(dio, "DIO64_In_Start");
	this->raw_DIO64_In_Read = (DIO64_In_Read)GetProcAddress(dio, "DIO64_In_Read");
	this->raw_DIO64_In_Status = (DIO64_In_Status)GetProcAddress(dio, "DIO64_In_Status");
	this->raw_DIO64_In_Stop = (DIO64_In_Stop)GetProcAddress(dio, "DIO64_In_Stop");
	
	this->raw_DIO64_Out_Config = (DIO64_Out_Config)GetProcAddress(dio, "DIO64_Out_Config");
	this->raw_DIO64_Out_ForceOutput = (DIO64_Out_ForceOutput)GetProcAddress(dio, "DIO64_Out_ForceOutput");
	this->raw_DIO64_Out_GetInput = (DIO64_Out_GetInput)GetProcAddress(dio, "DIO64_Out_GetInput");
	this->raw_DIO64_Out_Start = (DIO64_Out_Start)GetProcAddress(dio, "DIO64_Out_Start");
	this->raw_DIO64_Out_Status = (DIO64_Out_Status)GetProcAddress(dio, "DIO64_Out_Status");
	this->raw_DIO64_Out_Stop = (DIO64_Out_Stop)GetProcAddress(dio, "DIO64_Out_Stop");
	this->raw_DIO64_Out_Write = (DIO64_Out_Write)GetProcAddress(dio, "DIO64_Out_Write");

	//Open and Load DIO64
	if (!DIO_SAFEMODE)
	{
		try
		{
			int result;
			char* filename = "";
			WORD temp[4] = { -1, -1, -1, -1 };
			double tempd = 10000000;
			
			dioOpen( 0, 0 );
			dioLoad( 0, filename, 0, 4 );
			dioOutConfig( 0, 0, temp, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, tempd );
			// done initializing.
		}
		catch (myException& exception)
		{
			errBox( exception.what() );
		}
	}
}

std::array<std::array<std::string, 16>, 4> TtlSystem::getAllNames()
{
	return this->ttlNames;
}

void TtlSystem::startBoard()
{
	int result;
	DIO64STAT status;
	DWORD scansAvailable;
	dioOutStatus( 0, scansAvailable, status );
	// start the dio board!
	dioOutStart( 0 );
	return;
}

void TtlSystem::shadeTTLs(std::vector<std::pair<unsigned int, unsigned int>> shadeList)
{
	for (int shadeInc = 0; shadeInc < shadeList.size(); shadeInc++)
	{
		// shade it.
		this->ttlPushControls[shadeList[shadeInc].first][shadeList[shadeInc].second].SetCheck(BST_INDETERMINATE);
		this->ttlShadeStatus[shadeList[shadeInc].first][shadeList[shadeInc].second] = true;
		// a grey color is then used.
		this->ttlPushControls[shadeList[shadeInc].first][shadeList[shadeInc].second].colorState = 2;
		this->ttlPushControls[shadeList[shadeInc].first][shadeList[shadeInc].second].RedrawWindow();
	}
	return;
}
void TtlSystem::unshadeTTLs()
{
	for (int rowInc = 0; rowInc < this->getNumberOfTTLRows(); rowInc++)
	{
		for (int numberInc = 0; numberInc < this->getNumberOfTTLsPerRow(); numberInc++)
		{
			this->ttlShadeStatus[rowInc][numberInc] = false;
			if (this->ttlPushControls[rowInc][numberInc].colorState == 2)
			{
				this->ttlPushControls[rowInc][numberInc].colorState = 0;
				this->ttlPushControls[rowInc][numberInc].RedrawWindow();
			}			
			if (this->ttlStatus[rowInc][numberInc])
			{
				this->ttlPushControls[rowInc][numberInc].SetCheck(BST_CHECKED);
			}
			else
			{
				this->ttlPushControls[rowInc][numberInc].SetCheck(BST_UNCHECKED);
			}
		}
	}
	return;
}

void TtlSystem::initialize(POINT& upperLeftCornerLocation, HWND windowHandle, HINSTANCE programInstance, std::unordered_map<HWND, std::string>& toolTipText, std::vector<CToolTipCtrl*>& toolTips, MasterWindow* master)
{
	// title
	RECT location;
	location.left = upperLeftCornerLocation.x;
	location.top = upperLeftCornerLocation.y;
	location.right = location.left + 480;
	location.bottom = location.top + 25;
	ttlTitle.position = location;
	ttlTitle.Create("TTL Boards High / Low Control", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER, location, CWnd::FromHandle(windowHandle), ttlTitle.ID);
	// all number labels
	location.left += 32;
	location.right = location.left + 28;
	location.top += 25;
	location.bottom += 20;
	for (int ttlNumberInc = 0; ttlNumberInc < ttlNumberLabels.size(); ttlNumberInc++)
	{
		ttlNumberLabels[ttlNumberInc].position = location;
		ttlNumberLabels[ttlNumberInc].Create(std::to_string(ttlNumberInc + 1).c_str(), WS_CHILD | WS_VISIBLE | SS_SUNKEN, location,
			CWnd::FromHandle(windowHandle), ttlNumberLabels[ttlNumberInc].ID);
		location.left += 28;
		location.right += 28; 
	}
	// all row labels
	location.left = upperLeftCornerLocation.x;
	location.top = upperLeftCornerLocation.y + 45;
	location.right = location.left + 32;
	location.bottom = location.top + 28;
	for (int row = 0; row < ttlPushControls.size(); row++)
	{
		ttlRowLabels[row].position = location;
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
		ttlRowLabels[row].Create(rowName.c_str(), WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER, location, CWnd::FromHandle(windowHandle),
			ttlRowLabels[row].ID);
		location.top += 28;
		location.bottom += 28;
	}
	// all push buttons
	location.left = upperLeftCornerLocation.x + 32;
	location.top = upperLeftCornerLocation.y + 45;
	location.right = location.left + 28;
	location.bottom = location.top + 28;
	for (int row = 0; row < ttlPushControls.size(); row++)
	{
		for (int number = 0; number < ttlPushControls[row].size(); number++)
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
			name += std::to_string(number + 1);
			//this->ttlNames[row][number] = name;
			ttlPushControls[row][number].position = location;
			ttlPushControls[row][number].Create("", WS_CHILD | WS_VISIBLE | BS_RIGHT | BS_3STATE, location, CWnd::FromHandle(windowHandle), 
				ttlPushControls[row][number].ID);
			if (!ttlPushControls[row][number].setToolTip(this->ttlNames[row][number], toolTips, master))
			{
				MessageBox(0, "Tool tip creation failed!", 0, 0);
			}

			location.left += 28;
			location.right += 28;
		}

		location.left = upperLeftCornerLocation.x + 32;
		location.right = location.left + 28;
		location.top += 28;
		location.bottom += 28;
	}
	location.left = upperLeftCornerLocation.x;
	location.right = location.left + 480;
	location.bottom += 5;
	ttlHold.position = location;
	ttlHold.Create("HOLD", WS_TABSTOP | WS_VISIBLE | BS_AUTOCHECKBOX | WS_CHILD | BS_PUSHLIKE, location, CWnd::FromHandle(windowHandle), ttlHold.ID);
	if (!ttlHold.setToolTip("Press this button to change multiple TTLs simultaneously. Press the button, then change the ttls, then press the button again "
		"to release it. Upon releasing the button, the TTLs will change.", toolTips, master))
	{
		MessageBox(0, "Button Tool tip failed!", 0, 0);
	}
	upperLeftCornerLocation.y = location.bottom;
	return;
}

void TtlSystem::handleTTL_ScriptCommand(std::string command, std::pair<std::string, long> time, std::string name, std::vector<std::pair<unsigned int, unsigned int>>& ttlShadeLocations)
{
	/*

	ExperimentManager::eatComments(&fileStream);
	*/
	if (!this->isValidTTLName(name))
	{
		thrower("ERROR: the name " + name + " is not the name of a ttl!");
		return;
	}
	unsigned int row, collumn;
	int ttlLine = this->getNameIdentifier(name, row, collumn);
	ttlShadeLocations.push_back({ row, collumn });
	if (command == "on:")
	{
		errBox( "Setting On!" );
		this->ttlOn(row, collumn, time);
	}
	else if (command == "off:")
	{
		errBox( "Setting Off!" );
		this->ttlOff(row, collumn, time);
	}
	return;
}

// simple...
int TtlSystem::getNumberOfTTLRows()
{
	return ttlPushControls.size();
}

int TtlSystem::getNumberOfTTLsPerRow()
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

TtlSystem::~TtlSystem(){}

void TtlSystem::handleTTLPress(UINT id)
{
	if (id >= ttlPushControls.front().front().ID && id <= ttlPushControls.back().back().ID)
	{
		// figure out row #
		int row = (id - ttlPushControls.front().front().ID) / ttlPushControls[0].size();
		// figure out collumn #
		int number = (id - ttlPushControls.front().front().ID) % ttlPushControls[0].size();
		// if indeterminante, you can't change it, but that's fine, return true.
		if (ttlShadeStatus[row][number])
		{
			return;
		}
		if (holdStatus == false)
		{
			if (this->ttlStatus[row][number])
			{
				this->forceTTL(row, number, 0);
			}
			else
			{
				this->forceTTL(row, number, 1);
			}
		}
		else
		{
			if (this->ttlHoldStatus[row][number])
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
		return;
	}
	else
	{
		return;
	}
}

// this function handles when the hold button is pressed.
void TtlSystem::handleHoldPress()
{
	if (this->holdStatus == true)
	{
		this->holdStatus = false;
		// make changes
		for (int rowInc = 0; rowInc < ttlHoldStatus.size(); rowInc++)
		{
			for (int numberInc = 0; numberInc < ttlHoldStatus[0].size(); numberInc++)
			{
				if (ttlHoldStatus[rowInc][numberInc])
				{
					ttlPushControls[rowInc][numberInc].SetCheck(BST_CHECKED);					
					ttlStatus[rowInc][numberInc] = true;
					// actually change the ttl.
					this->forceTTL(rowInc, numberInc, 1);
				}
				else
				{
					ttlPushControls[rowInc][numberInc].SetCheck(BST_UNCHECKED);
					ttlStatus[rowInc][numberInc] = false;
					// actually change the ttl.
					this->forceTTL(rowInc, numberInc, 0);
				}
				ttlPushControls[rowInc][numberInc].colorState = 0;
				ttlPushControls[rowInc][numberInc].RedrawWindow();
			}
		}
	}
	else
	{
		this->holdStatus = true;
		ttlHoldStatus = ttlStatus;
	}
	return;
}

void TtlSystem::resetTTLEvents()
{
	this->ttlCommandFormList.clear();
	return;
}

HBRUSH TtlSystem::handleColorMessage(CWnd* window, std::unordered_map<std::string, HBRUSH> brushes, std::unordered_map<std::string, COLORREF> rGBs, CDC* cDC)
{
	
	DWORD controlID = window->GetDlgCtrlID();
	if (controlID >= ttlPushControls.front().front().ID && controlID <= ttlPushControls.back().back().ID)
	{
		// figure out row #
		int row = (controlID - ttlPushControls.front().front().ID) / ttlPushControls[0].size();
		// figure out collumn #
		int number = (controlID - ttlPushControls.front().front().ID) % ttlPushControls[0].size();
		if (this->ttlPushControls[row][number].colorState == -1)
		{
			cDC->SetBkColor(rGBs["Red"]);
			return brushes["Red"];
		}
		else if (this->ttlPushControls[row][number].colorState == 1)
		{
			cDC->SetBkColor(rGBs["Green"]);
			return brushes["Green"];
		}
		else if (this->ttlPushControls[row][number].colorState == 2)
		{
			cDC->SetBkColor(rGBs["White"]);
			return brushes["White"];
		}
		else
		{
			cDC->SetBkColor(rGBs["Medium Grey"]);
			return brushes["Medium Grey"];
		}
	}
	else if (controlID == ttlTitle.ID)
	{
		cDC->SetBkColor(rGBs["Medium Grey"]);
		cDC->SetTextColor(rGBs["Gold"]);
		return brushes["Medium Grey"];
	}
	else if (controlID >= ttlRowLabels.front().ID && controlID <= ttlRowLabels.back().ID)
	{
		cDC->SetBkColor(rGBs["Medium Grey"]);
		cDC->SetTextColor(rGBs["Gold"]);
		return brushes["Medium Grey"];
	}
	else if (controlID >= ttlNumberLabels.front().ID && controlID <= ttlNumberLabels.back().ID)
	{
		cDC->SetBkColor(rGBs["Medium Grey"]);
		cDC->SetTextColor(rGBs["Gold"]);
		return brushes["Medium Grey"];
	}
	else
	{
		return NULL;
	}
}

bool TtlSystem::isValidTTLName( std::string name )
{
	for (int rowInc = 0; rowInc < this->getNumberOfTTLRows(); rowInc++)
	{
		std::string rowStr;
		switch (rowInc)
		{
			case 0: rowStr = "A"; break;
			case 1: rowStr = "B"; break;
			case 2: rowStr = "C"; break;
			case 3: rowStr = "D"; break;
		}
		for (int numberInc = 0; numberInc < this->getNumberOfTTLsPerRow(); numberInc++)
		{
			// check default names
			unsigned int row, number;
			if (name == rowStr + std::to_string( numberInc + 1 ))
			{
				return true;
			}
			else if (this->getNameIdentifier( name, row, number ) != -1)
			{
				return true;
			}
		}
	}
	return false;
}

void TtlSystem::ttlOn(unsigned int row, unsigned int column, std::pair<std::string, long> time)
{
	TTL_CommandForm command;
	// make sure it's either a variable or a number that can be used.
	this->ttlCommandFormList.push_back({ {row, column}, time, true });
	return;
}

void TtlSystem::ttlOff(unsigned int row, unsigned int column, std::pair<std::string, long> time)
{
	// check to make sure either variable or actual value.
	this->ttlCommandFormList.push_back({ {row, column}, time, false });
	return;
}


void TtlSystem::ttlOnDirect( unsigned int row, unsigned int column, long time )
{
	TTL_Command command;
	command.line = { row, column };
	command.time = time;
	command.value = true;
	individualTTL_CommandList.push_back( command );
}


void TtlSystem::ttlOffDirect( unsigned int row, unsigned int column, long time )
{
	TTL_Command command;
	command.line = { row, column };
	command.time = time;
	command.value = false;
	individualTTL_CommandList.push_back( command );
}


void TtlSystem::stopBoard()
{
	dioOutStop( 0 );
	return;
}

double TtlSystem::getClockStatus()
{
	// initialize to zero so that in safemode goes directly to getting tick count.
	int result = 0;
	DIO64STAT stat;
	DWORD availableScans;
	try
	{
		dioOutStatus( 0, availableScans, stat );
	}
	catch ( myException& )
	{
		// get current time in ms...
		// ***NOT SURE*** if this is what I want. The vb6 code used...
		// return = Now * 24 * 60 * 60 * 1000
		return GetTickCount();
	}
		USHORT STUFF;
		return ((long)stat.time / 10000.0);
		// assuming the clock runs at 10 MHz, return in ms.
}

// forceTTL forces the actual ttl to a given value and changes the checkbox status to reflect that.
void TtlSystem::forceTTL(int row, int number, int state)
{
	// change the ttl checkbox.
	if (state == 0)
	{
		this->ttlPushControls[row][number].SetCheck(BST_UNCHECKED);
		this->ttlStatus[row][number] = false;
	}
	else
	{
		this->ttlPushControls[row][number].SetCheck(BST_CHECKED);
		this->ttlStatus[row][number] = true;
	}
	
	// change the output.
	int result = 0;
	std::array<std::bitset<16>, 4> ttlBits;
	for (int rowInc = 0; rowInc < 4; rowInc++)
	{
		for (int numberInc = 0; numberInc < 16; numberInc++)
		{
			if (this->ttlStatus[rowInc][numberInc])
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

	return;
}


void TtlSystem::setName(unsigned int row, unsigned int number, std::string name, std::vector<CToolTipCtrl*>& toolTips, MasterWindow* master)
{
	if (name == "")
	{
		// no empty names allowed.
		return;
	}
	this->ttlNames[row][number] = name;
	this->ttlPushControls[row][number].setToolTip(name, toolTips, master);
	return;
}

int TtlSystem::getNameIdentifier(std::string name, unsigned int& row, unsigned int& number)
{
	
	for (int rowInc = 0; rowInc < this->ttlNames.size(); rowInc++)
	{
		std::string rowName;
		switch (rowInc)
		{
			case 0: rowName = "A"; break;
			case 1: rowName = "B"; break;
			case 2: rowName = "C"; break;
			case 3: rowName = "D"; break;
		}
		for (unsigned int numberInc = 0; numberInc < this->ttlNames[rowInc].size(); numberInc++)
		{
			// check the names array.
			if (this->ttlNames[rowInc][numberInc] == name)
			{
				row = rowInc;
				number = numberInc;
				return rowInc * this->ttlNames[rowInc].size() + numberInc;
			}
			// check standard names which are always acceptable.
			if (name == rowName + std::to_string(numberInc))
			{
				row = rowInc;
				number = numberInc;
				return rowInc * this->ttlNames[rowInc].size() + numberInc;
			}
		}
	}
	// else not a name.
	return -1;
}


void TtlSystem::writeData()
{
	WORD temp[4] = { -1, -1, -1, -1 };
	double scan = 10000000;
	DWORD availableScans = 0;
	std::vector<WORD> arrayOfAllData( finalFormattedCommandForDIO.size() * 6 );
	DIO64STAT status;
	status.AIControl = 0;

	// Write to DIO board
	try
	{
		dioOutStop( 0 );
	}
	// literally.. just try.
	catch ( myException& ) {} 
	// scan rate = 10 MHz
	dioOutConfig( 0, 0, temp, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, scan );
	dioOutStatus( 0, availableScans, status );

	int count = 0;
	for (auto& element : arrayOfAllData)
	{
		// concatenate all the data at once.
		element = finalFormattedCommandForDIO[count / 6][count % 6];
		errBox( std::to_string( element ) );
		count++;
	}
	// now arrayOfAllData contains all the experiment data.
	dioOutWrite( 0, arrayOfAllData.data(), finalFormattedCommandForDIO.size(), status );
	return;
}


std::string TtlSystem::getName(unsigned int row, unsigned int number)
{
	return this->ttlNames[row][number];
}


bool TtlSystem::getTTL_Status(int row, int number)
{
	return this->ttlStatus[row][number];
}


// waits a time in ms, using the DIO clock
void TtlSystem::wait(double time)
{
	double startTime;
	int dummy;
	//'clockstatus function reads the DIO clock, units are ms
	startTime = getClockStatus();
	dummy = 0;
	while (time - abs(getClockStatus() - startTime) > 110 && getClockStatus() - startTime != 0)
	{
		// in... ms??? this seems too long.
		Sleep(100);
	}
	// check faster closer to the stop.
	while (time - abs(getClockStatus() - startTime) > 0.1 && getClockStatus() - startTime != 0){/*immediately check again*/}
	return;
}

// uses the last time of the ttl trigger to wait until the experiment is finished.
void TtlSystem::waitTillFinished()
{
	double totalTime = (this->finalFormattedCommandForDIO.back()[0] + 65535*this->finalFormattedCommandForDIO.back()[1])/10000 + 1;
	this->wait(totalTime);
	this->stopBoard();
	return;
}


void TtlSystem::interpretKey(std::unordered_map<std::string, std::vector<double>> key, unsigned int variationNum)
{
	this->individualTTL_CommandList.clear();

	for (int commandInc = 0; commandInc < this->ttlCommandFormList.size(); commandInc++)
	{
		TTL_Command tempCommand;
		tempCommand.line = this->ttlCommandFormList[commandInc].line;
		tempCommand.value = this->ttlCommandFormList[commandInc].value;
		errBox( " tempCommand.line = " + std::to_string( tempCommand.line.first )  + std::to_string( tempCommand.line.second ) );
		errBox( " tempCommand.value = " + std::to_string( tempCommand.value ) );
		// if no variable...
		if (this->ttlCommandFormList[commandInc].time.first == "")
		{
			tempCommand.time = this->ttlCommandFormList[commandInc].time.second;
		}
		else
		{
			tempCommand.time = key[this->ttlCommandFormList[commandInc].time.first][variationNum] + this->ttlCommandFormList[commandInc].time.second;
		}
		this->individualTTL_CommandList.push_back(tempCommand);
	}
	return;
}


void TtlSystem::analyzeCommandList()
{
	// each element of this is a different time (the double), and associated with each time is a vector which locates which commands were at this time, for
	// ease of retrieving all of the values in a moment.
	std::vector<std::pair<double, std::vector<unsigned int>>> timeOrganizer;
	/// organize all of the commands.
	for (int commandInc = 0; commandInc < individualTTL_CommandList.size(); commandInc++)
	{
		// if it stays -1 after the following, it's a new time.
		int timeIndex = -1;
		for (int timeInc = 0; timeInc < timeOrganizer.size(); timeInc++)
		{
			if (individualTTL_CommandList[commandInc].time == timeOrganizer[timeInc].first)
			{
				timeIndex = timeInc;
				break;
			}
		}
		if (timeIndex == -1)
		{
			timeOrganizer.push_back({ individualTTL_CommandList[commandInc].time, std::vector<unsigned int>(commandInc) });
		}
		else
		{
			timeOrganizer[timeIndex].second.push_back(commandInc);
		}
	}
	// this one will have all of the times ordered in sequence, in case the other doesn't.
	std::vector<std::pair<double, std::vector<unsigned int>>> orderedOrganizer;
	/// order the commands.
	while (timeOrganizer.size() != 0)
	{
		// find the lowest value time
		int lowestTime = MAXINT32;
		unsigned int index;
		for (int commandInc = 0; commandInc < timeOrganizer.size(); commandInc++)
		{
			if (timeOrganizer[commandInc].first < lowestTime)
			{
				lowestTime = timeOrganizer[commandInc].first;
				index = commandInc;
			}
		}
		orderedOrganizer.push_back(timeOrganizer[index]);
		timeOrganizer.erase(timeOrganizer.begin() + index);
	}
	/// now figure out the state of the system at each time.
	this->fullCommandList.clear();
	// give it the initial status.
	this->fullCommandList.push_back({0, this->ttlStatus});
	errBox( "orderedOrganizer.size() = " + std::to_string( orderedOrganizer.size() ) );
	if (orderedOrganizer.size() == 0)
	{
		thrower("ERROR: no ttl commands...?");
		return;
	}
	if (orderedOrganizer[0].first == 0)
	{
		// handle the zero case specially.
		for (int zeroInc = 0; zeroInc < orderedOrganizer[0].second.size(); zeroInc++)
		{
			// make sure to address he correct ttl. the ttl location is located in individuaTTL_CommandList but you need to make sure you access the correct 
			// command.
			unsigned int row =		individualTTL_CommandList[orderedOrganizer[0].second[zeroInc]].line.first;
			unsigned int column =	individualTTL_CommandList[orderedOrganizer[0].second[zeroInc]].line.second;
			this->fullCommandList[0].ttlStatus[row][column]	= individualTTL_CommandList[orderedOrganizer[0].second[zeroInc]].value;
			//... setting it to the command's state.
			// phew that's a long call for an =...
		}
	}
	
	for (int commandInc = 1; commandInc < orderedOrganizer.size(); commandInc++)
	{
		// first copy the last set so that things that weren't changed remain unchanged.
		this->fullCommandList.push_back(this->fullCommandList.back());
		fullCommandList.back().time = orderedOrganizer[commandInc].first;
		for (int zeroInc = 0; zeroInc < orderedOrganizer[commandInc].second.size(); zeroInc++)
		{
			// see description of this command above... update everything that changed at this time.
			this->fullCommandList.back().ttlStatus[individualTTL_CommandList[orderedOrganizer[commandInc].second[zeroInc]].line.first][individualTTL_CommandList[orderedOrganizer[commandInc].second[zeroInc]].line.second]
				= individualTTL_CommandList[orderedOrganizer[commandInc].second[zeroInc]].value;
		}
	}
	// phew.
	return;
}

void TtlSystem::convertToFinalFormat()
{
	this->finalFormattedCommandForDIO.clear();
	errBox( "fullCommandList.size() = " + std::to_string( fullCommandList.size() ) );
	// do bit arithmetic.
	for (int timeInc = 0; timeInc < this->fullCommandList.size(); timeInc++)
	{
		WORD lowordTime;
		WORD hiwordTime;
		lowordTime = fullCommandList[timeInc].time % 65535;
		hiwordTime = fullCommandList[timeInc].time / 65535;
		// each major index is a row (A, B, C, D), each minor index is a ttl state (0, 1) in that row.
		std::array<std::bitset<16>, 4> ttlBits;
		for (int rowInc = 0; rowInc < 4; rowInc++)
		{
			for (int numberInc = 0; numberInc < 16; numberInc++)
			{
				if (this->fullCommandList[timeInc].ttlStatus[rowInc][numberInc])
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
		// I need to put it as an int (just because I'm not actually sure how the bitset gets stored... it'd probably work just passing the address of the 
		// bitsets, but I'm sure this will work so whatever.)
		std::array<unsigned short, 6> tempCommand;
		tempCommand[0] = lowordTime;
		tempCommand[1] = hiwordTime;
		tempCommand[2] = static_cast <unsigned short>(ttlBits[0].to_ulong());
		tempCommand[3] = static_cast <unsigned short>(ttlBits[1].to_ulong());
		tempCommand[4] = static_cast <unsigned short>(ttlBits[2].to_ulong());
		tempCommand[5] = static_cast <unsigned short>(ttlBits[3].to_ulong());
		this->finalFormattedCommandForDIO.push_back(tempCommand);
	}
	return;
}


std::string TtlSystem::getErrorMessage(int errorCode)
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
