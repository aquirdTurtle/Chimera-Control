#include "stdafx.h"
#include "ChimeraStyleSheets.h"
#include <qfile.h>

QString chimeraStyleSheets::stdStyleSheet ()
{
	QFile File ("stylesheet.css");
	File.open (QFile::ReadOnly);
	QString stylesheet = QLatin1String (File.readAll ());
	return stylesheet;
}