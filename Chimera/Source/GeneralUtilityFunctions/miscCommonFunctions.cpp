// created by Mark O. Brown
#include "stdafx.h"
#include "miscCommonFunctions.h"
#include "my_str.h"
#include <string>
#include <filesystem>

unsigned long getNextFileIndex( std::string fileBase, std::string ext )
{
	// find the first data file that hasn't been already written, starting with fileBase1.h5
	unsigned long fileNum = 1;
	while (std::filesystem::exists((fileBase + str (fileNum) + ext).c_str ())){
		fileNum++;
	}
	return fileNum;
}
