#pragma once
#include <string>
#include <vector>
/*
]- This is a structure used for containing a set of parameters that define a profile.
]- It's used heavily by the configuration file system, but not exclusively by it.
*/
struct profileSettings
{
	std::string configuration;
	//std::string experiment;
	std::string parentFolderName;
	std::string sequence;
	// Note: The experiment (category) path include the expriment (category) name in the string.
	//std::string experimentPath;
	// Note: The experiment (category) path include the expriment (category) name in the string.
	std::string categoryPath;
	// needs some work.
	std::vector<std::string> sequenceConfigNames;
};

