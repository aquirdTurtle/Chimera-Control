#include "stdafx.h"
#include "CameraWindow.h"
#include "commonMessages.h"
#include "CameraSettingsControl.h"
#include "PlottingInfo.h"


IMPLEMENT_DYNAMIC(CameraWindow, CDialog)

BEGIN_MESSAGE_MAP(CameraWindow, CDialog)
	ON_WM_CTLCOLOR()
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_WM_VSCROLL()
	// menu stuff
	ON_COMMAND_RANGE(MENU_ID_RANGE_BEGIN, MENU_ID_RANGE_END, &CameraWindow::passCommonCommand)
	ON_COMMAND_RANGE(PICTURE_SETTINGS_ID_START, PICTURE_SETTINGS_ID_END, &CameraWindow::handlePictureSettings)
	// 
	ON_COMMAND(IDC_SET_IMAGE_PARAMETERS_BUTTON, &CameraWindow::readImageParameters)
	ON_COMMAND(IDC_SET_EM_GAIN_BUTTON, &CameraWindow::setEmGain)
	ON_COMMAND(IDC_ALERTS_BOX, &CameraWindow::passAlertPress)
	ON_COMMAND(IDC_SET_TEMPERATURE_BUTTON, &CameraWindow::passSetTemperaturePress)
	//
	ON_CBN_SELENDOK(IDC_TRIGGER_COMBO, &CameraWindow::passTrigger)
	//
	//ON_WM_LBUTTONDBLCLK()
	//ON_NOTIFY(LVN_COLUMNCLICK, IDC_PLOTTING_LISTVIEW, &CameraWindow::listViewLClick)
	ON_NOTIFY(NM_DBLCLK, IDC_PLOTTING_LISTVIEW, &CameraWindow::listViewDblClick)
	ON_NOTIFY(NM_RCLICK, IDC_PLOTTING_LISTVIEW, &CameraWindow::listViewRClick)
END_MESSAGE_MAP()

void CameraWindow::onCameraFinish()
{
	// notify the andor object that it is done.
	this->Andor.onFinish();
	this->Andor.pauseThread();
}

void CameraWindow::startCamera()
{
	Andor.setSystem( this );
}

bool CameraWindow::getCameraStatus()
{
	return this->Andor.isRunning();
}

void CameraWindow::listViewDblClick(NMHDR* info, LRESULT* lResult)
{
	this->dataHandler.handleDoubleClick();
	return;
}

void CameraWindow::listViewRClick(NMHDR* info, LRESULT* lResult)
{
	this->dataHandler.handleRClick();
	return;
}

/*
 *
 */
void CameraWindow::passSetTemperaturePress()
{
	this->CameraSettings.handleSetTemperaturePress();
	return;
}

/*
 *
 */
void CameraWindow::OnTimer(UINT_PTR id)
{
	this->CameraSettings.handleTimer();
	return;
}
/*
 *
 */
void CameraWindow::passAlertPress()
{
	this->alerts.handleCheckBoxPress();
	return;
}

/*
 *
 */
void CameraWindow::passTrigger()
{
	this->CameraSettings.handleTriggerControl(this);
	return;
}

void CameraWindow::handlePictureSettings(UINT id)
{
	this->CameraSettings.handlePictureSettings(id, &this->Andor);
}

BOOL CameraWindow::PreTranslateMessage(MSG* pMsg)
{
	for (int toolTipInc = 0; toolTipInc < this->tooltips.size(); toolTipInc++)
	{
		this->tooltips[toolTipInc]->RelayEvent(pMsg);
	}
	return CDialog::PreTranslateMessage(pMsg);
}

void CameraWindow::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* scrollbar)
{
	this->pics.handleScroll(nSBCode, nPos, scrollbar);
}
void CameraWindow::OnSize(UINT nType, int cx, int cy)
{
	AndorRunSettings settings = this->CameraSettings.getSettings();
	this->stats.rearrange(settings.cameraMode, settings.triggerMode, cx, cy, this->mainWindowFriend->getFonts());
	this->CameraSettings.rearrange(settings.cameraMode, settings.triggerMode, cx, cy, this->mainWindowFriend->getFonts());
	this->box.rearrange(settings.cameraMode, settings.triggerMode, cx, cy, this->mainWindowFriend->getFonts());
	this->pics.rearrange(settings.cameraMode, settings.triggerMode, cx, cy, this->mainWindowFriend->getFonts());
	this->alerts.rearrange(settings.cameraMode, settings.triggerMode, cx, cy, this->mainWindowFriend->getFonts());
	this->dataHandler.rearrange(settings.cameraMode, settings.triggerMode, cx, cy, this->mainWindowFriend->getFonts());
	return;
}

void CameraWindow::setEmGain()
{
	this->CameraSettings.setEmGain(&this->Andor);
}

void CameraWindow::prepareCamera()
{
	if ( this->Andor.isRunning() )
	{
		thrower( "System is already running! Please Abort to restart.\r\n" );
		return;
	}
	if ( this->dataHandler.getLocationSettingStatus() )
	{
		thrower( "Please finish selecting analysis points!" );
		return;
	}
	// biggest check here, camera settings includes a lot of things.
	CameraSettings.checkIfReady();
	try
	{
		this->Andor.getStatus();
		if ( ANDOR_SAFEMODE )
		{
			thrower( "DRV_IDLE" );
		}
	}
	catch ( my_exception& exception )
	{
		if ( exception.whatBare() != "DRV_IDLE" )
		{
			throw;
		}
	}
	this->pics.refreshBackgrounds(this);
	/// start the plotting thread.
	/*
	// set default colors and linewidths on plots
	this->currentRepetitionNumber = 1;
	// Create the Mutex. This function just opens the mutex if it already exists.
	plottingMutex = CreateMutexEx( 0, NULL, FALSE, MUTEX_ALL_ACCESS );
	// prepare for start of thread.
	plotThreadExitIndicator = true;
	/// TODO!
	// start thread.

	unsigned int * plottingThreadID = NULL;
	std::vector<std::string>* argPlotNames;
	argPlotNames = new std::vector<std::string>;
	for (int plotNameInc = 0; plotNameInc < eCurrentPlotNames.size(); plotNameInc++)
	{
	argPlotNames->push_back(eCurrentPlotNames[plotNameInc]);
	}
	// clear this before starting the thread.
	eImageVecQueue.clear();
	// start the plotting thread.
	ePlottingThreadHandle = (HANDLE)_beginthreadex(0, 0, plotterProcedure, argPlotNames, 0,
	plottingThreadID);
	*/

	/// start the camera.
	this->Andor.setSettings( CameraSettings.getSettings() );
	return;
}

std::string CameraWindow::getStartMessage()
{
	// get selected plots
	std::vector<std::string> plots = this->dataHandler.getActivePlotList();
	imageParameters currentImageParameters = this->CameraSettings.readImageParameters(this);
	bool errCheck = false;
	for ( int plotInc = 0; plotInc < plots.size(); plotInc++ )
	{
		PlottingInfo tempInfoCheck;
		if ( tempInfoCheck.loadPlottingInfoFromFile( PLOT_FILES_SAVE_LOCATION + plots[plotInc] + ".plot" ) == -1 )
		{
			thrower( "Failed to load a plot file: " + plots[plotInc] );
		}
		if ( tempInfoCheck.getPictureNumber() != this->CameraSettings.getSettings().picsPerRepetition )
		{
			thrower( "ERROR: one of the plots selected, " + plots[plotInc] + ", is not built for the currently "
					 "selected number of pictures per experiment. Please revise either the current setting or the plot"
					 " file." );
		}
		tempInfoCheck.setGroups( dataHandler.getAtomLocations() );
		std::vector<std::pair<int, int>> plotLocations = tempInfoCheck.getAllPixelLocations();
	}
	std::string dialogMsg;
	dialogMsg = "Starting Parameters:\r\n%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\r\n";
	dialogMsg += "Current Camera Temperature Setting: " + std::to_string(
		CameraSettings.getSettings().temperatureSetting ) + "\r\n";
	dialogMsg += "Exposure Times: ";
	for ( int exposureInc = 0; exposureInc < this->CameraSettings.getSettings().exposureTimes.size(); exposureInc++ )
	{
		dialogMsg += std::to_string( CameraSettings.getSettings().exposureTimes[exposureInc] * 1000 ) + ", ";
	}
	dialogMsg += "\r\n";

	dialogMsg += "Image Settings: " + std::to_string( currentImageParameters.leftBorder ) + " - " 
		+ std::to_string( currentImageParameters.rightBorder ) + ", " + std::to_string( 
			currentImageParameters.topBorder ) + " - " + std::to_string( currentImageParameters.bottomBorder ) + "\r\n";
	dialogMsg += "\r\n";
	dialogMsg += "Kintetic Cycle Time: " + std::to_string( CameraSettings.getSettings().kinetiCycleTime ) + "\r\n";
	dialogMsg += "Pictures per Repetition: " + std::to_string( CameraSettings.getSettings().picsPerRepetition ) 
		+ "\r\n";
	dialogMsg += "Repetitions per Variation: " + std::to_string( CameraSettings.getSettings().totalPicsInVariation ) 
		+ "\r\n";
	dialogMsg += "Variations per Experiment: " + std::to_string( CameraSettings.getSettings().totalVariations ) 
		+ "\r\n";
	dialogMsg += "Total Pictures per Experiment: " 
		+ std::to_string( CameraSettings.getSettings().totalPicsInExperiment ) + "\r\n";
	dialogMsg += "Real-Time Atom Detection Thresholds: ";

	for ( int exposureInc = 0; exposureInc < CameraSettings.getThresholds().size(); exposureInc++ )
	{
		dialogMsg += std::to_string( CameraSettings.getThresholds()[exposureInc] ) + ", ";
	}

	dialogMsg += "\r\n";
	dialogMsg += "Current Plotting Options: \r\n";

	for ( int plotInc = 0; plotInc < plots.size(); plotInc++ )
	{
		dialogMsg += "\t" + plots[plotInc] + "\r\n";
	}

	return dialogMsg;
}

void CameraWindow::setTimerText( std::string timerText )
{
	this->timer.setTimerDisplay( timerText );
}

void CameraWindow::setTimerColor(std::string color)
{
	this->timer.setColor( color );
	return;
}

void CameraWindow::OnCancel()
{
	passCommonCommand(ID_FILE_MY_EXIT);
	return;
}
std::vector<CToolTipCtrl*> CameraWindow::getToolTips()
{
	return this->tooltips;
}

BOOL CameraWindow::OnInitDialog()
{
	cameraPositions positions;
	// all of the initialization functions increment and use the id, so by the end it will be 3000 + # of controls.
	int id = 3000;
	positions.cssmPos = positions.amPos = positions.ksmPos = { 0,0 };
	box.initialize(positions.ksmPos, id, this, 480, this->mainWindowFriend->getFonts(), tooltips);
	
	this->CameraSettings.initialize(positions, id, this, this->mainWindowFriend->getFonts(), tooltips);
	alerts.initialize(positions, this, false, id, this->mainWindowFriend->getFonts(), tooltips);
	dataHandler.initialize(positions, id, this, this->mainWindowFriend->getFonts(), tooltips);
	POINT position = { 480, 0 };
	stats.initialize(position, this, id, this->mainWindowFriend->getFonts(), tooltips);
	position = { 757, 0 };
	pics.initialize(position, this, id, this->mainWindowFriend->getFonts(), tooltips);
	positions.ksmPos = positions.amPos = positions.cssmPos = { 757, 460 };
	timer.initialize( positions, this, false, id, this->mainWindowFriend->getFonts(), tooltips );
	
	// load the menu
	CMenu menu;
	menu.LoadMenu(IDR_MAIN_MENU);
	this->SetMenu(&menu);
	// final steps
	this->ShowWindow(SW_MAXIMIZE);
	this->SetTimer(NULL, 1000, NULL);
	return TRUE;
}

void CameraWindow::redrawPictures()
{
	this->pics.refreshBackgrounds(this);
}

HBRUSH CameraWindow::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	std::unordered_map<std::string, CBrush*> brushes = mainWindowFriend->getBrushes();
	std::unordered_map<std::string, COLORREF> rgbs = mainWindowFriend->getRGB();
	HBRUSH result;
	int num = pWnd->GetDlgCtrlID();

	result = *this->CameraSettings.handleColor(num, pDC, mainWindowFriend->getBrushes(), mainWindowFriend->getRGB());
	if (result) { return result; }

	switch (nCtlColor)
	{
		case CTLCOLOR_STATIC:
		{			
			CBrush* result = this->box.handleColoring(num, pDC, brushes, rgbs);
			if (result)
			{
				return *result;
			}
			else
			{
				pDC->SetTextColor(rgbs["White"]);
				pDC->SetBkColor(rgbs["Dark Grey Red"]);
				return *brushes["Dark Grey Red"];
			}
		}
		case CTLCOLOR_EDIT:
		{
			pDC->SetTextColor(rgbs["White"]);
			pDC->SetBkColor(rgbs["Dark Green"]);
			return *brushes["Dark Green"];
		}
		case CTLCOLOR_LISTBOX:
		{
			pDC->SetTextColor(rgbs["White"]);
			pDC->SetBkColor(rgbs["Dark Grey"]);
			return *brushes["Dark Grey"];
		}
		default:
		{
			return *brushes["Light Grey"];
		}
	}
}

void CameraWindow::passCommonCommand(UINT id)
{
	commonMessages::handleCommonMessage(id, this, mainWindowFriend, scriptingWindowFriend, this);
}

void CameraWindow::readImageParameters()
{
	this->redrawPictures();
	imageParameters parameters = this->CameraSettings.readImageParameters(this);
	this->pics.setParameters(parameters);
	this->pics.drawGrids(this, this->mainWindowFriend->getBrushes()["White"]);
	return;
}

void CameraWindow::changeBoxColor(colorBoxes<char> colors)
{
	this->box.changeColor(colors);
	return;
}
