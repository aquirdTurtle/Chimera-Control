#include "stdafx.h"
#include "DoubleEdit.h"

double DoubleEdit::getWindowTextAsDouble( )
{
	CString txt;
	GetWindowText( txt );
	try
	{
		return std::stod( str(txt) );
	}
	catch ( std::invalid_argument& err )
	{
		thrower( "Error: Failed to convert edit text to double." );
	}
}
