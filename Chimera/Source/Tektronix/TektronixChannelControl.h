#pragma once
#include "TektronixStructures.h"
#include "afxwin.h"
#include "CustomMfcControlWrappers/myButton.h"

class TektronixChannelControl
{
public:
	void initialize (POINT loc, CWnd* parent, int& id, std::string channel1Text, LONG width, UINT control_id);
	tektronicsChannelOutputForm getTekChannelSettings ();
	void setSettings (tektronicsChannelOutputForm info);
	void rearrange (int width, int height, fontMap fonts);
	void handleOnPress ();
	void handleFskPress ();
	void handleButton (UINT indicator);
private:
	Control<CStatic> channelLabel;
	Control<CleanCheck> controlButton;
	Control<CleanCheck> onOffButton;
	Control<CleanCheck> fskButton;
	Control<CEdit> power;
	Control<CEdit> mainFreq;
	Control<CEdit> fskFreq;
	tektronicsChannelOutputForm currentInfo;
};
