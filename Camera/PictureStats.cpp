#include "stdafx.h"
#include "PictureStats.h"
#include "reorganizeControl.h"

// as of right now, the position of this control is not affected by the mode or the trigger mode.
bool PictureStats::initialize(POINT& kineticPos, POINT& accumulatePos, POINT& continuousPos, HWND parentWindow)
{
	RECT initPos;
	this->pictureStatsHeader.kineticSeriesModePos = { kineticPos.x, kineticPos.y, kineticPos.x + 272, kineticPos.y + 25 };
	this->pictureStatsHeader.continuousSingleScansModePos = { continuousPos.x, continuousPos.y, continuousPos.x + 272, continuousPos.y + 25 };
	this->pictureStatsHeader.accumulateModePos = { accumulatePos.x, accumulatePos.y, accumulatePos.x + 272, accumulatePos.y + 25 };
	initPos = pictureStatsHeader.kineticSeriesModePos;
	pictureStatsHeader.hwnd = CreateWindowEx(0, "EDIT", "Raw Counts",
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY | ES_CENTER,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, parentWindow,
		(HMENU)IDC_ACCUMULATION_NUM_DISP, eHInst, NULL);
	pictureStatsHeader.fontType = "Heading";
	kineticPos.y += 25;
	continuousPos.y += 25;
	accumulatePos.y += 25;
	/// CURRENT IMAGE DATA
	// Current Accumulation Number Display
	this->repetitionIndicator.kineticSeriesModePos = {kineticPos.x, kineticPos.y, kineticPos.x + 272, kineticPos.y + 25};
	this->repetitionIndicator.continuousSingleScansModePos = { continuousPos.x, continuousPos.y, continuousPos.x + 272, continuousPos.y + 25 };
	this->repetitionIndicator.accumulateModePos  = { accumulatePos.x, accumulatePos.y, accumulatePos.x + 272, accumulatePos.y + 25 };
													 
	initPos = repetitionIndicator.kineticSeriesModePos;
	repetitionIndicator.hwnd = CreateWindowEx(0, "EDIT", "Repetition ?/?", 
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, parentWindow, 
		(HMENU)IDC_ACCUMULATION_NUM_DISP, eHInst, NULL);
	repetitionIndicator.fontType = "Normal";
	kineticPos.y += 25;
	continuousPos.y += 25;
	accumulatePos.y += 25;
	/// Picture labels ////////////////////////////////////////////////////////////
	
	//ePictureText
	this->collumnHeaders[0].kineticSeriesModePos = { kineticPos.x, kineticPos.y, kineticPos.x + 54, kineticPos.y + 25 };
	collumnHeaders[0].continuousSingleScansModePos = { continuousPos.x, continuousPos.y, continuousPos.x + 54, continuousPos.y + 25 };
	collumnHeaders[0].accumulateModePos = { accumulatePos.x, accumulatePos.y, accumulatePos.x + 54, accumulatePos.y + 25 };
	initPos = collumnHeaders[0].kineticSeriesModePos;
	collumnHeaders[0].hwnd = CreateWindowEx(0, "STATIC", "Pic:", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, parentWindow, 
		(HMENU)IDC_PICTURE_DISP, eHInst, NULL);
	collumnHeaders[0].fontType = "Small";
	kineticPos.y += 25;
	continuousPos.y += 25;
	accumulatePos.y += 25;

	// #1
	this->picNumberIndicators[0].kineticSeriesModePos = { kineticPos.x, kineticPos.y, kineticPos.x + 54, kineticPos.y + 25};
	picNumberIndicators[0].continuousSingleScansModePos = { continuousPos.x, continuousPos.y, continuousPos.x + 54, continuousPos.y + 25 };
	picNumberIndicators[0].accumulateModePos = { accumulatePos.x, accumulatePos.y, accumulatePos.x + 54, accumulatePos.y + 25 };

	initPos = picNumberIndicators[0].kineticSeriesModePos;
	picNumberIndicators[0].hwnd = CreateWindowEx(0, "STATIC", "#1:", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, parentWindow, 
		(HMENU)IDC_PICTURE_1_DISP, eHInst, NULL);
	picNumberIndicators[0].fontType = "Small";
	kineticPos.y += 25;
	continuousPos.y += 25;
	accumulatePos.y += 25;
	// #2
	this->picNumberIndicators[1].kineticSeriesModePos = { kineticPos.x, kineticPos.y, kineticPos.x + 54, kineticPos.y + 25 };
	this->picNumberIndicators[1].continuousSingleScansModePos = { continuousPos.x, continuousPos.y, continuousPos.x + 54, continuousPos.y + 25 };
	this->picNumberIndicators[1].accumulateModePos = { accumulatePos.x, accumulatePos.y, accumulatePos.x + 54, accumulatePos.y + 25 };

	initPos = picNumberIndicators[1].kineticSeriesModePos;
	picNumberIndicators[1].hwnd = CreateWindowEx(0, "STATIC", "#2:", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, parentWindow, 
		(HMENU)IDC_PICTURE_2_DISP, eHInst, NULL);
	picNumberIndicators[1].fontType = "Small";
	kineticPos.y += 25;
	continuousPos.y += 25;
	accumulatePos.y += 25;
	// #3
	this->picNumberIndicators[2].kineticSeriesModePos = { kineticPos.x, kineticPos.y, kineticPos.x + 54, kineticPos.y + 25 };
	this->picNumberIndicators[2].continuousSingleScansModePos = { continuousPos.x, continuousPos.y, continuousPos.x + 54, continuousPos.y + 25 };
	picNumberIndicators[2].accumulateModePos = { accumulatePos.x, accumulatePos.y, accumulatePos.x + 54, accumulatePos.y + 25 };
	initPos = picNumberIndicators[2].kineticSeriesModePos;
	picNumberIndicators[2].hwnd = CreateWindowEx(0, "STATIC", "#3:", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, parentWindow, 
		(HMENU)IDC_PICTURE_3_DISP, eHInst, NULL);
	picNumberIndicators[2].fontType = "Small";
	// reset to top
	kineticPos.y += 25;
	continuousPos.y += 25;
	accumulatePos.y += 25;

	// #4
	this->picNumberIndicators[3].kineticSeriesModePos = { kineticPos.x, kineticPos.y, kineticPos.x + 54, kineticPos.y + 25 };
	picNumberIndicators[3].continuousSingleScansModePos = { continuousPos.x, continuousPos.y, continuousPos.x + 54, continuousPos.y + 25 };
	picNumberIndicators[3].accumulateModePos = { accumulatePos.x, accumulatePos.y, accumulatePos.x + 54, accumulatePos.y + 25 };
	initPos = picNumberIndicators[3].kineticSeriesModePos;
	picNumberIndicators[3].hwnd = CreateWindowEx(0, "STATIC", "#4:", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, parentWindow, 
		(HMENU)IDC_PICTURE_4_DISP, eHInst, NULL);
	picNumberIndicators[3].fontType = "Small";
	kineticPos.y -= 100;
	continuousPos.y -= 100;
	accumulatePos.y -= 100;
	/// Max Count Edits
	// Max Count Display 742 - 480 )/2 = 131 
	this->collumnHeaders[1].kineticSeriesModePos = { kineticPos.x + 54, kineticPos.y, kineticPos.x + 108, kineticPos.y + 25};
	this->collumnHeaders[1].continuousSingleScansModePos = { continuousPos.x + 54, continuousPos.y, continuousPos.x + 108, continuousPos.y + 25 };
	this->collumnHeaders[1].accumulateModePos = { accumulatePos.x + 54, accumulatePos.y, accumulatePos.x + 108, accumulatePos.y + 25 };

	initPos = collumnHeaders[1].kineticSeriesModePos;
	collumnHeaders[1].hwnd = CreateWindowEx(0, "STATIC", "Max:", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, parentWindow, 
		(HMENU)IDC_MAX_COUNT_DISP, eHInst, NULL);
	collumnHeaders[1].fontType = "Small";
	kineticPos.y += 25;
	continuousPos.y += 25;
	accumulatePos.y += 25;
	// #1
	this->maxCounts[0].kineticSeriesModePos = { kineticPos.x + 54, kineticPos.y, kineticPos.x + 108, kineticPos.y + 25 };
	this->maxCounts[0].continuousSingleScansModePos = { continuousPos.x + 54, continuousPos.y, continuousPos.x + 108, continuousPos.y + 25 };
	this->maxCounts[0].accumulateModePos = { accumulatePos.x + 54, accumulatePos.y, accumulatePos.x + 108, accumulatePos.y + 25 };
	initPos = maxCounts[0].kineticSeriesModePos;
	maxCounts[0].hwnd = CreateWindowEx(0, "EDIT", "-", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, parentWindow, 
		(HMENU)IDC_MAX_COUNT_1_DISP, eHInst, NULL);
	maxCounts[0].fontType = "Small";
	kineticPos.y += 25;
	continuousPos.y += 25;
	accumulatePos.y += 25;
	// #2
	this->maxCounts[1].kineticSeriesModePos = { kineticPos.x + 54, kineticPos.y, kineticPos.x + 108, kineticPos.y + 25 };
	this->maxCounts[1].continuousSingleScansModePos = { continuousPos.x + 54, continuousPos.y, continuousPos.x + 108, continuousPos.y + 25 };
	this->maxCounts[1].accumulateModePos = { accumulatePos.x + 54, accumulatePos.y, accumulatePos.x + 108, accumulatePos.y + 25 };
	initPos = maxCounts[1].kineticSeriesModePos;
	maxCounts[1].hwnd = CreateWindowEx(0, "EDIT", "-", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, parentWindow, 
		(HMENU)IDC_MAX_COUNT_2_DISP, eHInst, NULL);
	maxCounts[1].fontType = "Small";
	kineticPos.y += 25;
	continuousPos.y += 25;
	accumulatePos.y += 25;
	// #3
	this->maxCounts[2].kineticSeriesModePos = { kineticPos.x + 54, kineticPos.y, kineticPos.x + 108, kineticPos.y + 25 };
	this->maxCounts[2].continuousSingleScansModePos = { continuousPos.x + 54, continuousPos.y, continuousPos.x + 108, continuousPos.y + 25 };
	this->maxCounts[2].accumulateModePos = { accumulatePos.x + 54, accumulatePos.y, accumulatePos.x + 108, accumulatePos.y + 25 };	initPos = maxCounts[2].kineticSeriesModePos;
	maxCounts[2].hwnd = CreateWindowEx(0, "EDIT", "-", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, parentWindow, 
		(HMENU)IDC_MAX_COUNT_3_DISP, eHInst, NULL);
	maxCounts[2].fontType = "Small";
	kineticPos.y += 25;
	continuousPos.y += 25;
	accumulatePos.y += 25;
	// #4
	this->maxCounts[3].kineticSeriesModePos = { kineticPos.x + 54, kineticPos.y, kineticPos.x + 108, kineticPos.y + 25 };
	this->maxCounts[3].continuousSingleScansModePos = { continuousPos.x + 54, continuousPos.y, continuousPos.x + 108, continuousPos.y + 25 };
	this->maxCounts[3].accumulateModePos = { accumulatePos.x + 54, accumulatePos.y, accumulatePos.x + 108, accumulatePos.y + 25 };	initPos = maxCounts[3].kineticSeriesModePos;
	maxCounts[3].hwnd = CreateWindowEx(0, "EDIT", "-", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, parentWindow, 
		(HMENU)IDC_MAX_COUNT_4_DISP, eHInst, NULL);
	maxCounts[3].fontType = "Small";
	// back to top.
	kineticPos.y -= 100;
	continuousPos.y -= 100;
	accumulatePos.y -= 100;
	/// Min Counts
	// Min Count Display
	this->collumnHeaders[2].kineticSeriesModePos = { kineticPos.x + 108, kineticPos.y, kineticPos.x + 162, kineticPos.y + 25 };
	this->collumnHeaders[2].continuousSingleScansModePos = { continuousPos.x + 108, continuousPos.y, continuousPos.x + 162, continuousPos.y + 25 };
	this->collumnHeaders[2].accumulateModePos = { accumulatePos.x + 108, accumulatePos.y, accumulatePos.x + 162, accumulatePos.y + 25 };												   
	initPos = collumnHeaders[2].kineticSeriesModePos;
	collumnHeaders[2].hwnd = CreateWindowEx(0, "STATIC", "Min:", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, parentWindow, 
		(HMENU)IDC_MIN_COUNT_DISP, eHInst, NULL);
	collumnHeaders[2].fontType = "Small";
	kineticPos.y += 25;
	continuousPos.y += 25;
	accumulatePos.y += 25;
	// #1
	this->minCounts[0].kineticSeriesModePos = { kineticPos.x + 108, kineticPos.y, kineticPos.x + 162, kineticPos.y + 25 };
	this->minCounts[0].continuousSingleScansModePos = { continuousPos.x + 108, continuousPos.y, continuousPos.x + 162, continuousPos.y + 25 };
	this->minCounts[0].accumulateModePos = { accumulatePos.x + 108, accumulatePos.y, accumulatePos.x + 162, accumulatePos.y + 25 };

	initPos = minCounts[0].kineticSeriesModePos;
	minCounts[0].hwnd = CreateWindowEx(0, "STATIC", "-", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, parentWindow, 
		(HMENU)IDC_MIN_COUNT_1_DISP, eHInst, NULL);
	minCounts[0].fontType = "Small";
	kineticPos.y += 25;
	continuousPos.y += 25;
	accumulatePos.y += 25;
	// #2
	this->minCounts[1].kineticSeriesModePos = { kineticPos.x + 108, kineticPos.y, kineticPos.x + 162, kineticPos.y + 25 };
	this->minCounts[1].continuousSingleScansModePos = { continuousPos.x + 108, continuousPos.y, continuousPos.x + 162, continuousPos.y + 25 };
	this->minCounts[1].accumulateModePos = { accumulatePos.x + 108, accumulatePos.y, accumulatePos.x + 162, accumulatePos.y + 25 };	initPos = minCounts[1].kineticSeriesModePos;
	minCounts[1].hwnd = CreateWindowEx(0, "STATIC", "-", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, parentWindow,
		(HMENU)IDC_MIN_COUNT_2_DISP, eHInst, NULL);
	minCounts[1].fontType = "Small";
	kineticPos.y += 25;
	continuousPos.y += 25;
	accumulatePos.y += 25;
	// #3
	this->minCounts[2].kineticSeriesModePos = { kineticPos.x + 108, kineticPos.y, kineticPos.x + 162, kineticPos.y + 25 };
	this->minCounts[2].continuousSingleScansModePos = { continuousPos.x + 108, continuousPos.y, continuousPos.x + 162, continuousPos.y + 25 };
	this->minCounts[2].accumulateModePos = { accumulatePos.x + 108, accumulatePos.y, accumulatePos.x + 162, accumulatePos.y + 25 };	initPos = minCounts[2].kineticSeriesModePos;
	minCounts[2].hwnd = CreateWindowEx(0, "STATIC", "-", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, parentWindow, 
		(HMENU)IDC_MIN_COUNT_3_DISP, eHInst, NULL);
	minCounts[2].fontType = "Small";
	kineticPos.y += 25;
	continuousPos.y += 25;
	accumulatePos.y += 25;
	// #4
	this->minCounts[3].kineticSeriesModePos = { kineticPos.x + 108, kineticPos.y, kineticPos.x + 162, kineticPos.y + 25 };
	this->minCounts[3].continuousSingleScansModePos = { continuousPos.x + 108, continuousPos.y, continuousPos.x + 162, continuousPos.y + 25 };
	this->minCounts[3].accumulateModePos = { accumulatePos.x + 108, accumulatePos.y, accumulatePos.x + 162, accumulatePos.y + 25 };
	initPos = minCounts[3].kineticSeriesModePos;
	minCounts[3].hwnd = CreateWindowEx(0, "STATIC", "-", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, parentWindow, 
		(HMENU)IDC_MIN_COUNT_4_DISP, eHInst, NULL);
	minCounts[3].fontType = "Small";
	kineticPos.y -= 100;
	continuousPos.y -= 100;
	accumulatePos.y -= 100;
	/// Average Counts
	this->collumnHeaders[3].kineticSeriesModePos = { kineticPos.x + 162, kineticPos.y, kineticPos.x + 216, kineticPos.y + 25 };
	collumnHeaders[3].continuousSingleScansModePos = { continuousPos.x + 162, continuousPos.y, continuousPos.x + 216, continuousPos.y + 25 };
	collumnHeaders[3].accumulateModePos = { accumulatePos.x + 162, accumulatePos.y, accumulatePos.x + 216, accumulatePos.y + 25 };
	initPos = collumnHeaders[3].kineticSeriesModePos;
	collumnHeaders[3].hwnd = CreateWindowEx(0, "STATIC", "Avg:", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, parentWindow,
		(HMENU)IDC_SELECTION_DISP, eHInst, NULL);
	collumnHeaders[3].fontType = "Small";
	kineticPos.y += 25;
	continuousPos.y += 25;
	accumulatePos.y += 25;
	// #1
	this->avgCounts[0].kineticSeriesModePos = { kineticPos.x + 162, kineticPos.y, kineticPos.x + 216, kineticPos.y + 25 };
	this->avgCounts[0].continuousSingleScansModePos = { continuousPos.x + 162, continuousPos.y, continuousPos.x + 216, continuousPos.y + 25 };
	this->avgCounts[0].accumulateModePos = { accumulatePos.x + 162, accumulatePos.y, accumulatePos.x + 216, accumulatePos.y + 25 };

	initPos = avgCounts[0].kineticSeriesModePos;
	avgCounts[0].hwnd = CreateWindowEx(0, "STATIC", "-", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, parentWindow,
		(HMENU)IDC_SELECTION_1_DISP, eHInst, NULL);
	avgCounts[0].fontType = "Small";
	kineticPos.y += 25;
	continuousPos.y += 25;
	accumulatePos.y += 25;
	// #2
	this->avgCounts[1].kineticSeriesModePos = { kineticPos.x + 162, kineticPos.y, kineticPos.x + 216, kineticPos.y + 25 };
	this->avgCounts[1].continuousSingleScansModePos = { continuousPos.x + 162, continuousPos.y, continuousPos.x + 216, continuousPos.y + 25 };
	this->avgCounts[1].accumulateModePos = { accumulatePos.x + 162, accumulatePos.y, accumulatePos.x + 216, accumulatePos.y + 25 };
	initPos = avgCounts[1].kineticSeriesModePos;
	avgCounts[1].hwnd = CreateWindowEx(0, "STATIC", "-", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, parentWindow,
		(HMENU)IDC_SELECTION_2_DISP, eHInst, NULL);
	avgCounts[1].fontType = "Small";
	kineticPos.y += 25;
	continuousPos.y += 25;
	accumulatePos.y += 25;
	// #3
	this->avgCounts[2].kineticSeriesModePos = { kineticPos.x + 162, kineticPos.y, kineticPos.x + 216, kineticPos.y + 25 };
	this->avgCounts[2].continuousSingleScansModePos = { continuousPos.x + 162, continuousPos.y, continuousPos.x + 216, continuousPos.y + 25 };
	this->avgCounts[2].accumulateModePos = { accumulatePos.x + 162, accumulatePos.y, accumulatePos.x + 216, accumulatePos.y + 25 };
	initPos = avgCounts[2].kineticSeriesModePos;
	avgCounts[2].hwnd = CreateWindowEx(0, "STATIC", "-", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, parentWindow,
		(HMENU)IDC_SELECTION_3_DISP, eHInst, NULL);
	avgCounts[2].fontType = "Small";
	kineticPos.y += 25;
	continuousPos.y += 25;
	accumulatePos.y += 25;
	// #4
	this->avgCounts[3].kineticSeriesModePos = { kineticPos.x + 162, kineticPos.y, kineticPos.x + 216, kineticPos.y + 25 };
	this->avgCounts[3].continuousSingleScansModePos = { continuousPos.x + 162, continuousPos.y, continuousPos.x + 216, continuousPos.y + 25 };
	this->avgCounts[3].accumulateModePos = { accumulatePos.x + 162, accumulatePos.y, accumulatePos.x + 216, accumulatePos.y + 25 };
	initPos = avgCounts[3].kineticSeriesModePos;
	avgCounts[3].hwnd = CreateWindowEx(0, "STATIC", "-", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, parentWindow,
		(HMENU)IDC_SELECTION_4_DISP, eHInst, NULL);
	avgCounts[3].fontType = "Small";
	kineticPos.y -= 100;
	continuousPos.y -= 100;
	accumulatePos.y -= 100;
	/// Selection Counts
	this->collumnHeaders[4].kineticSeriesModePos = { kineticPos.x + 216, kineticPos.y, kineticPos.x + 272, kineticPos.y + 25 };
	collumnHeaders[4].continuousSingleScansModePos = { continuousPos.x + 216, continuousPos.y, continuousPos.x + 272, continuousPos.y + 25 };
	collumnHeaders[4].accumulateModePos = { accumulatePos.x + 216, accumulatePos.y, accumulatePos.x + 272, accumulatePos.y + 25 };
	initPos = collumnHeaders[4].kineticSeriesModePos;
	collumnHeaders[4].hwnd = CreateWindowEx(0, "STATIC", "Sel:", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, parentWindow, 
		(HMENU)IDC_SELECTION_DISP, eHInst, NULL);
	collumnHeaders[4].fontType = "Small";
	kineticPos.y += 25;
	continuousPos.y += 25;
	accumulatePos.y += 25;
	// #1
	this->selCounts[0].kineticSeriesModePos = { kineticPos.x + 216, kineticPos.y, kineticPos.x + 272, kineticPos.y + 25 };
	this->selCounts[0].continuousSingleScansModePos = { continuousPos.x + 216, continuousPos.y, continuousPos.x + 272, continuousPos.y + 25 };
	this->selCounts[0].accumulateModePos = { accumulatePos.x + 216, accumulatePos.y, accumulatePos.x + 272, accumulatePos.y + 25 };

	initPos = selCounts[0].kineticSeriesModePos;
	selCounts[0].hwnd = CreateWindowEx(0, "STATIC", "-", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, parentWindow, 
		(HMENU)IDC_SELECTION_1_DISP, eHInst, NULL);
	selCounts[0].fontType = "Small";
	kineticPos.y += 25;
	continuousPos.y += 25;
	accumulatePos.y += 25;
	// #2
	this->selCounts[1].kineticSeriesModePos = { kineticPos.x + 216, kineticPos.y, kineticPos.x + 272, kineticPos.y + 25 };
	this->selCounts[1].continuousSingleScansModePos = { continuousPos.x + 216, continuousPos.y, continuousPos.x + 272, continuousPos.y + 25 };
	this->selCounts[1].accumulateModePos = { accumulatePos.x + 216, accumulatePos.y, accumulatePos.x + 272, accumulatePos.y + 25 };
	initPos = selCounts[1].kineticSeriesModePos;
	selCounts[1].hwnd = CreateWindowEx(0, "STATIC", "-", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, parentWindow, 
		(HMENU)IDC_SELECTION_2_DISP, eHInst, NULL);
	selCounts[1].fontType = "Small";
	kineticPos.y += 25;
	continuousPos.y += 25;
	accumulatePos.y += 25;
	// #3
	this->selCounts[2].kineticSeriesModePos = { kineticPos.x + 216, kineticPos.y, kineticPos.x + 272, kineticPos.y + 25 };
	this->selCounts[2].continuousSingleScansModePos = { continuousPos.x + 216, continuousPos.y, continuousPos.x + 272, continuousPos.y + 25 };
	this->selCounts[2].accumulateModePos = { accumulatePos.x + 216, accumulatePos.y, accumulatePos.x + 272, accumulatePos.y + 25 };	
	initPos = selCounts[2].kineticSeriesModePos;
	selCounts[2].hwnd = CreateWindowEx(0, "STATIC", "-", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, parentWindow, 
		(HMENU)IDC_SELECTION_3_DISP, eHInst, NULL);
	selCounts[2].fontType = "Small";
	kineticPos.y += 25;
	continuousPos.y += 25;
	accumulatePos.y += 25;
	// #4
	this->selCounts[3].kineticSeriesModePos = { kineticPos.x + 216, kineticPos.y, kineticPos.x + 272, kineticPos.y + 25 };
	this->selCounts[3].continuousSingleScansModePos = { continuousPos.x + 216, continuousPos.y, continuousPos.x + 272, continuousPos.y + 25 };
	this->selCounts[3].accumulateModePos = { accumulatePos.x + 216, accumulatePos.y, accumulatePos.x + 272, accumulatePos.y + 25 };	
	initPos = selCounts[3].kineticSeriesModePos;
	selCounts[3].hwnd = CreateWindowEx(0, "STATIC", "-", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, parentWindow, 
		(HMENU)IDC_SELECTION_4_DISP, eHInst, NULL);
	selCounts[3].fontType = "Small";
	kineticPos.y += 25;
	continuousPos.y += 25;
	accumulatePos.y += 25;
	return true;
}

bool PictureStats::reorganizeControls(RECT parentRectangle, std::string mode)
{
	reorganizeControl(this->pictureStatsHeader, mode, parentRectangle);
	reorganizeControl(this->repetitionIndicator, mode, parentRectangle);
	for (auto control : this->collumnHeaders)
	{
		reorganizeControl(control, mode, parentRectangle);
	}
	for (auto control : this->maxCounts)
	{
		reorganizeControl(control, mode, parentRectangle);
	}
	for (auto control : this->minCounts)
	{
		reorganizeControl(control, mode, parentRectangle);
	}
	for (auto control : this->picNumberIndicators)
	{
		reorganizeControl(control, mode, parentRectangle);
	}
	for (auto control : this->selCounts)
	{
		reorganizeControl(control, mode, parentRectangle);
	}
	for (auto control : this->avgCounts)
	{
		reorganizeControl(control, mode, parentRectangle);
	}
	return true;
}
bool PictureStats::reset()
{
	for (auto control : this->maxCounts)
	{
		SendMessage(control.hwnd, WM_SETTEXT, 0, (LPARAM)("-"));
	}
	for (auto control : this->minCounts)
	{
		SendMessage(control.hwnd, WM_SETTEXT, 0, (LPARAM)("-"));
	}
	for (auto control : this->avgCounts)
	{
		SendMessage(control.hwnd, WM_SETTEXT, 0, (LPARAM)("-"));
	}
	for (auto control : this->selCounts)
	{
		SendMessage(control.hwnd, WM_SETTEXT, 0, (LPARAM)("-"));
	}
	return true;
}

bool PictureStats::updateType(std::string typeText)
{
	SendMessage(this->pictureStatsHeader.hwnd, WM_SETTEXT, 0, (LPARAM)(typeText.c_str()));
	return true;
}


bool PictureStats::update(unsigned long selCounts, unsigned long maxCounts, unsigned long minCounts, double avgCounts, unsigned int image)
{
	if (eDataType == RAW_COUNTS)
	{
		SendMessage(this->maxCounts[image].hwnd, WM_SETTEXT, 0, (LPARAM)(std::to_string(maxCounts)).c_str());
		SendMessage(this->minCounts[image].hwnd, WM_SETTEXT, 0, (LPARAM)(std::to_string(minCounts)).c_str());
		SendMessage(this->selCounts[image].hwnd, WM_SETTEXT, 0, (LPARAM)(std::to_string(selCounts)).c_str());
		SendMessage(this->avgCounts[image].hwnd, WM_SETTEXT, 0, (LPARAM)(doubleToString(avgCounts, 1)).c_str());
	}
	else if (eDataType == CAMERA_PHOTONS)
	{
		double selPhotons, maxPhotons, minPhotons, avgPhotons;
		if (eEMGainMode)
		{
			selPhotons = (selCounts - convs.EMGain200BackgroundCount) * this->convs.countToCameraPhotonEM200;
			maxPhotons = (maxCounts - convs.EMGain200BackgroundCount) * this->convs.countToCameraPhotonEM200;
			minPhotons = (minCounts - convs.EMGain200BackgroundCount) * this->convs.countToCameraPhotonEM200;
			avgPhotons = (avgCounts - convs.EMGain200BackgroundCount) * this->convs.countToCameraPhotonEM200;
		}
		else
		{
			selPhotons = (selCounts - convs.conventionalBackgroundCount) * this->convs.countToCameraPhoton;
			maxPhotons = (maxCounts - convs.conventionalBackgroundCount) * this->convs.countToCameraPhoton;
			minPhotons = (minCounts - convs.conventionalBackgroundCount) * this->convs.countToCameraPhoton;
			avgPhotons = (avgCounts - convs.conventionalBackgroundCount) * this->convs.countToCameraPhoton;
		}
		SendMessage(this->maxCounts[image].hwnd, WM_SETTEXT, 0, (LPARAM)(doubleToString(maxPhotons, 1)).c_str());
		SendMessage(this->minCounts[image].hwnd, WM_SETTEXT, 0, (LPARAM)(doubleToString(minPhotons, 1)).c_str());
		SendMessage(this->selCounts[image].hwnd, WM_SETTEXT, 0, (LPARAM)(doubleToString(selPhotons, 1)).c_str());
		SendMessage(this->avgCounts[image].hwnd, WM_SETTEXT, 0, (LPARAM)(doubleToString(avgPhotons, 1)).c_str());
	}
	else if (eDataType == ATOM_PHOTONS)
	{
		double selPhotons, maxPhotons, minPhotons, avgPhotons;
		if (eEMGainMode)
		{
			selPhotons = (selCounts - convs.EMGain200BackgroundCount) * this->convs.countToScatteredPhotonEM200;
			maxPhotons = (maxCounts - convs.EMGain200BackgroundCount) * this->convs.countToScatteredPhotonEM200;
			minPhotons = (minCounts - convs.EMGain200BackgroundCount) * this->convs.countToScatteredPhotonEM200;
			avgPhotons = (avgCounts - convs.EMGain200BackgroundCount) * this->convs.countToScatteredPhotonEM200;
		}
		else
		{
			selPhotons = (selCounts - convs.conventionalBackgroundCount) * this->convs.countToScatteredPhoton;
			maxPhotons = (maxCounts - convs.conventionalBackgroundCount) * this->convs.countToScatteredPhoton;
			minPhotons = (minCounts - convs.conventionalBackgroundCount) * this->convs.countToScatteredPhoton;
			avgPhotons = (avgCounts - convs.conventionalBackgroundCount) * this->convs.countToScatteredPhoton;
		}
		SendMessage(this->maxCounts[image].hwnd, WM_SETTEXT, 0, (LPARAM)(doubleToString(maxPhotons, 1)).c_str());
		SendMessage(this->minCounts[image].hwnd, WM_SETTEXT, 0, (LPARAM)(doubleToString(minPhotons, 1)).c_str());
		SendMessage(this->selCounts[image].hwnd, WM_SETTEXT, 0, (LPARAM)(doubleToString(selPhotons, 1)).c_str());
		SendMessage(this->avgCounts[image].hwnd, WM_SETTEXT, 0, (LPARAM)(doubleToString(avgPhotons, 1)).c_str());
	}
	return true;
}