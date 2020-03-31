// created by Mark O. Brown
#include "stdafx.h"
#include "CameraImageDimensions.h"
#include "PrimaryWindows/AndorWindow.h"
#include "PrimaryWindows/MainWindow.h"
#include "CameraSettingsControl.h"
#include <boost/lexical_cast.hpp>

ImageDimsControl::ImageDimsControl()
{
	isReady = false;
}


void ImageDimsControl::initialize( POINT& pos, CWnd* parent, bool isTriggerModeSensitive, int& id )
{
	leftText.sPos = { pos.x, pos.y, pos.x += 80, pos.y + 25 };
	leftText.Create( "Left", NORM_STATIC_OPTIONS, leftText.sPos, parent, id++);
	//
	rightText.sPos = { pos.x, pos.y, pos.x += 80, pos.y + 25 };
	rightText.Create( "Right (/512)", NORM_STATIC_OPTIONS, rightText.sPos, parent, id++ );
	//
	horBinningText.sPos = { pos.x, pos.y, pos.x += 80, pos.y + 25 };
	horBinningText.Create( "H. Bin", NORM_STATIC_OPTIONS, horBinningText.sPos, parent, id++);
	//
	bottomLabel.sPos = { pos.x, pos.y, pos.x += 80, pos.y + 25 };
	bottomLabel.Create ("Bottom (/512)", NORM_STATIC_OPTIONS, bottomLabel.sPos, parent, id++);
	//
	topLabel.sPos = { pos.x, pos.y, pos.x += 80, pos.y + 25 };
	topLabel.Create ("Top", NORM_STATIC_OPTIONS, topLabel.sPos, parent, id++);
	//
	vertBinningText.sPos = { pos.x, pos.y, pos.x += 80, pos.y += 25 };
	vertBinningText.Create ("V. Bin", NORM_STATIC_OPTIONS, vertBinningText.sPos, parent, id++);
	pos.x -= 480;
	//
	leftEdit.sPos = { pos.x, pos.y, pos.x += 80, pos.y + 25 };
	leftEdit.Create( NORM_EDIT_OPTIONS | ES_CENTER | ES_MULTILINE | ES_WANTRETURN, leftEdit.sPos, parent, IDC_IMAGE_DIMS_START );
	leftEdit.SetWindowTextA( "1" );
	//
	rightEdit.sPos = { pos.x, pos.y, pos.x += 80, pos.y + 25 };
	rightEdit.Create( NORM_EDIT_OPTIONS | ES_CENTER | ES_MULTILINE | ES_WANTRETURN, rightEdit.sPos, parent, IDC_IMAGE_DIMS_START+1);
	rightEdit.SetWindowTextA( "50" );
	//
	horBinningEdit.sPos = { pos.x, pos.y, pos.x += 80, pos.y + 25 };
	horBinningEdit.Create( NORM_EDIT_OPTIONS | ES_CENTER | ES_MULTILINE | ES_WANTRETURN, horBinningEdit.sPos,
						   parent, IDC_IMAGE_DIMS_START+2 );
	horBinningEdit.SetWindowTextA( "1" );
	//
	bottomEdit.sPos = { pos.x, pos.y, pos.x += 80, pos.y + 25 };
	bottomEdit.Create( NORM_EDIT_OPTIONS | ES_CENTER | ES_MULTILINE | ES_WANTRETURN, bottomEdit.sPos, parent, IDC_IMAGE_DIMS_START +3);
	bottomEdit.SetWindowTextA( "1" );
	//
	topEdit.sPos = { pos.x, pos.y, pos.x += 80, pos.y + 25 };
	topEdit.Create( NORM_EDIT_OPTIONS | ES_CENTER | ES_MULTILINE | ES_WANTRETURN, topEdit.sPos, parent, IDC_IMAGE_DIMS_START + 4);
	topEdit.SetWindowTextA( "50" );
	//
	vertBinningEdit.sPos = { pos.x, pos.y, pos.x += 80, pos.y += 25 };
	vertBinningEdit.Create( NORM_EDIT_OPTIONS | ES_CENTER | ES_MULTILINE | ES_WANTRETURN, vertBinningEdit.sPos, parent, IDC_IMAGE_DIMS_START +5);
	vertBinningEdit.SetWindowTextA( "1" );
	pos.x -= 480;
}


void ImageDimsControl::handleSave(ConfigStream& saveFile )
{
	saveFile << "\nCAMERA_IMAGE_DIMENSIONS"
			 << "\n/*Left:*/ " << currentImageParameters.left
			 << "\n/*Right:*/ " << currentImageParameters.right
			 << "\n/*H-Bin:*/ " << currentImageParameters.horizontalBinning
			 << "\n/*Bottom:*/ " << currentImageParameters.bottom
			 << "\n/*Top:*/ " << currentImageParameters.top
			 << "\n/*V-Bin:*/ " << currentImageParameters.verticalBinning
			 << "\nEND_CAMERA_IMAGE_DIMENSIONS\n";
}


imageParameters ImageDimsControl::getImageDimSettingsFromConfig (ConfigStream& configFile, Version ver )
{
	imageParameters params;
	configFile >> params.left;
	configFile >> params.right;
	configFile >> params.horizontalBinning;
	configFile >> params.bottom;
	configFile >> params.top;
	configFile >> params.verticalBinning;
	return params;
}

void ImageDimsControl::handleOpen(ConfigStream& openFile, Version ver )
{
	ProfileSystem::checkDelimiterLine( openFile, "CAMERA_IMAGE_DIMENSIONS" );
	imageParameters params = getImageDimSettingsFromConfig ( openFile, ver );
	setImageParametersFromInput( params );
}


imageParameters ImageDimsControl::readImageParameters()
{
	// in case called before initialized
	if (!leftEdit)
	{
		return currentImageParameters;
	}
	// set all of the image parameters
	CString tempStr;
	leftEdit.GetWindowTextA( tempStr );
	try
	{
		currentImageParameters.left = boost::lexical_cast<int>( str( tempStr ) );
	}
	catch ( boost::bad_lexical_cast&)
	{
		isReady = false;
		throwNested ( "Left border argument not an integer!\r\n" );
	}
	leftEdit.RedrawWindow();
	rightEdit.GetWindowTextA( tempStr );
	try
	{
		currentImageParameters.right = boost::lexical_cast<int>( str( tempStr ) );
	}
	catch ( boost::bad_lexical_cast&)
	{
		isReady = false;
		throwNested ( "Right border argument not an integer!\r\n" );
	}
	rightEdit.RedrawWindow();
	//
	bottomEdit.GetWindowTextA( tempStr );
	try
	{
		currentImageParameters.bottom = boost::lexical_cast<int>( str( tempStr ) );
	}
	catch ( boost::bad_lexical_cast&)
	{
		isReady = false;
		throwNested ( "Top border argument not an integer!\r\n" );
	}
	bottomEdit.RedrawWindow();
	//
	topEdit.GetWindowTextA( tempStr );
	try
	{
		currentImageParameters.top = boost::lexical_cast<int>( str( tempStr ) );
	}
	catch ( boost::bad_lexical_cast&)
	{
		isReady = false;
		throwNested ( "Bottom border argument not an integer!\r\n" );
	}
	topEdit.RedrawWindow();
	horBinningEdit.GetWindowTextA( tempStr );
	try
	{
		currentImageParameters.horizontalBinning = boost::lexical_cast<int>( str( tempStr ) );
	}
	catch ( boost::bad_lexical_cast&)
	{
		isReady = false;
		throwNested ( "Horizontal binning argument not an integer!\r\n" );
	}
	horBinningEdit.RedrawWindow();
	vertBinningEdit.GetWindowTextA( tempStr );
	try
	{
		currentImageParameters.verticalBinning = boost::lexical_cast<int>( str( tempStr ) );
	}
	catch ( boost::bad_lexical_cast&)
	{
		isReady = false;
		throwNested ( "Vertical binning argument not an integer!\r\n" );
	}
	vertBinningEdit.RedrawWindow();

	// Check Image parameters
	try
	{
		currentImageParameters.checkConsistency ( );
	}
	catch ( Error& )
	{
		isReady = false;
		throw;
	}

	// made it through successfully.
	isReady = true;
	return currentImageParameters;
}


/*
 * I forget why I needed a second function for this.
 */
void ImageDimsControl::setImageParametersFromInput( imageParameters param )
{
	// set all of the image parameters
	currentImageParameters.left = param.left;
	leftEdit.SetWindowText( cstr( currentImageParameters.left ) );
	currentImageParameters.right = param.right;
	rightEdit.SetWindowText( cstr( currentImageParameters.right ) );
	currentImageParameters.bottom = param.bottom;
	bottomEdit.SetWindowText( cstr( currentImageParameters.bottom ) );
	currentImageParameters.top = param.top;
	topEdit.SetWindowText( cstr( currentImageParameters.top ) );
	currentImageParameters.horizontalBinning = param.horizontalBinning;
	horBinningEdit.SetWindowText( cstr( currentImageParameters.horizontalBinning ) );
	currentImageParameters.verticalBinning = param.verticalBinning;
	vertBinningEdit.SetWindowText( cstr( currentImageParameters.verticalBinning ) );
	// Check Image parameters
	try
	{
		currentImageParameters.checkConsistency( );
	}
	catch ( Error )
	{
		isReady = false;
		throw;
	}
	// made it through successfully.
	isReady = true;
}


bool ImageDimsControl::checkReady()
{
	if (isReady)
	{
		return true;
	}
	else
	{
		return false;
	}
}


imageParameters ImageDimsControl::getImageParameters()
{
	return currentImageParameters;
}


HBRUSH ImageDimsControl::colorEdits( HWND window, UINT message, WPARAM wParam, LPARAM lParam, MainWindow* mainWin )
{
	DWORD controlID = GetDlgCtrlID( (HWND)lParam );
	HDC hdcStatic = (HDC)wParam;
	imageParameters currentImageParameters = { 0,0,0,0,0,0 };

	if (controlID == topEdit.GetDlgCtrlID())
	{
		SetTextColor( hdcStatic, RGB( 255, 255, 255 ) );
		CString textEdit;
		topEdit.GetWindowTextA( textEdit );
		int bottom;
		try
		{
			bottom = boost::lexical_cast<int>( str( textEdit ) );
			if (bottom == currentImageParameters.top)
			{
				// good.
				SetTextColor( hdcStatic, RGB( 255, 255, 255 ) );
				SetBkColor( hdcStatic, RGB( 100, 110, 100 ) );
				// catch change of color and redraw window.
				if (topEdit.colorState != 0)
				{
					topEdit.colorState = 0;
					topEdit.RedrawWindow();
				}
				return *_myBrushes["Grey Green"];
			}
		}
		catch ( boost::bad_lexical_cast&)
		{
			// don't do anything with it.
		}
		SetTextColor( hdcStatic, RGB( 255, 255, 255 ) );
		SetBkColor( hdcStatic, RGB( 150, 100, 100 ) );
		// catch change of color and redraw window.
		if (topEdit.colorState != 1)
		{
			topEdit.colorState = 1;
			topEdit.RedrawWindow();
		}
		return *_myBrushes["Grey Red"];
	}
	else if (controlID == bottomEdit.GetDlgCtrlID())
	{
		SetTextColor( hdcStatic, RGB( 255, 255, 255 ) );
		CString textEdit;
		bottomEdit.GetWindowTextA( textEdit );
		int top;
		try
		{
			top = boost::lexical_cast<int>( str( textEdit ) );
			if (top == currentImageParameters.bottom)
			{
				// good.
				SetTextColor( hdcStatic, RGB( 255, 255, 255 ) );
				SetBkColor( hdcStatic, RGB( 100, 110, 100 ) );
				// catch change of color and redraw window.
				if (bottomEdit.colorState != 0)
				{
					bottomEdit.colorState = 0;
					bottomEdit.RedrawWindow();
				}
				return *_myBrushes["Grey Green"];
			}
		}
		catch ( boost::bad_lexical_cast&)
		{
			// don't do anything with it.
		}
		SetTextColor( hdcStatic, RGB( 255, 255, 255 ) );
		SetBkColor( hdcStatic, RGB( 150, 100, 100 ) );
		// catch change of color and redraw window.
		if (bottomEdit.colorState != 1)
		{
			bottomEdit.colorState = 1;
			bottomEdit.RedrawWindow();
		}
		return *_myBrushes["Grey Red"];
	}
	else if (controlID == vertBinningEdit.GetDlgCtrlID())
	{
		SetTextColor( hdcStatic, RGB( 255, 255, 255 ) );
		CString textEdit;
		vertBinningEdit.GetWindowTextA( textEdit );
		int verticalBin;
		try
		{
			verticalBin = boost::lexical_cast<int>( str( textEdit ) );
			if (verticalBin == currentImageParameters.verticalBinning)
			{
				// good.
				SetTextColor( hdcStatic, RGB( 255, 255, 255 ) );
				SetBkColor( hdcStatic, RGB( 100, 110, 100 ) );
				// catch change of color and redraw window.
				if (vertBinningEdit.colorState != 0)
				{
					vertBinningEdit.colorState = 0;
					vertBinningEdit.RedrawWindow();
				}
				return *_myBrushes["Grey Green"];
			}
		}
		catch ( boost::bad_lexical_cast&)
		{
			// don't do anything with it.
		}
		SetTextColor( hdcStatic, RGB( 255, 255, 255 ) );
		SetBkColor( hdcStatic, RGB( 150, 100, 100 ) );
		// catch change of color and redraw window.
		if (vertBinningEdit.colorState != 1)
		{
			vertBinningEdit.colorState = 1;
			vertBinningEdit.RedrawWindow();
		}
		return *_myBrushes["Grey Red"];
	}
	else if (controlID == leftEdit.GetDlgCtrlID())
	{
		SetTextColor( hdcStatic, RGB( 255, 255, 255 ) );
		CString textEdit;
		leftEdit.GetWindowTextA( textEdit );
		int leftSide;
		try
		{
			leftSide = boost::lexical_cast<int>( str( textEdit ) );
			if (leftSide == currentImageParameters.left)
			{
				// good.
				SetTextColor( hdcStatic, RGB( 255, 255, 255 ) );
				SetBkColor( hdcStatic, RGB( 100, 110, 100 ) );
				// catch change of color and redraw window.
				if (leftEdit.colorState != 0)
				{
					leftEdit.colorState = 0;
					leftEdit.RedrawWindow();
				}
				return *_myBrushes["Grey Green"];
			}
		}
		catch ( boost::bad_lexical_cast&)
		{
			// don't do anything with it.
		}
		SetTextColor( hdcStatic, RGB( 255, 255, 255 ) );
		SetBkColor( hdcStatic, RGB( 150, 100, 100 ) );
		// catch change of color and redraw window.
		if (leftEdit.colorState != 1)
		{
			leftEdit.colorState = 1;
			leftEdit.RedrawWindow();
		}
		return *_myBrushes["Grey Red"];
	}
	else if (controlID == rightEdit.GetDlgCtrlID())
	{
		SetTextColor( hdcStatic, RGB( 255, 255, 255 ) );
		CString textEdit;
		rightEdit.GetWindowTextA( textEdit );
		int rightSide;
		try
		{
			rightSide = boost::lexical_cast<int>( str( textEdit ) );
			if (rightSide == currentImageParameters.right)
			{
				// good.
				SetTextColor( hdcStatic, RGB( 255, 255, 255 ) );
				SetBkColor( hdcStatic, RGB( 100, 110, 100 ) );
				// catch change of color and redraw window.
				if (rightEdit.colorState != 0)
				{
					rightEdit.colorState = 0;
					rightEdit.RedrawWindow();
				}
				return *_myBrushes["Grey Green"];
			}
		}
		catch ( boost::bad_lexical_cast&)
		{
			// don't do anything with it.
		}
		SetTextColor( hdcStatic, RGB( 255, 255, 255 ) );
		SetBkColor( hdcStatic, RGB( 150, 100, 100 ) );
		// catch change of color and redraw window.
		if (rightEdit.colorState != 1)
		{
			rightEdit.colorState = 1;
			rightEdit.RedrawWindow();
		}
		return *_myBrushes["Grey Red"];
	}
	else if (controlID == horBinningEdit.GetDlgCtrlID())
	{
		SetTextColor( hdcStatic, RGB( 255, 255, 255 ) );
		CString textEdit;
		horBinningEdit.GetWindowTextA( textEdit );
		int horizontalBin;
		try
		{
			horizontalBin = boost::lexical_cast<int>( str( textEdit ) );
			if (horizontalBin == currentImageParameters.horizontalBinning)
			{
				// good.
				SetTextColor( hdcStatic, RGB( 255, 255, 255 ) );
				SetBkColor( hdcStatic, RGB( 100, 110, 100 ) );
				// catch change of color and redraw window.
				if (horBinningEdit.colorState != 0)
				{
					horBinningEdit.colorState = 0;
					horBinningEdit.RedrawWindow();
				}
				return *_myBrushes["Grey Green"];
			}
		}
		catch ( boost::bad_lexical_cast&)
		{
			// don't do anything with it.
		}
		SetTextColor( hdcStatic, RGB( 255, 255, 255 ) );
		SetBkColor( hdcStatic, RGB( 150, 100, 100 ) );
		// catch change of color and redraw window.
		if (horBinningEdit.colorState != 1)
		{
			horBinningEdit.colorState = 1;
			horBinningEdit.RedrawWindow();
		}
		return *_myBrushes["Grey Red"];
	}
	return FALSE;
}


void ImageDimsControl::rearrange( int width, int height, fontMap fonts )
{
	leftText.rearrange( width, height, fonts );
	rightText.rearrange(width, height, fonts );
	horBinningText.rearrange( width, height, fonts );
	bottomLabel.rearrange( width, height, fonts );
	topLabel.rearrange( width, height, fonts );
	vertBinningText.rearrange( width, height, fonts );
	leftEdit.rearrange( width, height, fonts );
	rightEdit.rearrange( width, height, fonts );
	horBinningEdit.rearrange( width, height, fonts );
	bottomEdit.rearrange( width, height, fonts );
	topEdit.rearrange( width, height, fonts );
	vertBinningEdit.rearrange( width, height, fonts );
}

