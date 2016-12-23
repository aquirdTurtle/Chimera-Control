#pragma once
#include "stdafx.h"

#include <string>
/*
* This function deals with the trailing \r\n on each line required for edit controls to make sure everything is consistent. It also makes sure that there is
* no crap at the beginning of the string, which happens sommetimes. str is the string which it does this to.
*/
void cleanString(std::string &str);

