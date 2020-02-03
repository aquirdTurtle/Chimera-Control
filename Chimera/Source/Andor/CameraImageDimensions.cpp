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


void ImageDimsControl::initialize( cameraPositions& pos, CWnd* parent, bool isTriggerModeSensitive, int& id )
{
	//
	leftText.seriesPos = { pos.seriesPos.x, pos.seriesPos.y, pos.seriesPos.x + 160, pos.seriesPos.y + 25 };
	leftText.amPos = { pos.amPos.x, pos.amPos.y, pos.amPos.x + 160, pos.amPos.y + 25 };
	leftText.videoPos = { pos.videoPos.x, pos.videoPos.y, pos.videoPos.x + 160, pos.videoPos.y + 25 };
	leftText.Create( "Left", NORM_STATIC_OPTIONS, leftText.seriesPos, parent, id++);
	//
	rightText.seriesPos = { pos.seriesPos.x + 160, pos.seriesPos.y, pos.seriesPos.x + 320, pos.seriesPos.y + 25 };
	rightText.amPos = { pos.amPos.x + 160, pos.amPos.y, pos.amPos.x + 320, pos.amPos.y + 25 };
	rightText.videoPos = { pos.videoPos.x + 160, pos.videoPos.y, pos.videoPos.x + 320, pos.videoPos.y + 25 };
	rightText.Create( "Right (/512)", NORM_STATIC_OPTIONS, rightText.seriesPos, parent, id++ );
	//
	horBinningText.seriesPos = { pos.seriesPos.x + 320, pos.seriesPos.y, pos.seriesPos.x + 480, pos.seriesPos.y += 25 };
	horBinningText.amPos = { pos.amPos.x + 320, pos.amPos.y, pos.amPos.x + 480, pos.amPos.y += 25 };
	horBinningText.videoPos = { pos.videoPos.x + 320, pos.videoPos.y, pos.videoPos.x + 480, pos.videoPos.y += 25 };
	horBinningText.Create( "H. Bin", NORM_STATIC_OPTIONS, horBinningText.seriesPos, parent, id++);
	//
	leftEdit.seriesPos = { pos.seriesPos.x, pos.seriesPos.y, pos.seriesPos.x + 160, pos.seriesPos.y + 25 };
	leftEdit.amPos = { pos.amPos.x, pos.amPos.y, pos.amPos.x + 160, pos.amPos.y + 25 };
	leftEdit.videoPos = { pos.videoPos.x, pos.videoPos.y, pos.videoPos.x + 160, pos.videoPos.y + 25 };
	leftEdit.Create( NORM_EDIT_OPTIONS | ES_CENTER | ES_MULTILINE | ES_WANTRETURN, leftEdit.seriesPos, parent, IDC_IMAGE_DIMS_START );
	leftEdit.SetWindowTextA( "1" );
	//
	rightEdit.seriesPos = { pos.seriesPos.x + 160, pos.seriesPos.y, pos.seriesPos.x + 320, pos.seriesPos.y + 25 };
	rightEdit.amPos = { pos.amPos.x + 160, pos.amPos.y, pos.amPos.x + 320, pos.amPos.y + 25 };
	rightEdit.videoPos = { pos.videoPos.x + 160, pos.videoPos.y, pos.videoPos.x + 320, pos.videoPos.y + 25 };
	rightEdit.Create( NORM_EDIT_OPTIONS | ES_CENTER | ES_MULTILINE | ES_WANTRETURN, rightEdit.seriesPos, parent, IDC_IMAGE_DIMS_START+1);
	rightEdit.SetWindowTextA( "50" );
	//
	horBinningEdit.seriesPos = { pos.seriesPos.x + 320, pos.seriesPos.y, pos.seriesPos.x + 480, pos.seriesPos.y += 25 };
	horBinningEdit.amPos = { pos.amPos.x + 320, pos.amPos.y, pos.amPos.x + 480, pos.amPos.y += 25 };
	horBinningEdit.videoPos = { pos.videoPos.x + 320, pos.videoPos.y, pos.videoPos.x + 480, pos.videoPos.y += 25 };
	horBinningEdit.Create( NORM_EDIT_OPTIONS | ES_CENTER | ES_MULTILINE | ES_WANTRETURN, horBinningEdit.seriesPos, 
						   parent, IDC_IMAGE_DIMS_START+2 );
	horBinningEdit.SetWindowTextA( "1" );
	//
	bottomLabel.seriesPos = { pos.seriesPos.x, pos.seriesPos.y, pos.seriesPos.x + 160, pos.seriesPos.y + 25 };
	bottomLabel.amPos = { pos.amPos.x, pos.amPos.y, pos.amPos.x + 160, pos.amPos.y + 25 };
	bottomLabel.videoPos = { pos.videoPos.x, pos.videoPos.y, pos.videoPos.x + 160, pos.videoPos.y + 25 };
	bottomLabel.Create( "Bottom (/512)", NORM_STATIC_OPTIONS, bottomLabel.seriesPos, parent, id++);

	//
	topLabel.seriesPos = { pos.seriesPos.x + 160, pos.seriesPos.y, pos.seriesPos.x + 320, pos.seriesPos.y + 25 };
	topLabel.amPos = { pos.amPos.x + 160, pos.amPos.y, pos.amPos.x + 320, pos.amPos.y + 25 };
	topLabel.videoPos = { pos.videoPos.x + 160, pos.videoPos.y, pos.videoPos.x + 320, pos.videoPos.y + 25 };
	topLabel.Create( "Top", NORM_STATIC_OPTIONS, topLabel.seriesPos, parent, id++);
	//
	vertBinningText.seriesPos = { pos.seriesPos.x + 320, pos.seriesPos.y, pos.seriesPos.x + 480, pos.seriesPos.y += 25 };
	vertBinningText.amPos = { pos.amPos.x + 320, pos.amPos.y, pos.amPos.x + 480, pos.amPos.y += 25 };
	vertBinningText.videoPos = { pos.videoPos.x + 320, pos.videoPos.y, pos.videoPos.x + 480, pos.videoPos.y += 25 };
	vertBinningText.Create( "V. Bin", NORM_STATIC_OPTIONS, vertBinningText.seriesPos, parent, id++);
	//
	bottomEdit.seriesPos = { pos.seriesPos.x, pos.seriesPos.y, pos.seriesPos.x + 160, pos.seriesPos.y + 25 };
	bottomEdit.amPos = { pos.amPos.x, pos.amPos.y, pos.amPos.x + 160, pos.amPos.y + 25 };
	bottomEdit.videoPos = { pos.videoPos.x, pos.videoPos.y, pos.videoPos.x + 160, pos.videoPos.y + 25 };
	bottomEdit.Create( NORM_EDIT_OPTIONS | ES_CENTER | ES_MULTILINE | ES_WANTRETURN, bottomEdit.seriesPos, parent, IDC_IMAGE_DIMS_START +3);
	bottomEdit.SetWindowTextA( "1" );
	//
	topEdit.seriesPos = { pos.seriesPos.x + 160, pos.seriesPos.y, pos.seriesPos.x + 320, pos.seriesPos.y + 25 };
	topEdit.amPos = { pos.amPos.x + 160, pos.amPos.y, pos.amPos.x + 320, pos.amPos.y + 25 };
	topEdit.videoPos = { pos.videoPos.x + 160, pos.videoPos.y, pos.videoPos.x + 320, pos.videoPos.y + 25 };
	topEdit.Create( NORM_EDIT_OPTIONS | ES_CENTER | ES_MULTILINE | ES_WANTRETURN, topEdit.seriesPos, parent, IDC_IMAGE_DIMS_START + 4);
	topEdit.SetWindowTextA( "50" );
	//
	vertBinningEdit.seriesPos = { pos.seriesPos.x + 320, pos.seriesPos.y, pos.seriesPos.x + 480, pos.seriesPos.y += 25 };
	vertBinningEdit.amPos = { pos.amPos.x + 320, pos.amPos.y, pos.amPos.x + 480, pos.amPos.y += 25 };
	vertBinningEdit.videoPos = { pos.videoPos.x + 320, pos.videoPos.y, pos.videoPos.x + 480, pos.videoPos.y += 25 };
	vertBinningEdit.Create( NORM_EDIT_OPTIONS | ES_CENTER | ES_MULTILINE | ES_WANTRETURN, vertBinningEdit.seriesPos, parent, IDC_IMAGE_DIMS_START +5);
	vertBinningEdit.SetWindowTextA( "1" );
}


void ImageDimsControl::handleSave( std::ofstream& saveFile )
{
	saveFile << "CAMERA_IMAGE_DIMENSIONS\n";
	saveFile << currentImageParameters.left << " " << currentImageParameters.right << " "
		<< currentImageParameters.horizontalBinning << "\n";
	saveFile << currentImageParameters.bottom << " " << currentImageParameters.top << " "
		<< currentImageParameters.verticalBinning << "\n";
	saveFile << "END_CAMERA_IMAGE_DIMENSIONS\n";
}


void ImageDimsControl::handleNew( std::ofstream& newfile )
{
	newfile << "CAMERA_IMAGE_DIMENSIONS\n";
	newfile << "1 512 1\n";
	newfile << "1 512 1\n";
	newfile << "END_CAMERA_IMAGE_DIMENSIONS\n";
}

imageParameters ImageDimsControl::getImageDimSettingsFromConfig ( std::ifstream& configFile, Version ver )
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

void ImageDimsControl::handleOpen( std::ifstream& openFile, Version ver )
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


void ImageDimsControl::rearrange( AndorRunModes::mode cameraMode, AndorTriggerMode::mode triggerMode, int width, int height,
								  fontMap fonts )
{
	leftText.rearrange( cameraMode, triggerMode, width, height, fonts );
	rightText.rearrange( cameraMode, triggerMode, width, height, fonts );
	horBinningText.rearrange( cameraMode, triggerMode, width, height, fonts );
	bottomLabel.rearrange( cameraMode, triggerMode, width, height, fonts );
	topLabel.rearrange( cameraMode, triggerMode, width, height, fonts );
	vertBinningText.rearrange( cameraMode, triggerMode, width, height, fonts );
	leftEdit.rearrange( cameraMode, triggerMode, width, height, fonts );
	rightEdit.rearrange( cameraMode, triggerMode, width, height, fonts );
	horBinningEdit.rearrange( cameraMode, triggerMode, width, height, fonts );
	bottomEdit.rearrange( cameraMode, triggerMode, width, height, fonts );
	topEdit.rearrange( cameraMode, triggerMode, width, height, fonts );
	vertBinningEdit.rearrange( cameraMode, triggerMode, width, height, fonts );
}

