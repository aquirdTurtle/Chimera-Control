#pragma once
//#include <Windows.h>
#include "TTLSystem.h"
#include "DACSystem.h"
#include "VariableSystem.h"
#include "Script.h"
#include "NoteSystem.h"
#include "ConfigurationFileSystem.h"
#include <string>
#include "constants.h"
#include "RunInfo.h"

class MasterWindow
{
	public:
		static MasterWindow* InitializeWindowInfo(HINSTANCE hInstance);
		MasterWindow(int start) : masterScript{ std::string("Master"), start }, profile{ PROFILES_PATH } {}
		~MasterWindow();

	private:
		LRESULT MasterWindowMessageHandler(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		static LRESULT CALLBACK MasterWindowProcedure(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		std::string title;
		int InitializeWindowControls();
		HWND masterWindowHandle;
		TTLSystem ttlBoard;
		DACSystem dacBoards;
		VariableSystem variables;
		Script masterScript;
		ConfigurationFileSystem profile;
		NoteSystem notes;
		RunInfo systemRunningInfo;
		HINSTANCE programInstance;
		// colors
		HBRUSH greenBrush = CreateSolidBrush(RGB(50, 200, 50));
		HBRUSH redBrush = CreateSolidBrush(RGB(200, 50, 50));
		//static HINSTANCE cHInstance;
		/// Friends (many friends)
		friend INT_PTR TTLSystem::colorTTLs(HWND window, UINT message, WPARAM wParam, LPARAM lParam, MasterWindow* Master);
		// configuration system friends
		friend bool ConfigurationFileSystem::orientationChangeHandler(MasterWindow& Master);
		friend bool ConfigurationFileSystem::newConfiguration(MasterWindow& Master);
		friend bool ConfigurationFileSystem::openConfiguration(std::string configurationNameToOpen, MasterWindow& Master);
		friend bool ConfigurationFileSystem::openExperiment(std::string experimentToOpen, MasterWindow& Master);
		friend bool ConfigurationFileSystem::experimentChangeHandler(MasterWindow& Master);
		friend bool ConfigurationFileSystem::saveConfigurationOnly(MasterWindow& Master);
		friend bool ConfigurationFileSystem::saveConfigurationAs(MasterWindow& Master);
		friend bool ConfigurationFileSystem::renameConfiguration(MasterWindow& Master);
		friend bool ConfigurationFileSystem::saveCategoryOnly(MasterWindow& Master);
		friend bool ConfigurationFileSystem::newCategory(MasterWindow& Master);
		friend bool ConfigurationFileSystem::saveCategoryAs(MasterWindow& Master);
		friend bool ConfigurationFileSystem::openCategory(std::string categoryToOpen, MasterWindow& Master);
		friend bool ConfigurationFileSystem::categoryChangeHandler(MasterWindow& Master);
		friend bool ConfigurationFileSystem::saveExperimentOnly(MasterWindow& Master);
		friend bool ConfigurationFileSystem::saveExperimentAs(MasterWindow& Master);
		friend bool ConfigurationFileSystem::newExperiment(MasterWindow& Master);
		friend bool ConfigurationFileSystem::addToSequence(MasterWindow& Master);
		friend bool ConfigurationFileSystem::loadNullSequence(MasterWindow& Master);
		friend bool ConfigurationFileSystem::saveSequence(MasterWindow& Master);
		friend bool ConfigurationFileSystem::saveSequenceAs(MasterWindow& Master);
		friend bool ConfigurationFileSystem::renameSequence(MasterWindow& Master);
		friend bool ConfigurationFileSystem::newSequence(MasterWindow& Master);
		friend bool ConfigurationFileSystem::openSequence(std::string sequenceName, MasterWindow& Master);
		friend bool ConfigurationFileSystem::initialize(POINT& topLeftPosition, MasterWindow& Master);
		// script friends
		friend std::string Script::getSyntaxColor(std::string word, std::string editType, MasterWindow& Master);
		friend bool Script::updateChildCombo(MasterWindow& Master);
		friend bool Script::changeView(std::string viewName, MasterWindow& Master);
		friend bool Script::saveScript(MasterWindow& Master);
		friend bool Script::saveScriptAs(std::string location, MasterWindow& Master);
		friend bool Script::checkChildSave(MasterWindow& Master);
		friend bool Script::checkSave(MasterWindow& Master);
		friend bool Script::renameScript(MasterWindow& Master);
		friend bool Script::deleteScript(MasterWindow& Master);
		friend bool Script::newScript(MasterWindow& Master);
		friend bool Script::openParentScript(std::string parentScriptName, MasterWindow& Master);
		friend bool Script::considerCurrentLocation(MasterWindow& Master);
		friend bool Script::initialize(int width, int height, POINT& startingLocation, MasterWindow& Master);
};