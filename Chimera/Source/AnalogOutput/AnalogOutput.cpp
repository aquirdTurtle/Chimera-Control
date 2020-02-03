// created by Mark O. Brown
#include "stdafx.h"
#include "AnalogOutput.h"
#include "boost\lexical_cast.hpp"

AnalogOutput::AnalogOutput( )
{

}


void AnalogOutput::initialize ( POINT& pos, CWnd* parent, int id, cToolTips& toolTips, int whichDac)
{
	// create label
	label.sPos = { pos.x, pos.y, pos.x + 20, pos.y + 20 };
	label.Create ( cstr(whichDac), WS_CHILD | WS_VISIBLE | SS_CENTER,
								 label.sPos, parent, id++ );
	label.setToolTip ( info.name + "\r\n" + info.note, toolTips, parent );
	// create edit
	edit.sPos = { pos.x + 20, pos.y, pos.x + 160, pos.y += 20 };
	edit.colorState = 0;
	edit.Create ( WS_CHILD | WS_VISIBLE | WS_BORDER, edit.sPos, parent, ID_DAC_FIRST_EDIT + whichDac );
	edit.SetWindowText ( "0" );
	edit.setToolTip ( info.name + "\r\n" + info.note, toolTips, parent );

}


void AnalogOutput::setEditColorState ( int state )
{
	edit.colorState = state;
	edit.RedrawWindow ( );
}

double AnalogOutput::getVal ( bool useDefault )
{
	CString text;
	edit.GetWindowTextA ( text );
	double val;
	try
	{
		val = boost::lexical_cast<double>( str ( text ) );
	}
	catch ( boost::bad_lexical_cast& )
	{
		if ( useDefault )
		{
			val = 0;
		}
		else
		{
			throwNested ( "value entered in DAC #" + str ( dacNum ) + " (" + text.GetString ( )
						  + ") failed to convert to a double!" );
		}
	}
	return val;
}

void AnalogOutput::rearrange ( UINT width, UINT height, fontMap fonts )
{
	edit.rearrange ( width, height, fonts );
	label.rearrange ( width, height, fonts );
}


void AnalogOutput::updateEdit ( bool roundToDacPrecision )
{
	std::string valStr = roundToDacPrecision ? str ( roundToDacResolution ( info.currVal ), 13, true, false, true )
		: str ( info.currVal, 5, false, false, true );
	int sel, sel_end;
	// preserve the selection location, especially important for quick changes.
	edit.GetSel ( sel, sel_end );
	edit.SetWindowTextA ( cstr ( valStr ) );
	edit.SetSel ( sel, sel );
	edit.colorState = 0;
}

void AnalogOutput::setName ( std::string name, cToolTips& toolTips, CWnd* master )
{
	if ( name == "" )
	{
		// no empty names allowed.
		return;
	}
	std::transform ( name.begin ( ), name.end ( ), name.begin ( ), ::tolower );
	info.name = name;
	edit.setToolTip ( info.name + "\r\n" + info.note, toolTips, master );
}


void AnalogOutput::handleEdit ( bool roundToDacPrecision )
{
	CString text;
	edit.GetWindowTextA ( text );
	bool matches = false;
	std::string textStr ( text );
	try
	{
		if ( roundToDacPrecision )
		{
			double roundNum = roundToDacResolution ( info.currVal );
			if ( fabs ( roundToDacResolution ( info.currVal ) - boost::lexical_cast<double>( textStr ) ) < 1e-8 )
			{
				matches = true;
			}
		}
		else
		{
			if ( fabs ( info.currVal - boost::lexical_cast<double>( str ( text ) ) ) < 1e-8 )
			{
				matches = true;
			}
		}
	}
	catch ( boost::bad_lexical_cast& ) { /* failed to convert to double. Effectively, doesn't match. */ }
	if ( matches )
	{
		// mark this to change color.
		edit.colorState = 0;
		edit.RedrawWindow ( );
	}
	else
	{
		edit.colorState = 1;
		edit.RedrawWindow ( );
	}
}

double AnalogOutput::roundToDacResolution ( double num )
{
	double dacResolution = 10.0 / pow ( 2, 16 );
	return long ( ( num + dacResolution / 2 ) / dacResolution ) * dacResolution;
}


void AnalogOutput::setNote ( std::string note, cToolTips& toolTips, CWnd* master )
{
	info.note = note;
	edit.setToolTip ( info.name + "\r\n" + info.note, toolTips, master );
}

HBRUSH AnalogOutput::handleColorMessage ( int id, CWnd* window, CDC* cDC )
{
	if ( id == label.GetDlgCtrlID ( ))
	{ 
		cDC->SetBkColor ( _myRGBs[ "Static-Bkgd" ] );
		cDC->SetTextColor ( _myRGBs[ "Text" ] );
		return *_myBrushes[ "Static-Bkgd" ];
	}
	else if ( id == edit.GetDlgCtrlID ( ) )
	{
		if ( edit.colorState == 0 )
		{
			// default.
			cDC->SetTextColor ( _myRGBs[ "AuxWin-Text" ] );
			cDC->SetBkColor ( _myRGBs[ "Interactable-Bkgd" ] );
			return *_myBrushes[ "Interactable-Bkgd" ];
		}
		else if ( edit.colorState == 1 )
		{
			// in this case, the actuall setting hasn't been changed despite the edit being updated.
			cDC->SetTextColor ( _myRGBs[ "White" ] );
			cDC->SetBkColor ( _myRGBs[ "Red" ] );
			return *_myBrushes[ "Red" ];
		}
		else if ( edit.colorState == -1 )
		{
			// in use during experiment.
			cDC->SetTextColor ( _myRGBs[ "Black" ] );
			cDC->SetBkColor ( _myRGBs[ "White" ] );
			return *_myBrushes[ "White" ];
		}
		else
		{
			thrower ( "Unknown color state for AO system edit???" );
		}
	}
	return NULL;
}

void AnalogOutput::shade ( )
{
	edit.colorState = -1;
	edit.SetReadOnly ( true );
	edit.InvalidateRect ( NULL );
}

void AnalogOutput::disable ( )
{
	edit.EnableWindow ( 0 );
}

void AnalogOutput::unshade ( )
{
	edit.EnableWindow ( );
	if ( edit.colorState == -1 )
	{
		edit.colorState = 0;
		edit.SetReadOnly ( false );
		edit.RedrawWindow ( );
	}
}


bool AnalogOutput::handleArrow ( CWnd* focus, bool up )
{
	if ( focus == &edit )
	{
		CString ctxt;
		edit.GetWindowTextA( ctxt );
		std::string txt = str ( ctxt );
		// make sure value in edit matches current value, else unclear what this functionality should do.
		double val;
		try
		{
			val = boost::lexical_cast<double>( txt );
		}
		catch ( boost::bad_lexical_cast )
		{
			return true;
		}
		if ( fabs ( val - info.currVal ) > 1e-12 )
		{
			return true;
		}
		// okay all good if reach here.
		int cursorPos, end;
		edit.GetSel ( cursorPos, end );
		UINT decimalPos = txt.find ( "." );
		// if no decimal...
		if ( decimalPos == std::string::npos )
		{
			// value is an integer, decimal is effectively at end.
			decimalPos = txt.size ( );
		}
		// the order of the first digit
		double size = pow ( 10, decimalPos - 1 );
		// handle the extra decimal character with the ternary operator here. 
		int editPlace = (cursorPos > decimalPos ? cursorPos - 1 : cursorPos);
		double inc = size / pow ( 10, editPlace);
		info.currVal += up? inc : -inc;
		updateEdit ( false );
		
		edit.GetWindowTextA ( ctxt );
		std::string txt2 = str ( ctxt );

		if ( txt.find ( "-" ) == std::string::npos && txt2.find("-") != std::string::npos)
		{
			// then need to shift cursor to account for the negative.
			edit.GetSel ( cursorPos, end );
			edit.SetSel ( cursorPos + 1, cursorPos + 1 );
		}
		else if ( txt.find ( "-" ) != std::string::npos && txt2.find ( "-" ) == std::string::npos )
		{
			// then need to shift cursor to account for the negative.
			edit.GetSel ( cursorPos, end );
			edit.SetSel ( cursorPos - 1, cursorPos - 1 );
		}
		return true;
	}
	return false;
}
