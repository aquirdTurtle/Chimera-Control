#include "stdafx.h"
#include "commonFunctions.h"
#include "myAgilent.h"
#include "MainWindow.h"
#include "CameraWindow.h"
#include "DeviceWindow.h"

MainWindow::MainWindow(UINT id) : CDialog(id), profile(PROFILES_PATH), masterConfig(MASTER_CONFIGURATION_FILE_ADDRESS)
{
	mainRGBs["Light Green"] = RGB(163, 190, 140);
	mainRGBs["Slate Grey"] = RGB(101, 115, 126);
	mainRGBs["Pale Pink"] = RGB(180, 142, 173);
	mainRGBs["Musky Red"] = RGB(191, 97, 106);
	mainRGBs["Solarized Red"] = RGB(220, 50, 47);
	mainRGBs["Solarized Violet"] = RGB(108, 113, 196);
	mainRGBs["Solarized Cyan"] = RGB(42, 161, 152);
	mainRGBs["Solarized Green"] = RGB(133, 153, 0);
	mainRGBs["Solarized Blue"] = RGB(38, 139, 210);
	mainRGBs["Solarized Magenta"] = RGB(211, 54, 130);
	mainRGBs["Solarized Orange"] = RGB(203, 75, 22);
	mainRGBs["Solarized Yellow"] = RGB(181, 137, 0);
	mainRGBs["Slate Green"] = RGB(23, 84, 81);
	mainRGBs["Dark Grey"] = RGB(15, 15, 15);
	mainRGBs["Dark Grey Red"] = RGB(20, 12, 12);
	mainRGBs["Medium Grey"] = RGB(30, 30, 30);
	mainRGBs["Light Grey"] = RGB(60, 60, 60);
	mainRGBs["Green"] = RGB(50, 200, 50);
	mainRGBs["Red"] = RGB(200, 50, 50);
	mainRGBs["Blue"] = RGB(50, 50, 200);
	mainRGBs["Gold"] = RGB(218, 165, 32);
	mainRGBs["White"] = RGB(255, 255, 255);
	mainRGBs["Light Red"] = RGB(255, 100, 100);
	mainRGBs["Dark Red"] = RGB(150, 0, 0);
	mainRGBs["Light Blue"] = RGB(100, 100, 255);
	mainRGBs["Forest Green"] = RGB(34, 139, 34);
	mainRGBs["Dark Green"] = RGB(0, 50, 0);
	mainRGBs["Dull Red"] = RGB(107, 35, 35);
	mainRGBs["Dark Lavender"] = RGB(100, 100, 205);
	mainRGBs["Teal"] = RGB(0, 255, 255);
	mainRGBs["Tan"] = RGB(210, 180, 140);
	mainRGBs["Purple"] = RGB(147, 112, 219);
	mainRGBs["Orange"] = RGB(255, 165, 0);
	mainRGBs["Brown"] = RGB(139, 69, 19);
	mainRGBs["Black"] = RGB(0, 0, 0);
	mainRGBs["Dark Blue"] = RGB(0, 0, 75);
	// there are less brushes because these are only used for backgrounds.
	mainBrushes["Dark Red"] = new CBrush;
	mainBrushes["Dark Red"]->CreateSolidBrush(mainRGBs["Dark Red"]);
	mainBrushes["Gold"] = new CBrush;
	mainBrushes["Gold"]->CreateSolidBrush(mainRGBs["Gold"]);
	mainBrushes["Dark Grey"] = new CBrush;
	mainBrushes["Dark Grey"]->CreateSolidBrush(mainRGBs["Dark Grey"]);
	mainBrushes["Dark Grey Red"] = new CBrush;
	mainBrushes["Dark Grey Red"]->CreateSolidBrush(mainRGBs["Dark Grey Red"]);
	mainBrushes["Medium Grey"] = new CBrush;
	mainBrushes["Medium Grey"]->CreateSolidBrush(mainRGBs["Medium Grey"]);
	mainBrushes["Light Grey"] = new CBrush;
	mainBrushes["Light Grey"]->CreateSolidBrush(mainRGBs["Light Grey"]);
	mainBrushes["Green"] = new CBrush;
	mainBrushes["Green"]->CreateSolidBrush(mainRGBs["Green"]);
	mainBrushes["Red"] = new CBrush;
	mainBrushes["Red"]->CreateSolidBrush(mainRGBs["Red"]);
	mainBrushes["White"] = new CBrush;
	mainBrushes["White"]->CreateSolidBrush(mainRGBs["White"]);
	mainBrushes["Dull Red"] = new CBrush;
	mainBrushes["Dull Red"]->CreateSolidBrush(mainRGBs["Dull Red"]);
	mainBrushes["Dark Blue"] = new CBrush;
	mainBrushes["Dark Blue"]->CreateSolidBrush(mainRGBs["Dark Blue"]);
	mainBrushes["Dark Green"] = new CBrush;
	mainBrushes["Dark Green"]->CreateSolidBrush(mainRGBs["Dark Green"]);
	/// the following are all equivalent to:
	// mainFonts["Font name"] = new CFont;
	// mainFonts["Font name"].CreateFontA(...);
	(mainFonts["Smaller Font Max"] = new CFont)
		->CreateFontA(27, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
					  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Arial"));
	(mainFonts["Normal Font Max"] = new CFont)
		->CreateFontA(34, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
					  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Arial"));
	(mainFonts["Heading Font Max"] = new CFont)
		->CreateFontA(42, 0, 0, 0, FW_DONTCARE, TRUE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
					  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Old Sans Black"));
	(mainFonts["Code Font Max"] = new CFont)
		->CreateFontA(32, 0, 0, 0, 700, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
					  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Consolas"));
	(mainFonts["Larger Font Max"] = new CFont)
		->CreateFontA(40, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
					  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Arial"));
	(mainFonts["Smaller Font Large"] = new CFont)
		->CreateFontA(14, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
					  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Arial"));
	(mainFonts["Normal Font Large"] = new CFont)
		->CreateFontA(20, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
					  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Arial"));
	(mainFonts["Heading Font Large"] = new CFont)
		->CreateFontA(28, 0, 0, 0, FW_DONTCARE, TRUE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
					  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Old Sans Black"));
	(mainFonts["Code Font Large"] = new CFont)
		->CreateFontA(16, 0, 0, 0, 700, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
					  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Consolas"));
	(mainFonts["Larger Font Large"] = new CFont)
		->CreateFontA(40, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
					  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Arial"));
	(mainFonts["Smaller Font Med"] = new CFont)
		->CreateFontA(8, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
					  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Arial"));
	(mainFonts["Normal Font Med"] = new CFont)
		->CreateFontA(12, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
					  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Arial"));
	(mainFonts["Heading Font Med"] = new CFont)
		->CreateFontA(16, 0, 0, 0, FW_DONTCARE, TRUE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
					  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Old Sans Black"));
	(mainFonts["Code Font Med"] = new CFont)
		->CreateFontA(10, 0, 0, 0, 700, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
					  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Consolas"));
	(mainFonts["Larger Font Med"] = new CFont)
		->CreateFontA(22, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
					  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Arial"));
	(mainFonts["Smaller Font Small"] = new CFont)
		->CreateFontA(6, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
					  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Arial"));
	(mainFonts["Normal Font Small"] = new CFont)
		->CreateFontA(8, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
					  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Arial"));
	(mainFonts["Heading Font Small"] = new CFont)
		->CreateFontA(12, 0, 0, 0, FW_DONTCARE, TRUE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
					  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Old Sans Black"));
	(mainFonts["Code Font Small"] = new CFont)
		->CreateFontA(7, 0, 0, 0, 700, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
					  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Consolas"));
	(mainFonts["Larger Font Small"] = new CFont)
		->CreateFontA(16, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
					  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Arial"));
}


IMPLEMENT_DYNAMIC(MainWindow, CDialog)

BEGIN_MESSAGE_MAP( MainWindow, CDialog )
	ON_WM_CTLCOLOR()
	ON_WM_SIZE()
	ON_COMMAND_RANGE( ID_ACCELERATOR_ESC, ID_ACCELERATOR_ESC, &MainWindow::passCommonCommand )
	ON_COMMAND_RANGE( ID_ACCELERATOR_F5, ID_ACCELERATOR_F5, &MainWindow::passCommonCommand )
	ON_COMMAND_RANGE(ID_ACCELERATOR_F2, ID_ACCELERATOR_F2, &MainWindow::passCommonCommand )
	ON_COMMAND_RANGE( ID_ACCELERATOR_F1, ID_ACCELERATOR_F1, &MainWindow::passCommonCommand )
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
	ON_NOTIFY(NM_DBLCLK, IDC_SMS_TEXTING_LISTVIEW, &MainWindow::handleDblClick)
	ON_NOTIFY(NM_RCLICK, IDC_SMS_TEXTING_LISTVIEW, &MainWindow::handleRClick)

	ON_REGISTERED_MESSAGE( eRepProgressMessageID, &MainWindow::onRepProgress )
	ON_REGISTERED_MESSAGE( eStatusTextMessageID, &MainWindow::onStatusTextMessage )
	ON_REGISTERED_MESSAGE( eErrorTextMessageID, &MainWindow::onErrorMessage )
	ON_REGISTERED_MESSAGE( eFatalErrorMessageID, &MainWindow::onFatalErrorMessage )
	ON_REGISTERED_MESSAGE( eColoredEditMessageID, &MainWindow::onColoredEditMessage )
	ON_REGISTERED_MESSAGE( eDebugMessageID, &MainWindow::onDebugMessage )

	ON_COMMAND_RANGE( IDC_MAIN_STATUS_BUTTON, IDC_MAIN_STATUS_BUTTON, &MainWindow::passClear )
	ON_COMMAND_RANGE( IDC_ERROR_STATUS_BUTTON, IDC_ERROR_STATUS_BUTTON, &MainWindow::passClear )
	ON_COMMAND_RANGE( IDC_DEBUG_STATUS_BUTTON, IDC_DEBUG_STATUS_BUTTON, &MainWindow::passClear )

END_MESSAGE_MAP()


void MainWindow::handlePause()
{
	if (masterThreadManager.runningStatus())
	{
		if (masterThreadManager.getIsPaused())
		{
			// then it's currently paused, so unpause it.
			menu.CheckMenuItem(ID_RUNMENU_PAUSE, MF_UNCHECKED);
			masterThreadManager.unPause();
			comm.sendColorBox( Master, 'G' );
		}
		else
		{
			// then not paused so pause it.
			menu.CheckMenuItem(ID_RUNMENU_PAUSE, MF_CHECKED);
			comm.sendColorBox( Master, 'Y' );
			masterThreadManager.pause();
		}
	}
	else
	{
		comm.sendStatus("Can't pause, experiment was not running.\r\n");
	}
}


LRESULT MainWindow::onRepProgress(WPARAM wParam, LPARAM lParam)
{
	repetitionControl.updateNumber(lParam);
	return NULL;
}


void MainWindow::handleSaveConfig(std::ofstream& saveFile)
{
	notes.handleSaveConfig(saveFile);
	settings.handleSaveConfig(saveFile);
	debugger.handleSaveConfig(saveFile);
	repetitionControl.handleSaveConfig(saveFile);
}


void MainWindow::handleOpeningConfig(std::ifstream& configFile, double version)
{
	notes.handleOpenConfig( configFile,  version );
	settings.handleOpenConfig( configFile, version );
	debugger.handleOpenConfig( configFile, version );
	repetitionControl.handleOpenConfig(configFile, version);
}


void MainWindow::OnSize(UINT nType, int cx, int cy)
{
	profile.rearrange(cx, cy, getFonts());
	notes.rearrange(cx, cy, getFonts());
	debugger.rearrange(cx, cy, getFonts());
	settings.rearrange(cx, cy, getFonts());
	mainStatus.rearrange(cx, cy, getFonts());
	debugStatus.rearrange(cx, cy, getFonts());
	errorStatus.rearrange(cx, cy, getFonts());
	texter.rearrange(cx, cy, getFonts());
	shortStatus.rearrange(cx, cy, getFonts());
	boxes.rearrange("", "", cx, cy, getFonts());
	repetitionControl.rearrange(cx, cy, getFonts());
	RedrawWindow();
}


void MainWindow::setNiawgRunningState( bool newRunningState )
{
	niawg.setRunningState( newRunningState );
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


fontMap MainWindow::getFonts()
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


UINT MainWindow::getRepNumber()
{
	return repetitionControl.getRepetitionNumber();
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
	std::string logFolderNameStart = "Date " + str(datePointerStart.tm_year + 1900) + "-" + str(datePointerStart.tm_mon + 1) + "-"
		+ str(datePointerStart.tm_mday) + " Time " + str(datePointerStart.tm_hour) + "-" + str(datePointerStart.tm_min) + "-"
		+ str(datePointerStart.tm_sec);
	// initialize default file names and open the files.
	std::vector<std::fstream> default_hConfigVerticalScriptFile, default_hConfigHorizontalScriptFile, default_vConfigVerticalScriptFile,
		default_vConfigHorizontalScriptFile;
	default_hConfigVerticalScriptFile.push_back(std::fstream(DEFAULT_SCRIPT_FOLDER_PATH + str("DEFAULT_HCONFIG_VERTICAL_SCRIPT.nScript")));
	default_hConfigHorizontalScriptFile.push_back(std::fstream(DEFAULT_SCRIPT_FOLDER_PATH + str("DEFAULT_HCONFIG_HORIZONTAL_SCRIPT.nScript")));
	default_vConfigVerticalScriptFile.push_back(std::fstream(DEFAULT_SCRIPT_FOLDER_PATH + str("DEFAULT_VCONFIG_VERTICAL_SCRIPT.nScript")));
	default_vConfigHorizontalScriptFile.push_back(std::fstream(DEFAULT_SCRIPT_FOLDER_PATH + str("DEFAULT_VCONFIG_HORIZONTAL_SCRIPT.nScript")));

	// check errors
	if (!default_hConfigVerticalScriptFile[0].is_open())
	{
		errBox(str("FATAL ERROR: Couldn't open default file. Was looking for file ") + DEFAULT_SCRIPT_FOLDER_PATH + "DEFAULT_HCONFIG_VERTICAL_SCRIPT.nScript");
		return 0;
	}
	if (!default_hConfigHorizontalScriptFile[0].is_open())
	{
		errBox(str("FATAL ERROR: Couldn't open default file. Was looking for file ") + DEFAULT_SCRIPT_FOLDER_PATH + "DEFAULT_HCONFIG_HORIZONTAL_SCRIPT.nScript");
		return 0;
	}
	if (!default_vConfigVerticalScriptFile[0].is_open())
	{
		errBox(str("FATAL ERROR: Couldn't open default file. Was looking for file ") + DEFAULT_SCRIPT_FOLDER_PATH + "DEFAULT_VCONFIG_VERTICAL_SCRIPT.nScript");
		return 0;
	}
	if (!default_vConfigHorizontalScriptFile[0].is_open())
	{
		errBox(str("FATAL ERROR: Couldn't open default file. Was looking for file ") + DEFAULT_SCRIPT_FOLDER_PATH + "DEFAULT_VCONFIG_HORIZONTAL_SCRIPT.nScript");
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
	catch (Error& except)
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

	MessageBox(NULL, (LPCSTR)str(maximumNumberofWaveforms).c_str(), NULL, MB_OK);
	MessageBox(NULL, (LPCSTR)str(waveformQuantum).c_str(), NULL, MB_OK);
	MessageBox(NULL, (LPCSTR)str(minimumWaveformSize).c_str(), NULL, MB_OK);
	MessageBox(NULL, (LPCSTR)str(maximumWaveformSize).c_str(), NULL, MB_OK);
	*/
	try
	{
		niawg.setDefaultWaveforms( this );
		// but the default starts in the horizontal configuration, so switch back and start in this config.
		setOrientation( HORIZONTAL_ORIENTATION );
		restartNiawgDefaults();
	}
	catch (Error& exception)
	{
		errBox("ERROR: failed to start niawg default waveforms! Niawg gave the following error message: " + exception.whatStr());
	}
	// not done with the script, it will not stay on the NIAWG, so I need to keep track of it so thatI can reload it onto the NIAWG when necessary.	
	/// Initialize Windows
	TheScriptingWindow = new ScriptingWindow;
	TheCameraWindow = new CameraWindow;
	TheDeviceWindow = new DeviceWindow;
	TheScriptingWindow->getFriends(this, TheCameraWindow, TheDeviceWindow);
	TheCameraWindow->getFriends(this, TheScriptingWindow, TheDeviceWindow);
	TheDeviceWindow->getFriends(this, TheScriptingWindow, TheCameraWindow);

	try
	{
		TheScriptingWindow->Create(IDD_LARGE_TEMPLATE, 0);
		//TheScriptingWindow->ShowWindow(SW_SHOW);
		TheCameraWindow->Create(IDD_LARGE_TEMPLATE1, 0);
		//TheCameraWindow->ShowWindow(SW_SHOW);
		TheDeviceWindow->Create(IDD_LARGE_TEMPLATE, 0);
		//TheDeviceWindow->ShowWindow(SW_SHOW);
	}
	catch (Error& err)
	{
		errBox(err.what());
	}
	// initialize the COMM.
	comm.initialize( this, TheScriptingWindow, TheCameraWindow, TheDeviceWindow );
	int id = 1000;
	POINT controlLocation = { 0,0 };
	mainStatus.initialize( controlLocation, this, id, 975, "EXPERIMENT STATUS", RGB( 50, 50, 250 ), mainFonts, tooltips );
	controlLocation = { 480, 0 };
	errorStatus.initialize( controlLocation, this, id, 480, "ERROR STATUS", RGB( 200, 0, 0 ), mainFonts, tooltips );
	debugStatus.initialize( controlLocation, this, id, 480, "DEBUG STATUS", RGB( 13, 152, 186 ), mainFonts, tooltips );
	controlLocation = { 960, 0 };
	profile.initialize( controlLocation, this, id, mainFonts, tooltips );
	controlLocation = { 960, 235 };
	notes.initialize( controlLocation, this, id, mainFonts, tooltips );
	controlLocation = { 1440, 95 };
	repetitionControl.initialize(controlLocation, tooltips, this, id);
	settings.initialize( id, controlLocation, this, mainFonts, tooltips );
	debugger.initialize( id, controlLocation, this, mainFonts, tooltips );
	texter.initialize( controlLocation, this, false, id, mainFonts, tooltips );
	controlLocation = { 960, 910 };
	boxes.initialize( controlLocation, id, this, 960, mainFonts, tooltips );
	shortStatus.initialize( controlLocation, this, id, mainFonts, tooltips );
	menu.LoadMenu(IDR_MAIN_MENU);
	SetMenu(&menu);	
	// just initializes the rectangles.
	TheCameraWindow->redrawPictures( true );
	try
	{
		masterConfig.load(this, TheDeviceWindow, TheCameraWindow);
	}
	catch (Error& err)
	{
		errBox(err.what());
	}
	std::string initializationString;
	initializationString += TheDeviceWindow->getSystemStatusMsg();
	errBox(initializationString);	
	ShowWindow(SW_MAXIMIZE);
	TheCameraWindow->ShowWindow( SW_MAXIMIZE );
	TheScriptingWindow->ShowWindow( SW_MAXIMIZE );
	TheDeviceWindow->ShowWindow( SW_MAXIMIZE );

	return TRUE;
}


bool MainWindow::masterIsRunning()
{
	return masterThreadManager.runningStatus();
}


RunInfo MainWindow::getRunInfo()
{
	return systemRunningInfo;
}


void MainWindow::restartNiawgDefaults()
{
	niawg.restartDefault();
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
	try
	{
		commonFunctions::handleCommonMessage ( id, this, this, TheScriptingWindow, TheCameraWindow, TheDeviceWindow );
	}
	catch (Error& exception)
	{
		errBox ( exception.what () );
	}
}


void MainWindow::startMaster(MasterThreadInput* input)
{
	masterThreadManager.startExperimentThread(input);
}


void MainWindow::fillMasterThreadInput(MasterThreadInput* input)
{
	input->masterScriptAddress = profile.getMasterAddressFromConfig();
	input->programIntensity = settings.getOptions().programIntensity;
	input->repetitionNumber = getRepNumber();
	input->debugOptions = debugger.getOptions();
	input->profile = profile.getCurrentProfileSettings();
	input->niawg = &niawg;
	input->comm = &comm;
}


profileSettings MainWindow::getCurrentProfileSettings()
{
	return profile.getCurrentProfileSettings();
}


void MainWindow::checkProfileReady()
{
	profile.allSettingsReadyCheck( TheScriptingWindow, this, TheDeviceWindow, TheCameraWindow );
}


void MainWindow::checkProfileSave()
{
	profile.checkSaveEntireProfile( TheScriptingWindow, this, TheDeviceWindow, TheCameraWindow );
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
		thrower("The Main window's getNotes function was called with a bad argument:" 
			    + whichLevel + ". Acceptable arguments are \"experiment\", \"category\", and \"configuration\". "
			    "This throw can be continued successfully, the notes will just be recorded.");
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


void MainWindow::handleDblClick(NMHDR * pNotifyStruct, LRESULT * result)
{
	texter.updatePersonInfo();
	//variables.updateVariableInfo(this, TheScriptingWindow);
	profile.updateConfigurationSavedStatus(false);
}


void MainWindow::handleRClick(NMHDR * pNotifyStruct, LRESULT * result)
{
	texter.deletePersonInfo();
	//variables.deleteVariable();
	profile.updateConfigurationSavedStatus(false);
}


void MainWindow::handleExperimentCombo()
{
	try
	{
		profile.experimentChangeHandler(TheScriptingWindow, this);
		TheDeviceWindow->setConfigActive(false);
	}
	catch (Error& err)
	{
		getComm()->sendError(err.what());
	}
}


void MainWindow::handleCategoryCombo()
{
	try
	{
		profile.categoryChangeHandler(TheScriptingWindow, this);
		TheDeviceWindow->setConfigActive(false);
	}
	catch (Error& err)
	{
		getComm()->sendError(err.what());
	}
}


void MainWindow::handleConfigurationCombo()
{
	try
	{
		profile.configurationChangeHandler(TheScriptingWindow, this, TheDeviceWindow, TheCameraWindow);
		TheDeviceWindow->setConfigActive(true);
	}
	catch (Error& err)
	{
		getComm()->sendError(err.what());
	}
}


void MainWindow::handleSequenceCombo()
{
	try
	{
		profile.sequenceChangeHandler();
	}
	catch (Error& err)
	{
		getComm()->sendError(err.what());
	}
}


void MainWindow::handleOrientationCombo()
{
	try
	{
		profile.orientationChangeHandler(this);
		TheDeviceWindow->setConfigActive(false);
	}
	catch (Error& except)
	{
		comm.sendColorBox( Niawg, 'R' );
		comm.sendError("ERROR: failed to change orientation: " + except.whatStr());
	}
}

void MainWindow::changeBoxColor( systemInfo<char> colors )
{
	boxes.changeColor( colors );
	if (colors.camera == 'R' || colors.intensity == 'R' || colors.niawg == 'R')
	{
		changeShortStatusColor("R");
	}
	else if (colors.camera == 'Y' || colors.intensity == 'Y' || colors.niawg == 'Y')
	{
		changeShortStatusColor("Y");
	}
	else if (colors.camera == 'G' || colors.intensity == 'G' || colors.niawg == 'G')
	{
		changeShortStatusColor("G");
	}
	else
	{
		changeShortStatusColor("B");
	}
}


void MainWindow::setMainOptions(mainOptions options)
{
	settings.setOptions(options);
}


void MainWindow::abortMasterThread()
{
	/// needs implementation...
	if (masterThreadManager.runningStatus())
	{
		masterThreadManager.abort();
	}
	else
	{
		thrower("Can't abort, experiment was not running.\r\n");
	}
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
	comm.sendColorBox( Niawg, 'R' );
	std::string orientation = getCurrentProfileSettings().orientation;
	try
	{
		niawg.restartDefault();
		comm.sendError("EXITED WITH ERROR!");
		comm.sendColorBox( Niawg, 'R' );
		comm.sendStatus("EXITED WITH ERROR!\r\nInitialized Default Waveform\r\n");
	}
	catch (Error& except)
	{
		comm.sendError("EXITED WITH ERROR! " + except.whatStr());
		comm.sendColorBox( Niawg, 'R' );
		comm.sendStatus("EXITED WITH ERROR!\r\nNIAWG RESTART FAILED!\r\n");
	}
	setNiawgRunningState( false );
	return 0;
}

// I think I can delete this...
LRESULT MainWindow::onNormalFinishMessage(WPARAM wParam, LPARAM lParam)
{
	myAgilent::agilentDefault();
	std::string msgText = "Passively Outputting Default Waveform";
	setShortStatus(msgText);
	changeShortStatusColor("B");
	comm.sendColorBox( Niawg, 'B' );
	std::string orientation = getCurrentProfileSettings().orientation;
	try
	{
		niawg.restartDefault();
	}
	catch (Error& except)
	{
		comm.sendError("ERROR! The niawg finished normally, but upon restarting the default waveform, threw the "
			"following error: " + except.whatStr());
		comm.sendColorBox( Niawg, 'B' );
		comm.sendStatus("ERROR!\r\n");
		return -3;
	}
	setNiawgRunningState( false );
	return 0;
}


void MainWindow::handleFinish()
{
	time_t t = time(0);
	struct tm now;
	localtime_s(&now, &t);
	std::string message = "Experiment Completed at ";
	if (now.tm_hour < 10)
	{
		message += "0";
	}
	message += str(now.tm_hour) + ":";
	if (now.tm_min < 10)
	{
		message += "0";
	}
	message += str(now.tm_min) + ":";
	if (now.tm_sec < 10)
	{
		message += "0";
	}
	message += str(now.tm_sec);

	try
	{
		texter.sendMessage(message, &python, "Finished");
	}
	catch (Error& err)
	{
		comm.sendError(err.what());
	}
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


brushMap MainWindow::getBrushes()
{
	return mainBrushes;
}


rgbMap MainWindow::getRgbs()
{
	return mainRGBs;
}
