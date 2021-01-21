#include "stdafx.h"
#include <ConfigurationSystems/ConfigStream.h>
#include "ChimeraStyleSheets.h"
#include <qfile.h>

QString chimeraStyleSheets::stdStyleSheet (){
	QFile stylesheetFile("stylesheet.css");
	stylesheetFile.open (QFile::ReadOnly);
	QString stylesheet = QLatin1String (stylesheetFile.readAll ());

	QFile defs ("StylesheetDefs.txt");
	defs.open (QFile::ReadOnly);
	QString defTxt = QLatin1String (defs.readAll ());
	ConfigStream defStream (str(defTxt));
	while (defStream) {
		std::string name, replVal;
		defStream >> name >> replVal;
		stylesheet.replace (qstr (name), qstr (replVal));
	}
	return stylesheet;
}