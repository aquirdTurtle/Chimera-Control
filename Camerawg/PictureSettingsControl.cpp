
#include "stdafx.h"
#include "PictureSettingsControl.h"
#include "Commctrl.h"
#include "reorganizeControl.h"
#include "Andor.h"
#include "CameraSettingsControl.h"

bool PictureSettingsControl::initialize(POINT& kineticPos, POINT& continuousPos, POINT& accumulatePos, CWnd* parent, int& id)
{
	// introducing things row by row
	/// Set Picture Options
	setPictureOptionsButton.ksmPos = { kineticPos.x, kineticPos.y, kineticPos.x + 480, 
		kineticPos.y + 25 };
	setPictureOptionsButton.amPos = { accumulatePos.x, accumulatePos.y, accumulatePos.x + 480,
		accumulatePos.y + 25 };
	setPictureOptionsButton.cssmPos = { continuousPos.x, continuousPos.y, continuousPos.x + 480,
		continuousPos.y + 25 };
	setPictureOptionsButton.ID = id++;
	setPictureOptionsButton.Create("Set Picture Options", WS_CHILD | WS_VISIBLE, setPictureOptionsButton.ksmPos, 
		parent, setPictureOptionsButton.ID);
	kineticPos.y += 25;
	accumulatePos.y += 25;
	continuousPos.y += 25;
	/// Picture Numbers
	pictureLabel.ksmPos = { kineticPos.x, kineticPos.y, kineticPos.x + 100,
		kineticPos.y + 20 };
	pictureLabel.amPos = { accumulatePos.x, accumulatePos.y, accumulatePos.x + 100,
		accumulatePos.y + 20 };
	pictureLabel.cssmPos = { continuousPos.x, continuousPos.y, continuousPos.x + 100,
		continuousPos.y + 20 };
	pictureLabel.ID = id++;
	pictureLabel.Create("Picture #:", WS_CHILD | WS_VISIBLE, pictureLabel.ksmPos, parent, pictureLabel.ID);
	for (int picInc = 0; picInc < 4; picInc++)
	{
		pictureNumbers[picInc].ksmPos = { kineticPos.x + 100 + 95 * picInc, kineticPos.y,											   
			kineticPos.x + 100 + 95*(picInc + 1), kineticPos.y + 20 };
		pictureNumbers[picInc].amPos = { accumulatePos.x + 100 + 95 * picInc, accumulatePos.y,
			accumulatePos.x + 100 + 95 * (picInc + 1), accumulatePos.y + 20 };
		pictureNumbers[picInc].cssmPos = { continuousPos.x + 100 + 95 * picInc, continuousPos.y,
			continuousPos.x + 100 + 95 * (picInc + 1), continuousPos.y + 20 };		
		pictureNumbers[picInc].ID = id++;
		this->pictureNumbers[picInc].Create(std::to_string(picInc + 1).c_str(),
			WS_CHILD | WS_VISIBLE | SS_CENTER, pictureNumbers[picInc].ksmPos, parent, pictureNumbers[picInc].ID);
	}
	kineticPos.y += 20;
	accumulatePos.y += 20;
	continuousPos.y += 20;

	/// Total picture number
	totalPicNumberLabel.ksmPos = { kineticPos.x, kineticPos.y, kineticPos.x + 100, 
		kineticPos.y + 20 };
	totalPicNumberLabel.amPos = { accumulatePos.x, accumulatePos.y, accumulatePos.x + 100,
											  accumulatePos.y + 20 };
	totalPicNumberLabel.cssmPos = { continuousPos.x, continuousPos.y, continuousPos.x + 100,
														 continuousPos.y + 20 };
	totalPicNumberLabel.ID = id++;
	totalPicNumberLabel.Create("Total Picture #", WS_CHILD | WS_VISIBLE,
		totalPicNumberLabel.ksmPos, parent, totalPicNumberLabel.ID);
	for (int picInc = 0; picInc < 4; picInc++)
	{
		totalNumberChoice[picInc].ksmPos = { kineticPos.x + 100 + 95 * picInc, kineticPos.y,
			kineticPos.x + 100 + 95 * (picInc + 1), kineticPos.y + 20 };
		totalNumberChoice[picInc].amPos = { accumulatePos.x + 100 + 95 * picInc, accumulatePos.y,
			accumulatePos.x + 100 + 95 * (picInc + 1), accumulatePos.y + 20 };
		totalNumberChoice[picInc].cssmPos = { continuousPos.x + 100 + 95 * picInc, continuousPos.y,
			continuousPos.x + 100 + 95 * (picInc + 1), continuousPos.y + 20 };
		totalNumberChoice[picInc].ID = id++;
		if (picInc == 0)
		{
			// first of group
			this->totalNumberChoice[picInc].Create("", WS_CHILD | WS_VISIBLE | BS_CENTER | WS_GROUP | BS_AUTORADIOBUTTON,
				totalNumberChoice[picInc].ksmPos, parent, totalNumberChoice[picInc].ID);
			totalNumberChoice[picInc].SetCheck(1);
		}
		else
		{
			// members of group.
			totalNumberChoice[picInc].Create("", WS_CHILD | WS_VISIBLE | BS_CENTER | BS_AUTORADIOBUTTON,
				totalNumberChoice[picInc].ksmPos, parent, totalNumberChoice[picInc].ID);
		}
	}
	kineticPos.y += 20;
	accumulatePos.y += 20;
	continuousPos.y += 20;

	/// Exposure Times
	exposureLabel.ksmPos = { kineticPos.x, kineticPos.y, kineticPos.x + 100, 
		kineticPos.y + 20 };
	exposureLabel.amPos = { accumulatePos.x, accumulatePos.y, accumulatePos.x + 100,
		accumulatePos.y + 20 };
	exposureLabel.cssmPos = { continuousPos.x, continuousPos.y, continuousPos.x + 100,
		continuousPos.y + 20 };
	exposureLabel.ID = id++;
	exposureLabel.Create("Exposure (ms):", WS_CHILD | WS_VISIBLE, exposureLabel.ksmPos, parent, exposureLabel.ID);
	exposureTimes.resize(4);
	for (int picInc = 0; picInc < 4; picInc++)
	{
		exposureEdits[picInc].ksmPos = { kineticPos.x + 100 + 95 * picInc, kineticPos.y,
			kineticPos.x + 100 + 95 * (picInc + 1), kineticPos.y + 20 };
		exposureEdits[picInc].amPos = { accumulatePos.x + 100 + 95 * picInc, accumulatePos.y,
			accumulatePos.x + 100 + 95 * (picInc + 1), accumulatePos.y + 20 };
		exposureEdits[picInc].cssmPos = { continuousPos.x + 100 + 95 * picInc, continuousPos.y,
			continuousPos.x + 100 + 95 * (picInc + 1), continuousPos.y + 20 };
		exposureEdits[picInc].ID = id++;
		// first of group
		exposureEdits[picInc].Create(WS_CHILD | WS_VISIBLE | WS_BORDER, exposureEdits[picInc].ksmPos, parent, exposureEdits[picInc].ID);
		exposureEdits[picInc].SetWindowTextA("20");
		exposureTimes[picInc] = 20;
	}
	kineticPos.y += 20;
	accumulatePos.y += 20;
	continuousPos.y += 20;

	/// Thresholds
	thresholdLabel.ksmPos = { kineticPos.x, kineticPos.y, kineticPos.x + 100, 
		kineticPos.y + 20 };
	thresholdLabel.amPos = { accumulatePos.x, accumulatePos.y, accumulatePos.x + 100,
		accumulatePos.y + 20 };
	thresholdLabel.cssmPos = { continuousPos.x, continuousPos.y, continuousPos.x + 100,
		continuousPos.y + 20 };
	thresholdLabel.ID = id++;
	thresholdLabel.Create("Threshold (cts)", WS_CHILD | WS_VISIBLE, thresholdLabel.ksmPos, parent, thresholdLabel.ID);
	for (int picInc = 0; picInc < 4; picInc++)
	{
		thresholdEdits[picInc].ksmPos = { kineticPos.x + 100 + 95 * picInc, kineticPos.y,
			kineticPos.x + 100 + 95 * (picInc + 1), kineticPos.y + 20 };
		thresholdEdits[picInc].amPos = { accumulatePos.x + 100 + 95 * picInc, accumulatePos.y,
			accumulatePos.x + 100 + 95 * (picInc + 1), accumulatePos.y + 20 };
		thresholdEdits[picInc].cssmPos = { continuousPos.x + 100 + 95 * picInc, continuousPos.y,
			continuousPos.x + 100 + 95 * (picInc + 1), continuousPos.y + 20 };
		thresholdEdits[picInc].ID = id++;
		// first of group
		thresholdEdits[picInc].Create(WS_CHILD | WS_VISIBLE | WS_BORDER, thresholdEdits[picInc].ksmPos, parent,
			thresholdEdits[picInc].ID);
		thresholdEdits[picInc].SetWindowTextA("100");
		this->thresholds[picInc] = 100;
	}	
	kineticPos.y += 20;
	accumulatePos.y += 20;
	continuousPos.y += 20;

	/// Yellow --> Blue Color
	yellowBlueLabel.ksmPos = { kineticPos.x, kineticPos.y, kineticPos.x + 100,
		kineticPos.y + 20 };
	yellowBlueLabel.amPos = { accumulatePos.x, accumulatePos.y, accumulatePos.x + 100,
		accumulatePos.y + 20 };
	yellowBlueLabel.cssmPos = { continuousPos.x, continuousPos.y, continuousPos.x + 100,
		continuousPos.y + 20 };
	yellowBlueLabel.ID = id++;
	yellowBlueLabel.Create("Yellow --> Blue", WS_CHILD | WS_VISIBLE, yellowBlueLabel.ksmPos, parent, yellowBlueLabel.ID);
	kineticPos.y += 20;
	accumulatePos.y += 20;
	continuousPos.y += 20;

	/// Red --> Blue color
	redBlueLabel.ksmPos = { kineticPos.x, kineticPos.y, kineticPos.x + 100, 
		kineticPos.y + 20 };
	redBlueLabel.amPos = { accumulatePos.x, accumulatePos.y, accumulatePos.x + 100,
		accumulatePos.y + 20 };
	redBlueLabel.cssmPos = { continuousPos.x, continuousPos.y, continuousPos.x + 100,
		continuousPos.y + 20 };
	redBlueLabel.ID = id++;
	redBlueLabel.Create("Red --> Blue", WS_CHILD | WS_VISIBLE, redBlueLabel.ksmPos, parent, redBlueLabel.ID);
	kineticPos.y += 20;
	accumulatePos.y += 20;
	continuousPos.y += 20;

	/// Black --> White color
	blackWhiteLabel.ksmPos = { kineticPos.x, kineticPos.y, kineticPos.x + 100,
		kineticPos.y + 20 };
	blackWhiteLabel.amPos = { accumulatePos.x, accumulatePos.y, accumulatePos.x + 100,
		accumulatePos.y + 20 };
	blackWhiteLabel.cssmPos = { continuousPos.x, continuousPos.y, continuousPos.x + 100,
		continuousPos.y + 20 };
	blackWhiteLabel.ID = id++;
	blackWhiteLabel.Create("Black --> White", WS_CHILD | WS_VISIBLE, blackWhiteLabel.ksmPos, 
		parent, blackWhiteLabel.ID);
	kineticPos.y += 20;
	accumulatePos.y += 20;
	continuousPos.y += 20;

	/// The radio buttons
	for (int picInc = 0; picInc < 4; picInc++)
	{
		kineticPos.y -= 60;
		accumulatePos.y -= 60;
		continuousPos.y -= 60;
		yellowBlueRadios[picInc].ksmPos = { kineticPos.x + 100 + 95 * picInc, kineticPos.y,
			kineticPos.x + 100 + 95 * (picInc + 1), kineticPos.y + 20 };
		yellowBlueRadios[picInc].amPos = { accumulatePos.x + 100 + 95 * picInc, accumulatePos.y,
			accumulatePos.x + 100 + 95 * (picInc + 1), accumulatePos.y + 20 };
		yellowBlueRadios[picInc].cssmPos = { continuousPos.x + 100 + 95 * picInc, continuousPos.y,
			continuousPos.x + 100 + 95 * (picInc + 1), continuousPos.y + 20 };
		this->yellowBlueRadios[picInc].ID = id++;
		yellowBlueRadios[picInc].Create("", BS_CENTER | WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | WS_GROUP,
			yellowBlueRadios[picInc].ksmPos, parent, yellowBlueRadios[picInc].ID);
		kineticPos.y += 20;
		accumulatePos.y += 20;
		continuousPos.y += 20;
		redBlueRadios[picInc].ksmPos = { kineticPos.x + 100 + 95 * picInc, kineticPos.y,
			kineticPos.x + 100 + 95 * (picInc + 1), kineticPos.y + 20 };
		redBlueRadios[picInc].amPos = { accumulatePos.x + 100 + 95 * picInc, accumulatePos.y,
			accumulatePos.x + 100 + 95 * (picInc + 1), accumulatePos.y + 20 };
		redBlueRadios[picInc].cssmPos = { continuousPos.x + 100 + 95 * picInc, continuousPos.y,
			continuousPos.x + 100 + 95 * (picInc + 1), continuousPos.y + 20 };
		redBlueRadios[picInc].ID = id++;
		redBlueRadios[picInc].Create("", BS_CENTER | WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
			redBlueRadios[picInc].ksmPos, parent, redBlueRadios[picInc].ID);
		kineticPos.y += 20;
		accumulatePos.y += 20;
		continuousPos.y += 20;
		blackWhiteRadios[picInc].ksmPos = { kineticPos.x + 100 + 95 * picInc, kineticPos.y,
			kineticPos.x + 100 + 95 * (picInc + 1), kineticPos.y + 20 };
		blackWhiteRadios[picInc].amPos = { accumulatePos.x + 100 + 95 * picInc, accumulatePos.y,
			accumulatePos.x + 100 + 95 * (picInc + 1), accumulatePos.y + 20 };
		blackWhiteRadios[picInc].cssmPos = { continuousPos.x + 100 + 95 * picInc, continuousPos.y,
			continuousPos.x + 100 + 95 * (picInc + 1), continuousPos.y + 20 };
		blackWhiteRadios[picInc].ID = id++;
		// first of group
		blackWhiteRadios[picInc].Create("", BS_CENTER | WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
			blackWhiteRadios[picInc].ksmPos, parent, blackWhiteRadios[picInc].ID);
		blackWhiteRadios[picInc].SetCheck(1);
		this->colors[picInc] = 2;
		kineticPos.y += 20;
		accumulatePos.y += 20;
		continuousPos.y += 20;
	}

	// above, the total number was set to 1.
	this->enablePictureControls(0);
	this->disablePictureControls(1);
	this->disablePictureControls(2);
	this->disablePictureControls(3);
	// should move up
	this->picsPerRepetitionUnofficial = 1;
	return true;
}

bool PictureSettingsControl::disablePictureControls(int pic)
{
	if (pic > 3)
	{
		return false;
	}
	exposureEdits[pic].EnableWindow(0);
	thresholdEdits[pic].EnableWindow(0);
	blackWhiteRadios[pic].EnableWindow(0);
	yellowBlueRadios[pic].EnableWindow(0);
	redBlueRadios[pic].EnableWindow(0);
	return true;
}

bool PictureSettingsControl::enablePictureControls(int pic)
{
	if (pic > 3)
	{
		return false;
	}
	exposureEdits[pic].EnableWindow();
	thresholdEdits[pic].EnableWindow();
	blackWhiteRadios[pic].EnableWindow();
	yellowBlueRadios[pic].EnableWindow();
	redBlueRadios[pic].EnableWindow();
	return true;
}


INT_PTR PictureSettingsControl::colorControls(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
	DWORD controlID = GetDlgCtrlID((HWND)lParam);
	HDC hdcStatic = (HDC)wParam;
	/// Exposures
	if (controlID >= exposureEdits.front().ID && controlID <= exposureEdits.back().ID)
	{
		int picNum = controlID - exposureEdits.front().ID;
		SetTextColor(hdcStatic, RGB(255, 255, 255));
		//TCHAR textEdit[256];
		CString text;
		exposureEdits[picNum].GetWindowTextA(text);
		int exposure;
		try
		{
			exposure = std::stof(std::string(text));// / 1000.0f;
			double dif = std::fabs(exposure/1000.0 - exposureTimes[picNum]);
			if (dif < 0.000000001)
			{
				// good.
				SetTextColor(hdcStatic, RGB(255, 255, 255));
				SetBkColor(hdcStatic, RGB(100, 110, 100));
				// catch change of color and redraw window.
				if (exposureEdits[picNum].colorState != 0)
				{
					exposureEdits[picNum].colorState = 0;
					exposureEdits[picNum].RedrawWindow();
				}
				//return (INT_PTR)eGreyGreenBrush;
			}
		}
		catch (std::exception&)
		{
			// don't do anything with it.
		}
		SetTextColor(hdcStatic, RGB(255, 255, 255));
		SetBkColor(hdcStatic, RGB(150, 100, 100));
		// catch change of color and redraw window.
		if (exposureEdits[picNum].colorState != 1)
		{
			exposureEdits[picNum].colorState = 1;
			exposureEdits[picNum].RedrawWindow();
		}
		//return (INT_PTR)eGreyRedBrush;
	}
	/// Thresholds
	else if (controlID >= this->thresholdEdits.front().ID && controlID <= this->thresholdEdits.back().ID)
	{
		int picNum = controlID - thresholdEdits.front().ID;
		SetTextColor(hdcStatic, RGB(255, 255, 255));
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
				SetTextColor(hdcStatic, RGB(255, 255, 255));
				SetBkColor(hdcStatic, RGB(100, 110, 100));
				// catch change of color and redraw window.
				if (thresholdEdits[picNum].colorState != 0)
				{
					thresholdEdits[picNum].colorState = 0;
					thresholdEdits[picNum].RedrawWindow();
				}
				//return (INT_PTR)eGreyGreenBrush;
			}
		}
		catch (std::exception&)
		{
			// don't do anything with it.
		}
		SetTextColor(hdcStatic, RGB(255, 255, 255));
		SetBkColor(hdcStatic, RGB(150, 100, 100));
		// catch change of color and redraw window.
		if (exposureEdits[picNum].colorState != 1)
		{
			exposureEdits[picNum].colorState = 1;
			exposureEdits[picNum].RedrawWindow();
		}
		return (INT_PTR)eGreyRedBrush;
	}
	else
	{
		return NULL;
	}
}

bool PictureSettingsControl::handleOptionChange(HWND window, UINT msg, WPARAM wParam, LPARAM lParam, AndorCamera* andorObj, Communicator* comm)
{
	int controlID = LOWORD(wParam);
	if (controlID >= this->totalNumberChoice.front().ID && controlID <= this->totalNumberChoice.back().ID)
	{
		int picNum = controlID - totalNumberChoice.front().ID;
		this->picsPerRepetitionUnofficial = picNum + 1;
		// not all settings are changed here, and some are used to recalculate totals.
		AndorRunSettings settings = andorObj->getSettings();
		settings.picsPerRepetition = this->picsPerRepetitionUnofficial;
		settings.totalPicsInVariation = settings.picsPerRepetition  * settings.repetitionsPerVariation;
		settings.totalPicsInExperiment = settings.totalVariations * settings.totalPicsInVariation;
		andorObj->setSettings(settings);
		for (int picInc = 0; picInc < 4; picInc++)
		{
			if (picInc <= picNum)
			{
				this->enablePictureControls(picInc);
			}
			else
			{
				this->disablePictureControls(picInc);
			}
		}
		return true;

	}
	else if (controlID == this->setPictureOptionsButton.ID)
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
			int exposure;
			try
			{

				exposure = std::stof(std::string(textEdit));

				exposureTimes[exposureInc] = exposure / 1000.0f;
			}
			catch (std::invalid_argument)
			{
				errBox("ERROR: failed to convert exposure number " + std::to_string(exposureInc + 1) + " to an integer.");
			}
			// refresh for new color
			exposureEdits[exposureInc].RedrawWindow();
		}
		/// set the exposure times via andor
		this->setExposureTimes(andorObj, comm);
		return true;
	}
	else if (controlID >= this->yellowBlueRadios[0].ID && controlID <= blackWhiteRadios[3].ID)
	{
		controlID -= yellowBlueRadios[0].ID;
		int pic = controlID / 3;
		int color = controlID % 3;
		this->colors[pic] = color;

	}
	else
	{
		return false;
	}
}

bool PictureSettingsControl::setExposureTimes(AndorCamera* andorObj, Communicator* comm)
{
	return this->setExposureTimes(this->exposureTimes, andorObj, comm);
}

bool PictureSettingsControl::setExposureTimes(std::vector<float> times, AndorCamera* andorObj, Communicator* comm)
{
	std::vector<float> exposuresToSet;
	exposuresToSet = times;
	exposuresToSet.resize(this->picsPerRepetitionUnofficial);
	AndorRunSettings settings = andorObj->getSettings();
	settings.exposureTimes = exposuresToSet;
	andorObj->setSettings(settings);
	// try to set this time.
	if (andorObj->setExposures(comm) < 0)
	{
		comm->sendError("ERROR: failed to set exposure times.", "", "R");
		return false;
	}

	// now check actual times.
	try { this->parent2->checkTimings(exposuresToSet, comm); }
	catch (std::runtime_error& e) { throw; }

	for (int exposureInc = 0; exposureInc < exposuresToSet.size(); exposureInc++)
	{
		this->exposureTimes[exposureInc] = exposuresToSet[exposureInc];
	}

	if (this->exposureTimes.size() <= 0)
	{
		// this shouldn't happend
		comm->sendError("ERROR: reached bad location where exposure times was of zero size, "
			"but this should have been detected earlier in the code.", "", "R");
		return false;
	}
	// now output things.
	for (int exposureInc = 0; exposureInc < 4; exposureInc++)
	{
		exposureEdits[exposureInc].SetWindowTextA(std::to_string(this->exposureTimes[exposureInc] * 1000).c_str());
	}
	/*
	SendMessage(eKineticCycleTimeDispHandle.hwnd, WM_SETTEXT, 0, 
		(LPARAM)std::to_string(eKineticCycleTime * 1000).c_str());
	SendMessage(eAccumulationTimeDisp.hwnd, WM_SETTEXT, 0, 
		(LPARAM)std::to_string(eAccumulationTime * 1000).c_str());
	eCameraFileSystem.updateSaveStatus(false);
	*/
	return true;
}

std::vector<float> PictureSettingsControl::getUsedExposureTimes()
{
	std::vector<float> usedTimes;
	usedTimes = this->exposureTimes;
	usedTimes.resize(this->picsPerRepetitionUnofficial);
	return usedTimes;
}

/*
 * modifies values for exposures, accumlation time, kinetic cycle time as the andor camera reports them.
 */
void PictureSettingsControl::confirmAcquisitionTimings(Communicator* comm)
{
	std::vector<float> usedExposures;
	usedExposures = this->exposureTimes;
	usedExposures.resize(this->picsPerRepetitionUnofficial);
	try
	{
		parent2->checkTimings(usedExposures, comm);
	}
	catch (std::runtime_error)
	{
		throw;
	}
	for (int exposureInc = 0; exposureInc < usedExposures.size(); exposureInc++)
	{
		this->exposureTimes[exposureInc] = usedExposures[exposureInc];
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
bool PictureSettingsControl::reorganizeControls(std::string cameraMode, std::string triggerMode, int width, int height)
{
	this->setPictureOptionsButton.rearrange(cameraMode, triggerMode, width, height);
	totalPicNumberLabel.rearrange(cameraMode, triggerMode, width, height);
	pictureLabel.rearrange(cameraMode, triggerMode, width, height);
	exposureLabel.rearrange(cameraMode, triggerMode, width, height);
	thresholdLabel.rearrange(cameraMode, triggerMode, width, height);
	blackWhiteLabel.rearrange(cameraMode, triggerMode, width, height);
	redBlueLabel.rearrange(cameraMode, triggerMode, width, height);
	yellowBlueLabel.rearrange(cameraMode, triggerMode, width, height);

	for (auto& control : this->pictureNumbers)
	{
		control.rearrange(cameraMode, triggerMode, width, height);
	}
	for (auto& control : this->totalNumberChoice)
	{
		control.rearrange(cameraMode, triggerMode, width, height);
	}
	for (auto& control : this->exposureEdits)
	{
		control.rearrange(cameraMode, triggerMode, width, height);
	}
	for (auto& control : this->thresholdEdits)
	{
		control.rearrange(cameraMode, triggerMode, width, height);
	}
	for (auto& control : this->redBlueRadios)
	{
		control.rearrange(cameraMode, triggerMode, width, height);
	}
	for (auto& control : this->yellowBlueRadios)
	{
		control.rearrange(cameraMode, triggerMode, width, height);
	}
	for (auto& control : this->blackWhiteRadios)
	{
		control.rearrange(cameraMode, triggerMode, width, height);
	}
	return false;
}
