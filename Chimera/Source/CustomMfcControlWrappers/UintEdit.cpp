#include "stdafx.h"
#include "UintEdit.h"
#include <boost/lexical_cast.hpp>

unsigned UintEdit::getWindowTextAsUINT()
{
	CString txt;
	GetWindowText (txt);
	try
	{
		return boost::lexical_cast<unsigned>(str (txt));
	}
	catch (boost::bad_lexical_cast&)
	{
		throwNested ("Failed to convert edit text to double.");
	}
}
