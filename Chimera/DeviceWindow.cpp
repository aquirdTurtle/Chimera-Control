#include "stdafx.h"
#include "DeviceWindow.h"
#include "Control.h"
#include "TtlSettingsDialog.h"
#include "DacSettingsDialog.h"
#include "TextPromptDialog.h"
#include "TtlSystem.h"
#include "explorerOpen.h"
#include "commonFunctions.h"

IMPLEMENT_DYNAMIC( DeviceWindow, CDialog )

BEGIN_MESSAGE_MAP( DeviceWindow, CDialog )
	
	ON_WM_CTLCOLOR()
	ON_WM_SIZE()
	
	ON_COMMAND_RANGE( MENU_ID_RANGE_BEGIN, MENU_ID_RANGE_END, &DeviceWindow::passCommonCommand)
	ON_COMMAND_RANGE( TTL_ID_BEGIN, TTL_ID_END, &DeviceWindow::handleTtlPush )

	ON_COMMAND( TTL_HOLD, &DeviceWindow::handlTtlHoldPush )
	ON_COMMAND( ID_DAC_SET_BUTTON, &DeviceWindow::SetDacs )
	ON_COMMAND( IDC_ZERO_TTLS, &DeviceWindow::zeroTtls )
	ON_COMMAND( IDC_ZERO_DACS, &DeviceWindow::zeroDacs )
	ON_COMMAND( IDOK, &DeviceWindow::handleEnter)
	
	ON_COMMAND_RANGE(IDC_TOP_BOTTOM_CHANNEL1_BUTTON, IDC_FLASHING_AGILENT_COMBO, &DeviceWindow::handleAgilentOptions)
	ON_COMMAND_RANGE(TOP_ON_OFF, AXIAL_FSK, &DeviceWindow::handleTektronicsButtons)	
	ON_CBN_SELENDOK( IDC_TOP_BOTTOM_AGILENT_COMBO, &DeviceWindow::handleTopBottomAgilentCombo )
	ON_CBN_SELENDOK( IDC_AXIAL_UWAVE_AGILENT_COMBO, &DeviceWindow::handleAxialUWaveAgilentCombo )
	ON_CBN_SELENDOK( IDC_FLASHING_AGILENT_COMBO, &DeviceWindow::handleFlashingAgilentCombo )	

	ON_CONTROL_RANGE( EN_CHANGE, ID_DAC_FIRST_EDIT, (ID_DAC_FIRST_EDIT + 23), &DeviceWindow::DacEditChange )
	ON_NOTIFY( LVN_COLUMNCLICK, IDC_CONFIG_VARS_LISTVIEW, &DeviceWindow::ConfigVarsColumnClick )
	ON_NOTIFY( NM_DBLCLK, IDC_CONFIG_VARS_LISTVIEW, &DeviceWindow::ConfigVarsDblClick )
	ON_NOTIFY( NM_RCLICK, IDC_CONFIG_VARS_LISTVIEW, &DeviceWindow::ConfigVarsRClick )

	ON_NOTIFY( NM_DBLCLK, IDC_GLOBAL_VARS_LISTVIEW, &DeviceWindow::GlobalVarDblClick )
	ON_NOTIFY( NM_RCLICK, IDC_GLOBAL_VARS_LISTVIEW, &DeviceWindow::GlobalVarRClick )
	ON_NOTIFY_RANGE( NM_CUSTOMDRAW, IDC_GLOBAL_VARS_LISTVIEW, IDC_GLOBAL_VARS_LISTVIEW, &DeviceWindow::drawVariables)
	ON_NOTIFY_RANGE( NM_CUSTOMDRAW, IDC_CONFIG_VARS_LISTVIEW, IDC_CONFIG_VARS_LISTVIEW, &DeviceWindow::drawVariables)
END_MESSAGE_MAP()


std::pair<UINT, UINT> DeviceWindow::getTtlBoardSize()
{
	return ttlBoard.getTtlBoardSize();
}


void DeviceWindow::handleSaveConfig(std::ofstream& saveFile)
{
	// order matters.
	configVariables.handleSaveConfig(saveFile);
	ttlBoard.handleSaveConfig(saveFile);
	dacBoards.handleSaveConfig(saveFile);
	topBottomAgilent.handleSavingConfig(saveFile);
	uWaveAxialAgilent.handleSavingConfig(saveFile);
	flashingAgilent.handleSavingConfig(saveFile);
	tektronics1.handleSaveConfig(saveFile);
	tektronics2.handleSaveConfig(saveFile);
}


void DeviceWindow::handleOpeningConfig(std::ifstream& configFile, double version)
{
	configVariables.handleOpenConfig(configFile, version);

	ttlBoard.handleOpenConfig(configFile, version);
	dacBoards.handleOpenConfig(configFile, version, &ttlBoard);

	topBottomAgilent.readConfigurationFile(configFile);
	uWaveAxialAgilent.readConfigurationFile(configFile);
	flashingAgilent.readConfigurationFile(configFile);

	tektronics1.handleOpeningConfig(configFile, version);
	tektronics2.handleOpeningConfig(configFile, version);
}


UINT DeviceWindow::getNumberOfDacs()
{
	return dacBoards.getNumberOfDacs();
}


std::array<std::array<std::string, 16>, 4> DeviceWindow::getTtlNames()
{
	return ttlBoard.getAllNames();
}


std::array<std::string, 24> DeviceWindow::getDacNames()
{
	return dacBoards.getAllNames();
}


void DeviceWindow::drawVariables(UINT id, NMHDR* pNMHDR, LRESULT* pResult)
{
	if (id == IDC_GLOBAL_VARS_LISTVIEW)
	{
		globalVariables.handleDraw(pNMHDR, pResult);
	}
	else
	{
		configVariables.handleDraw(pNMHDR, pResult);
	}
}


void DeviceWindow::ConfigVarsDblClick(NMHDR * pNotifyStruct, LRESULT * result)
{
	std::vector<Script*> scriptList;
	try
	{
		configVariables.updateVariableInfo(scriptList, mainWindowFriend, this);
	}
	catch (Error& exception)
	{
		sendErr("Variables Double Click Handler : " + exception.whatStr() + "\r\n");
	}
	mainWindowFriend->updateConfigurationSavedStatus(false);
}


void DeviceWindow::ConfigVarsRClick(NMHDR * pNotifyStruct, LRESULT * result)
{
	try
	{
		configVariables.deleteVariable();
	}
	catch (Error& exception)
	{
		sendErr("Variables Right Click Handler : " + exception.whatStr() + "\r\n");
	}
	mainWindowFriend->updateConfigurationSavedStatus(false);
}

std::vector<variable> DeviceWindow::getAllVariables()
{
	std::vector<variable> vars = configVariables.getEverything();
	std::vector<variable> vars2 = globalVariables.getEverything();
	vars.insert(vars.end(), vars2.begin(), vars2.end());
	return vars;
}


void DeviceWindow::GlobalVarDblClick(NMHDR * pNotifyStruct, LRESULT * result)
{
	std::vector<Script*> scriptList;
	try
	{
		globalVariables.updateVariableInfo(scriptList, mainWindowFriend, this);
	}
	catch (Error& exception)
	{
		sendErr("Global Variables Double Click Handler : " + exception.whatStr() + "\r\n");
	}
}


void DeviceWindow::GlobalVarRClick(NMHDR * pNotifyStruct, LRESULT * result)
{
	try
	{
		globalVariables.deleteVariable();
	}
	catch (Error& exception)
	{
		sendErr("Global Variables Right Click Handler : " + exception.whatStr() + "\r\n");
	}
	//masterConfig.save(&ttlBoard, &dacBoards, &globalVariables);
}



void DeviceWindow::ConfigVarsColumnClick(NMHDR * pNotifyStruct, LRESULT * result)
{
	try
	{
		configVariables.handleColumnClick(pNotifyStruct, result);
	}
	catch (Error& exception)
	{
		sendErr("Handling config variable listview click : " + exception.whatStr() + "\r\n");
	}
	mainWindowFriend->updateConfigurationSavedStatus(false);
}


void DeviceWindow::clearVariables()
{
	configVariables.clearVariables();
}


void DeviceWindow::addVariable(std::string name, bool timelike, bool singleton, double value, int item)
{
	variable var;
	var.name = name;
	var.timelike = timelike;
	var.constant = singleton;
	var.ranges.push_back({ value, 0, 1, false, true });
	configVariables.addConfigVariable(var, item);
}


void DeviceWindow::passCommonCommand(UINT id)
{
	try
	{
		commonFunctions::handleCommonMessage(id, this, mainWindowFriend, scriptingWindowFriend, cameraWindowFriend, this);
	}
	catch (Error& err)
	{
		// catch any extra errors that handleCommonMessage doesn't explicitly handle.
		errBox(err.what());
	}
}

void DeviceWindow::getFriends(MainWindow* mainWin, ScriptingWindow* scriptWin, CameraWindow* camWin)
{
	mainWindowFriend = mainWin;
	scriptingWindowFriend = scriptWin;
	cameraWindowFriend = camWin;
}


void DeviceWindow::passRoundToDac()
{
	dacBoards.handleRoundToDac(menu);
}


void DeviceWindow::handleTektronicsButtons(UINT id)
{
	if (id >= TOP_ON_OFF && id <= BOTTOM_FSK)
	{
		tektronics1.handleButtons(id - TOP_ON_OFF);
	}
	if (id >= EO_ON_OFF && id <= AXIAL_FSK)
	{
		tektronics2.handleButtons(id - EO_ON_OFF);
	}
	mainWindowFriend->updateConfigurationSavedStatus(false);
}


void DeviceWindow::handleEnter()
{
	errBox("Hello, there!");
}


void DeviceWindow::setConfigActive(bool active)
{
	configVariables.setActive(active);
}


UINT DeviceWindow::getTotalVariationNumber()
{
	return configVariables.getTotalVariationNumber();
}


void DeviceWindow::OnSize(UINT nType, int cx, int cy)
{
	tektronics1.rearrange(cx, cy, getFonts());
	tektronics2.rearrange(cx, cy, getFonts());

	topBottomAgilent.rearrange(cx, cy, getFonts());
	uWaveAxialAgilent.rearrange(cx, cy, getFonts());
	flashingAgilent.rearrange(cx, cy, getFonts());

	RhodeSchwarzGenerator.rearrange(cx, cy, getFonts());

	ttlBoard.rearrange(cx, cy, getFonts());
	dacBoards.rearrange(cx, cy, getFonts());

	masterKey.rearrange(cx, cy, getFonts());

	configVariables.rearrange(cx, cy, getFonts());
	globalVariables.rearrange(cx, cy, getFonts());

	statusBox.rearrange("", "", cx, cy, getFonts());
}


fontMap DeviceWindow::getFonts()
{
	return mainWindowFriend->getFonts();
}


void DeviceWindow::handleAgilentOptions( UINT id )
{
	// zero the id.
	id -= IDC_TOP_BOTTOM_CHANNEL1_BUTTON;
	int agilentNum = id / 8;
	// figure out which box it was.
	Agilent* agilent = NULL;
	if (agilentNum == 0)
	{
		agilent = &topBottomAgilent;
	}
	else if (agilentNum == 1)
	{
		agilent = &uWaveAxialAgilent;
	}
	else if (agilentNum == 2)
	{
		agilent = &flashingAgilent;
	}
	else
	{
		errBox( "4th agilent???" );
		return;
	}
	// call the correct function.
	if (id % 8 == 0)
	{
		// channel 1
		agilent->handleChannelPress( 1 );
	}
	else if (id % 8 == 1)
	{
		// channel 2
		agilent->handleChannelPress( 2 );
	}
	else if (id % 8 == 3)
	{
		// TODO
		// sync 
		//agilent->handleSync();
	}
	// else it's a boring combo that I don't care about at the moment.
}


void DeviceWindow::handleTopBottomAgilentCombo()
{
	topBottomAgilent.handleCombo();
}


void DeviceWindow::handleAxialUWaveAgilentCombo()
{
	uWaveAxialAgilent.handleCombo();
}


void DeviceWindow::handleFlashingAgilentCombo()
{
	flashingAgilent.handleCombo();
}


void DeviceWindow::sendErr(std::string msg)
{
	mainWindowFriend->getComm()->sendError(msg);
}


void DeviceWindow::sendStat(std::string msg)
{
	mainWindowFriend->getComm()->sendStatus(msg);
}


void DeviceWindow::zeroDacs()
{
	try
	{
		dacBoards.resetDacEvents();
		ttlBoard.resetTtlEvents();
		for (int dacInc = 0; dacInc < 24; dacInc++)
		{
			dacBoards.prepareDacForceChange( dacInc, 0, &ttlBoard );
		}
		dacBoards.analyzeDacCommands(0);
		dacBoards.makeFinalDataFormat(0);
		dacBoards.stopDacs(); 
		dacBoards.configureClocks(0);
		dacBoards.writeDacs(0);
		dacBoards.startDacs();
		ttlBoard.analyzeCommandList(0);
		ttlBoard.convertToFinalFormat(0);
		ttlBoard.writeData(0);
		ttlBoard.startBoard();
		ttlBoard.waitTillFinished(0);
		sendStat( "Zero'd DACs.\r\n");
	}
	catch (Error& exception)
	{
		sendStat( "Failed to Zero DACs!!!\r\n" );
		sendErr( exception.what() );
	}
}


void DeviceWindow::zeroTtls()
{
	try
	{
		ttlBoard.zeroBoard();
		sendStat( "Zero'd TTLs.\r\n" );
	}
	catch (Error& exception)
	{
		sendStat( "Failed to Zero TTLs!!!\r\n" );
		sendErr( exception.what() );
	}
}

void DeviceWindow::loadMotSettings()
{
	try
	{
		sendStat("Loading MOT Configuration...\r\n" );
		//
		MasterThreadInput* input = new MasterThreadInput;
		input->quiet = true;
		input->ttls = &ttlBoard;
		input->dacs = &dacBoards;
		input->globalControl = &globalVariables;
		// don't get configuration variables. The MOT shouldn't depend on config variables.
		input->vars = globalVariables.getEverything();
		// Only set it once, clearly.
		input->repetitionNumber = 1;
		input->key = &masterKey;
		input->masterScriptAddress = MOT_ROUTINE_ADDRESS;
		input->rsg = &RhodeSchwarzGenerator;
		input->gpib = &gpib;
		input->agilents.push_back( &topBottomAgilent );
		input->agilents.push_back( &uWaveAxialAgilent );
		input->tektronics1 = &tektronics1;
		input->tektronics2 = &tektronics2;

		manager.loadMotSettings(input);
	}
	catch (Error& exception)
	{
		sendStat(": " + exception.whatStr() + " " + exception.whatStr() + "\r\n" );
	}
}


// Gets called after alt-f4 or X button is pressed.
void DeviceWindow::OnCancel()
{
	passCommonCommand(ID_FILE_MY_EXIT);
}


void DeviceWindow::fillMasterThreadInput(MasterThreadInput* input)
{
	input->ttls = &ttlBoard;
	input->dacs = &dacBoards;
	input->globalControl = &globalVariables;

	// load the variables. This little loop is for letting configuration variables overwrite the globals.
	std::vector<variable> configVars = configVariables.getEverything();
	std::vector<variable> globals = globalVariables.getEverything();
	std::vector<variable> experimentVars = configVars;

	for (auto& globalVar : globals)
	{
		globalVar.overwritten = false;
		bool nameExists = false;
		for (auto& configVar : experimentVars)
		{
			if (configVar.name == globalVar.name)
			{
				globalVar.overwritten = true;
				configVar.overwritten = true;
			}
		}
		if (!globalVar.overwritten)
		{
			experimentVars.push_back(globalVar);
		}
	}
	input->vars = experimentVars;
	globalVariables.setUsages(globals);
	input->key = &masterKey;
	input->rsg = &RhodeSchwarzGenerator;
	input->gpib = &gpib;
	input->agilents.push_back(&topBottomAgilent);
	input->agilents.push_back(&uWaveAxialAgilent);
	tektronics1.getSettings();
	tektronics2.getSettings();
	input->tektronics1 = &tektronics1;
	input->tektronics2 = &tektronics2;
}


void DeviceWindow::changeBoxColor(systemInfo<char> colors)
{
	statusBox.changeColor(colors);
}


void DeviceWindow::handleAbort()
{
	ttlBoard.unshadeTtls();
	dacBoards.unshadeDacs();
}


void DeviceWindow::handleMasterConfigSave(std::stringstream& configStream)
{
	// save info
	/// ttls
	for (int ttlRowInc = 0; ttlRowInc < ttlBoard.getTtlBoardSize().first; ttlRowInc++)
	{
		for (int ttlNumberInc = 0; ttlNumberInc < ttlBoard.getTtlBoardSize().second; ttlNumberInc++)
		{
			std::string name = ttlBoard.getName(ttlRowInc, ttlNumberInc);
			if (name == "")
			{
				// then no name has been set, so create the default name.
				switch (ttlRowInc)
				{
					case 0:
						name = "A";
						break;
					case 1:
						name = "B";
						break;
					case 2:
						name = "C";
						break;
					case 3:
						name = "D";
						break;
				}
				name += str(ttlNumberInc);
			}
			configStream << name << "\n";

			configStream << ttlBoard.getDefaultTtl(ttlRowInc, ttlNumberInc) << "\n";
		}
	}
	// DAC Names
	for (int dacInc = 0; dacInc < dacBoards.getNumberOfDacs(); dacInc++)
	{
		std::string name = dacBoards.getName(dacInc);
		std::pair<double, double> minMax = dacBoards.getDacRange(dacInc);
		if (name == "")
		{
			// then the name hasn't been set, so create the default name
			name = "Dac" + str(dacInc);
		}
		configStream << name << "\n";
		configStream << minMax.first << " - " << minMax.second << "\n";
		configStream << dacBoards.getDefaultValue(dacInc) << "\n";
	}

	// Number of Variables
	configStream << globalVariables.getCurrentNumberOfVariables() << "\n";
	/// Variables

	for (int varInc = 0; varInc < globalVariables.getCurrentNumberOfVariables(); varInc++)
	{
		variable info = globalVariables.getVariableInfo(varInc);
		configStream << info.name << " ";
		configStream << info.ranges.front().initialValue << "\n";
		// all globals are constants, no need to output anything else.
	}

}


void DeviceWindow::handleMasterConfigOpen(std::stringstream& configStream, double version)
{
	ttlBoard.resetTtlEvents();
	ttlBoard.prepareForce();
	dacBoards.resetDacEvents();
	dacBoards.prepareForce();
	// save info
	for (int ttlRowInc = 0; ttlRowInc < ttlBoard.getTtlBoardSize().first; ttlRowInc++)
	{
		for (int ttlNumberInc = 0; ttlNumberInc < ttlBoard.getTtlBoardSize().second; ttlNumberInc++)
		{
			std::string name;
			std::string statusString;
			bool status;
			configStream >> name;
			configStream >> statusString;
			try
			{
				// should actually be zero or one, but just just convert to bool
				status = std::stoi(statusString);
			}
			catch (std::invalid_argument& exception)
			{
				thrower("ERROR: Failed to load one of the default ttl values!");
			}

			ttlBoard.setName(ttlRowInc, ttlNumberInc, name, toolTips, this);
			ttlBoard.forceTtl(ttlRowInc, ttlNumberInc, status);
			ttlBoard.updateDefaultTtl(ttlRowInc, ttlNumberInc, status);
		}
	}
	// getting dacs.
	for (int dacInc = 0; dacInc < dacBoards.getNumberOfDacs(); dacInc++)
	{
		std::string name;
		std::string defaultValueString;
		double defaultValue;
		std::string minString;
		std::string maxString;
		double min;
		double max;
		configStream >> name;
		if (version >= 1.2)
		{
			configStream >> minString;
			std::string trash;
			configStream >> trash;
			if (trash != "-")
			{
				thrower("ERROR: Expected \"-\" in config file between min and max values!");
			}
			configStream >> maxString;
		}
		configStream >> defaultValueString;
		try
		{
			defaultValue = std::stod(defaultValueString);
			if (version >= 1.2)
			{
				min = std::stod(minString);
				max = std::stod(maxString);
			}
			else
			{
				min = -10;
				max = 10;
			}
		}
		catch (std::invalid_argument& exception)
		{
			thrower("ERROR: Failed to load one of the default DAC values!");
		}
		dacBoards.setName(dacInc, name, toolTips, this);
		dacBoards.setMinMax(dacInc, min, max);
		dacBoards.prepareDacForceChange(dacInc, defaultValue, &ttlBoard);
		dacBoards.setDefaultValue(dacInc, defaultValue);
	}
	// variables.
	if (version >= 1.1)
	{
		int varNum;
		configStream >> varNum;
		if (varNum < 0 || varNum > 1000)
		{
			int answer = MessageBox(cstr("ERROR: variable number retrieved from file appears suspicious. The number is "
									+ str(varNum) + ". Is this accurate?"), "ERROR", MB_YESNO);
			if (answer == IDNO)
			{
				// don't try to load anything.
				varNum = 0;
				return;
			}
		}
		// Number of Variables
		globalVariables.clearVariables();
		for (int varInc = 0; varInc < varNum; varInc++)
		{
			variable tempVar;
			tempVar.constant = true;
			tempVar.overwritten = false;
			tempVar.active = false;
			double value;
			configStream >> tempVar.name;
			configStream >> value;
			tempVar.ranges.push_back({ value, value, 0, false, true });
			globalVariables.addGlobalVariable(tempVar, varInc);
		}
	}
	variable tempVar;
	tempVar.name = "";
	globalVariables.addGlobalVariable(tempVar, -1);
}


void DeviceWindow::LogSettings()
{
	//logger.generateLog(this);
	logger.exportLog();
}


void DeviceWindow::SetDacs()
{
	// have the dac values change
	try
	{
		sendStat("----------------------\r\n");
		dacBoards.resetDacEvents();
		ttlBoard.resetTtlEvents();
		sendStat( "Setting Dacs...\r\n" );
		dacBoards.handleButtonPress( &ttlBoard );
		dacBoards.analyzeDacCommands(0);
		dacBoards.makeFinalDataFormat(0);
		// start the boards which actually sets the dac values.
		dacBoards.stopDacs();
		dacBoards.configureClocks(0);
		sendStat( "Writing New Dac Settings...\r\n" );
		dacBoards.writeDacs(0);
		dacBoards.startDacs();
		ttlBoard.analyzeCommandList(0);
		ttlBoard.convertToFinalFormat(0);
		ttlBoard.writeData(0);
		ttlBoard.startBoard();
		ttlBoard.waitTillFinished(0);
		sendStat( "Finished Setting Dacs.\r\n" );
	}
	catch (Error& exception)
	{
		errBox( exception.what() );
		sendStat( ": " + exception.whatStr() + "\r\n" );
		sendErr( exception.what() );
	}
	mainWindowFriend->updateConfigurationSavedStatus(false);
}


void DeviceWindow::DacEditChange(UINT id)
{
	try
	{
		dacBoards.handleEditChange(id - ID_DAC_FIRST_EDIT);
	}
	catch (Error& err)
	{
		sendErr(err.what());
	}
}


void DeviceWindow::handleTtlPush(UINT id)
{
	try
	{
		ttlBoard.handleTTLPress( id );
	}
	catch (Error& exception)
	{
		sendErr( "TTL Press Handler Failed: " + exception.whatStr() + "\r\n" );
	}
	mainWindowFriend->updateConfigurationSavedStatus(false);
}


void DeviceWindow::handlTtlHoldPush()
{
	try
	{
		ttlBoard.handleHoldPress();
	}
	catch (Error& exception)
	{
		sendErr( "TTL Hold Handler Failed: " + exception.whatStr() + "\r\n" );
	}
	mainWindowFriend->updateConfigurationSavedStatus(false);
}


void DeviceWindow::ViewOrChangeTTLNames()
{
	ttlInputStruct input;
	input.ttls = &ttlBoard;
	input.toolTips = toolTips;
	TtlSettingsDialog dialog(&input, IDD_VIEW_AND_CHANGE_TTL_NAMES);
	dialog.DoModal();
}


void DeviceWindow::ViewOrChangeDACNames()
{
	dacInputStruct input;
	input.dacs = &dacBoards;
	input.toolTips = toolTips;
	DacSettingsDialog dialog(&input, IDD_VIEW_AND_CHANGE_DAC_NAMES);
	dialog.DoModal();
}

void DeviceWindow::Exit()
{
	EndDialog(0);
}


HBRUSH DeviceWindow::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	brushMap brushes = mainWindowFriend->getBrushes();
	rgbMap rgbs = mainWindowFriend->getRgbs();
	HBRUSH result = ttlBoard.handleColorMessage(pWnd, brushes, rgbs, pDC);
	if (result != NULL)
	{
		return result;
	}
	result = dacBoards.handleColorMessage(pWnd, brushes, rgbs, pDC);
	if (result != NULL)
	{
		return result;
	}
	result = flashingAgilent.handleColorMessage(pWnd, brushes, rgbs, pDC);
	if (result != NULL)
	{
		return result;
	}
	result = uWaveAxialAgilent.handleColorMessage(pWnd, brushes, rgbs, pDC);
	if (result != NULL)
	{
		return result;
	}
	result = topBottomAgilent.handleColorMessage(pWnd, brushes, rgbs, pDC);
	if (result != NULL)
	{
		return result;
	}
	result = tektronics1.handleColorMessage(pWnd, brushes, rgbs, pDC);
	if (result != NULL)
	{
		return result;
	}
	result = tektronics2.handleColorMessage(pWnd, brushes, rgbs, pDC);
	if (result != NULL)
	{
		return result;
	}

	// default colors
	switch (nCtlColor)
	{
		case CTLCOLOR_STATIC:
		{
			pDC->SetTextColor(rgbs["Gold"]);
			pDC->SetBkColor(rgbs["Medium Grey"]);
			return *brushes["Medium Grey"];
		}
		case CTLCOLOR_EDIT:
		{
			pDC->SetTextColor(rgbs["White"]);
			pDC->SetBkColor(rgbs["Dark Grey"]);
			return *brushes["Dark Grey"];
		}
		case CTLCOLOR_LISTBOX:
		{
			pDC->SetTextColor(rgbs["White"]);
			pDC->SetBkColor(rgbs["Dark Grey"]);
			return *brushes["Dark Grey"];
		}
		default:
			return *brushes["Light Grey"];
	}
}


BOOL DeviceWindow::PreTranslateMessage(MSG* pMsg)
{
	for (int toolTipInc = 0; toolTipInc < toolTips.size(); toolTipInc++)
	{
		toolTips[toolTipInc]->RelayEvent(pMsg);
	}
	return CDialog::PreTranslateMessage(pMsg);
}


BOOL DeviceWindow::OnInitDialog()
{
	int id = 4000;
	POINT controlLocation{ 0, 0 };

	statusBox.initialize(controlLocation, id, this, 480, mainWindowFriend->getFonts(), toolTips);
	ttlBoard.initialize( controlLocation, toolTips, this, id );
	dacBoards.initialize( controlLocation, toolTips, this, id );
	masterKey.initialize( controlLocation, this, id );

	try
	{
		// masterConfig.load(&ttlBoard, dacBoards, toolTips, this, &globalVariables);
		POINT statusLoc = { 960, 0 };
		tektronics1.initialize(statusLoc, this, id, "Top / Bottom", "Top", "Bottom", 480);
		tektronics2.initialize(statusLoc, this, id, "EO / Axial", "EO", "Axial", 480);
		RhodeSchwarzGenerator.initialize(controlLocation, toolTips, this, id);
		controlLocation = POINT{ 480, 0 };
		topBottomAgilent.initialize(controlLocation, toolTips, this, id, "USB0::2391::11271::MY52801397::0::INSTR", "Top/Bottom Agilent");
		uWaveAxialAgilent.initialize(controlLocation, toolTips, this, id, "STUFF...", "U-Wave / Axial Agilent");
		flashingAgilent.initialize(controlLocation, toolTips, this, id, "STUFF...", "Flashing Agilent");
		controlLocation = POINT{ 1440, 0 };
		globalVariables.initialize(controlLocation, toolTips, this, id, "GLOBAL VARIABLES");
		configVariables.initialize(controlLocation, toolTips, this, id, "CONFIGURATION VARIABLES");
		configVariables.setActive(false);

	}
	catch (Error& exeption)
	{
		errBox(exeption.what());
	}
	
	menu.LoadMenu(IDR_MAIN_MENU);
	SetMenu(&menu);
	return TRUE;
}


std::string DeviceWindow::getSystemStatusMsg()
{
	// controls are done. Report the initialization status...
	std::string msg;
	msg += "<<Code Safemode Settings>>\n";
	msg += "TTL System... ";
	if (!DIO_SAFEMODE)
	{
		msg += "ACTIVE!\n";
	}
	else
	{
		msg += "DISABLED!\n";
	}
	msg += "DAC System... ";
	if (!DAQMX_SAFEMODE)
	{
		msg += "ACTIVE!\n";
	}
	else
	{
		msg += "DISABLED!\n";
	}
	msg += "GPIB System... ";
	if (!GPIB_SAFEMODE)
	{
		msg += "ACTIVE!\n";
	}
	else
	{
		msg += "DISABLED!\n";
	}
	msg += "Agilent System... ";
	if (!AGILENT_SAFEMODE)
	{
		msg += "ACTIVE!\n";
	}
	else
	{
		msg += "DISABLED!\n";
	}
	// 
	msg += "\n<<Device Connectivity>>\n";

	msg += "TTL Board: ";
	try
	{
		msg += ttlBoard.getSystemInfo() + "\n";
	}
	catch (Error& err)
	{
		msg += "Failed! " + err.whatStr() + "\n";
	}

	msg += dacBoards.getDacSystemInfo() + "\n";
	msg += "Top / Bottom Agilent: " + topBottomAgilent.getDeviceIdentity();
	msg += "U Wave / Axial Agilent: " + uWaveAxialAgilent.getDeviceIdentity();
	msg += "Flashing Agilent: " + flashingAgilent.getDeviceIdentity();
	msg += "Tektronics 1: " + gpib.queryIdentity(TEKTRONICS_AFG_1_ADDRESS) + "\n";
	msg += "Tektronics 2: " + gpib.queryIdentity(TEKTRONICS_AFG_2_ADDRESS) + "\n";
	msg += "RSG: " + gpib.queryIdentity(RSG_ADDRESS) + "\n";
	return msg;
}
