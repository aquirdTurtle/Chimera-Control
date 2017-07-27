#include "stdafx.h"
#include "MasterWindow.h"
#include "Control.h"
//#include "newVariableDialogProcedure.h"
#include "constants.h"
#include "TtlSettingsDialog.h"
#include "DacSettingsDialog.h"
#include "textPromptDialogProcedure.h"
#include "TtlSystem.h"
#include "explorerOpen.h"

IMPLEMENT_DYNAMIC( MasterWindow, CDialog )

BEGIN_MESSAGE_MAP( MasterWindow, CDialog )
	ON_WM_CTLCOLOR()
	ON_WM_TIMER()
	ON_WM_SIZE()
	ON_COMMAND_RANGE( TTL_ID_BEGIN, TTL_ID_END, &MasterWindow::handleTTLPush )
	ON_COMMAND( TTL_HOLD, &MasterWindow::handlTTLHoldPush )
	ON_COMMAND( ID_TTLS_VIEW_OR_CHANGE_TTL_NAMES, &MasterWindow::ViewOrChangeTTLNames )
	ON_COMMAND( ID_DACS_VIEW_OR_CHANGE_DAC_NAMES, &MasterWindow::ViewOrChangeDACNames )
	ON_COMMAND( ID_MASTER_SAVEMASTERCONFIGURATION, MasterWindow::SaveMasterConfig )
	ON_COMMAND( IDM_EXIT, &MasterWindow::Exit )
	ON_COMMAND( ID_MASTERSCRIPT_SAVESCRIPT, &MasterWindow::SaveMasterScript )
	ON_COMMAND( ID_MASTERSCRIPT_SAVESCRIPTAS, &MasterWindow::SaveMasterScriptAs )
	ON_COMMAND( ID_MASTERSCRIPT_NEWSCRIPT, &MasterWindow::NewMasterScript )
	ON_COMMAND( ID_MASTERSCRIPT_NEWFUNCTION, &MasterWindow::NewMasterFunction)
	ON_COMMAND( ID_MASTERSCRIPT_OPENSCRIPT, &MasterWindow::OpenMasterScript )
	ON_COMMAND( ID_MASTERSCRIPT_SAVEFUNCTION, &MasterWindow::SaveMasterFunction )
	ON_COMMAND( ID_CONFIGURATION_DELETE_CURRENT_CONFIGURATION, &MasterWindow::DeleteConfiguration )
	ON_COMMAND( ID_CONFIGURATION_NEW_CONFIGURATION, &MasterWindow::NewConfiguration )
	ON_COMMAND( ID_CONFIGURATION_SAVECONFIGURATIONSETTINGS, &MasterWindow::SaveConfiguration )
	ON_COMMAND( ID_CONFIGURATION_SAVE_CONFIGURATION_AS, &MasterWindow::SaveConfigurationAs )
	ON_COMMAND( ID_CONFIGURATION_RENAME_CURRENT_CONFIGURATION, &MasterWindow::RenameConfiguration )
	ON_COMMAND( ID_CATEGORY_SAVECATEGORYSETTINGS, &MasterWindow::SaveCategory )
	ON_COMMAND( ID_CATEGORY_SAVECATEGORYSETTINGSAS, &MasterWindow::SaveCategoryAs )
	ON_COMMAND( ID_CATEGORY_RENAME_CURRENT_CATEGORY, &MasterWindow::RenameCategory )
	ON_COMMAND( ID_CATEGORY_DELETE_CURRENT_CATEGORY, &MasterWindow::DeleteCategory )
	ON_COMMAND( ID_CATEGORY_NEW_CATEGORY, &MasterWindow::NewCategory )
	ON_COMMAND( ID_EXPERIMENT_NEW_EXPERIMENT_TYPE, &MasterWindow::NewExperiment )
	ON_COMMAND( ID_EXPERIMENT_DELETE_CURRENT_EXPERIMENT, &MasterWindow::DeleteExperiment )
	ON_COMMAND( ID_EXPERIMENT_SAVEEXPERIMENTSETTINGS, &MasterWindow::SaveExperiment )
	ON_COMMAND( ID_EXPERIMENT_SAVEEXPERIMENTSETTINGSAS, &MasterWindow::SaveExperimentAs )
	ON_COMMAND( ID_SEQUENCE_ADD_TO_SEQUENCE, &MasterWindow::AddToSequence )
	ON_COMMAND( ID_SEQUENCE_DELETE_SEQUENCE, &MasterWindow::DeleteSequence )
	ON_COMMAND( ID_SEQUENCE_RESET_SEQUENCE, &MasterWindow::ResetSequence )
	ON_COMMAND( ID_SEQUENCE_RENAMESEQUENCE, &MasterWindow::RenameSequence )
	ON_COMMAND( ID_SEQUENCE_NEW_SEQUENCE, &MasterWindow::NewSequence )
	//ON_COMMAND( SET_REPETITION_ID, &MasterWindow::SetRepetitionNumber )
	ON_COMMAND( ID_ERROR_CLEAR, &MasterWindow::ClearError )
	ON_COMMAND( ID_STATUS_CLEAR, &MasterWindow::ClearGeneral )
	ON_COMMAND( ID_DAC_SET_BUTTON, &MasterWindow::SetDacs )
	ON_COMMAND( ID_FILE_EXPORTSETTINGSLOG, &MasterWindow::LogSettings )
	ON_COMMAND( ID_FILE_RUN, &MasterWindow::StartExperiment )
	ON_COMMAND( ID_FILE_PAUSE, &MasterWindow::HandlePause )
	ON_COMMAND( ID_FILE_ABORT, &MasterWindow::HandleAbort )
	ON_COMMAND( ID_PROFILE_SAVE_PROFILE, &MasterWindow::SaveEntireProfile )
	ON_COMMAND( IDC_ZERO_TTLS, &MasterWindow::zeroTtls )
	ON_COMMAND( IDC_ZERO_DACS, &MasterWindow::zeroDacs )
	ON_COMMAND(IDOK, &MasterWindow::handleEnter)

	ON_COMMAND_RANGE( IDC_SHOW_TTLS, IDC_SHOW_TTLS, &MasterWindow::handleOptionsPress )
	ON_COMMAND_RANGE( IDC_SHOW_DACS, IDC_SHOW_DACS, &MasterWindow::handleOptionsPress )
	ON_COMMAND_RANGE(IDC_TOP_BOTTOM_CHANNEL1_BUTTON, IDC_FLASHING_AGILENT_COMBO, &MasterWindow::handleAgilentOptions)
	ON_COMMAND_RANGE(TOP_ON_OFF, AXIAL_FSK, &MasterWindow::handleTektronicsButtons)
	/*
	#define AXIAL_ON_OFF 1191
	#define BOTTOM_ON_OFF 1173
	#define TOP_ON_OFF 1167
	#define EO_ON_OFF 1185

	#define AXIAL_FSK 1192
	#define BOTTOM_FSK 1174
	#define TOP_FSK 1168
	#define EO_FSK 1186
	*/
	
	ON_CBN_SELENDOK( IDC_TOP_BOTTOM_AGILENT_COMBO, &MasterWindow::handleTopBottomAgilentCombo )
	ON_CBN_SELENDOK( IDC_AXIAL_UWAVE_AGILENT_COMBO, &MasterWindow::handleAxialUWaveAgilentCombo )
	ON_CBN_SELENDOK( IDC_FLASHING_AGILENT_COMBO, &MasterWindow::handleFlashingAgilentCombo )
	ON_CBN_SELENDOK( EXPERIMENT_COMBO_ID, &MasterWindow::OnExperimentChanged )
	ON_CBN_SELENDOK( CATEGORY_COMBO_ID, &MasterWindow::OnCategoryChanged )
	ON_CBN_SELENDOK( CONFIGURATION_COMBO_ID, &MasterWindow::OnConfigurationChanged )
	ON_CBN_SELENDOK( SEQUENCE_COMBO_ID, &MasterWindow::OnSequenceChanged )
	ON_CBN_SELENDOK( ORIENTATION_COMBO_ID, &MasterWindow::OnOrientationChanged )
	ON_CBN_SELENDOK( FUNCTION_COMBO_ID, &MasterWindow::HandleFunctionChange )

	

	ON_CONTROL_RANGE( EN_CHANGE, ID_DAC_FIRST_EDIT, (ID_DAC_FIRST_EDIT + 23), &MasterWindow::DAC_EditChange )

	ON_EN_CHANGE( MASTER_RICH_EDIT, &MasterWindow::EditChange )
	ON_EN_CHANGE( CONFIGURATION_NOTES_ID, &MasterWindow::ConfigurationNotesChange )
	ON_EN_CHANGE( CATEGORY_NOTES_ID, &MasterWindow::CategoryNotesChange )
	ON_EN_CHANGE( EXPERIMENT_NOTES_ID, &MasterWindow::ExperimentNotesChange )

	ON_NOTIFY( LVN_COLUMNCLICK, IDC_CONFIG_VARS_LISTVIEW, &MasterWindow::ConfigVarsColumnClick )
	ON_NOTIFY( NM_DBLCLK, IDC_CONFIG_VARS_LISTVIEW, &MasterWindow::ConfigVarsDblClick )
	ON_NOTIFY( NM_RCLICK, IDC_CONFIG_VARS_LISTVIEW, &MasterWindow::ConfigVarsRClick )

	ON_NOTIFY( NM_DBLCLK, IDC_GLOBAL_VARS_LISTVIEW, &MasterWindow::GlobalVarDblClick )
	ON_NOTIFY( NM_RCLICK, IDC_GLOBAL_VARS_LISTVIEW, &MasterWindow::GlobalVarRClick )
	ON_NOTIFY_RANGE( NM_CUSTOMDRAW, IDC_GLOBAL_VARS_LISTVIEW, IDC_GLOBAL_VARS_LISTVIEW, &MasterWindow::drawVariables)
	ON_NOTIFY_RANGE( NM_CUSTOMDRAW, IDC_CONFIG_VARS_LISTVIEW, IDC_CONFIG_VARS_LISTVIEW, &MasterWindow::drawVariables)

END_MESSAGE_MAP()


void MasterWindow::handleTektronicsButtons(UINT id)
{
	profile.updateConfigurationSavedStatus(false);
	if (id >= TOP_ON_OFF && id <= BOTTOM_FSK)
	{
		tektronics1.handleButtons(id - TOP_ON_OFF);
	}
	if (id >= EO_ON_OFF && id <= AXIAL_FSK)
	{
		tektronics2.handleButtons(id - EO_ON_OFF);
	}
}


void MasterWindow::drawVariables(UINT id, NMHDR* pNMHDR, LRESULT* pResult)
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

void MasterWindow::NewMasterFunction()
{
	try
	{
		masterScript.newFunction(this);
	}
	catch (Error& exception)
	{
		errorStatus.addStatusText("New Master function Failed: " + exception.whatStr() + "\r\n", 0);
	}
}

void MasterWindow::handleEnter()
{
	errBox("Hello, there!");
}

fontMap MasterWindow::getFonts()
{
	return masterFonts;
}

void MasterWindow::OnSize(UINT nType, int cx, int cy)
{
	tektronics1.rearrange(cx, cy, getFonts());
	tektronics2.rearrange(cx, cy, getFonts());
	niawgSocket.rearrange(cx, cy, getFonts());

	topBottomAgilent.rearrange(cx, cy, getFonts());
	uWaveAxialAgilent.rearrange(cx, cy, getFonts());
	flashingAgilent.rearrange(cx, cy, getFonts());

	profile.rearrange(cx, cy, getFonts());
	notes.rearrange(cx, cy, getFonts());
	masterScript.rearrange(cx, cy, getFonts());

	errorStatus.rearrange(cx, cy, getFonts());
	generalStatus.rearrange(cx, cy, getFonts());

	RhodeSchwarzGenerator.rearrange(cx, cy, getFonts());

	configVariables.rearrange(cx, cy, getFonts());
	globalVariables.rearrange(cx, cy, getFonts());
	
	ttlBoard.rearrange(cx, cy, getFonts());
	dacBoards.rearrange(cx, cy, getFonts());

	repetitionControl.rearrange(cx, cy, getFonts());
	debugControl.rearrange(cx, cy, getFonts());
	masterKey.rearrange(cx, cy, getFonts());
}


void MasterWindow::handleAgilentOptions( UINT id )
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
		// sync 
		//agilent->handleSync();
	}
	// else it's a boring combo that I don't care about at the moment.
}


void MasterWindow::handleTopBottomAgilentCombo()
{
	topBottomAgilent.handleCombo();
}


void MasterWindow::handleAxialUWaveAgilentCombo()
{
	uWaveAxialAgilent.handleCombo();
}


void MasterWindow::handleFlashingAgilentCombo()
{
	flashingAgilent.handleCombo();
}


void MasterWindow::GlobalVarDblClick( NMHDR * pNotifyStruct, LRESULT * result )
{
	std::vector<Script*> scriptList;
	scriptList.push_back( &masterScript );
	try
	{
		globalVariables.updateVariableInfo( scriptList, this );
	}
	catch (Error& exception)
	{
		errorStatus.addStatusText( "Global Variables Double Click Handler : " + exception.whatStr() + "\r\n", 0 );
	}
	masterConfig.save( &ttlBoard, &dacBoards, &globalVariables );
}


void MasterWindow::GlobalVarRClick( NMHDR * pNotifyStruct, LRESULT * result )
{
	try
	{
		globalVariables.deleteVariable();
	}
	catch (Error& exception)
	{
		errorStatus.addStatusText( "Global Variables Right Click Handler : " + exception.whatStr() + "\r\n", 0 );
	}
	masterConfig.save( &ttlBoard, &dacBoards, &globalVariables );
}


void MasterWindow::handleOptionsPress( UINT id )
{
	debugControl.handlePress( id );
}


void MasterWindow::zeroDacs()
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
		generalStatus.addStatusText( "Zero'd DACs.\r\n", 0 );
	}
	catch (Error& exception)
	{
		generalStatus.addStatusText( "Failed to Zero DACs!!!\r\n", 0 );
		errorStatus.addStatusText( exception.what(), 0 );
	}
}


void MasterWindow::zeroTtls()
{
	try
	{
		ttlBoard.zeroBoard();
		generalStatus.addStatusText( "Zero'd TTLs.\r\n", 0 );
	}
	catch (Error& exception)
	{
		generalStatus.addStatusText( "Failed to Zero TTLs!!!\r\n", 0 );
		errorStatus.addStatusText( exception.what(), 0 );
	}
}


void MasterWindow::HandleAbort()
{
	/// needs implementation...
	if ( manager.runningStatus() )
	{
		manager.abort();
		ttlBoard.unshadeTtls();
		dacBoards.unshadeDacs();
	}
	else
	{
		generalStatus.addStatusText( "Can't abort, experiment was not running.\r\n", 0 );
	}
}


void MasterWindow::HandlePause()
{
	if ( manager.runningStatus() )
	{
		CMenu *menu = GetMenu();
		if ( menu == NULL )
		{
			errBox( "WTF no menu?" );
		}
		if ( manager.getIsPaused() )
		{
			// then it's currently paused, so unpause it.
			menu->CheckMenuItem( ID_FILE_PAUSE, MF_UNCHECKED | MF_BYCOMMAND );
			manager.unPause();
		}
		else
		{
			// then not paused so pause it.
			menu->CheckMenuItem( ID_FILE_PAUSE, MF_CHECKED | MF_BYCOMMAND );
			manager.pause();
		}
	}
	else
	{
		generalStatus.addStatusText( "Can't pause, experiment was not running.\r\n", 0 );
	}
}

void MasterWindow::loadMotSettings()
{
	try
	{
		generalStatus.addStatusText("Loading MOT Configuration...\r\n", 1);
		//
		manager.loadMotSettings(this);
	}
	catch (Error& exception)
	{
		generalStatus.addStatusText(": " + exception.whatStr() + " " + exception.whatStr() + "\r\n", 1);
	}
	/*	
	/// Set TTL values.
	// switch on MOT light, set power, set detuning
	// forceTtl(row, number, state), A = 0, B = 1, C = 2, D = 3
	// MOT AO RF
	ttlBoard.forceTtl( 0, 0, 1 );
	// MOT slave shutter
	ttlBoard.forceTtl( 3, 0, 1 );
	// RP shutter open
	ttlBoard.forceTtl( 1, 1, 1 );
	// F=2 shutter open (blocks side MOT beam)
	ttlBoard.forceTtl( 2, 13, 1);
	// switch on repump light
	ttlBoard.forceTtl( 0, 1, 1 );
	// Switch on side beam
	ttlBoard.forceTtl( 0, 5, 1 );
	// Make sure raman shutter is closed initially
	ttlBoard.forceTtl( 2, 4, 0 );
	// (value time line) in vb6
	dacBoards.prepareDacForceChange(22, 0.084, &ttlBoard);
	// trap light detuning 0 for MOT detuning (0 is 10 MHz detuning, -.3 is 17 MHz detuning)
	dacBoards.prepareDacForceChange( 20, 0.1, &ttlBoard );
	// trap light power (0.2 Full power)
	dacBoards.prepareDacForceChange( 9, 0.28, &ttlBoard );
	// trap light AO detuning
	dacBoards.prepareDacForceChange( 8, 1, &ttlBoard );
	// setting power (was 0.285)
	dacBoards.prepareDacForceChange( 11, 0.32, &ttlBoard );
	//setting frequency via offset lock
	dacBoards.prepareDacForceChange( 21, 0, &ttlBoard );
	//quad coils on// CR
	dacBoards.prepareDacForceChange( 6, 7, &ttlBoard );
	/// mot coils.
	// Right, 0.117 from the perspective of looking east to west across the table '0.64
	dacBoards.prepareDacForceChange( 0, 0.66, &ttlBoard );
	// Left, -0.64
	dacBoards.prepareDacForceChange( 1, -0.575, &ttlBoard );
	// Front, -1.272
	dacBoards.prepareDacForceChange( 2, -1.11, &ttlBoard );
	// Back, 1.272
	//dacBoards.prepareDacForceChange( 3, 0.5, &ttlBoard );
	dacBoards.prepareDacForceChange( 3, 1.11, &ttlBoard );
	// Bottom, -1.24
	dacBoards.prepareDacForceChange( 4, -1.38, &ttlBoard );
	// top
	dacBoards.prepareDacForceChange( 5, 1.31, &ttlBoard );
	// start the boards which actually sets the dac values.
	try
	{
		dacBoards.analyzeDacCommands();
		dacBoards.makeFinalDataFormat();
		dacBoards.configureClocks();
		dacBoards.stopDacs();
		dacBoards.writeDacs();
		dacBoards.startDacs();
		ttlBoard.analyzeCommandList();
		ttlBoard.convertToFinalFormat();
		ttlBoard.writeData();
		ttlBoard.startBoard();
		ttlBoard.waitTillFinished();
		generalStatus.appendText( "Finished Loading MOT Configuration!\r\n", 1 );
	}
	catch ( Error& exception )
	{
		errorStatus.appendText( exception.what(), 1 );
		generalStatus.appendText( ": " + exception.whatStr() + "\r\n", 1 );
	}
	*/
}

void MasterWindow::OnCancel()
{
	int answer = MessageBox( "Are you sure you'd like to exit?", "Exit", MB_OKCANCEL );
	if (answer == IDOK)
	{
		try
		{
			profile.checkSaveEntireProfile(this);
			masterScript.checkSave(this);
			CDialog::OnCancel();
		}
		catch (Error& err)
		{
			generalStatus.addStatusText(": " + err.whatStr() + "\r\n", 0);
		}
	}
}

void MasterWindow::HandleFunctionChange()
{
	try
	{
		masterScript.functionChangeHandler(this);
	}
	catch (Error& err)
	{
		errBox(err.what());
	}
}

void MasterWindow::StartExperiment()
{	
	// check to make sure ready.
	try
	{
		generalStatus.addStatusText( "............................\r\n", 1 );
		generalStatus.addStatusText( "Checking if Ready...\n", 0 );
		profile.allSettingsReadyCheck( this );
		masterScript.checkSave( this );
		generalStatus.addStatusText( "Starting Experiment Thread...\r\n", 0 );
		manager.startExperimentThread( this );
	}
	catch (Error& exception)
	{
		generalStatus.addStatusText( ": " + exception.whatStr() + "\r\n", 0 );
	}
}


void MasterWindow::ConfigurationNotesChange()
{
	profile.updateConfigurationSavedStatus(false);
}


void MasterWindow::CategoryNotesChange()
{
	profile.updateCategorySavedStatus(false);
}


void MasterWindow::ExperimentNotesChange()
{
	profile.updateExperimentSavedStatus(false);
}


void MasterWindow::SaveEntireProfile()
{
	try
	{
		profile.saveEntireProfile(this);
	}
	catch (Error& err)
	{
		errorStatus.addStatusText(err.what());
	}
}


void MasterWindow::LogSettings()
{
	logger.generateLog(this);
	logger.exportLog();
}


void MasterWindow::SetDacs()
{
	// have the dac values change
	try
	{
		generalStatus.addStatusText("----------------------\r\n", 1);
		dacBoards.resetDacEvents();
		ttlBoard.resetTtlEvents();
		generalStatus.addStatusText( "Setting Dacs...\r\n", 0 );
		dacBoards.handleButtonPress( &ttlBoard );
		dacBoards.analyzeDacCommands(0);
		dacBoards.makeFinalDataFormat(0);
		// start the boards which actually sets the dac values.
		dacBoards.stopDacs();
		dacBoards.configureClocks(0);
		generalStatus.addStatusText( "Writing New Dac Settings...\r\n", 0 );
		dacBoards.writeDacs(0);
		dacBoards.startDacs();
		ttlBoard.analyzeCommandList(0);
		ttlBoard.convertToFinalFormat(0);
		ttlBoard.writeData(0);
		ttlBoard.startBoard();
		ttlBoard.waitTillFinished(0);
		generalStatus.addStatusText( "Finished Setting Dacs.\r\n", 0 );
	}
	catch (Error& exception)
	{
		errBox( exception.what() );
		generalStatus.addStatusText( ": " + exception.whatStr() + "\r\n", 0 );
		errorStatus.addStatusText( exception.what(), 0 );
	}
	profile.updateConfigurationSavedStatus(false);
}


void MasterWindow::DAC_EditChange(UINT id)
{
	try
	{
		dacBoards.handleEditChange(id - ID_DAC_FIRST_EDIT);
	}
	catch (Error& err)
	{
		errorStatus.addStatusText(err.what());
	}
}


void MasterWindow::ClearError()
{
	errorStatus.clear();
}

void MasterWindow::ClearGeneral()
{
	generalStatus.clear();
}

/*
void MasterWindow::SetRepetitionNumber()
{
	try
	{
		repetitionControl.handleButtonPush();
	}
	catch (Error& exception)
	{
		errorStatus.addStatusText( "Set Repetition Number : " + exception.whatStr() + "\r\n", 0 );
	}
	profile.updateConfigurationSavedStatus(false);
}
*/


void MasterWindow::ConfigVarsColumnClick(NMHDR * pNotifyStruct, LRESULT * result)
{
	try
	{
		configVariables.handleColumnClick( pNotifyStruct, result );
	}
	catch(Error& exception )
	{
		errorStatus.addStatusText("Handling config variable listview click : " + exception.whatStr() + "\r\n", 0);
	}
	profile.updateConfigurationSavedStatus(false);
}


void MasterWindow::OnExperimentChanged()
{
	try
	{
		profile.experimentChangeHandler( this );
	}
	catch (Error& exception)
	{
		errorStatus.addStatusText( "Handling Experiment Selection Change : " + exception.whatStr() + "\r\n", 0 );
	}
}


void MasterWindow::OnCategoryChanged()
{
	try
	{
		profile.categoryChangeHandler( this );
	}
	catch (Error& exception)
	{
		errorStatus.addStatusText( "Handling Category Selection Change : " + exception.whatStr() + "\r\n", 0 );
	}
}


void MasterWindow::OnConfigurationChanged()
{
	try
	{
		profile.configurationChangeHandler( this );
	}
	catch (Error& exception)
	{
		errorStatus.addStatusText( "Handling Configuration Selection Change Failed: " + exception.whatStr() + "\r\n", 0 );
	}
}


void MasterWindow::OnSequenceChanged()
{
	try
	{
		profile.sequenceChangeHandler( this );
	}
	catch (Error& exception)
	{
		errorStatus.addStatusText( "Handling Sequence Selection Change Failed: " + exception.whatStr() + "\r\n", 0 );
	}
}


void MasterWindow::OnOrientationChanged()
{
	try
	{
		profile.orientationChangeHandler( this );
	}
	catch (Error& exception)
	{
		errorStatus.addStatusText( "Handling Orientation Selection Change Failed: " + exception.whatStr() + "\r\n", 0 );
	}
}


void MasterWindow::DeleteConfiguration()
{
	try
	{
		profile.deleteConfiguration();
	}
	catch (Error& exception)
	{
		errorStatus.addStatusText( "Deleting Configuration Failed: " + exception.whatStr() + "\r\n", 0 );
	}
}


void MasterWindow::NewConfiguration()
{
	try
	{
		profile.newConfiguration( this );
	}
	catch (Error& exception)
	{
		errorStatus.addStatusText( "New Configuration Failed: " + exception.whatStr() + "\r\n", 0 );
	}
}


void MasterWindow::SaveConfiguration()
{
	try
	{
		profile.saveConfiguration( this );
	}
	catch (Error& exception)
	{
		errorStatus.addStatusText( "Saving Configuration Failed: " + exception.whatStr() + "\r\n", 0 );
	}
}


void MasterWindow::SaveConfigurationAs()
{
	try
	{
		profile.saveConfigurationAs( this );
	}
	catch (Error& exception)
	{
		errorStatus.addStatusText( "Saving Configuration As Failed: " + exception.whatStr() + "\r\n", 0 );
	}
}


void MasterWindow::RenameConfiguration()
{
	try
	{
		profile.renameConfiguration( this );
	}
	catch (Error& exception)
	{
		errorStatus.addStatusText( "Renaming Configuration Failed: " + exception.whatStr() + "\r\n", 0 );
	}
}


void MasterWindow::SaveCategory()
{
	try
	{
		profile.saveCategory( this );
	}
	catch (Error& exception)
	{
		errorStatus.addStatusText( "Saving Category Failed: " + exception.whatStr() + "\r\n", 0 );
	}
}


void MasterWindow::SaveCategoryAs()
{
	try
	{
		profile.saveCategoryAs( this );
	}
	catch (Error& exception)
	{
		errorStatus.addStatusText( "Saving Category As Failed: " + exception.whatStr() + "\r\n", 0 );
	}
}


void MasterWindow::RenameCategory()
{
	try
	{
		profile.renameCategory();
	}
	catch (Error& exception)
	{
		errorStatus.addStatusText( "Renaming Category Failed: " + exception.whatStr() + "\r\n", 0 );
	}
}


void MasterWindow::DeleteCategory()
{
	try
	{
		profile.deleteCategory();
	}
	catch (Error& exception)
	{
		errorStatus.addStatusText("Deleting Category Failed: " + exception.whatStr() + "\r\n", 0);
	}
}


void MasterWindow::NewCategory()
{
	try
	{
		profile.newCategory( this );
	}
	catch (Error& exception)
	{
		errorStatus.addStatusText("New Category Failed: " + exception.whatStr() + "\r\n", 0);
	}
}


void MasterWindow::NewExperiment()
{
	try
	{
		profile.newExperiment( this );
	}
	catch (Error& exception)
	{
		errorStatus.addStatusText("New Experiment : " + exception.whatStr() + "\r\n", 0);
	}
}


void MasterWindow::DeleteExperiment()
{
	try
	{
		profile.deleteExperiment();
	}
	catch (Error& exception)
	{
		errorStatus.addStatusText("Delete Experiment : " + exception.whatStr() + "\r\n", 0);
	}
}


void MasterWindow::SaveExperiment()
{
	try
	{
		profile.saveExperiment( this );
	}
	catch (Error& exception)
	{
		errorStatus.addStatusText("ERROR: Save Experiment : " + exception.whatStr() + "\r\n", 0);
	}
}


void MasterWindow::SaveExperimentAs()
{
	try
	{
		profile.saveExperimentAs( this );
	}
	catch (Error& exception)
	{
		errorStatus.addStatusText( "Save Experiment As : " + exception.whatStr() + "\r\n", 0 );
	}
}


void MasterWindow::AddToSequence()
{
	try
	{
		profile.addToSequence( this );
	}
	catch (Error& exception)
	{
		errorStatus.addStatusText("Add to Sequence : " + exception.whatStr() + "\r\n", 0);
	}
}


void MasterWindow::DeleteSequence()
{
	try
	{
		profile.deleteSequence( this );
	}
	catch (Error& exception)
	{
		errorStatus.addStatusText("Delete Sequence : " + exception.whatStr() + "\r\n", 0);
	}
}


void MasterWindow::ResetSequence()
{
	//profile.???
}


void MasterWindow::RenameSequence()
{
	try
	{
		profile.renameSequence( this );
	}
	catch (Error& exception)
	{
		errorStatus.addStatusText("Rename Sequence : " + exception.whatStr() + "\r\n", 0);
	}
}


void MasterWindow::NewSequence()
{
	try
	{
		profile.newSequence( this );
	}
	catch (Error& exception)
	{
		errorStatus.addStatusText("New Sequence : " + exception.whatStr() + "\r\n", 0);
	}
}


//

void MasterWindow::SaveMasterScript()
{
	try
	{
		masterScript.saveScript( this );
	}
	catch ( Error& exception )
	{
		errorStatus.addStatusText("Save Master Script Failed: " + exception.whatStr() + "\r\n", 0);
	}
}


void MasterWindow::SaveMasterScriptAs()
{
	if (profile.getCurrentPathIncludingCategory() == "")
	{
		MessageBox("Please select a category before trying to save a the Master Script!", 0, 0);
	}
	std::string scriptName = (const char*)DialogBoxParam(programInstance, MAKEINTRESOURCE(IDD_TEXT_PROMPT_DIALOG), 0, (DLGPROC)textPromptDialogProcedure, (LPARAM)"Please enter new master script name.");
	if (scriptName == "")
	{
		return;
	}
	try
	{
		masterScript.saveScriptAs( profile.getCurrentPathIncludingCategory() + scriptName + MASTER_SCRIPT_EXTENSION, this );
	}
	catch(Error& exception )
	{
		errorStatus.addStatusText("Save Master Script As : " + exception.whatStr() + "\r\n", 0);
	}
}


void MasterWindow::NewMasterScript()
{
	try
	{
		masterScript.newScript( this );
	}
	catch ( Error& exception )
	{
		errorStatus.addStatusText( "New Master Script Failed: " + exception.whatStr() + "\r\n", 0 );
	}
	profile.updateConfigurationSavedStatus(false);
}


void MasterWindow::OpenMasterScript()
{
	std::string address = explorerOpen(this, "*.mScript\0All\0 * .*", profile.getCurrentPathIncludingCategory());
	if (address == "")
	{
		// user canceled.
		return;
	}
	try
	{
		masterScript.openParentScript( address, this );
	}
	catch(Error& exception )
	{
		errorStatus.addStatusText("Open Master Script : " + exception.whatStr() + "" + exception.whatStr() + "\r\n", 0);
	}
	profile.updateConfigurationSavedStatus(false);
}


void MasterWindow::SaveMasterFunction()
{
	try
	{
		masterScript.saveAsFunction();
	}
	catch (Error& exception)
	{
		errorStatus.addStatusText("Save Master Script Function Failed: " + exception.whatStr() + "\r\n", 0);
	}
}


void MasterWindow::ConfigVarsDblClick(NMHDR * pNotifyStruct, LRESULT * result)
{	
	std::vector<Script*> scriptList;
	scriptList.push_back(&masterScript);
	try
	{
		configVariables.updateVariableInfo( scriptList, this );
	}
	catch(Error& exception )
	{
		errorStatus.addStatusText("Variables Double Click Handler : " + exception.whatStr() + "\r\n", 0);
	}
	profile.updateConfigurationSavedStatus(false);
}


void MasterWindow::ConfigVarsRClick(NMHDR * pNotifyStruct, LRESULT * result)
{
	try
	{
		configVariables.deleteVariable();
	}
	catch(Error& exception)
	{
		errorStatus.addStatusText("Variables Right Click Handler : " + exception.whatStr() + "\r\n", 0);
	}
	profile.updateConfigurationSavedStatus(false);
}


void MasterWindow::OnTimer(UINT TimerVal)
{
	try
	{
		masterScript.handleTimerCall( this );
	}
	catch (Error& exception )
	{
		errorStatus.addStatusText("Timer Call Handler Failed: " + exception.whatStr() + "\r\n", 0);
	}
}


void MasterWindow::EditChange()
{
	try
	{
		masterScript.handleEditChange( this );
	}
	catch ( Error& exception )
	{
		errorStatus.addStatusText( "Edit Change Handler Failed: " + exception.whatStr() + "\r\n", 0 );
	}
}


void MasterWindow::handleTTLPush(UINT id)
{
	try
	{
		ttlBoard.handleTTLPress( id );
	}
	catch (Error& exception)
	{
		errorStatus.addStatusText( "TTL Press Handler Failed: " + exception.whatStr() + "\r\n", 0 );
	}
	profile.updateConfigurationSavedStatus(false);
}


void MasterWindow::handlTTLHoldPush()
{
	try
	{
		ttlBoard.handleHoldPress();
	}
	catch (Error& exception)
	{
		errorStatus.addStatusText( "TTL Hold Handler Failed: " + exception.whatStr() + "\r\n", 0 );
	}
	profile.updateConfigurationSavedStatus(false);
}


void MasterWindow::ViewOrChangeTTLNames()
{
	ttlInputStruct input;
	input.ttls = &ttlBoard;
	input.toolTips = toolTips;
	TtlSettingsDialog dialog(&input, IDD_VIEW_AND_CHANGE_TTL_NAMES);
	dialog.DoModal();
}


void MasterWindow::ViewOrChangeDACNames()
{
	dacInputStruct input;
	input.dacs = &dacBoards;
	input.toolTips = toolTips;
	DacSettingsDialog dialog(&input, IDD_VIEW_AND_CHANGE_DAC_NAMES);
	dialog.DoModal();
}

void MasterWindow::SaveMasterConfig()
{
	try
	{
		masterConfig.save( &ttlBoard, &dacBoards, &globalVariables);
	}
	catch (Error& exception)
	{
		errorStatus.addStatusText("Master Configuration Save Handler : " + exception.whatStr() + "\r\n", 0);
	}
}


void MasterWindow::Exit()
{
	EndDialog(0);
}

MasterWindow::~MasterWindow()
{
	if (masterWindowHandle)
	{
		DestroyWindow();
	}
}

HBRUSH MasterWindow::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH result = ttlBoard.handleColorMessage(pWnd, masterBrushes, masterRGBs, pDC);
	if (result != NULL)
	{
		return result;
	}
	result = dacBoards.handleColorMessage(pWnd, masterBrushes, masterRGBs, pDC);
	if (result != NULL)
	{
		return result;
	}
	result = debugControl.handleColorMessage(pWnd, masterBrushes, masterRGBs, pDC);
	if (result != NULL)
	{
		return result;
	}
	result = flashingAgilent.handleColorMessage(pWnd, masterBrushes, masterRGBs, pDC);
	if (result != NULL)
	{
		return result;
	}
	result = uWaveAxialAgilent.handleColorMessage(pWnd, masterBrushes, masterRGBs, pDC);
	if (result != NULL)
	{
		return result;
	}
	result = topBottomAgilent.handleColorMessage(pWnd, masterBrushes, masterRGBs, pDC);
	if (result != NULL)
	{
		return result;
	}
	result = tektronics1.handleColorMessage(pWnd, masterBrushes, masterRGBs, pDC);
	if (result != NULL)
	{
		return result;
	}
	result = tektronics2.handleColorMessage(pWnd, masterBrushes, masterRGBs, pDC);
	if (result != NULL)
	{
		return result;
	}
	result = repetitionControl.handleColorMessage(pWnd, masterBrushes, masterRGBs, pDC);
	if (result != NULL)
	{
		return result;
	}

	// default colors
	switch (nCtlColor)
	{

		case CTLCOLOR_STATIC:
		{
			pDC->SetTextColor(masterRGBs["Gold"]);
			pDC->SetBkColor(masterRGBs["Medium Grey"]);
			return masterBrushes["Medium Grey"];
		}
		case CTLCOLOR_EDIT:
		{
			pDC->SetTextColor(masterRGBs["White"]);
			pDC->SetBkColor(masterRGBs["Dark Grey"]);
			return masterBrushes["Dark Grey"];
		}
		case CTLCOLOR_LISTBOX:
		{
			pDC->SetTextColor(masterRGBs["White"]);
			pDC->SetBkColor(masterRGBs["Dark Grey"]);
			return masterBrushes["Dark Grey"];
		}
		default:
			return masterBrushes["Light Grey"];
	}
}


BOOL MasterWindow::PreTranslateMessage(MSG* pMsg)
{
	for (int toolTipInc = 0; toolTipInc < toolTips.size(); toolTipInc++)
	{
		toolTips[toolTipInc]->RelayEvent(pMsg);
	}
	return CDialog::PreTranslateMessage(pMsg);
}


BOOL MasterWindow::OnInitDialog()
{
	int id = 1000;
	POINT controlLocation{ 0, 0 };
	profile.initialize( controlLocation, toolTips, this, id );
	globalVariables.initialize( controlLocation, toolTips, this, id, "GLOBAL VARIABLES" );
	configVariables.initialize( controlLocation, toolTips, this, id, "CONFIGURATION VARIABLES" );
	configVariables.setActive(false);
	repetitionControl.initialize( controlLocation, toolTips, this, id );
	ttlBoard.initialize( controlLocation, toolTipText, toolTips, this, id );
	dacBoards.initialize( controlLocation, toolTips, this, id );
	masterKey.initialize( controlLocation, this, id );
	try
	{
		masterConfig.load( &ttlBoard, dacBoards, toolTips, this, &globalVariables );
	}
	catch (Error& exeption)
	{
		errBox( exeption.what() );
	}

	RECT controlArea = { 960, 0, 1320, 540 };
	POINT statusLoc = { 960, 0 };
	generalStatus.initialize(statusLoc, this, id, 360, 300, "General Status", masterRGBs["Light Blue"],
							 getFonts(), toolTips);
	statusLoc = { 960, 300 };
	errorStatus.initialize(statusLoc, this, id, 360, 300, "Error Status", masterRGBs["Light Red"],
						   getFonts(), toolTips);
	statusLoc = { 960, 600 };
	tektronics1.initialize(statusLoc, this, id, "Top / Bottom", "Top", "Bottom", 360);
	tektronics2.initialize(statusLoc, this, id, "EO / Axial", "EO", "Axial", 360);
	niawgSocket.initialize(statusLoc, this, id);
	controlLocation = POINT{ 480, 90 };
	notes.initialize( controlLocation, this, id );
	notes.setActiveControls("none");
	RhodeSchwarzGenerator.initialize( controlLocation, toolTips, this, id );
	debugControl.initialize( controlLocation, this, toolTips, id );
	topBottomAgilent.initialize( controlLocation, toolTips, this, id, "USB0::2391::11271::MY52801397::0::INSTR", "Top/Bottom Agilent" );
	uWaveAxialAgilent.initialize( controlLocation, toolTips, this, id, "STUFF...", "U-Wave / Axial Agilent" );
	flashingAgilent.initialize( controlLocation, toolTips, this, id, "STUFF...", "Flashing Agilent" );
	controlLocation = POINT{ 1320, 0 };
	masterScript.initialize( 600, 1080, controlLocation, toolTips, this, id );

	// controls are done. Report the initialization status...
	std::string msg;
	msg +=	"==========================================\n"
			"=============MASTER CONTROL===============\n"
			"==========================================\n"
			"==The Quantum Gas Assembly Control System=\n"
			"==========================================\n\n";
	
	msg += "<<Code Safemode Settings>>\n";
	msg += "TTL System... ";
	if ( !DIO_SAFEMODE )
	{
		msg += "ACTIVE!\n";
	}
	else
	{
		msg += "DISABLED!\n";
	}
	msg += "DAC System... ";
	if ( !DAQMX_SAFEMODE )
	{
		msg += "ACTIVE!\n";
	}
	else
	{
		msg += "DISABLED!\n";
	}
	msg += "GPIB System... ";
	if ( !GPIB_SAFEMODE )
	{
		msg += "ACTIVE!\n";
	}
	else
	{
		msg += "DISABLED!\n";
	}
	msg += "Agilent System... ";
	if ( !AGILENT_SAFEMODE )
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
	msg += "Tektronics 1: " + gpib.queryIdentity( TEKTRONICS_AFG_1_ADDRESS ) + "\n";
	msg += "Tektronics 2: " + gpib.queryIdentity( TEKTRONICS_AFG_2_ADDRESS ) + "\n";
	msg += "RSG: " + gpib.queryIdentity( RSG_ADDRESS ) + "\n";
	errBox( msg );
	
	menu.LoadMenu(IDC_MASTERCODE);
	SetMenu(&menu);
	ShowWindow(SW_MAXIMIZE);
	return TRUE;
}

rgbMap MasterWindow::getRGBs()
{
	return masterRGBs;
}