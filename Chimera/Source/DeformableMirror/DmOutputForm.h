#pragma once
#include "ParameterSystem/Expression.h"

struct DMOutputForm
{
	Expression coma = "0";
	Expression comaAng = "0";
	Expression astig = "0";
	Expression astigAng = "0";
	Expression trefoil = "0";
	Expression trefoilAng = "0";
	Expression spherical = "0";
	std::string base;
};
