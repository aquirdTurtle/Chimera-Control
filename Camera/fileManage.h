#pragma once
#include <vector>
#include <string>
#include "Windows.h"

namespace fileManage
{
	std::vector<std::string> searchForFiles(std::string locationToSearch, std::string extensions);

	int reloadCombo(HWND comboToReload, std::string serachLocation, std::string extension, std::string nameToLoad);

}