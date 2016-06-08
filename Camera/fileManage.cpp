#include "stdafx.h"
#include "fileManage.h"
#include <vector>
#include <string>

namespace fileManage
{
	std::vector<std::string> searchForFiles(std::string locationToSearch, std::string extensions)
	{
		// Re-add the entries back in and figure out which one is the current one.
		std::vector<std::string> names;
		std::string search_path = locationToSearch + "\\" + extensions;
		WIN32_FIND_DATA fd;
		HANDLE hFind;
		if (extensions == "*")
		{
			hFind = FindFirstFileEx(search_path.c_str(), FindExInfoStandard, &fd, FindExSearchLimitToDirectories, NULL, 0);
		}
		else
		{
			hFind = FindFirstFile(search_path.c_str(), &fd);
		}
		if (hFind != INVALID_HANDLE_VALUE)
		{
			do
			{
				// if looking for folders
				if (extensions == "*")
				{
					if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
					{
						if (std::string(fd.cFileName) != "." && std::string(fd.cFileName) != "..")
						{
							names.push_back(fd.cFileName);
						}
					}
				}
				else
				{
					if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
					{
						names.push_back(fd.cFileName);
					}
				}
			} while (FindNextFile(hFind, &fd));
			FindClose(hFind);
		}

		// Remove suffix from file names and...
		for (int configListInc = 0; configListInc < names.size(); configListInc++)
		{
			if (extensions == "*" || extensions == "*.*" || extensions == "*.hSubConfig" || extensions == "*.vSubConfig" || extensions == "*.plot")
			{
				names[configListInc] = names[configListInc].substr(0, names[configListInc].size() - (extensions.size() - 1));
			}
			else
			{
				names[configListInc] = names[configListInc].substr(0, names[configListInc].size() - extensions.size());
			}
		}
		// Make the final vector out of the unique objects left.
		return names;
	}

	int reloadCombo(HWND comboToReload, std::string serachLocation, std::string extension, std::string nameToLoad)
	{
		std::vector<std::string> names;
		// search for folders
		names = fileManage::searchForFiles(serachLocation, extension);

		/// Get current selection
		long long itemIndex = SendMessage(comboToReload, CB_GETCURSEL, 0, 0);
		TCHAR experimentConfigToOpen[256];
		std::string currentSelection;
		int currentInc = -1;
		if (itemIndex != -1)
		{
			// Send CB_GETLBTEXT message to get the item.
			SendMessage(comboToReload, (UINT)CB_GETLBTEXT, (WPARAM)itemIndex, (LPARAM)experimentConfigToOpen);
			currentSelection = experimentConfigToOpen;
		}
		/// Reset stuffs
		SendMessage(comboToReload, CB_RESETCONTENT, 0, 0);
		// Send list to object
		for (int comboInc = 0; comboInc < names.size(); comboInc++)
		{
			if (nameToLoad == names[comboInc])
			{
				currentInc = comboInc;
			}
			TCHAR * name = (TCHAR*)names[comboInc].c_str();
			SendMessage(comboToReload, CB_ADDSTRING, 0, (LPARAM)(name));
		}
		// Set initial value
		SendMessage(comboToReload, CB_SETCURSEL, currentInc, 0);
		return 0;
	}

}