// created by Mark O. Brown
// This file includes the decalarations of all of my external (global) variables. Declaring them here is a nice way of including all of the extenals in a file
// without making it look like a normal variable.
#include "stdafx.h"
#include "externals.h"
#include "Windows.h"
#include "constants.h"
#include "ProfileSystem.h"
#include "ParameterSystem.h"
#include "DebugOptionsControl.h"

std::vector<std::string> WAVEFORM_NAME_FILES = std::vector<std::string>(4 * MAX_NIAWG_SIGNALS);
std::vector<std::string> WAVEFORM_TYPE_FOLDERS = std::vector<std::string>(4 * MAX_NIAWG_SIGNALS);

CWnd* eMainWindowHwnd;
std::unordered_map<std::string, COLORREF> _myRGBs = {
	{ "Slate Grey", RGB ( 101, 115, 126 ) },
	{ "Black", RGB ( 0, 0, 0 ) },
	{ "Dark Grey", RGB ( 18, 18, 18 ) },
	{ "Medium Grey", RGB ( 25, 25, 25 ) },
	{ "Light Grey", RGB ( 40, 40, 40 ) },
	{ "Light Grey 2", RGB ( 60, 60, 60 ) },
	{ "Near-White", RGB ( 200, 200, 200 ) },
	{ "White", RGB ( 255, 255, 255 ) },
	{ "Light Green", RGB ( 163, 190, 140 ) },
	{ "Pale Pink", RGB ( 180, 142, 173 ) },
	{ "Musky Red", RGB ( 191, 97, 106 ) },
	// this "base04", while not listed on the solarized web site, is used by the visual studio solarized for edit area.
	// it's a nice darker color that matches the solarized pallete.
	{ "Solarized Base04", RGB ( 0, 30, 38 ) },
	{ "Solarized Base03", RGB ( 0, 43, 54 ) },
	{ "Solarized Base02", RGB ( 7, 54, 66 ) },
	{ "Solarized Base01", RGB ( 88, 110, 11 ) },
	{ "Solarized Base00", RGB ( 101, 123, 131 ) },
	{ "Solarized Base0", RGB ( 131, 148, 150 ) },
	{ "Solarized Base1", RGB ( 101, 123, 131 ) },
	{ "Solarized Base2", RGB ( 238, 232, 213 ) },
	{ "Solarized Base3", RGB ( 253, 246, 227 ) },
	{ "Solarized Red", RGB ( 220, 50, 47 ) },
	{ "Solarized Violet", RGB ( 108, 113, 196 ) },
	{ "Solarized Cyan", RGB ( 42, 161, 152 ) },
	{ "Solarized Green", RGB ( 133, 153, 0 ) },
	{ "Solarized Blue", RGB ( 38, 139, 210 ) },
	{ "Solarized Magenta", RGB ( 211, 54, 130 ) },
	{ "Solarized Orange", RGB ( 203, 75, 22 ) },
	{ "Solarized Yellow", RGB ( 181, 137, 0 ) },
	{ "Slate Green", RGB ( 23, 84, 81 ) },
	{ "Dark Grey Red", RGB ( 40, 20, 20 ) },
	{ "Green", RGB ( 50, 200, 50 ) },
	{ "Red", RGB ( 200, 50, 50 ) },
	{ "Blue", RGB ( 50, 50, 200 ) },
	{ "Gold", RGB ( 218, 165, 32 ) },
	{ "Light Red", RGB ( 255, 100, 100 ) },
	{ "Dark Red", RGB ( 150, 0, 0 ) },
	{ "Light Blue", RGB ( 100, 100, 255 ) },
	{ "Forest Green", RGB ( 34, 139, 34 ) },
	{ "Dark Green", RGB ( 0, 50, 0 ) },
	{ "Dull Red", RGB ( 107, 35, 35 ) },
	{ "Dark Lavender", RGB ( 100, 100, 205 ) },
	{ "Teal", RGB ( 0, 255, 255 ) },
	{ "Tan", RGB ( 210, 180, 140 ) },
	{ "Purple", RGB ( 147, 112, 219 ) },
	{ "Orange", RGB ( 255, 165, 0 ) },
	{ "Brown", RGB ( 139, 69, 19 ) },
	{ "Dark Blue", RGB ( 0, 0, 75 ) }
};

std::unordered_map<std::string, CBrush*> _myBrushes;

void initMyColors( )
{
	static bool __initBrushesFlag = false;
	if ( __initBrushesFlag )
	{
		return;
	}
	/// Light Solarized Theme
	/*
	std::string staticBkgd = "Light Grey", interactableBkgd = "Solarized Base2";
	_myRGBs[ "Text" ] = _myRGBs[ "Solarized Base0" ];
	_myRGBs[ "Main-Bkgd" ] = _myRGBs[ "Solarized Base3" ];
	_myRGBs[ "MainWin-Text" ] = _myRGBs[ "Solarized Blue" ];
	_myRGBs[ "ScriptWin-Text" ] = _myRGBs[ "Solarized Red" ];
	_myRGBs[ "AndorWin-Text" ] = _myRGBs[ "Solarized Green" ];
	_myRGBs[ "AuxWin-Text" ] = _myRGBs[ "Solarized Yellow" ];
	_myRGBs[ "BasWin-Text" ] = _myRGBs[ "Solarized Orange" ];
	_myRGBs[ "Button-Color" ] = _myRGBs[ "Solarized Base2" ];
	_myRGBs[ "Text-Emph" ] = _myRGBs[ "Solarized Base0" ];
	_myRGBs[ "Static-Bkgd" ] = _myRGBs[ staticBkgd ];
	_myRGBs[ "Interactable-Bkgd" ] = _myRGBs[ interactableBkgd ];
	*/
	/// Dark Solarized Theme
	/*	
	std::string staticBkgd = "Medium Grey", interactableBkgd = "Solarized Base02";
	_myRGBs[ "Text" ] = _myRGBs[ "Solarized Base0" ];
	_myBrushes[ "Main-Bkgd" ] = mainBrushes[ "Solarized Base04" ];
	_myRGBs[ "MainWin-Text" ] = _myRGBs[ "Solarized Blue" ];
	_myRGBs[ "ScriptWin-Text" ] = _myRGBs[ "Solarized Red" ];
	_myRGBs[ "AndorWin-Text" ] = _myRGBs[ "Solarized Green" ];
	_myRGBs[ "AuxWin-Text" ] = _myRGBs[ "Solarized Yellow" ];
	_myRGBs[ "BasWin-Text" ] = _myRGBs[ "Solarized Orange" ];

	_myRGBs[ "Static-Bkgd" ] = _myRGBs[ staticBkgd ];
	_myRGBs[ "Interactable-Bkgd" ] = _myRGBs[ interactableBkgd ];
	_myBrushes[ "Static-Bkgd" ] = mainBrushes[ staticBkgd ];
	_myBrushes[ "Interactable-Bkgd" ] = mainBrushes[ interactableBkgd ];
	*/
	/// Dark Theme
	std::string coloredText = "Light Blue";
	_myRGBs[ "Text" ] = _myRGBs[ "Near-White" ];
	_myRGBs[ "Text-Emph" ] = _myRGBs[ coloredText ];
	_myRGBs[ "Main-Bkgd" ] = _myRGBs[ "Dark Grey" ];
	_myRGBs[ "MainWin-Text" ] = _myRGBs[ coloredText ];
	_myRGBs[ "ScriptWin-Text" ] = _myRGBs[ coloredText ];
	_myRGBs[ "AndorWin-Text" ] = _myRGBs[ coloredText ];
	_myRGBs[ "AuxWin-Text" ] = _myRGBs[ coloredText ];
	_myRGBs[ "BasWin-Text" ] = _myRGBs[ coloredText ];
	_myRGBs[ "Disabled-Bkgd" ] = _myRGBs[ "Dark Grey Red" ];
	_myRGBs[ "Static-Bkgd" ] = _myRGBs[ "Medium Grey" ];
	_myRGBs[ "Button-Color" ] = _myRGBs[ "Dark Grey" ];
	_myRGBs[ "Interactable-Bkgd" ] = _myRGBs[ "Light Grey" ];
	// create corresponding brushes
	for ( auto& rgb : _myRGBs )
	{
		(_myBrushes[rgb.first] = new CBrush)->CreateSolidBrush ( rgb.second );
	}
	__initBrushesFlag = true;
};


/// some globals for niawg stuff, only for niawg stuff so I keep it here...?
const std::array<int, 2> AXES = { Axes::Vertical, Axes::Horizontal };
// the following is used to receive the index of whatever axis is not your current axis.
const std::array<int, 2> ALT_AXES = { Axes::Horizontal, Axes::Vertical };
const std::array<std::string, 2> AXES_NAMES = { "Vertical", "Horizontal" };

bool eWaitError = false;
bool eAbortNiawgFlag = false;

// thread messages
// register messages for main window.

HANDLE eWaitingForNIAWGEvent;
HANDLE eNIAWGWaitThreadHandle;

