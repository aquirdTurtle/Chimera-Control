#pragma once
#include <string>
#include <vector>
#include "profileSettings.h"

/*
]- This is a structure used for containing a set of parameters that define a profile.
]- It's used heavily by the configuration file system, but not exclusively by it.
*/
struct profileSettings
{
	std::string configuration;
	std::string parentFolderName;
	// Note: The category path include the category name in the string.
	std::string categoryPath;
};


struct seqSettings
{
	std::string name;
	std::vector<profileSettings> sequence;
};

