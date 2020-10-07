// created by Mark O. Brown
#pragma once

#include "LowLevel/constants.h"
#include <string>
#include <vector>

/*
]- This is a structure used for containing a set of parameters that define a profile.
]- It's used heavily by the configuration file system, but not exclusively by it.
*/
struct profileSettings{
	std::string configuration;
	std::string parentFolderName;
	std::string configLocation;
	std::string configFilePath ();
};


struct seqSettings{
	std::string name;
	std::vector<profileSettings> sequence;
};
 
Q_DECLARE_METATYPE(profileSettings)

