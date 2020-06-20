#include "stdafx.h"
#include "my_str.h"

template <> std::string str<QString> (QString input, const int precision, bool eatZeros, bool toLower, bool zeroPad, 
	bool useScientificNotation){
	std::string txt = input.toLatin1();
	//auto txt = input.toStdString ();
	return str (txt, precision, eatZeros, toLower, zeroPad);
}


