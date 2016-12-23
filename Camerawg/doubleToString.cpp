#include "stdafx.h"
#include "doubleToString.h"
#include <sstream>
#include <iomanip>

std::string doubleToString(double number, long precision)
{
	std::stringstream stream;
	stream << std::fixed << std::setprecision(precision) << number;
	return stream.str();
}