#pragma once
#include <qstring.h>
#include <qmap.h>

class chimeraStyleSheets {
	public:
		static QString stdStyleSheet ();
		static QMap<QString , QString > getDefs();
};
