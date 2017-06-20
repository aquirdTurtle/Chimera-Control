
#include "stdafx.h"
#include "PictureSettingsControl.h"
#include "Commctrl.h"
#include "reorganizeControl.h"
#include "Andor.h"
#include "CameraSettingsControl.h"
#include "CameraWindow.h"

void PictureSettingsControl::cameraIsOn( bool state )
{
	this->setPictureOptionsButton.EnableWindow( !state );
	return;
}

void PictureSettingsControl::initialize( cameraPositions& pos, CWnd* parent, int& id )
{
	if (id != PICTURE_SETTINGS_ID_START)
	{
		throw;
	}
	// introducing things row by row
	/// Set Picture Options
	setPictureOptionsButton.seriesPos = { pos.seriesPos.x, pos.seriesPos.y, pos.seriesPos.x + 480, pos.seriesPos.y += 25 };
	setPictureOptionsButton.amPos = { pos.amPos.x, pos.amPos.y, pos.amPos.x + 480, pos.amPos.y += 25 };
	setPictureOptionsButton.videoPos = { pos.videoPos.x, pos.videoPos.y, pos.videoPos.x + 480, pos.videoPos.y += 25 };
	setPictureOptionsButton.ID = id++;
	setPictureOptionsButton.Create( "Set Picture Options", WS_CHILD | WS_VISIBLE, setPictureOptionsButton.seriesPos, parent, 
									setPictureOptionsButton.ID );
	setPictureOptionsButton.fontType = Normal;
	/// Picture Numbers
	pictureLabel.seriesPos = { pos.seriesPos.x, pos.seriesPos.y, pos.seriesPos.x + 100, pos.seriesPos.y + 20 };
	pictureLabel.amPos = { pos.amPos.x, pos.amPos.y, pos.amPos.x + 100,	pos.amPos.y + 20 };
	pictureLabel.videoPos = { pos.videoPos.x, pos.videoPos.y, pos.videoPos.x + 100,	pos.videoPos.y + 20 };
	pictureLabel.ID = id++;
	pictureLabel.Create( "Picture #:", WS_CHILD | WS_VISIBLE, pictureLabel.seriesPos, parent, pictureLabel.ID );
	for (int picInc = 0; picInc < 4; picInc++)
	{
		pictureNumbers[picInc].seriesPos = { pos.seriesPos.x + 100 + 95 * picInc, pos.seriesPos.y, 
											 pos.seriesPos.x + 100 + 95 * (picInc + 1), pos.seriesPos.y + 20 };
		pictureNumbers[picInc].amPos = { pos.amPos.x + 100 + 95 * picInc, pos.amPos.y, pos.amPos.x + 100 + 95 * (picInc + 1), 
										 pos.amPos.y + 20 };
		pictureNumbers[picInc].videoPos = { pos.videoPos.x + 100 + 95 * picInc, pos.videoPos.y, pos.videoPos.x + 100 + 95 * (picInc + 1), 
											pos.videoPos.y + 20 };
		pictureNumbers[picInc].ID = id++;
		pictureNumbers[picInc].Create( cstr( picInc + 1 ), WS_CHILD | WS_VISIBLE | SS_CENTER, pictureNumbers[picInc].seriesPos, parent, 
									   pictureNumbers[picInc].ID );
	}
	pos.seriesPos.y += 20;
	pos.amPos.y += 20;
	pos.videoPos.y += 20;

	/// Total picture number
	totalPicNumberLabel.seriesPos = { pos.seriesPos.x, pos.seriesPos.y, pos.seriesPos.x + 100, pos.seriesPos.y + 20 };
	totalPicNumberLabel.amPos = { pos.amPos.x, pos.amPos.y, pos.amPos.x + 100, pos.amPos.y + 20 };
	totalPicNumberLabel.videoPos = { pos.videoPos.x, pos.videoPos.y, pos.videoPos.x + 100, pos.videoPos.y + 20 };
	totalPicNumberLabel.ID = id++;
	totalPicNumberLabel.Create( "Total Picture #", WS_CHILD | WS_VISIBLE, totalPicNumberLabel.seriesPos, parent, totalPicNumberLabel.ID );
	for (int picInc = 0; picInc < 4; picInc++)
	{
		totalNumberChoice[picInc].seriesPos = { pos.seriesPos.x + 100 + 95 * picInc, pos.seriesPos.y, 
												pos.seriesPos.x + 100 + 95 * (picInc + 1), pos.seriesPos.y + 20 };
		totalNumberChoice[picInc].amPos = { pos.amPos.x + 100 + 95 * picInc, pos.amPos.y, pos.amPos.x + 100 + 95 * (picInc + 1), 
											pos.amPos.y + 20 };
		totalNumberChoice[picInc].videoPos = { pos.videoPos.x + 100 + 95 * picInc, pos.videoPos.y, pos.videoPos.x + 100 + 95 * (picInc + 1),
											   pos.videoPos.y + 20 };
		totalNumberChoice[picInc].ID = id++;
		if (picInc == 0)
		{
			// first of group
			totalNumberChoice[picInc].Create( "", WS_CHILD | WS_VISIBLE | BS_CENTER | WS_GROUP | BS_AUTORADIOBUTTON,
											  totalNumberChoice[picInc].seriesPos, parent, totalNumberChoice[picInc].ID );
			totalNumberChoice[picInc].SetCheck( 1 );
		}
		else
		{
			// members of group.
			totalNumberChoice[picInc].Create( "", WS_CHILD | WS_VISIBLE | BS_CENTER | BS_AUTORADIOBUTTON,
											  totalNumberChoice[picInc].seriesPos, parent, totalNumberChoice[picInc].ID );
		}
	}
	pos.seriesPos.y += 20;
	pos.amPos.y += 20;
	pos.videoPos.y += 20;

	/// Exposure Times
	exposureLabel.seriesPos = { pos.seriesPos.x, pos.seriesPos.y, pos.seriesPos.x + 100, pos.seriesPos.y + 20 };
	exposureLabel.amPos = { pos.amPos.x, pos.amPos.y, pos.amPos.x + 100, pos.amPos.y + 20 };
	exposureLabel.videoPos = { pos.videoPos.x, pos.videoPos.y, pos.videoPos.x + 100, pos.videoPos.y + 20 };
	exposureLabel.ID = id++;
	exposureLabel.Create( "Exposure (ms):", WS_CHILD | WS_VISIBLE, exposureLabel.seriesPos, parent, exposureLabel.ID );
	exposureTimesUnofficial.resize( 4 );
	for (int picInc = 0; picInc < 4; picInc++)
	{
		exposureEdits[picInc].seriesPos = { pos.seriesPos.x + 100 + 95 * picInc, pos.seriesPos.y,
			pos.seriesPos.x + 100 + 95 * (picInc + 1), pos.seriesPos.y + 20 };
		exposureEdits[picInc].amPos = { pos.amPos.x + 100 + 95 * picInc, pos.amPos.y,
			pos.amPos.x + 100 + 95 * (picInc + 1), pos.amPos.y + 20 };
		exposureEdits[picInc].videoPos = { pos.videoPos.x + 100 + 95 * picInc, pos.videoPos.y,
			pos.videoPos.x + 100 + 95 * (picInc + 1), pos.videoPos.y + 20 };
		exposureEdits[picInc].ID = id++;
		// first of group
		exposureEdits[picInc].Create( WS_CHILD | WS_VISIBLE | WS_BORDER, exposureEdits[picInc].seriesPos, parent, exposureEdits[picInc].ID );
		exposureEdits[picInc].SetWindowTextA( "20" );
		exposureTimesUnofficial[picInc] = 20 / 1000.0;
	}
	pos.seriesPos.y += 20;
	pos.amPos.y += 20;
	pos.videoPos.y += 20;

	/// Thresholds
	thresholdLabel.seriesPos = { pos.seriesPos.x, pos.seriesPos.y, pos.seriesPos.x + 100, pos.seriesPos.y + 20 };
	thresholdLabel.amPos = { pos.amPos.x, pos.amPos.y, pos.amPos.x + 100, pos.amPos.y + 20 };
	thresholdLabel.videoPos = { pos.videoPos.x, pos.videoPos.y, pos.videoPos.x + 100, pos.videoPos.y + 20 };
	thresholdLabel.ID = id++;
	thresholdLabel.Create( "Threshold (cts)", WS_CHILD | WS_VISIBLE, thresholdLabel.seriesPos, parent, thresholdLabel.ID );
	for (int picInc = 0; picInc < 4; picInc++)
	{
		thresholdEdits[picInc].seriesPos = { pos.seriesPos.x + 100 + 95 * picInc, pos.seriesPos.y,
			pos.seriesPos.x + 100 + 95 * (picInc + 1), pos.seriesPos.y + 20 };
		thresholdEdits[picInc].amPos = { pos.amPos.x + 100 + 95 * picInc, pos.amPos.y,
			pos.amPos.x + 100 + 95 * (picInc + 1), pos.amPos.y + 20 };
		thresholdEdits[picInc].videoPos = { pos.videoPos.x + 100 + 95 * picInc, pos.videoPos.y,
			pos.videoPos.x + 100 + 95 * (picInc + 1), pos.videoPos.y + 20 };
		thresholdEdits[picInc].ID = id++;
		// first of group
		thresholdEdits[picInc].Create( WS_CHILD | WS_VISIBLE | WS_BORDER, thresholdEdits[picInc].seriesPos, parent, 
									   thresholdEdits[picInc].ID );
		thresholdEdits[picInc].SetWindowTextA( "100" );
		this->thresholds[picInc] = 100;
	}
	pos.seriesPos.y += 20;
	pos.amPos.y += 20;
	pos.videoPos.y += 20;

	/// Yellow --> Blue Color
	viridaLabel.seriesPos = { pos.seriesPos.x, pos.seriesPos.y, pos.seriesPos.x + 100, pos.seriesPos.y += 20 };
	viridaLabel.amPos = { pos.amPos.x, pos.amPos.y, pos.amPos.x + 100, pos.amPos.y += 20 };
	viridaLabel.videoPos = { pos.videoPos.x, pos.videoPos.y, pos.videoPos.x + 100, pos.videoPos.y += 20 };
	viridaLabel.ID = id++;
	viridaLabel.Create( "Virida", WS_CHILD | WS_VISIBLE, viridaLabel.seriesPos, parent, viridaLabel.ID );

	/// Red --> Blue color
	infernoLabel.seriesPos = { pos.seriesPos.x, pos.seriesPos.y, pos.seriesPos.x + 100, pos.seriesPos.y += 20 };
	infernoLabel.amPos = { pos.amPos.x, pos.amPos.y, pos.amPos.x + 100, pos.amPos.y += 20 };
	infernoLabel.videoPos = { pos.videoPos.x, pos.videoPos.y, pos.videoPos.x + 100, pos.videoPos.y += 20 };
	infernoLabel.ID = id++;
	infernoLabel.Create( "Inferno", WS_CHILD | WS_VISIBLE, infernoLabel.seriesPos, parent, infernoLabel.ID );

	/// Black --> White color
	blackWhiteLabel.seriesPos = { pos.seriesPos.x, pos.seriesPos.y, pos.seriesPos.x + 100, pos.seriesPos.y += 20 };
	blackWhiteLabel.amPos = { pos.amPos.x, pos.amPos.y, pos.amPos.x + 100, pos.amPos.y += 20 };
	blackWhiteLabel.videoPos = { pos.videoPos.x, pos.videoPos.y, pos.videoPos.x + 100, pos.videoPos.y += 20 };
	blackWhiteLabel.ID = id++;
	blackWhiteLabel.Create( "Greyscale", WS_CHILD | WS_VISIBLE, blackWhiteLabel.seriesPos, parent, blackWhiteLabel.ID );

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
		this->veridaRadios[picInc].ID = id++;
		veridaRadios[picInc].Create( "", BS_CENTER | WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | WS_GROUP,
										 veridaRadios[picInc].seriesPos, parent, veridaRadios[picInc].ID );
		pos.seriesPos.y += 20;
		pos.amPos.y += 20;
		pos.videoPos.y += 20;
		infernoRadios[picInc].seriesPos = { pos.seriesPos.x + 100 + 95 * picInc, pos.seriesPos.y,
			pos.seriesPos.x + 100 + 95 * (picInc + 1), pos.seriesPos.y + 20 };
		infernoRadios[picInc].amPos = { pos.amPos.x + 100 + 95 * picInc, pos.amPos.y,
			pos.amPos.x + 100 + 95 * (picInc + 1), pos.amPos.y + 20 };
		infernoRadios[picInc].videoPos = { pos.videoPos.x + 100 + 95 * picInc, pos.videoPos.y,
			pos.videoPos.x + 100 + 95 * (picInc + 1), pos.videoPos.y + 20 };
		infernoRadios[picInc].ID = id++;
		infernoRadios[picInc].Create( "", BS_CENTER | WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
									  infernoRadios[picInc].seriesPos, parent, infernoRadios[picInc].ID );
		pos.seriesPos.y += 20;
		pos.amPos.y += 20;
		pos.videoPos.y += 20;
		blackWhiteRadios[picInc].seriesPos = { pos.seriesPos.x + 100 + 95 * picInc, pos.seriesPos.y,
			pos.seriesPos.x + 100 + 95 * (picInc + 1), pos.seriesPos.y + 20 };
		blackWhiteRadios[picInc].amPos = { pos.amPos.x + 100 + 95 * picInc, pos.amPos.y,
			pos.amPos.x + 100 + 95 * (picInc + 1), pos.amPos.y + 20 };
		blackWhiteRadios[picInc].videoPos = { pos.videoPos.x + 100 + 95 * picInc, pos.videoPos.y,
			pos.videoPos.x + 100 + 95 * (picInc + 1), pos.videoPos.y + 20 };
		blackWhiteRadios[picInc].ID = id++;
		// first of group
		blackWhiteRadios[picInc].Create( "", BS_CENTER | WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
										 blackWhiteRadios[picInc].seriesPos, parent, blackWhiteRadios[picInc].ID );
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
	if (id - 1 != PICTURE_SETTINGS_ID_END)
	{
		throw;
	}


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
	if (id >= exposureEdits.front().ID && id <= exposureEdits.back().ID)
	{
		int picNum = id - exposureEdits.front().ID;
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
			exposure = std::stof(std::string(text));// / 1000.0f;
			double dif = std::fabs(exposure/1000.0 - exposureTimesUnofficial[picNum]);
			if (dif < 0.000000001)
			{
				// good.
				colorer->SetBkColor(rgbs["Dark Green"]);
				// catch change of color and redraw window.
				if (exposureEdits[picNum].colorState != 0)
				{
					exposureEdits[picNum].colorState = 0;
					exposureEdits[picNum].RedrawWindow();
				}
				return brushes["Dark Green"];
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
	else if (id >= this->thresholdEdits.front().ID && id <= this->thresholdEdits.back().ID)
	{
		int picNum = id - thresholdEdits.front().ID;
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
			threshold = std::stoi(std::string(text));
			double dif = std::fabs(threshold - thresholds[picNum]);
			if (dif < 0.000000001)
			{
				// good.
				colorer->SetBkColor(rgbs["Dark Green"]);
				// catch change of color and redraw window.
				if (thresholdEdits[picNum].colorState != 0)
				{
					thresholdEdits[picNum].colorState = 0;
					thresholdEdits[picNum].RedrawWindow();
				}
				return brushes["Dark Green"];
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

int PictureSettingsControl::getPicsPerRepetition()
{
	return picsPerRepetitionUnofficial;
}

void PictureSettingsControl::handleOptionChange(UINT id, AndorCamera* andorObj)
{
	if (id >= totalNumberChoice.front().ID && id <= this->totalNumberChoice.back().ID)
	{
		int picNum = id - totalNumberChoice.front().ID;
		picsPerRepetitionUnofficial = picNum + 1;
		// not all settings are changed here, and some are used to recalculate totals.
		AndorRunSettings settings = andorObj->getSettings();
		settings.picsPerRepetition = picsPerRepetitionUnofficial;
		settings.totalPicsInVariation = settings.picsPerRepetition  * settings.repetitionsPerVariation;
		settings.totalPicsInExperiment = settings.totalVariations * settings.totalPicsInVariation;
		for (int picInc = 0; picInc < 4; picInc++)
		{
			if (picInc <= picNum)
			{
				enablePictureControls(picInc);
			}
			else
			{
				disablePictureControls(picInc);
			}
		}
	}
	else if (id == setPictureOptionsButton.ID)
	{
		// grab the thresholds
		for (int thresholdInc = 0; thresholdInc < 4; thresholdInc++)
		{
			CString textEdit;
			thresholdEdits[thresholdInc].GetWindowTextA(textEdit);
			int threshold;
			try
			{
				threshold = std::stoi(std::string(textEdit));
				thresholds[thresholdInc] = threshold;
			}
			catch (std::invalid_argument)
			{
				errBox("ERROR: failed to convert threshold number " + std::to_string(thresholdInc + 1) + " to an integer.");
			}
			thresholdEdits[thresholdInc].RedrawWindow();
		}
		// grab the exposures.
		for (int exposureInc = 0; exposureInc < 4; exposureInc++)
		{
			CString textEdit;
			exposureEdits[exposureInc].GetWindowTextA(textEdit);
			double exposure;
			try
			{
				exposure = std::stof(std::string(textEdit));
				exposureTimesUnofficial[exposureInc] = exposure / 1000.0f;
			}
			catch (std::invalid_argument)
			{
				errBox("ERROR: failed to convert exposure number " + std::to_string(exposureInc + 1) + " to an integer.");
			}
			// refresh for new color
			exposureEdits[exposureInc].RedrawWindow();
		}
		/// set the exposure times via andor
		setExposureTimes(andorObj);
	}
	else if (id >= veridaRadios[0].ID && id <= blackWhiteRadios[3].ID)
	{
		id -= veridaRadios[0].ID;
		int pic = id / 3;
		int color = id % 3;
		colors[pic] = color;
	}
}

void PictureSettingsControl::setExposureTimes(AndorCamera* andorObj)
{
	setExposureTimes( this->exposureTimesUnofficial, andorObj );
}

void PictureSettingsControl::setExposureTimes(std::vector<float> times, AndorCamera* andorObj)
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

	for (int exposureInc = 0; exposureInc < exposuresToSet.size(); exposureInc++)
	{
		this->exposureTimesUnofficial[exposureInc] = exposuresToSet[exposureInc];
	}

	if (this->exposureTimesUnofficial.size() <= 0)
	{
		// this shouldn't happend
		thrower("ERROR: reached bad location where exposure times was of zero size, but this should have been detected earlier in the "
				 "code.");
	}
	// now output things.
	for (int exposureInc = 0; exposureInc < 4; exposureInc++)
	{
		exposureEdits[exposureInc].SetWindowTextA(std::to_string(this->exposureTimesUnofficial[exposureInc] * 1000).c_str());
	}
	/*
	SendMessage(eKineticCycleTimeDispHandle.hwnd, WM_SETTEXT, 0, 
		(LPARAM)std::to_string(eKineticCycleTime * 1000).c_str());
	SendMessage(eAccumulationTimeDisp.hwnd, WM_SETTEXT, 0, 
		(LPARAM)std::to_string(eAccumulationTime * 1000).c_str());
	eCameraFileSystem.updateSaveStatus(false);
	*/
}

std::vector<float> PictureSettingsControl::getUsedExposureTimes()
{
	std::vector<float> usedTimes;
	usedTimes = this->exposureTimesUnofficial;
	usedTimes.resize(this->picsPerRepetitionUnofficial);
	return usedTimes;
}

/*
 * modifies values for exposures, accumlation time, kinetic cycle time as the andor camera reports them.
 */
void PictureSettingsControl::confirmAcquisitionTimings()
{
	std::vector<float> usedExposures;
	usedExposures = this->exposureTimesUnofficial;
	usedExposures.resize(this->picsPerRepetitionUnofficial);
	try
	{
		parentSettingsControl->checkTimings(usedExposures);
	}
	catch (std::runtime_error)
	{
		throw;
	}
	for (int exposureInc = 0; exposureInc < usedExposures.size(); exposureInc++)
	{
		this->exposureTimesUnofficial[exposureInc] = usedExposures[exposureInc];
	}
	
	return;
}

/**/
std::array<int, 4> PictureSettingsControl::getThresholds()
{
	return this->thresholds;
}

void PictureSettingsControl::setThresholds(std::array<int, 4> newThresholds)
{
	this->thresholds = newThresholds;
	for (int thresholdInc = 0; thresholdInc < thresholds.size(); thresholdInc++)
	{
		thresholdEdits[thresholdInc].SetWindowTextA(std::to_string(thresholds[thresholdInc]).c_str());
	}
	return;
}

void PictureSettingsControl::setPicturesPerExperiment(unsigned int pics, AndorCamera* andorObj)
{
	if (pics > 4)
	{
		return;
	}
	this->picsPerRepetitionUnofficial = pics;
	AndorRunSettings settings = andorObj->getSettings();
	settings.picsPerRepetition = this->picsPerRepetitionUnofficial;
	settings.totalPicsInVariation = settings.picsPerRepetition  * settings.repetitionsPerVariation;
	settings.totalPicsInExperiment = settings.totalVariations * settings.totalPicsInVariation;
	for (int picInc = 0; picInc < 4; picInc++)
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
			this->enablePictureControls(picInc);
		}
		else
		{
			this->disablePictureControls(picInc);
		}
	}

	return;
}

/*

*/
std::array<int, 4> PictureSettingsControl::getPictureColors()
{
	return this->colors;
}

/*

*/
void PictureSettingsControl::rearrange(std::string cameraMode, std::string triggerMode, int width, int height, 
									   fontMap fonts)
{
	setPictureOptionsButton.rearrange(cameraMode, triggerMode, width, height, fonts);
	totalPicNumberLabel.rearrange(cameraMode, triggerMode, width, height, fonts);
	pictureLabel.rearrange(cameraMode, triggerMode, width, height, fonts);
	exposureLabel.rearrange(cameraMode, triggerMode, width, height, fonts);
	thresholdLabel.rearrange(cameraMode, triggerMode, width, height, fonts);
	blackWhiteLabel.rearrange(cameraMode, triggerMode, width, height, fonts);
	infernoLabel.rearrange(cameraMode, triggerMode, width, height, fonts);
	viridaLabel.rearrange(cameraMode, triggerMode, width, height, fonts);

	for (auto& control : this->pictureNumbers)
	{
		control.rearrange(cameraMode, triggerMode, width, height, fonts);
	}
	for (auto& control : this->totalNumberChoice)
	{
		control.rearrange(cameraMode, triggerMode, width, height, fonts);
	}
	for (auto& control : this->exposureEdits)
	{
		control.rearrange(cameraMode, triggerMode, width, height, fonts);
	}
	for (auto& control : this->thresholdEdits)
	{
		control.rearrange(cameraMode, triggerMode, width, height, fonts);
	}
	for (auto& control : this->infernoRadios)
	{
		control.rearrange(cameraMode, triggerMode, width, height, fonts);
	}
	for (auto& control : this->veridaRadios)
	{
		control.rearrange(cameraMode, triggerMode, width, height, fonts);
	}
	for (auto& control : this->blackWhiteRadios)
	{
		control.rearrange(cameraMode, triggerMode, width, height, fonts);
	}
}
