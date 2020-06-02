// created by Mark O. Brown
#include "stdafx.h"
#include "AnalogOutput.h"
#include "boost\lexical_cast.hpp"
#include <QlineEdit>

AnalogOutput::AnalogOutput( ){}

void AnalogOutput::initialize ( POINT& pos, IChimeraWindowWidget* parent, int whichDac) {
	label = new QLabel (cstr (whichDac), parent);
	label->setGeometry (QRect{ QPoint{pos.x, pos.y}, QPoint{pos.x + 20, pos.y + 20} });
	label->setToolTip ( (info.name + "\n" + info.note).c_str() );

	edit = new CQLineEdit ("0", parent);
	edit->setGeometry ({ QPoint{pos.x + 20, pos.y},QPoint{pos.x + 160, pos.y += 20} });
	edit->setToolTip ( (info.name + "\r\n" + info.note).c_str() );
	parent->connect (edit, &QLineEdit::textChanged, 		
		[this, parent]() {
			handleEdit ();
		});
	edit->setStyleSheet ("QLineEdit { border: none }");
}

double AnalogOutput::getVal ( bool useDefault )
{
	double val;
	try
	{
		val = boost::lexical_cast<double>( str (edit->text () ) );
	}
	catch ( boost::bad_lexical_cast& )
	{
		if ( useDefault )
		{
			val = 0;
		}
		else
		{
			throwNested ( "value entered in DAC #" + str ( dacNum ) + " (" + str(edit->text())
						  + ") failed to convert to a double!" );
		}
	}
	return val;
}


void AnalogOutput::updateEdit ( bool roundToDacPrecision )
{
	std::string valStr = roundToDacPrecision ? str ( roundToDacResolution ( info.currVal ), 13, true, false, true )
		: str ( info.currVal, 5, false, false, true );
	edit->setText (cstr (valStr));
}

void AnalogOutput::setName ( std::string name )
{
	if ( name == "" )
	{
		// no empty names allowed.
		return;
	}
	std::transform ( name.begin ( ), name.end ( ), name.begin ( ), ::tolower );
	info.name = name;
	edit->setToolTip ( cstr(info.name + "\r\n" + info.note));
}


void AnalogOutput::handleEdit ( bool roundToDacPrecision )
{
	bool matches = false;
	try
	{
		if ( roundToDacPrecision )
		{
			double roundNum = roundToDacResolution ( info.currVal );
			if ( fabs ( roundToDacResolution ( info.currVal ) - boost::lexical_cast<double>( str(edit->text()) ) ) < 1e-8 )
			{
				matches = true;
			}
		}
		else
		{
			if ( fabs ( info.currVal - boost::lexical_cast<double>( str (edit->text ()) ) ) < 1e-8 )
			{
				matches = true;
			}
		}
	}
	catch ( boost::bad_lexical_cast& ) { /* failed to convert to double. Effectively, doesn't match. */ }
}

double AnalogOutput::roundToDacResolution ( double num )
{
	double dacResolution = 10.0 / pow ( 2, 16 );
	return long ( ( num + dacResolution / 2 ) / dacResolution ) * dacResolution;
}


void AnalogOutput::setNote ( std::string note )
{
	info.note = note;
	edit->setToolTip ( (info.name + "\r\n" + info.note).c_str());
}

void AnalogOutput::disable ( )
{
	edit->setEnabled (false);
}

bool AnalogOutput::handleArrow ( CWnd* focus, bool up )
{
	/*
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
	}*/
	return false;
}
