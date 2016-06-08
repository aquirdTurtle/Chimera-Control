#pragma once

#include "Windows.h"

namespace commonMessages 
{
	/// Call to direct message to appropriate function in this namespace
	bool handleCommonMessage(HWND window, UINT msg, WPARAM wParam, LPARAM lParam);
	/// Run Menu
	int startSystem(HWND parentWindow, WPARAM wParam);
	int abortSystem(HWND parentWindow);
	/// File Menu
	int saveAll(HWND parentWindow);
	int exitProgram(HWND parentWindow);
	/// Scripting Menu
	// Intensity
	int newIntensityScript(HWND parentWindow);
	int openIntensityScript(HWND parentWindow);
	int saveIntensityScript(HWND parentWindow);
	int saveIntensityScriptAs(HWND parentWindow);
	// Horizontal
	int newVerticalScript(HWND parentWindow);
	int openVerticalScript(HWND parentWindow);
	int saveVerticalScript(HWND parentWindow);
	int saveVerticalScriptAs(HWND parentWindow);
	// Vertical
	int newHorizontalScript(HWND parentWindow);
	int openHorizontalScript(HWND parentWindow);
	int saveHorizontalScript(HWND parentWindow);
	int saveHorizontalScriptAs(HWND parentWindow);

	/// Profile Menu
	/*
	int newExperimentType(HWND parentWindow);
	int newCategory(HWND parentWindow);
	int saveConfigurationAs(HWND parentWindow);
	int renameCurrentExperimentType(HWND parentWindow);
	int deleteCurrentExperimentType(HWND parentWindow);
	int renameCurrentCategory(HWND parentWindow);
	int deleteCurrentCategory(HWND parentWindow);
	int newConfiguration(HWND parentWindow);
	int renameCurrentConfiguration(HWND parentWindow);
	int deleteCurrentConfiguration(HWND parentWindow);

	int newSequence(HWND parentWindow);
	int deleteSequence(HWND parentWindow);
	int addToSequence(HWND parentWindow);
	int resetSequence(HWND parentWindow);
	int saveSequence(HWND parentWindow);
	*/
	int saveProfile(HWND parentWindow);
	
	int helpWindow(HWND parentWindow);

	int reloadNIAWGDefaults(HWND parentWindow);
};
