#include "stdafx.h"
#include "MainWindow.h"
#include "initializeMainWindow.h"
#include "commonFunctions.h"
#include "myAgilent.h"
#include "NiawgController.h"
#include "CameraWindow.h"
#include "myErrorHandler.h"

IMPLEMENT_DYNAMIC(MainWindow, CDialog)

BEGIN_MESSAGE_MAP( MainWindow, CDialog )
	ON_WM_CTLCOLOR()
	ON_COMMAND_RANGE( ID_ACCELERATOR_ESC, ID_ACCELERATOR_ESC, &MainWindow::passCommonCommand )
	ON_COMMAND_RANGE( ID_ACCELERATOR_F5, ID_ACCELERATOR_F5, &MainWindow::passCommonCommand )
	ON_COMMAND_RANGE( MENU_ID_RANGE_BEGIN, MENU_ID_RANGE_END, &MainWindow::passCommonCommand )
	ON_COMMAND_RANGE( IDC_DEBUG_OPTIONS_RANGE_BEGIN, IDC_DEBUG_OPTIONS_RANGE_END, &MainWindow::passDebugPress )
	ON_COMMAND_RANGE( IDC_MAIN_OPTIONS_RANGE_BEGIN, IDC_MAIN_OPTIONS_RANGE_END, &MainWindow::passMainOptionsPress )
	// 

	ON_CBN_SELENDOK( IDC_EXPERIMENT_COMBO, &MainWindow::handleExperimentCombo )
	ON_CBN_SELENDOK( IDC_CATEGORY_COMBO, &MainWindow::handleCategoryCombo )
	ON_CBN_SELENDOK( IDC_CONFIGURATION_COMBO, &MainWindow::handleConfigurationCombo )
	ON_CBN_SELENDOK( IDC_SEQUENCE_COMBO, &MainWindow::handleSequenceCombo )
	ON_CBN_SELENDOK( IDC_ORIENTATION_COMBO, &MainWindow::handleOrientationCombo )
	// 
	ON_NOTIFY( NM_DBLCLK, IDC_VARIABLES_LISTVIEW, &MainWindow::listViewDblClick )
	ON_NOTIFY( NM_RCLICK, IDC_VARIABLES_LISTVIEW, &MainWindow::handleRClick )
	ON_REGISTERED_MESSAGE( eStatusTextMessageID, &MainWindow::onStatusTextMessage )
	ON_REGISTERED_MESSAGE( eErrorTextMessageID, &MainWindow::onErrorMessage )
	ON_REGISTERED_MESSAGE( eFatalErrorMessageID, &MainWindow::onFatalErrorMessage )
	ON_REGISTERED_MESSAGE( eNormalFinishMessageID, &MainWindow::onNormalFinishMessage )
	ON_REGISTERED_MESSAGE( eColoredEditMessageID, &MainWindow::onColoredEditMessage )
	ON_REGISTERED_MESSAGE( eDebugMessageID, &MainWindow::onDebugMessage )

	ON_COMMAND_RANGE( IDC_MAIN_STATUS_BUTTON, IDC_MAIN_STATUS_BUTTON, &MainWindow::passClear )
	ON_COMMAND_RANGE( IDC_ERROR_STATUS_BUTTON, IDC_ERROR_STATUS_BUTTON, &MainWindow::passClear )
	ON_COMMAND_RANGE( IDC_DEBUG_STATUS_BUTTON, IDC_DEBUG_STATUS_BUTTON, &MainWindow::passClear )
END_MESSAGE_MAP()

void MainWindow::setNiawgRunningState( bool newRunningState )
{
	niawg.setRunningState( newRunningState );
	return;
}

bool MainWindow::niawgIsRunning()
{
	return niawg.isRunning();
}

BOOL MainWindow::PreTranslateMessage(MSG* pMsg)
{
	for (int toolTipInc = 0; toolTipInc < tooltips.size(); toolTipInc++)
	{
		tooltips[toolTipInc]->RelayEvent(pMsg);
	}
	return CDialog::PreTranslateMessage(pMsg);
}

void MainWindow::setNiawgDefaults()
{
	niawg.setDefaultWaveforms(this);
}

std::unordered_map<std::string, CFont*> MainWindow::getFonts()
{
	return mainFonts;
}

void MainWindow::passClear(UINT id)
{
	if (id == IDC_MAIN_STATUS_BUTTON)
	{
		mainStatus.clear();
	}
	else if (id == IDC_ERROR_STATUS_BUTTON)
	{
		errorStatus.clear();
	}
	else if (id == IDC_DEBUG_STATUS_BUTTON)
	{
		debugStatus.clear();
	}
}

void MainWindow::OnCancel()
{
	passCommonCommand(ID_FILE_MY_EXIT);
}


void MainWindow::OnClose()
{
	passCommonCommand(WM_CLOSE);
}


void MainWindow::stopNiawg()
{
	niawg.configureOutputEnabled(VI_FALSE);
	niawg.abortGeneration();
}

BOOL MainWindow::OnInitDialog()
{
	/// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///
	///				Initialize NIAWG
	///
	// get time now
	time_t dateStart = time(0);
	struct tm datePointerStart;
	localtime_s(&datePointerStart, &dateStart);
	std::string logFolderNameStart = "Date " + std::to_string(datePointerStart.tm_year + 1900) + "-" + std::to_string(datePointerStart.tm_mon + 1) + "-"
		+ std::to_string(datePointerStart.tm_mday) + " Time " + std::to_string(datePointerStart.tm_hour) + "-" + std::to_string(datePointerStart.tm_min) + "-"
		+ std::to_string(datePointerStart.tm_sec);
	// initialize default file names and open the files.
	std::vector<std::fstream> default_hConfigVerticalScriptFile, default_hConfigHorizontalScriptFile, default_vConfigVerticalScriptFile,
		default_vConfigHorizontalScriptFile;
	default_hConfigVerticalScriptFile.push_back(std::fstream(DEFAULT_SCRIPT_FOLDER_PATH + "DEFAULT_HCONFIG_VERTICAL_SCRIPT.nScript"));
	default_hConfigHorizontalScriptFile.push_back(std::fstream(DEFAULT_SCRIPT_FOLDER_PATH + "DEFAULT_HCONFIG_HORIZONTAL_SCRIPT.nScript"));
	default_vConfigVerticalScriptFile.push_back(std::fstream(DEFAULT_SCRIPT_FOLDER_PATH + "DEFAULT_VCONFIG_VERTICAL_SCRIPT.nScript"));
	default_vConfigHorizontalScriptFile.push_back(std::fstream((DEFAULT_SCRIPT_FOLDER_PATH + "DEFAULT_VCONFIG_HORIZONTAL_SCRIPT.nScript")));

	// check errors
	if (!default_hConfigVerticalScriptFile[0].is_open())
	{
		errBox("FATAL ERROR: Couldn't open default file. Was looking for file " + DEFAULT_SCRIPT_FOLDER_PATH + "DEFAULT_HCONFIG_VERTICAL_SCRIPT.nScript");
		return 0;
	}
	if (!default_hConfigHorizontalScriptFile[0].is_open())
	{
		errBox("FATAL ERROR: Couldn't open default file. Was looking for file " + DEFAULT_SCRIPT_FOLDER_PATH + "DEFAULT_HCONFIG_HORIZONTAL_SCRIPT.nScript");
		return 0;
	}
	if (!default_vConfigVerticalScriptFile[0].is_open())
	{
		errBox("FATAL ERROR: Couldn't open default file. Was looking for file " + DEFAULT_SCRIPT_FOLDER_PATH + "DEFAULT_VCONFIG_VERTICAL_SCRIPT.nScript");
		return 0;
	}
	if (!default_vConfigHorizontalScriptFile[0].is_open())
	{
		errBox("FATAL ERROR: Couldn't open default file. Was looking for file " + DEFAULT_SCRIPT_FOLDER_PATH + "DEFAULT_VCONFIG_HORIZONTAL_SCRIPT.nScript");
		return 0;
	}

	// parameters for variables used by the default file. (there shouldn't be any, these are essentially just placeholders so that I can use the same functions.
	std::vector<char> defXVarNames, defYVarNames;
	// parameters for variables used by the default file. (there shouldn't be any, these are essentially just placeholders so that I can use the same functions.
	std::vector<std::string> defXVarFileNames, defYVarFileNames;
	// parameters for variables used by the default file. (there shouldn't be any, these are essentially just placeholders so that I can use the same functions.
	std::vector<std::fstream> defXVarFiles;
	if (!NIAWG_SAFEMODE)
	{
		std::ofstream hConfigVerticalDefaultScriptLog(EXPERIMENT_LOGGING_FILES_PATH + logFolderNameStart + "\\Default hConfig Vertical Script.script");
		std::ofstream hConfigHorizontalDefaultScriptLog(EXPERIMENT_LOGGING_FILES_PATH + logFolderNameStart + "\\Default hConfig Horizontal Script.script");
		std::ofstream vConfigVerticalDefaultScriptLog(EXPERIMENT_LOGGING_FILES_PATH + logFolderNameStart + "\\Default vConfig Vertical Script.script");
		std::ofstream vConfigHorizontalDefaultScriptLog(EXPERIMENT_LOGGING_FILES_PATH + logFolderNameStart + "\\Default vConfig Horizontal Script.script");
		hConfigVerticalDefaultScriptLog << default_hConfigVerticalScriptFile[0].rdbuf();
		hConfigHorizontalDefaultScriptLog << default_hConfigHorizontalScriptFile[0].rdbuf();
		vConfigVerticalDefaultScriptLog << default_vConfigVerticalScriptFile[0].rdbuf();
		vConfigHorizontalDefaultScriptLog << default_vConfigHorizontalScriptFile[0].rdbuf();
	}
	try
	{
		niawg.initialize();
	}
	catch (myException& except)
	{
		errBox("ERROR: NIAWG Did not start smoothly: " + except.whatStr());
		return -1;
	}

	// Use this section of code to output some characteristics of the 5451. If you want.
	/*
	ViInt32 maximumNumberofWaveforms, waveformQuantum, minimumWaveformSize, maximumWaveformSize;

	if (myNIAWG::NIAWG_CheckWindowsError(niFgen_QueryArbWfmCapabilities(eSessionHandle, &maximumNumberofWaveforms, &waveformQuantum, 
		&minimumWaveformSize, &maximumWaveformSize)))
	{
		return -1;
	}

	MessageBox(NULL, (LPCSTR)std::to_string(maximumNumberofWaveforms).c_str(), NULL, MB_OK);
	MessageBox(NULL, (LPCSTR)std::to_string(waveformQuantum).c_str(), NULL, MB_OK);
	MessageBox(NULL, (LPCSTR)std::to_string(minimumWaveformSize).c_str(), NULL, MB_OK);
	MessageBox(NULL, (LPCSTR)std::to_string(maximumWaveformSize).c_str(), NULL, MB_OK);
	*/
	try
	{
		niawg.setDefaultWaveforms( this );
		// but the default starts in the horizontal configuration, so switch back and start in this config.
		setOrientation( HORIZONTAL_ORIENTATION );
		restartNiawgDefaults();
	}
	catch (myException& exception)
	{
		errBox("ERROR: failed to start niawg default waveforms! Niawg gave the following error message: " + exception.whatStr());
	}
	eCurrentScript = "DefaultScript";
	// not done with the script, it will not stay on the NIAWG, so I need to keep track of it so thatI can reload it onto the NIAWG when necessary.	
	/// Initialize Windows
	TheScriptingWindow = new ScriptingWindow;
	TheCameraWindow = new CameraWindow(this, TheScriptingWindow);
	TheScriptingWindow->getFriends(this, TheCameraWindow);
	TheScriptingWindow->Create(IDD_LARGE_TEMPLATE, 0);
	TheScriptingWindow->ShowWindow(SW_SHOW);
	// initialize the camera window	
	TheCameraWindow->Create(IDD_LARGE_TEMPLATE, 0);
	TheCameraWindow->ShowWindow(SW_SHOW);
	// initialize the COMM.
	comm.initialize( this, TheScriptingWindow, TheCameraWindow );
	int id = 1000;
	POINT statusPos = { 0,0 };
	mainStatus.initialize( statusPos, this, id, 975, "EXPERIMENT STATUS", RGB( 50, 50, 250 ), mainFonts, tooltips );
	statusPos = { 480, 0 };
	errorStatus.initialize( statusPos, this, id, 480, "ERROR STATUS", RGB( 200, 0, 0 ), mainFonts, tooltips );
	debugStatus.initialize( statusPos, this, id, 480, "DEBUG STATUS", RGB( 13, 152, 186 ), mainFonts, tooltips );
	POINT configStart = { 960, 0 };
	profile.initializeControls( configStart, this, id, mainFonts, tooltips );
	POINT notesStart = { 960, 235 };
	notes.initializeControls( notesStart, this, id, mainFonts, tooltips );
	POINT controlLocation = { 1440, 95 };
	variables.initializeControls( controlLocation, this, id, mainFonts, tooltips );
	settings.initialize( id, controlLocation, this, mainFonts, tooltips );
	debugger.initialize( id, controlLocation, this, mainFonts, tooltips );
	texter.initializeControls( controlLocation, this, false, id, mainFonts, tooltips );
	POINT statusLocations = { 960, 910 };
	boxes.initialize( statusLocations, id, this, 960, mainFonts, tooltips );
	shortStatus.initialize( statusLocations, this, id, mainFonts, tooltips );
	niawg.initialize();
	CMenu menu;
	menu.LoadMenu(IDR_MAIN_MENU);
	SetMenu(&menu);
	ShowWindow(SW_MAXIMIZE);
	// just initializes the rectangles.
	TheCameraWindow->redrawPictures( true );
	return TRUE;
}


void MainWindow::restartNiawgDefaults()
{
	niawg.restartDefault();
}


void MainWindow::startExperiment( experimentThreadInput* input )
{
	input->niawg = &niawg;
	manager.startThread( input );
}

HBRUSH MainWindow::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	switch (nCtlColor)
	{
		case CTLCOLOR_STATIC:
		{
			int num = pWnd->GetDlgCtrlID();
			CBrush* ret = shortStatus.handleColor(pWnd, pDC, mainRGBs, mainBrushes);
			if (ret)
			{
				return *ret;
			}
			ret = boxes.handleColoring( num, pDC, mainBrushes, mainRGBs );
			if ( ret )
			{
				return *ret;
			}
			else
			{
				pDC->SetTextColor(mainRGBs["White"]);
				pDC->SetBkColor(mainRGBs["Medium Grey"]);
				return *mainBrushes["Medium Grey"];
			}
		}
		case CTLCOLOR_EDIT:
		{
			pDC->SetTextColor(mainRGBs["White"]);
			pDC->SetBkColor(mainRGBs["Dark Blue"]);
			return *mainBrushes["Dark Blue"];
		}
		case CTLCOLOR_LISTBOX:
		{
			pDC->SetTextColor(mainRGBs["White"]);
			pDC->SetBkColor(mainRGBs["Dark Grey"]);
			return *mainBrushes["Dark Grey"];
		}
		default:
		{
			return *mainBrushes["Light Grey"];
		}
	}
	return NULL;
}


void MainWindow::passCommonCommand(UINT id)
{
	// pass the command id to the common function, filling in the pointers to the windows which own objects needed.
	commonFunctions::handleCommonMessage(id, this, this, TheScriptingWindow, TheCameraWindow);
}


profileSettings MainWindow::getCurentProfileSettings()
{
	return profile.getCurrentProfileSettings();
}


void MainWindow::checkProfileReady()
{
	profile.allSettingsReadyCheck( TheScriptingWindow, this );
}


void MainWindow::checkProfileSave()
{
	profile.checkSaveEntireProfile( TheScriptingWindow, this );
}


void MainWindow::setOrientation(std::string orientation)
{
	profile.setOrientation( orientation );
}


void MainWindow::updateConfigurationSavedStatus(bool status)
{
	profile.updateConfigurationSavedStatus(status);
}


std::string MainWindow::getNotes(std::string whichLevel)
{
	std::transform(whichLevel.begin(), whichLevel.end(), whichLevel.begin(), ::tolower);
	if (whichLevel == "experiment")
	{
		return notes.getExperimentNotes();
	}
	else if (whichLevel == "category")
	{
		return notes.getCategoryNotes();
	}
	else if (whichLevel == "configuration")
	{
		return notes.getConfigurationNotes();
	}
	else
	{
		throw std::invalid_argument(("The Main window's getNotes function was called with a bad argument:" 
			+ whichLevel + ". Acceptable arguments are \"experiment\", \"category\", and \"configuration\". "
			"This throw can be continued successfully, the notes will just be recorded.").c_str());
	}
	return "";
}


void MainWindow::setNotes(std::string whichLevel, std::string newNotes)
{
	std::transform(whichLevel.begin(), whichLevel.end(), whichLevel.begin(), ::tolower);
	if (whichLevel == "experiment")
	{
		notes.setExperimentNotes(newNotes);
	}
	else if (whichLevel == "category")
	{
		notes.setCategoryNotes(newNotes);
	}
	else if (whichLevel == "configuration")
	{
		notes.setConfigurationNotes(newNotes);
	}
	else
	{
		thrower( "ERROR: The Main window's setNotes function was called with a bad argument:" + whichLevel + ". Acceptable arguments are "
				 "\"experiment\", \"category\", and \"configuration\"." );
	}
}


std::vector<variable> MainWindow::getAllVariables()
{
	return variables.getAllVariables();
}


void MainWindow::clearVariables()
{
	variables.clearVariables();
}


void MainWindow::addVariable(std::string name, bool timelike, bool singleton, double value, int item)
{
	variables.addVariable(name, timelike, singleton, value, item);
}


debugInfo MainWindow::getDebuggingOptions()
{
	return debugger.getOptions();
}


void MainWindow::setDebuggingOptions(debugInfo options)
{
	debugger.setOptions(options);
}


mainOptions MainWindow::getMainOptions()
{
	return settings.getOptions();
}


void MainWindow::updateStatusText(std::string whichStatus, std::string text)
{
	std::transform(whichStatus.begin(), whichStatus.end(), whichStatus.begin(), ::tolower);
	if (whichStatus == "error")
	{
		errorStatus.addStatusText(text);
	}
	else if (whichStatus == "debug")
	{
		debugStatus.addStatusText(text);
	}
	else if (whichStatus == "main")
	{
		mainStatus.addStatusText(text);
	}
	else
	{
		thrower( "Main Window's updateStatusText function recieved a bad argument for which status"
				 " control to update. Options are \"error\", \"debug\", and \"main\", but recieved " + whichStatus);
	}
}


void MainWindow::addTimebar(std::string whichStatus)
{
	std::transform(whichStatus.begin(), whichStatus.end(), whichStatus.begin(), ::tolower);
	if (whichStatus == "error")
	{
		errorStatus.appendTimebar();
	}
	else if (whichStatus == "debug")
	{
		debugStatus.appendTimebar();
	}
	else if (whichStatus == "main")
	{
		mainStatus.appendTimebar();
	}
	else
	{
		thrower("Main Window's addTimebar function recieved a bad argument for which status"
			" control to update. Options are \"error\", \"debug\", and \"main\", but recieved " + whichStatus + ". This"
			"exception can be safely ignored.");
	}
}


void MainWindow::setShortStatus(std::string text)
{
	shortStatus.setText(text);
}


void MainWindow::changeShortStatusColor(std::string color)
{
	shortStatus.setColor(color);
}


void MainWindow::passDebugPress(UINT id)
{
	debugger.handleEvent(id, this);
}


void MainWindow::passMainOptionsPress(UINT id)
{
	settings.handleEvent(id, this);
}


void MainWindow::listViewDblClick(NMHDR * pNotifyStruct, LRESULT * result)
{
	variables.updateVariableInfo(this, TheScriptingWindow);
	profile.updateConfigurationSavedStatus(false);
}


void MainWindow::handleRClick(NMHDR * pNotifyStruct, LRESULT * result)
{
	variables.deleteVariable();
	profile.updateConfigurationSavedStatus(false);
}


void MainWindow::handleExperimentCombo()
{
	profile.experimentChangeHandler(TheScriptingWindow, this);
}


void MainWindow::handleCategoryCombo()
{
	profile.categoryChangeHandler(TheScriptingWindow, this);
}


void MainWindow::handleConfigurationCombo()
{
	profile.configurationChangeHandler(TheScriptingWindow, this);
}


void MainWindow::handleSequenceCombo()
{
	profile.sequenceChangeHandler();
}


void MainWindow::handleOrientationCombo()
{
	try
	{
		profile.orientationChangeHandler(this);
	}
	catch (myException& except)
	{
		colorBoxes<char> colors = { /*niawg*/'R', /*camera*/'-', /*intensity*/'-' };
		comm.sendColorBox( colors );
		comm.sendError("ERROR: failed to change orientation: " + except.whatStr());
	}
}

void MainWindow::changeBoxColor( colorBoxes<char> colors )
{
	boxes.changeColor( colors );
}


void MainWindow::setMainOptions(mainOptions options)
{
	settings.setOptions(options);
}


LRESULT MainWindow::onStatusTextMessage(WPARAM wParam, LPARAM lParam)
{
	char* pointerToMessage = (char*)lParam;
	std::string statusMessage(pointerToMessage);
	delete[] pointerToMessage;
	mainStatus.addStatusText(statusMessage);
	return 0;
}


LRESULT MainWindow::onErrorMessage(WPARAM wParam, LPARAM lParam)
{
	char* pointerToMessage = (char*)lParam;
	std::string statusMessage(pointerToMessage);
	delete[] pointerToMessage;
	errorStatus.addStatusText(statusMessage);
	return 0;
}


LRESULT MainWindow::onFatalErrorMessage(WPARAM wParam, LPARAM lParam)
{
	// normal msg stuff
	char* pointerToMessage = (char*)lParam;
	std::string statusMessage(pointerToMessage);
	delete[] pointerToMessage;
	errorStatus.addStatusText(statusMessage);
	//
	myAgilent::agilentDefault();
	std::string msgText = "Exited with Error!\r\nPassively Outputting Default Waveform.";
	changeShortStatusColor("R");
	colorBoxes<char> colors = { /*niawg*/'R', /*camera*/'-', /*intensity*/'-' };
	comm.sendColorBox( colors );
	std::string orientation = getCurentProfileSettings().orientation;
	try
	{
		niawg.restartDefault();
		colorBoxes<char> colors = { /*niawg*/'R', /*camera*/'-', /*intensity*/'-' };
		comm.sendError("EXITED WITH ERROR!");
		comm.sendColorBox( colors );
		comm.sendStatus("EXITED WITH ERROR!\r\nInitialized Default Waveform\r\n");
	}
	catch (myException& except)
	{
		colorBoxes<char> colors = { /*niawg*/'R', /*camera*/'-', /*intensity*/'-' };
		comm.sendError("EXITED WITH ERROR! " + except.whatStr());
		comm.sendColorBox( colors );
		comm.sendStatus("EXITED WITH ERROR!\r\nNIAWG RESTART FAILED!\r\n");
	}
	setNiawgRunningState( false );
	return 0;
}


LRESULT MainWindow::onNormalFinishMessage(WPARAM wParam, LPARAM lParam)
{
	myAgilent::agilentDefault();
	std::string msgText = "Passively Outputting Default Waveform";
	setShortStatus(msgText);
	changeShortStatusColor("B");
	colorBoxes<char> colors = { /*niawg*/'R', /*camera*/'-', /*intensity*/'-' };
	comm.sendColorBox( colors );
	std::string orientation = getCurentProfileSettings().orientation;
	try
	{
		niawg.restartDefault();
	}
	catch (myException& except)
	{
		colorBoxes<char> colors = { /*niawg*/'R', /*camera*/'-', /*intensity*/'-' };
		comm.sendError("ERROR! The niawg finished normally, but upon restarting the default waveform, threw the "
			"following error: " + except.whatStr());
		comm.sendColorBox( colors );
		comm.sendStatus("ERROR!\r\n");
		return -3;
	}
	setNiawgRunningState( false );
	return 0;
}

Communicator* MainWindow::getComm()
{
	return &comm;
}


LRESULT MainWindow::onColoredEditMessage(WPARAM wParam, LPARAM lParam)
{
	char* pointerToMessage = (char*)lParam;
	std::string statusMessage(pointerToMessage);
	delete[] pointerToMessage;
	setShortStatus(statusMessage);
	return 0;
}


LRESULT MainWindow::onDebugMessage(WPARAM wParam, LPARAM lParam)
{
	char* pointerToMessage = (char*)lParam;
	std::string statusMessage(pointerToMessage);
	delete[] pointerToMessage;
	debugStatus.addStatusText(statusMessage);
	return 0;
}


std::unordered_map<std::string, CBrush*> MainWindow::getBrushes()
{
	return mainBrushes;
}


std::unordered_map<std::string, COLORREF> MainWindow::getRGB()
{
	return mainRGBs;
}


CSocket* MainWindow::getSocket()
{
	return &masterSocket;
}
