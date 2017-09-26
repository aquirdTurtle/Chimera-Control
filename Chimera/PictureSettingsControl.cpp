
#include "stdafx.h"
#include "PictureSettingsControl.h"
#include "Commctrl.h"
#include "Andor.h"
#include "CameraSettingsControl.h"
#include "CameraWindow.h"


void PictureSettingsControl::handleNewConfig( std::ofstream& newFile )
{
	newFile << "PICTURE_SETTINGS\n";
	newFile << 1 << "\n";
	for ( auto color : colors )
	{
		newFile << 0 << " ";
	}
	newFile << "\n";
	for ( auto exposure : exposureTimesUnofficial )
	{
		// in seconds
		newFile << 0.025 << " ";
	}
	newFile << "\n";
	for ( auto threshold : thresholds )
	{
		newFile << 200 << " ";
	}
	newFile << "\n";
	newFile << "END_PICTURE_SETTINGS\n";
}


void PictureSettingsControl::handleSaveConfig(std::ofstream& saveFile)
{
	saveFile << "PICTURE_SETTINGS\n";
	saveFile << picsPerRepetitionUnofficial << "\n";
	for (auto color : colors)
	{
		saveFile << color << " ";
	}
	saveFile << "\n";
	for (auto exposure : exposureTimesUnofficial)
	{
		saveFile << exposure << " ";
	}
	saveFile << "\n";
	for (auto threshold : thresholds)
	{
		saveFile << threshold << " ";
	}
	saveFile << "\n";
	saveFile << "END_PICTURE_SETTINGS\n";
}


void PictureSettingsControl::handleOpenConfig(std::ifstream& openFile, double version, AndorCamera* andor)
{
	ProfileSystem::checkDelimiterLine(openFile, "PICTURE_SETTINGS");
	UINT picsPerRep;
	openFile >> picsPerRep;
	setUnofficialPicsPerRep( picsPerRep, andor );
	std::array<int, 4> fileThresholds;
	for (auto& color : colors)
	{
		openFile >> color;
	}
	for (auto& exposure : exposureTimesUnofficial)
	{
		openFile >> exposure;
	}
	for (auto& threshold : fileThresholds )
	{
		openFile >> threshold;
	}
	setExposureTimes(andor);
	setThresholds( fileThresholds );
	ProfileSystem::checkDelimiterLine(openFile, "END_PICTURE_SETTINGS");
}


void PictureSettingsControl::initialize( cameraPositions& pos, CWnd* parent, int& id )
{
	// introducing things row by row
	/// Set Picture Options
	UINT runningCount = 0;
	/*
	setPictureOptionsButton.seriesPos = { pos.seriesPos.x, pos.seriesPos.y, pos.seriesPos.x + 480, pos.seriesPos.y += 25 };
	setPictureOptionsButton.amPos = { pos.amPos.x, pos.amPos.y, pos.amPos.x + 480, pos.amPos.y += 25 };
	setPictureOptionsButton.videoPos = { pos.videoPos.x, pos.videoPos.y, pos.videoPos.x + 480, pos.videoPos.y += 25 };
	setPictureOptionsButton.Create( "Set Picture Options", WS_CHILD | WS_VISIBLE, setPictureOptionsButton.seriesPos, parent, 
									PICTURE_SETTINGS_ID_START + runningCount++);
	*/

	/// Picture Numbers
	pictureLabel.seriesPos = { pos.seriesPos.x, pos.seriesPos.y, pos.seriesPos.x + 100, pos.seriesPos.y + 20 };
	pictureLabel.amPos = { pos.amPos.x, pos.amPos.y, pos.amPos.x + 100,	pos.amPos.y + 20 };
	pictureLabel.videoPos = { pos.videoPos.x, pos.videoPos.y, pos.videoPos.x + 100,	pos.videoPos.y + 20 };
	pictureLabel.Create( "Picture #:", WS_CHILD | WS_VISIBLE, pictureLabel.seriesPos, parent, 
						 PICTURE_SETTINGS_ID_START + runningCount++);
	
	for (int picInc = 0; picInc < 4; picInc++)
	{
		pictureNumbers[picInc].seriesPos = { pos.seriesPos.x + 100 + 95 * picInc, pos.seriesPos.y, 
											 pos.seriesPos.x + 100 + 95 * (picInc + 1), pos.seriesPos.y + 20 };
		pictureNumbers[picInc].amPos = { pos.amPos.x + 100 + 95 * picInc, pos.amPos.y, pos.amPos.x + 100 + 95 * (picInc + 1), 
										 pos.amPos.y + 20 };
		pictureNumbers[picInc].videoPos = { pos.videoPos.x + 100 + 95 * picInc, pos.videoPos.y, 
											pos.videoPos.x + 100 + 95 * (picInc + 1), pos.videoPos.y + 20 };
		pictureNumbers[picInc].Create( cstr( picInc + 1 ), WS_CHILD | WS_VISIBLE | SS_CENTER, 
									   pictureNumbers[picInc].seriesPos, parent, PICTURE_SETTINGS_ID_START+runningCount++);
	}
	pos.seriesPos.y += 20;
	pos.amPos.y += 20;
	pos.videoPos.y += 20;

	/// Total picture number
	totalPicNumberLabel.seriesPos = { pos.seriesPos.x, pos.seriesPos.y, pos.seriesPos.x + 100, pos.seriesPos.y + 20 };
	totalPicNumberLabel.amPos = { pos.amPos.x, pos.amPos.y, pos.amPos.x + 100, pos.amPos.y + 20 };
	totalPicNumberLabel.videoPos = { pos.videoPos.x, pos.videoPos.y, pos.videoPos.x + 100, pos.videoPos.y + 20 };
	totalPicNumberLabel.Create( "Total Picture #", WS_CHILD | WS_VISIBLE, totalPicNumberLabel.seriesPos, parent, 
								PICTURE_SETTINGS_ID_START + runningCount++ );
	for (int picInc = 0; picInc < 4; picInc++)
	{
		totalNumberChoice[picInc].seriesPos = { pos.seriesPos.x + 100 + 95 * picInc, pos.seriesPos.y, 
												pos.seriesPos.x + 100 + 95 * (picInc + 1), pos.seriesPos.y + 20 };
		totalNumberChoice[picInc].amPos = { pos.amPos.x + 100 + 95 * picInc, pos.amPos.y, pos.amPos.x + 100 + 95 * (picInc + 1), 
											pos.amPos.y + 20 };
		totalNumberChoice[picInc].videoPos = { pos.videoPos.x + 100 + 95 * picInc, pos.videoPos.y, pos.videoPos.x + 100 + 95 * (picInc + 1),
											   pos.videoPos.y + 20 };
		if (picInc == 0)
		{
			// first of group
			totalNumberChoice[picInc].Create( "", WS_CHILD | WS_VISIBLE | BS_CENTER | WS_GROUP | BS_AUTORADIOBUTTON,
											  totalNumberChoice[picInc].seriesPos, parent, 
											  PICTURE_SETTINGS_ID_START + runningCount++ );
			totalNumberChoice[picInc].SetCheck( 1 );
		}
		else
		{
			// members of group.
			totalNumberChoice[picInc].Create( "", WS_CHILD | WS_VISIBLE | BS_CENTER | BS_AUTORADIOBUTTON,
											  totalNumberChoice[picInc].seriesPos, parent, 
											  PICTURE_SETTINGS_ID_START + runningCount++ );
		}
	}
	pos.seriesPos.y += 20;
	pos.amPos.y += 20;
	pos.videoPos.y += 20;

	/// Exposure Times
	exposureLabel.seriesPos = { pos.seriesPos.x, pos.seriesPos.y, pos.seriesPos.x + 100, pos.seriesPos.y + 20 };
	exposureLabel.amPos = { pos.amPos.x, pos.amPos.y, pos.amPos.x + 100, pos.amPos.y + 20 };
	exposureLabel.videoPos = { pos.videoPos.x, pos.videoPos.y, pos.videoPos.x + 100, pos.videoPos.y + 20 };
	exposureLabel.Create( "Exposure (ms):", WS_CHILD | WS_VISIBLE, exposureLabel.seriesPos, parent, 
						  PICTURE_SETTINGS_ID_START + runningCount++ );
	exposureTimesUnofficial.resize( 4 );

	for (int picInc = 0; picInc < 4; picInc++)
	{
		exposureEdits[picInc].seriesPos = { pos.seriesPos.x + 100 + 95 * picInc, pos.seriesPos.y,
											pos.seriesPos.x + 100 + 95 * (picInc + 1), pos.seriesPos.y + 20 };
		exposureEdits[picInc].amPos = { pos.amPos.x + 100 + 95 * picInc, pos.amPos.y,
										pos.amPos.x + 100 + 95 * (picInc + 1), pos.amPos.y + 20 };
		exposureEdits[picInc].videoPos = { pos.videoPos.x + 100 + 95 * picInc, pos.videoPos.y,
										   pos.videoPos.x + 100 + 95 * (picInc + 1), pos.videoPos.y + 20 };
		// first of group
		exposureEdits[picInc].Create( WS_CHILD | WS_VISIBLE | WS_BORDER, exposureEdits[picInc].seriesPos, parent, 
									  PICTURE_SETTINGS_ID_START + runningCount++ );
		exposureEdits[picInc].SetWindowTextA( "26.0" );
		exposureTimesUnofficial[picInc] = 26 / 1000.0f;
	}
	pos.seriesPos.y += 20;
	pos.amPos.y += 20;
	pos.videoPos.y += 20;

	/// Thresholds
	thresholdLabel.seriesPos = { pos.seriesPos.x, pos.seriesPos.y, pos.seriesPos.x + 100, pos.seriesPos.y + 20 };
	thresholdLabel.amPos = { pos.amPos.x, pos.amPos.y, pos.amPos.x + 100, pos.amPos.y + 20 };
	thresholdLabel.videoPos = { pos.videoPos.x, pos.videoPos.y, pos.videoPos.x + 100, pos.videoPos.y + 20 };
	thresholdLabel.Create( "Threshold (cts)", WS_CHILD | WS_VISIBLE, thresholdLabel.seriesPos, parent, 
						   PICTURE_SETTINGS_ID_START + runningCount++ );
	for (int picInc = 0; picInc < 4; picInc++)
	{
		thresholdEdits[picInc].seriesPos = { pos.seriesPos.x + 100 + 95 * picInc, pos.seriesPos.y,
			pos.seriesPos.x + 100 + 95 * (picInc + 1), pos.seriesPos.y + 20 };
		thresholdEdits[picInc].amPos = { pos.amPos.x + 100 + 95 * picInc, pos.amPos.y,
			pos.amPos.x + 100 + 95 * (picInc + 1), pos.amPos.y + 20 };
		thresholdEdits[picInc].videoPos = { pos.videoPos.x + 100 + 95 * picInc, pos.videoPos.y,
			pos.videoPos.x + 100 + 95 * (picInc + 1), pos.videoPos.y + 20 };
		// first of group
		thresholdEdits[picInc].Create( WS_CHILD | WS_VISIBLE | WS_BORDER, thresholdEdits[picInc].seriesPos, parent, 
									   PICTURE_SETTINGS_ID_START + runningCount++ );
		thresholdEdits[picInc].SetWindowTextA( "100" );
		thresholds[picInc] = 100;
	}
	pos.seriesPos.y += 20;
	pos.amPos.y += 20;
	pos.videoPos.y += 20;

	/// Yellow --> Blue Color
	viridaLabel.seriesPos = { pos.seriesPos.x, pos.seriesPos.y, pos.seriesPos.x + 100, pos.seriesPos.y += 20 };
	viridaLabel.amPos = { pos.amPos.x, pos.amPos.y, pos.amPos.x + 100, pos.amPos.y += 20 };
	viridaLabel.videoPos = { pos.videoPos.x, pos.videoPos.y, pos.videoPos.x + 100, pos.videoPos.y += 20 };
	viridaLabel.Create( "Virida", WS_CHILD | WS_VISIBLE, viridaLabel.seriesPos, parent,
						PICTURE_SETTINGS_ID_START + runningCount++ );

	/// Red --> Blue color
	infernoLabel.seriesPos = { pos.seriesPos.x, pos.seriesPos.y, pos.seriesPos.x + 100, pos.seriesPos.y += 20 };
	infernoLabel.amPos = { pos.amPos.x, pos.amPos.y, pos.amPos.x + 100, pos.amPos.y += 20 };
	infernoLabel.videoPos = { pos.videoPos.x, pos.videoPos.y, pos.videoPos.x + 100, pos.videoPos.y += 20 };
	infernoLabel.Create( "Inferno", WS_CHILD | WS_VISIBLE, infernoLabel.seriesPos, parent, 
						 PICTURE_SETTINGS_ID_START + runningCount++ );


	/// Black --> White color
	blackWhiteLabel.seriesPos = { pos.seriesPos.x, pos.seriesPos.y, pos.seriesPos.x + 100, pos.seriesPos.y += 20 };
	blackWhiteLabel.amPos = { pos.amPos.x, pos.amPos.y, pos.amPos.x + 100, pos.amPos.y += 20 };
	blackWhiteLabel.videoPos = { pos.videoPos.x, pos.videoPos.y, pos.videoPos.x + 100, pos.videoPos.y += 20 };
	blackWhiteLabel.Create( "Greyscale", WS_CHILD | WS_VISIBLE, blackWhiteLabel.seriesPos, parent, 
							PICTURE_SETTINGS_ID_START + runningCount++ );

	/// The radio buttons
	for (int picInc = 0; picInc < 4; picInc++)
	{
		pos.seriesPos.y -= 60;
		pos.amPos.y -= 60;
		pos.videoPos.y -= 60;
		veridaRadios[picInc].seriesPos = { pos.seriesPos.x + 100 + 95 * picInc, pos.seriesPos.y,
			pos.seriesPos.x + 100 + 95 * (picInc + 1), pos.seriesPos.y + 20 };
		veridaRadios[picInc].amPos = { pos.amPos.x + 100 + 95 * picInc, pos.amPos.y,
			pos.amPos.x + 100 + 95 * (picInc + 1), pos.amPos.y + 20 };
		veridaRadios[picInc].videoPos = { pos.videoPos.x + 100 + 95 * picInc, pos.videoPos.y,
			pos.videoPos.x + 100 + 95 * (picInc + 1), pos.videoPos.y + 20 };
		veridaRadios[picInc].Create( "", BS_CENTER | WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | WS_GROUP,
										 veridaRadios[picInc].seriesPos, parent, 
									 PICTURE_SETTINGS_ID_START + runningCount++ );
		pos.seriesPos.y += 20;
		pos.amPos.y += 20;
		pos.videoPos.y += 20;
		infernoRadios[picInc].seriesPos = { pos.seriesPos.x + 100 + 95 * picInc, pos.seriesPos.y,
			pos.seriesPos.x + 100 + 95 * (picInc + 1), pos.seriesPos.y + 20 };
		infernoRadios[picInc].amPos = { pos.amPos.x + 100 + 95 * picInc, pos.amPos.y,
			pos.amPos.x + 100 + 95 * (picInc + 1), pos.amPos.y + 20 };
		infernoRadios[picInc].videoPos = { pos.videoPos.x + 100 + 95 * picInc, pos.videoPos.y,
			pos.videoPos.x + 100 + 95 * (picInc + 1), pos.videoPos.y + 20 };
		infernoRadios[picInc].Create( "", BS_CENTER | WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
									  infernoRadios[picInc].seriesPos, parent, 
									  PICTURE_SETTINGS_ID_START + runningCount++ );
		pos.seriesPos.y += 20;
		pos.amPos.y += 20;
		pos.videoPos.y += 20;
		blackWhiteRadios[picInc].seriesPos = { pos.seriesPos.x + 100 + 95 * picInc, pos.seriesPos.y,
			pos.seriesPos.x + 100 + 95 * (picInc + 1), pos.seriesPos.y + 20 };
		blackWhiteRadios[picInc].amPos = { pos.amPos.x + 100 + 95 * picInc, pos.amPos.y,
			pos.amPos.x + 100 + 95 * (picInc + 1), pos.amPos.y + 20 };
		blackWhiteRadios[picInc].videoPos = { pos.videoPos.x + 100 + 95 * picInc, pos.videoPos.y,
			pos.videoPos.x + 100 + 95 * (picInc + 1), pos.videoPos.y + 20 };
		// first of group
		blackWhiteRadios[picInc].Create( "", BS_CENTER | WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
										 blackWhiteRadios[picInc].seriesPos, parent, 
										 PICTURE_SETTINGS_ID_START + runningCount++ );
		blackWhiteRadios[picInc].SetCheck( 1 );
		colors[picInc] = 2;
		pos.seriesPos.y += 20;
		pos.amPos.y += 20;
		pos.videoPos.y += 20;
	}

	// above, the total number was set to 1.
	enablePictureControls( 0 );
	disablePictureControls( 1 );
	disablePictureControls( 2 );
	disablePictureControls( 3 );
	// should move up
	picsPerRepetitionUnofficial = 1;
}


void PictureSettingsControl::disablePictureControls(int pic)
{
	if (pic > 3)
	{
		return;
	}
	exposureEdits[pic].EnableWindow(0);
	thresholdEdits[pic].EnableWindow(0);
	blackWhiteRadios[pic].EnableWindow(0);
	veridaRadios[pic].EnableWindow(0);
	infernoRadios[pic].EnableWindow(0);
}


void PictureSettingsControl::enablePictureControls( int pic )
{
	if (pic > 3)
	{
		return;
	}
	exposureEdits[pic].EnableWindow();
	thresholdEdits[pic].EnableWindow();
	blackWhiteRadios[pic].EnableWindow();
	veridaRadios[pic].EnableWindow();
	infernoRadios[pic].EnableWindow();
}


CBrush* PictureSettingsControl::colorControls(int id, CDC* colorer, brushMap brushes, rgbMap rgbs)
{
	/// Exposures
	if (id >= exposureEdits.front().GetDlgCtrlID() && id <= exposureEdits.back().GetDlgCtrlID())
	{
		int picNum = id - exposureEdits.front().GetDlgCtrlID();
		if (!exposureEdits[picNum].IsWindowEnabled())
		{
			return NULL;
		}
		colorer->SetTextColor(rgbs["White"]);
		//TCHAR textEdit[256];
		CString text;
		exposureEdits[picNum].GetWindowTextA(text);
		double exposure;
		try
		{
			exposure = std::stof(str(text));// / 1000.0f;
			double dif = std::fabs(exposure/1000.0 - exposureTimesUnofficial[picNum]);
			if (dif < 0.000000001)
			{
				// good.
				colorer->SetBkColor(rgbs["Solarized Green"]);
				// catch change of color and redraw window.
				if (exposureEdits[picNum].colorState != 0)
				{
					exposureEdits[picNum].colorState = 0;
					exposureEdits[picNum].RedrawWindow();
				}
				return brushes["Solarized Green"];
			}
		}
		catch (std::exception&)
		{
			// don't do anything with it.
		}
		colorer->SetBkColor(rgbs["Red"]);
		// catch change of color and redraw window.
		if (exposureEdits[picNum].colorState != 1)
		{
			exposureEdits[picNum].colorState = 1;
			exposureEdits[picNum].RedrawWindow();
		}
		return brushes["Red"];
	}
	/// Thresholds
	else if (id >= thresholdEdits.front().GetDlgCtrlID() && id <= thresholdEdits.back().GetDlgCtrlID())
	{
		int picNum = id - thresholdEdits.front().GetDlgCtrlID();
		if (!thresholdEdits[picNum].IsWindowEnabled())
		{
			return NULL;
		}
		colorer->SetTextColor(RGB(255, 255, 255));
		CString text;
		thresholdEdits[picNum].GetWindowTextA(text);
		int threshold;
		try
		{
			threshold = std::stoi(str(text));
			double dif = std::fabs(threshold - thresholds[picNum]);
			if (dif < 0.000000001)
			{
				// good.
				colorer->SetBkColor(rgbs["Solarized Green"]);
				// catch change of color and redraw window.
				if (thresholdEdits[picNum].colorState != 0)
				{
					thresholdEdits[picNum].colorState = 0;
					thresholdEdits[picNum].RedrawWindow();
				}
				return brushes["Solarized Green"];
			}
		}
		catch (std::exception&)
		{
			// don't do anything with it.
		}
		colorer->SetBkColor(rgbs["Red"]);
		// catch change of color and redraw window.
		if (exposureEdits[picNum].colorState != 1)
		{
			exposureEdits[picNum].colorState = 1;
			exposureEdits[picNum].RedrawWindow();
		}
		return brushes["Red"];
	}
	else
	{
		return NULL;
	}
}


UINT PictureSettingsControl::getPicsPerRepetition()
{
	return picsPerRepetitionUnofficial;
}


void PictureSettingsControl::setUnofficialPicsPerRep( UINT picNum, AndorCamera* andorObj )
{
	picsPerRepetitionUnofficial = picNum;
	// not all settings are changed here, and some are used to recalculate totals.
	AndorRunSettings settings = andorObj->getSettings( );
	settings.picsPerRepetition = picsPerRepetitionUnofficial;
	settings.totalPicsInVariation = settings.picsPerRepetition  * settings.repetitionsPerVariation;
	if ( settings.totalVariations * settings.totalPicsInVariation > INT_MAX )
	{
		thrower( "ERROR: too many pictures to take! Maximum number of pictures possible is " + str( INT_MAX ) );
	}
	settings.totalPicsInExperiment = int( settings.totalVariations * settings.totalPicsInVariation );
	andorObj->setSettings( settings );
	for ( UINT picInc = 0; picInc < 4; picInc++ )
	{
		if ( picInc < picNum )
		{
			enablePictureControls( picInc );
		}
		else
		{
			disablePictureControls( picInc );
		}
		if ( picInc == picNum-1 )
		{
			totalNumberChoice[picInc].SetCheck( 1 );
		}
		else
		{
			totalNumberChoice[picInc].SetCheck( 0 );
		}
	}
}


void PictureSettingsControl::handleOptionChange(int id, AndorCamera* andorObj)
{
	if (id >= totalNumberChoice.front().GetDlgCtrlID() && id <= totalNumberChoice.back().GetDlgCtrlID())
	{
		int picNum = id - totalNumberChoice.front().GetDlgCtrlID();
		// this message can weirdly get set after a configuration opens as well, it only means to set the number if the 
		// relevant button is now checked.
		if ( totalNumberChoice[picNum].GetCheck( ) )
		{
			setUnofficialPicsPerRep( picNum + 1, andorObj );
		}
	}
	else if (id >= veridaRadios[0].GetDlgCtrlID() && id <= blackWhiteRadios[3].GetDlgCtrlID())
	{
		id -= veridaRadios[0].GetDlgCtrlID();
		int pic = id / 3;
		int color = id % 3;
		colors[pic] = color;
	}
}


void PictureSettingsControl::setExposureTimes(AndorCamera* andorObj)
{
	setExposureTimes( exposureTimesUnofficial, andorObj );
}


void PictureSettingsControl::setExposureTimes(std::vector<float>& times, AndorCamera* andorObj)
{
	std::vector<float> exposuresToSet;
	exposuresToSet = times;
	exposuresToSet.resize(picsPerRepetitionUnofficial);
	AndorRunSettings settings = andorObj->getSettings();
	settings.exposureTimes = exposuresToSet;
	andorObj->setSettings(settings);
	// try to set this time.
	andorObj->setExposures();
	// now check actual times.
	try { parentSettingsControl->checkTimings(exposuresToSet); }
	catch (std::runtime_error&) { throw; }

	for (UINT exposureInc = 0; exposureInc < exposuresToSet.size(); exposureInc++)
	{
		exposureTimesUnofficial[exposureInc] = exposuresToSet[exposureInc];
	}

	if (exposureTimesUnofficial.size() <= 0)
	{
		// this shouldn't happend
		thrower("ERROR: reached bad location where exposure times was of zero size, but this should have been detected earlier in the "
				 "code.");
	}
	// now output things.
	for (int exposureInc = 0; exposureInc < 4; exposureInc++)
	{
		exposureEdits[exposureInc].SetWindowTextA(cstr(this->exposureTimesUnofficial[exposureInc] * 1000));
	}
}



std::vector<float> PictureSettingsControl::getUsedExposureTimes()
{
	updateSettings( );
	std::vector<float> usedTimes;
	usedTimes = exposureTimesUnofficial;
	usedTimes.resize(picsPerRepetitionUnofficial);
	return usedTimes;
}

/*
 * modifies values for exposures, accumlation time, kinetic cycle time as the andor camera reports them.
 */
void PictureSettingsControl::confirmAcquisitionTimings()
{
	std::vector<float> usedExposures;
	usedExposures = exposureTimesUnofficial;
	usedExposures.resize(picsPerRepetitionUnofficial);
	try
	{
		parentSettingsControl->checkTimings(usedExposures);
	}
	catch (std::runtime_error)
	{
		throw;
	}
	for (UINT exposureInc = 0; exposureInc < usedExposures.size(); exposureInc++)
	{
		exposureTimesUnofficial[exposureInc] = usedExposures[exposureInc];
	}
}

/**/
std::array<int, 4> PictureSettingsControl::getThresholds()
{
	updateSettings();
	return thresholds;
}

void PictureSettingsControl::setThresholds(std::array<int, 4> newThresholds)
{
	thresholds = newThresholds;
	for (UINT thresholdInc = 0; thresholdInc < thresholds.size(); thresholdInc++)
	{
		thresholdEdits[thresholdInc].SetWindowTextA(cstr(thresholds[thresholdInc]));
	}
}

void PictureSettingsControl::setPicturesPerExperiment(UINT pics, AndorCamera* andorObj)
{
	if (pics > 4)
	{
		return;
	}
	picsPerRepetitionUnofficial = pics;
	AndorRunSettings settings = andorObj->getSettings();
	settings.picsPerRepetition = picsPerRepetitionUnofficial;
	settings.totalPicsInVariation = settings.picsPerRepetition  * settings.repetitionsPerVariation;
	if (settings.totalVariations * settings.totalPicsInVariation > INT_MAX)
	{
		thrower( "ERROR: Trying to take too many pictures! Maximum picture number is " + str( INT_MAX ) );
	}
	settings.totalPicsInExperiment = int(settings.totalVariations * settings.totalPicsInVariation);
	for (UINT picInc = 0; picInc < 4; picInc++)
	{
		if (picInc == pics - 1)
		{
			totalNumberChoice[picInc].SetCheck(1);
		}
		else
		{
			totalNumberChoice[picInc].SetCheck(0);
		}

		if (picInc < picsPerRepetitionUnofficial)
		{
			enablePictureControls(picInc);
		}
		else
		{
			disablePictureControls(picInc);
		}
	}
}


/*
*/
std::array<int, 4> PictureSettingsControl::getPictureColors()
{
	return colors;
}


void PictureSettingsControl::updateSettings( )
{
	// grab the thresholds
	for ( int thresholdInc = 0; thresholdInc < 4; thresholdInc++ )
	{
		CString textEdit;
		thresholdEdits[thresholdInc].GetWindowTextA( textEdit );
		int threshold;
		try
		{
			threshold = std::stoi( str( textEdit ) );
			thresholds[thresholdInc] = threshold;
		}
		catch ( std::invalid_argument )
		{
			errBox( "ERROR: failed to convert threshold number " + str( thresholdInc + 1 ) + " to an integer." );
		}
		thresholdEdits[thresholdInc].RedrawWindow( );
	}
	// grab the exposures.
	for ( int exposureInc = 0; exposureInc < 4; exposureInc++ )
	{
		CString textEdit;
		exposureEdits[exposureInc].GetWindowTextA( textEdit );
		float exposure;
		try
		{
			exposure = std::stof( str( textEdit ) );
			exposureTimesUnofficial[exposureInc] = exposure / 1000.0f;
		}
		catch ( std::invalid_argument )
		{
			errBox( "ERROR: failed to convert exposure number " + str( exposureInc + 1 ) + " to an integer." );
		}
		// refresh for new color
		exposureEdits[exposureInc].RedrawWindow( );
	}
	/// set the exposure times via andor
	//setExposureTimes( andorObj );
}


void PictureSettingsControl::rearrange(std::string cameraMode, std::string triggerMode, int width, int height, 
									   fontMap fonts)
{
	totalPicNumberLabel.rearrange(cameraMode, triggerMode, width, height, fonts);
	pictureLabel.rearrange(cameraMode, triggerMode, width, height, fonts);
	exposureLabel.rearrange(cameraMode, triggerMode, width, height, fonts);
	thresholdLabel.rearrange(cameraMode, triggerMode, width, height, fonts);
	blackWhiteLabel.rearrange(cameraMode, triggerMode, width, height, fonts);
	infernoLabel.rearrange(cameraMode, triggerMode, width, height, fonts);
	viridaLabel.rearrange(cameraMode, triggerMode, width, height, fonts);

	for (auto& control : pictureNumbers)
	{
		control.rearrange(cameraMode, triggerMode, width, height, fonts);
	}
	for (auto& control : totalNumberChoice)
	{
		control.rearrange(cameraMode, triggerMode, width, height, fonts);
	}
	for (auto& control : exposureEdits)
	{
		control.rearrange(cameraMode, triggerMode, width, height, fonts);
	}
	for (auto& control : thresholdEdits)
	{
		control.rearrange(cameraMode, triggerMode, width, height, fonts);
	}
	for (auto& control : infernoRadios)
	{
		control.rearrange(cameraMode, triggerMode, width, height, fonts);
	}
	for (auto& control : veridaRadios)
	{
		control.rearrange(cameraMode, triggerMode, width, height, fonts);
	}
	for (auto& control : blackWhiteRadios)
	{
		control.rearrange(cameraMode, triggerMode, width, height, fonts);
	}
}
