#pragma once
#include "Windows.h"
#include "Control.h"
#include <vector>

class ExperimentTimer
{
	public:
		ExperimentTimer();
		~ExperimentTimer();
		int initializeControls(POINT& topLeftPositionKinetic, POINT& topLeftPositionAccumulate, POINT& topLeftPositionContinuous, HWND parentWindow, bool isTriggerModeSensitive);
		int update(int currentAccumulationNumber, int accumulationsPerVariation, int numberOfVariations, HWND parentWindow);
		int reorganizeControls(RECT parentRectangle, std::string mode);
		int getColorID(void);
		void setColorID(int newColorID);
		int setTimerDisplay(std::string newText);
	private:
		HwndControl timeDisplay;
		HwndControl variationProgress;
		HwndControl overallProgress;
		long long lastTime;
		int timeColorID;
		std::vector<double> recentDataPoints;

};