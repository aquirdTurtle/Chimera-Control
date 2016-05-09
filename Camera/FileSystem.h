#pragma once
#include <string>

class FileSystem 
{
	public:
		FileSystem(std::string fileSystemPath);
		~FileSystem();
		int openConfiguration(std::string configurationNameToOpen);
		int saveConfiguration();
		int saveConfigurationAs(std::string newConfigurationName);
		int newConfiguration(std::string newConfigurationName);
		int renameConfiguration(std::string newConfigurationName);
		int deleteConfiguration();
		int checkSave();
	private:
		std::string configurationName;
		std::string FILE_SYSTEM_PATH;
		bool configurationSaved;
};
