#include "stdafx.h"
#include "DoubleEdit.h"
#include <boost/lexical_cast.hpp>

double DoubleEdit::getWindowTextAsDouble( )
{
	CString txt;
	GetWindowText( txt );
	try
	{
		return boost::lexical_cast<double>( str(txt) );
	}
	catch ( boost::bad_lexical_cast& )
	{
		thrower( "Error: Failed to convert edit text to double." );
	}
}
