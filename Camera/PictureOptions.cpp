
#include "stdafx.h"

#include "PictureOptions.h"

#include "Commctrl.h"
#include "reorganizeControl.h"
#include "myAndor.h"

bool PictureOptions::initialize(POINT& kineticPos, POINT& continuousPos, POINT& accumulatePos, HWND parent, int& id)
{
	// introducing things row by row

	setPictureOptionsButton;
	/// Set Picture Options
	RECT itemLoc;
	itemLoc = setPictureOptionsButton.kineticSeriesModePos = { kineticPos.x, kineticPos.y, kineticPos.x + 480, 
		kineticPos.y + 25 };
	setPictureOptionsButton.accumulateModePos = { accumulatePos.x, accumulatePos.y, accumulatePos.x + 480,
		accumulatePos.y + 25 };
	setPictureOptionsButton.continuousSingleScansModePos = { continuousPos.x, continuousPos.y, continuousPos.x + 480,
		continuousPos.y + 25 };
	setPictureOptionsButton.ID = id++;
	setPictureOptionsButton.hwnd = CreateWindowEx(0, "BUTTON", "Set Picture Options", WS_CHILD | WS_VISIBLE,
		itemLoc.left, itemLoc.top, itemLoc.right - itemLoc.left, itemLoc.bottom - itemLoc.top,
		parent, (HMENU)setPictureOptionsButton.ID, eHInst, NULL);
	kineticPos.y += 25;
	accumulatePos.y += 25;
	continuousPos.y += 25;
	/// Picture Numbers
	itemLoc;
	itemLoc = totalPicNumberLabel.kineticSeriesModePos = { kineticPos.x, kineticPos.y, kineticPos.x + 100, 
		kineticPos.y + 20 };
	totalPicNumberLabel.accumulateModePos = { accumulatePos.x, accumulatePos.y, accumulatePos.x + 100,
		accumulatePos.y + 20 };
	totalPicNumberLabel.continuousSingleScansModePos = { continuousPos.x, continuousPos.y, continuousPos.x + 100, 
		continuousPos.y + 20 };
	totalPicNumberLabel.ID = id++;
	totalPicNumberLabel.hwnd = CreateWindowEx(0, "STATIC", "Picture #:", WS_CHILD | WS_VISIBLE, 
								itemLoc.left, itemLoc.top, itemLoc.right - itemLoc.left, itemLoc.bottom - itemLoc.top, 
								parent, (HMENU)totalPicNumberLabel.ID, eHInst, NULL);
	for (int picInc = 0; picInc < 4; picInc++)
	{
		itemLoc = pictureNumbers[picInc].kineticSeriesModePos = { kineticPos.x + 100 + 95 * picInc, kineticPos.y,											   
			kineticPos.x + 100 + 95*(picInc + 1), kineticPos.y + 20 };
		pictureNumbers[picInc].accumulateModePos = { accumulatePos.x + 100 + 95 * picInc, accumulatePos.y,
			accumulatePos.x + 100 + 95 * (picInc + 1), accumulatePos.y + 20 };
		pictureNumbers[picInc].continuousSingleScansModePos = { continuousPos.x + 100 + 95 * picInc, continuousPos.y,
			continuousPos.x + 100 + 95 * (picInc + 1), continuousPos.y + 20 };		
		pictureNumbers[picInc].ID = id++;
		this->pictureNumbers[picInc].hwnd = CreateWindowEx(0, "STATIC", std::to_string(picInc + 1).c_str(),
							WS_CHILD | WS_VISIBLE | SS_CENTER, itemLoc.left, itemLoc.top, itemLoc.right - itemLoc.left, 
							itemLoc.bottom - itemLoc.top, parent, (HMENU)pictureNumbers[picInc].ID, eHInst, NULL);
	}
	kineticPos.y += 20;
	accumulatePos.y += 20;
	continuousPos.y += 20;

	/// Total picture number
	itemLoc = totalPicNumberLabel.kineticSeriesModePos = { kineticPos.x, kineticPos.y, kineticPos.x + 100, 
		kineticPos.y + 20 };
	totalPicNumberLabel.accumulateModePos = { accumulatePos.x, accumulatePos.y, accumulatePos.x + 100,
											  accumulatePos.y + 20 };
	totalPicNumberLabel.continuousSingleScansModePos = { continuousPos.x, continuousPos.y, continuousPos.x + 100,
														 continuousPos.y + 20 };
	totalPicNumberLabel.ID = id++;
	totalPicNumberLabel.hwnd = CreateWindowEx(0, "STATIC", "Total Picture #", WS_CHILD | WS_VISIBLE,
				itemLoc.left, itemLoc.top, itemLoc.right - itemLoc.left, itemLoc.bottom - itemLoc.top,
				parent, (HMENU)totalPicNumberLabel.ID, eHInst, NULL);
	for (int picInc = 0; picInc < 4; picInc++)
	{
		itemLoc = totalNumberChoice[picInc].kineticSeriesModePos = { kineticPos.x + 100 + 95 * picInc, kineticPos.y,
			kineticPos.x + 100 + 95 * (picInc + 1), kineticPos.y + 20 };
		totalNumberChoice[picInc].accumulateModePos = { accumulatePos.x + 100 + 95 * picInc, accumulatePos.y,
			accumulatePos.x + 100 + 95 * (picInc + 1), accumulatePos.y + 20 };
		totalNumberChoice[picInc].continuousSingleScansModePos = { continuousPos.x + 100 + 95 * picInc, continuousPos.y,
			continuousPos.x + 100 + 95 * (picInc + 1), continuousPos.y + 20 };
		totalNumberChoice[picInc].ID = id++;
		if (picInc == 0)
		{
			// first of group
			this->totalNumberChoice[picInc].hwnd = CreateWindowEx(0, "BUTTON", "",
				WS_CHILD | WS_VISIBLE | BS_CENTER | WS_GROUP | BS_AUTORADIOBUTTON, itemLoc.left, itemLoc.top, 
				itemLoc.right - itemLoc.left,
				itemLoc.bottom - itemLoc.top, parent, (HMENU)totalNumberChoice[picInc].ID, eHInst, NULL);
			SendMessage(totalNumberChoice[picInc].hwnd, BM_SETCHECK, BST_CHECKED, 0);
		}
		else
		{
			// members of group.
			this->totalNumberChoice[picInc].hwnd = CreateWindowEx(0, "BUTTON", "",
				WS_CHILD | WS_VISIBLE | BS_CENTER | BS_AUTORADIOBUTTON,
				itemLoc.left, itemLoc.top, itemLoc.right - itemLoc.left, itemLoc.bottom - itemLoc.top, 
				parent, (HMENU)totalNumberChoice[picInc].ID, eHInst, NULL);
		}
	}
	kineticPos.y += 20;
	accumulatePos.y += 20;
	continuousPos.y += 20;

	/// Exposure Times
	itemLoc = exposureLabel.kineticSeriesModePos = { kineticPos.x, kineticPos.y, kineticPos.x + 100, 
		kineticPos.y + 20 };
	exposureLabel.accumulateModePos = { accumulatePos.x, accumulatePos.y, accumulatePos.x + 100,
		accumulatePos.y + 20 };
	exposureLabel.continuousSingleScansModePos = { continuousPos.x, continuousPos.y, continuousPos.x + 100,
		continuousPos.y + 20 };
	exposureLabel.ID = id++;
	exposureLabel.hwnd = CreateWindowEx(0, "STATIC", "Exposure (ms):", WS_CHILD | WS_VISIBLE,
		itemLoc.left, itemLoc.top, itemLoc.right - itemLoc.left, itemLoc.bottom - itemLoc.top,
		parent, (HMENU)exposureLabel.ID, eHInst, NULL);
	exposureTimes.resize(4);
	for (int picInc = 0; picInc < 4; picInc++)
	{
		itemLoc = exposureEdits[picInc].kineticSeriesModePos = { kineticPos.x + 100 + 95 * picInc, kineticPos.y,
			kineticPos.x + 100 + 95 * (picInc + 1), kineticPos.y + 20 };
		exposureEdits[picInc].accumulateModePos = { accumulatePos.x + 100 + 95 * picInc, accumulatePos.y,
			accumulatePos.x + 100 + 95 * (picInc + 1), accumulatePos.y + 20 };
		exposureEdits[picInc].continuousSingleScansModePos = { continuousPos.x + 100 + 95 * picInc, continuousPos.y,
			continuousPos.x + 100 + 95 * (picInc + 1), continuousPos.y + 20 };
		exposureEdits[picInc].ID = id++;
		// first of group
		this->exposureEdits[picInc].hwnd = CreateWindowEx(0, "EDIT", "20", WS_CHILD | WS_VISIBLE | WS_BORDER,
			itemLoc.left, itemLoc.top, itemLoc.right - itemLoc.left, itemLoc.bottom - itemLoc.top, parent, 
			(HMENU)exposureEdits[picInc].ID, eHInst, NULL);
		exposureTimes[picInc] = 20;
	}
	kineticPos.y += 20;
	accumulatePos.y += 20;
	continuousPos.y += 20;

	/// Thresholds
	itemLoc = thresholdLabel.kineticSeriesModePos = { kineticPos.x, kineticPos.y, kineticPos.x + 100, 
		kineticPos.y + 20 };
	thresholdLabel.accumulateModePos = { accumulatePos.x, accumulatePos.y, accumulatePos.x + 100,
		accumulatePos.y + 20 };
	thresholdLabel.continuousSingleScansModePos = { continuousPos.x, continuousPos.y, continuousPos.x + 100,
		continuousPos.y + 20 };
	thresholdLabel.ID = id++;
	thresholdLabel.hwnd = CreateWindowEx(0, "STATIC", "Threshold (cts)", WS_CHILD | WS_VISIBLE,
		itemLoc.left, itemLoc.top, itemLoc.right - itemLoc.left, itemLoc.bottom - itemLoc.top,
		parent, (HMENU)thresholdLabel.ID, eHInst, NULL);

	for (int picInc = 0; picInc < 4; picInc++)
	{
		itemLoc = thresholdEdits[picInc].kineticSeriesModePos = { kineticPos.x + 100 + 95 * picInc, kineticPos.y,
			kineticPos.x + 100 + 95 * (picInc + 1), kineticPos.y + 20 };
		thresholdEdits[picInc].accumulateModePos = { accumulatePos.x + 100 + 95 * picInc, accumulatePos.y,
			accumulatePos.x + 100 + 95 * (picInc + 1), accumulatePos.y + 20 };
		thresholdEdits[picInc].continuousSingleScansModePos = { continuousPos.x + 100 + 95 * picInc, continuousPos.y,
			continuousPos.x + 100 + 95 * (picInc + 1), continuousPos.y + 20 };
		thresholdEdits[picInc].ID = id++;
		// first of group
		this->thresholdEdits[picInc].hwnd = CreateWindowEx(0, "EDIT", "100", WS_CHILD | WS_VISIBLE | WS_BORDER,
			itemLoc.left, itemLoc.top, itemLoc.right - itemLoc.left, itemLoc.bottom - itemLoc.top, parent, 
			(HMENU)thresholdEdits[picInc].ID, eHInst, NULL);
		this->thresholds[picInc] = 100;
	}	
	kineticPos.y += 20;
	accumulatePos.y += 20;
	continuousPos.y += 20;

	/// Yellow --> Blue Color
	itemLoc = yellowBlueLabel.kineticSeriesModePos = { kineticPos.x, kineticPos.y, kineticPos.x + 100,
		kineticPos.y + 20 };
	yellowBlueLabel.accumulateModePos = { accumulatePos.x, accumulatePos.y, accumulatePos.x + 100,
		accumulatePos.y + 20 };
	yellowBlueLabel.continuousSingleScansModePos = { continuousPos.x, continuousPos.y, continuousPos.x + 100,
		continuousPos.y + 20 };
	yellowBlueLabel.ID = id++;
	yellowBlueLabel.hwnd = CreateWindowEx(0, "STATIC", "Yellow --> Blue", WS_CHILD | WS_VISIBLE,
		itemLoc.left, itemLoc.top, itemLoc.right - itemLoc.left, itemLoc.bottom - itemLoc.top,
		parent, (HMENU)yellowBlueLabel.ID, eHInst, NULL);

	kineticPos.y += 20;
	accumulatePos.y += 20;
	continuousPos.y += 20;

	/// Red --> Blue color
	itemLoc = redBlueLabel.kineticSeriesModePos = { kineticPos.x, kineticPos.y, kineticPos.x + 100, 
		kineticPos.y + 20 };
	redBlueLabel.accumulateModePos = { accumulatePos.x, accumulatePos.y, accumulatePos.x + 100,
		accumulatePos.y + 20 };
	redBlueLabel.continuousSingleScansModePos = { continuousPos.x, continuousPos.y, continuousPos.x + 100,
		continuousPos.y + 20 };
	redBlueLabel.ID = id++;
	redBlueLabel.hwnd = CreateWindowEx(0, "STATIC", "Red --> Blue", WS_CHILD | WS_VISIBLE,
		itemLoc.left, itemLoc.top, itemLoc.right - itemLoc.left, itemLoc.bottom - itemLoc.top,
		parent, (HMENU)redBlueLabel.ID, eHInst, NULL);
	kineticPos.y += 20;
	accumulatePos.y += 20;
	continuousPos.y += 20;

	/// Black --> White color
	itemLoc = blackWhiteLabel.kineticSeriesModePos = { kineticPos.x, kineticPos.y, kineticPos.x + 100,
		kineticPos.y + 20 };
	blackWhiteLabel.accumulateModePos = { accumulatePos.x, accumulatePos.y, accumulatePos.x + 100,
		accumulatePos.y + 20 };
	blackWhiteLabel.continuousSingleScansModePos = { continuousPos.x, continuousPos.y, continuousPos.x + 100,
		continuousPos.y + 20 };
	blackWhiteLabel.ID = id++;
	blackWhiteLabel.hwnd = CreateWindowEx(0, "STATIC", "Black --> White", WS_CHILD | WS_VISIBLE,
		itemLoc.left, itemLoc.top, itemLoc.right - itemLoc.left, itemLoc.bottom - itemLoc.top,
		parent, (HMENU)blackWhiteLabel.ID, eHInst, NULL);

	kineticPos.y += 20;
	accumulatePos.y += 20;
	continuousPos.y += 20;

	/// The radio buttons
	for (int picInc = 0; picInc < 4; picInc++)
	{
		kineticPos.y -= 60;
		accumulatePos.y -= 60;
		continuousPos.y -= 60;
		itemLoc = yellowBlueRadios[picInc].kineticSeriesModePos = { kineticPos.x + 100 + 95 * picInc, kineticPos.y,
			kineticPos.x + 100 + 95 * (picInc + 1), kineticPos.y + 20 };
		yellowBlueRadios[picInc].accumulateModePos = { accumulatePos.x + 100 + 95 * picInc, accumulatePos.y,
			accumulatePos.x + 100 + 95 * (picInc + 1), accumulatePos.y + 20 };
		yellowBlueRadios[picInc].continuousSingleScansModePos = { continuousPos.x + 100 + 95 * picInc, continuousPos.y,
			continuousPos.x + 100 + 95 * (picInc + 1), continuousPos.y + 20 };
		this->yellowBlueRadios[picInc].ID = id++;
		this->yellowBlueRadios[picInc].hwnd = CreateWindowEx(0, "BUTTON", "",
			BS_CENTER | WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | WS_GROUP,
			itemLoc.left, itemLoc.top, itemLoc.right - itemLoc.left, itemLoc.bottom - itemLoc.top, parent,
			(HMENU)this->yellowBlueRadios[picInc].ID, eHInst, NULL);
		kineticPos.y += 20;
		accumulatePos.y += 20;
		continuousPos.y += 20;
		itemLoc = redBlueRadios[picInc].kineticSeriesModePos = { kineticPos.x + 100 + 95 * picInc, kineticPos.y,
			kineticPos.x + 100 + 95 * (picInc + 1), kineticPos.y + 20 };
		redBlueRadios[picInc].accumulateModePos = { accumulatePos.x + 100 + 95 * picInc, accumulatePos.y,
			accumulatePos.x + 100 + 95 * (picInc + 1), accumulatePos.y + 20 };
		redBlueRadios[picInc].continuousSingleScansModePos = { continuousPos.x + 100 + 95 * picInc, continuousPos.y,
			continuousPos.x + 100 + 95 * (picInc + 1), continuousPos.y + 20 };
		redBlueRadios[picInc].ID = id++;
		this->redBlueRadios[picInc].hwnd = CreateWindowEx(0, "BUTTON", "", 
			BS_CENTER | WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
			itemLoc.left, itemLoc.top, itemLoc.right - itemLoc.left, itemLoc.bottom - itemLoc.top, parent, 
			(HMENU)redBlueRadios[picInc].ID, eHInst, NULL);
		kineticPos.y += 20;
		accumulatePos.y += 20;
		continuousPos.y += 20;
		itemLoc = blackWhiteRadios[picInc].kineticSeriesModePos = { kineticPos.x + 100 + 95 * picInc, kineticPos.y,
			kineticPos.x + 100 + 95 * (picInc + 1), kineticPos.y + 20 };
		blackWhiteRadios[picInc].accumulateModePos = { accumulatePos.x + 100 + 95 * picInc, accumulatePos.y,
			accumulatePos.x + 100 + 95 * (picInc + 1), accumulatePos.y + 20 };
		blackWhiteRadios[picInc].continuousSingleScansModePos = { continuousPos.x + 100 + 95 * picInc, continuousPos.y,
			continuousPos.x + 100 + 95 * (picInc + 1), continuousPos.y + 20 };
		blackWhiteRadios[picInc].ID = id++;
		// first of group
		this->blackWhiteRadios[picInc].hwnd = CreateWindowEx(0, "BUTTON", "",
			BS_CENTER | WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
			itemLoc.left, itemLoc.top, itemLoc.right - itemLoc.left, itemLoc.bottom - itemLoc.top, parent,
			(HMENU)blackWhiteRadios[picInc].ID, eHInst, NULL);
		SendMessage(blackWhiteRadios[picInc].hwnd, BM_SETCHECK, BST_CHECKED, 0);
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
	this->picsPerExperiment = 1;
	return true;
}

bool PictureOptions::disablePictureControls(int pic)
{
	if (pic > 3)
	{
		return false;
	}
	EnableWindow(exposureEdits[pic].hwnd, false);
	EnableWindow(thresholdEdits[pic].hwnd, false);
	EnableWindow(blackWhiteRadios[pic].hwnd, false);
	EnableWindow(yellowBlueRadios[pic].hwnd, false);
	EnableWindow(redBlueRadios[pic].hwnd, false);
	return true;
}

bool PictureOptions::enablePictureControls(int pic)
{
	if (pic > 3)
	{
		return false;
	}
	EnableWindow(exposureEdits[pic].hwnd, true);
	EnableWindow(thresholdEdits[pic].hwnd, true);
	EnableWindow(blackWhiteRadios[pic].hwnd, true);
	EnableWindow(yellowBlueRadios[pic].hwnd, true);
	EnableWindow(redBlueRadios[pic].hwnd, true);
	return true;
}


INT_PTR PictureOptions::colorControls(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
	DWORD controlID = GetDlgCtrlID((HWND)lParam);
	HDC hdcStatic = (HDC)wParam;
	/// Exposures
	if (controlID >= exposureEdits.front().ID && controlID <= exposureEdits.back().ID)
	{
		int picNum = controlID - exposureEdits.front().ID;
		SetTextColor(hdcStatic, RGB(255, 255, 255));
		TCHAR textEdit[256];
		SendMessage(exposureEdits[picNum].hwnd, WM_GETTEXT, 256, (LPARAM)textEdit);
		int exposure;
		try
		{
			exposure = std::stof(std::string(textEdit));// / 1000.0f;
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
					RedrawWindow(exposureEdits[picNum].hwnd, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
				}
				return (INT_PTR)eGreyGreenBrush;
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
			RedrawWindow(exposureEdits[picNum].hwnd, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
		}
		return (INT_PTR)eGreyRedBrush;
	}
	/// Thresholds
	else if (controlID >= this->thresholdEdits.front().ID && controlID <= this->thresholdEdits.back().ID)
	{
		int picNum = controlID - thresholdEdits.front().ID;
		SetTextColor(hdcStatic, RGB(255, 255, 255));
		TCHAR textEdit[256];
		SendMessage(thresholdEdits[picNum].hwnd, WM_GETTEXT, 256, (LPARAM)textEdit);
		int threshold;
		try
		{
			threshold = std::stoi(std::string(textEdit));
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
					RedrawWindow(thresholdEdits[picNum].hwnd, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
				}
				return (INT_PTR)eGreyGreenBrush;
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
			RedrawWindow(exposureEdits[picNum].hwnd, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
		}
		return (INT_PTR)eGreyRedBrush;
	}
	else
	{
		return NULL;
	}
}

bool PictureOptions::handleOptionChange(HWND window, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int controlID = LOWORD(wParam);
	if (controlID >= this->totalNumberChoice.front().ID && controlID <= this->totalNumberChoice.back().ID)
	{
		int picNum = controlID - totalNumberChoice.front().ID;
		this->picsPerExperiment = picNum + 1;
		ePicturesPerRepetition = this->picsPerExperiment;
		ePicturesPerVariation = ePicturesPerRepetition * eRepetitionsPerVariation;
		eTotalNumberOfPicturesInSeries = eCurrentTotalVariationNumber * ePicturesPerVariation;
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
			TCHAR textEdit[256];
			SendMessage(thresholdEdits[thresholdInc].hwnd, WM_GETTEXT, 256, (LPARAM)textEdit);
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
			RedrawWindow(thresholdEdits[thresholdInc].hwnd, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
		}
		// grab the exposures.
		for (int exposureInc = 0; exposureInc < 4; exposureInc++)
		{
			TCHAR textEdit[256];
			SendMessage(exposureEdits[exposureInc].hwnd, WM_GETTEXT, 256, (LPARAM)textEdit);
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
			RedrawWindow(exposureEdits[exposureInc].hwnd, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
		}
		/// set the exposure times via andor
		this->setExposureTimes();
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

bool PictureOptions::setExposureTimes()
{
	return this->setExposureTimes(this->exposureTimes);
}

bool PictureOptions::setExposureTimes(std::vector<float> times)
{
	std::vector<float> exposuresToSet;
	exposuresToSet = times;
	exposuresToSet.resize(this->picsPerExperiment);

	// try to set this time.
	if (myAndor::setExposures(exposuresToSet) < 0)
	{
		appendText("ERROR: failed to set exposure times.", IDC_ERROR_EDIT);
		return false;
	}

	// now check actual times.
	try { myAndor::checkAcquisitionTimings(eKineticCycleTime, eAccumulationTime, exposuresToSet); }
	catch (std::runtime_error& e) { throw; }

	for (int exposureInc = 0; exposureInc < exposuresToSet.size(); exposureInc++)
	{
		this->exposureTimes[exposureInc] = exposuresToSet[exposureInc];
	}

	if (this->exposureTimes.size() <= 0)
	{
		// this shouldn't happend
		appendText("ERROR: reached bad location where exposure times was of zero size, but this should have been detected earlier in the code.", IDC_ERROR_EDIT);
		return false;
	}
	// now output things.
	for (int exposureInc = 0; exposureInc < 4; exposureInc++)
	{
		SendMessage(this->exposureEdits[exposureInc].hwnd, WM_SETTEXT, 0,
			(LPARAM)std::to_string(this->exposureTimes[exposureInc] * 1000).c_str());
	}
	SendMessage(eKineticCycleTimeDispHandle.hwnd, WM_SETTEXT, 0, 
		(LPARAM)std::to_string(eKineticCycleTime * 1000).c_str());
	SendMessage(eAccumulationTimeDisp.hwnd, WM_SETTEXT, 0, 
		(LPARAM)std::to_string(eAccumulationTime * 1000).c_str());
	eCameraFileSystem.updateSaveStatus(false);
	return true;
}

std::vector<float> PictureOptions::getUsedExposureTimes()
{
	std::vector<float> usedTimes;
	usedTimes = this->exposureTimes;
	usedTimes.resize(this->picsPerExperiment);
	return usedTimes;
}

/*
 * modifies values for exposures, accumlation time, kinetic cycle time as the andor camera reports them.
 */
void PictureOptions::confirmAcquisitionTimings()
{
	std::vector<float> usedExposures;
	usedExposures = this->exposureTimes;
	usedExposures.resize(this->picsPerExperiment);
	try
	{
		myAndor::checkAcquisitionTimings(eKineticCycleTime, eAccumulationTime, usedExposures);
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
std::array<int, 4> PictureOptions::getThresholds()
{
	return this->thresholds;
}

void PictureOptions::setThresholds(std::array<int, 4> newThresholds)
{
	this->thresholds = newThresholds;
	for (int thresholdInc = 0; thresholdInc < thresholds.size(); thresholdInc++)
	{
		SendMessage(thresholdEdits[thresholdInc].hwnd, WM_SETTEXT, 0, 
			(LPARAM)std::to_string(thresholds[thresholdInc]).c_str());
	}
	return;
}

void PictureOptions::setPicturesPerExperiment(unsigned int pics)
{
	if (pics > 4)
	{
		return;
	}
	this->picsPerExperiment = pics;
	ePicturesPerRepetition = this->picsPerExperiment;
	ePicturesPerVariation = ePicturesPerRepetition * eRepetitionsPerVariation;
	eTotalNumberOfPicturesInSeries = eCurrentTotalVariationNumber * ePicturesPerVariation;
	//SendMessage(this->totalNumberChoice[pics].hwnd, BM_SETCHECtK, BST_CHECKED, 0);
	for (int picInc = 0; picInc < 4; picInc++)
	{
		if (picInc == pics - 1)
		{
			SendMessage(this->totalNumberChoice[picInc].hwnd, BM_SETCHECK, BST_CHECKED, 0);
		}
		else
		{
			SendMessage(this->totalNumberChoice[picInc].hwnd, BM_SETCHECK, BST_UNCHECKED, 0);
		}

		if (picInc < picsPerExperiment)
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
std::array<int, 4> PictureOptions::getPictureColors()
{
	return this->colors;
}

/*

*/
bool PictureOptions::reorganizeControls(RECT parentRectangle, std::string cameraMode)
{
	reorganizeControl(setPictureOptionsButton, cameraMode, parentRectangle);
	reorganizeControl(totalPicNumberLabel, cameraMode, parentRectangle);
	reorganizeControl(pictureLabel, cameraMode, parentRectangle);
	reorganizeControl(exposureLabel, cameraMode, parentRectangle);
	reorganizeControl(thresholdLabel, cameraMode, parentRectangle);
	reorganizeControl(blackWhiteLabel, cameraMode, parentRectangle);
	reorganizeControl(redBlueLabel, cameraMode, parentRectangle);
	reorganizeControl(yellowBlueLabel, cameraMode, parentRectangle);
	for (auto control : this->pictureNumbers)
	{
		reorganizeControl(control, cameraMode, parentRectangle);
	}
	for (auto control : this->totalNumberChoice)
	{
		reorganizeControl(control, cameraMode, parentRectangle);
	}
	for (auto control : this->exposureEdits)
	{
		reorganizeControl(control, cameraMode, parentRectangle);
	}
	for (auto control : this->thresholdEdits)
	{
		reorganizeControl(control, cameraMode, parentRectangle);
	}
	for (auto control : this->redBlueRadios)
	{
		reorganizeControl(control, cameraMode, parentRectangle);
	}
	for (auto control : this->yellowBlueRadios)
	{
		reorganizeControl(control, cameraMode, parentRectangle);
	}
	for (auto control : this->blackWhiteRadios)
	{
		reorganizeControl(control, cameraMode, parentRectangle);
	}
	return false;
}


