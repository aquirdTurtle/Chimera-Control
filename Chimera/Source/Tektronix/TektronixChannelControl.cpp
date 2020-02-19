#include "stdafx.h"
#include "TektronixChannelControl.h"

void TektronixChannelControl::initialize (POINT loc, CWnd* parent, int& id, std::string channelText, LONG width,
	UINT control_id)
{
	auto& cid = control_id;
	channelLabel.sPos = { loc.x, loc.y, loc.x + width, loc.y += 20 };
	channelLabel.Create (cstr (channelText), NORM_STATIC_OPTIONS, channelLabel.sPos, parent, id++);

	controlButton.sPos = { loc.x, loc.y, loc.x + width, loc.y += 20 };
	controlButton.Create ("", NORM_CHECK_OPTIONS, controlButton.sPos, parent, cid++);

	onOffButton.sPos = { loc.x, loc.y, loc.x + width, loc.y += 20 };
	onOffButton.Create ("", NORM_CHECK_OPTIONS, onOffButton.sPos, parent, cid++);

	fskButton.sPos = { loc.x, loc.y, loc.x + width, loc.y += 20 };
	fskButton.Create ("", NORM_CHECK_OPTIONS, fskButton.sPos, parent, cid++);

	power.sPos = { loc.x, loc.y, loc.x + width, loc.y += 20 };
	power.Create (NORM_EDIT_OPTIONS | ES_AUTOHSCROLL, power.sPos, parent, cid++);
	power.EnableWindow (0);

	mainFreq.sPos = { loc.x, loc.y, loc.x + width, loc.y += 20 };
	mainFreq.Create (NORM_EDIT_OPTIONS | ES_AUTOHSCROLL, mainFreq.sPos, parent, cid++);
	mainFreq.EnableWindow (0);

	fskFreq.sPos = { loc.x, loc.y, loc.x + width, loc.y += 20 };
	fskFreq.Create (NORM_EDIT_OPTIONS | ES_AUTOHSCROLL, fskFreq.sPos, parent, cid++);
	fskFreq.EnableWindow (0);
}


void TektronixChannelControl::handleButton (UINT indicator)
{
	if (indicator == onOffButton.GetDlgCtrlID ())
	{
		handleOnPress ();
	}
	else if (indicator == fskButton.GetDlgCtrlID ())
	{
		handleFskPress ();
	}
}


void TektronixChannelControl::handleOnPress ()
{
	if (onOffButton.GetCheck ())
	{
		fskButton.EnableWindow ();
		power.EnableWindow ();
		mainFreq.EnableWindow ();
		if (fskButton.GetCheck ())
		{
			fskFreq.EnableWindow ();
		}
		else
		{
			fskFreq.EnableWindow (0);
		}
	}
	else
	{
		fskButton.EnableWindow (0);
		power.EnableWindow (0);
		mainFreq.EnableWindow (0);
		fskFreq.EnableWindow (0);
	}
}


void TektronixChannelControl::handleFskPress ()
{
	bool status = fskButton.GetCheck ();
	if (status)
	{
		fskFreq.EnableWindow ();
	}
	else
	{
		fskFreq.EnableWindow (0);
	}
}


// TODO: Gonna need to add a check if what gets returned is a double or a variable.
tektronixChannelOutput TektronixChannelControl::getTekChannelSettings ()
{
	currentInfo.control = controlButton.GetCheck ();
	currentInfo.on = onOffButton.GetCheck ();
	currentInfo.fsk = fskButton.GetCheck ();

	CString text;
	power.GetWindowTextA (text);
	currentInfo.power = str (text, 13, false, true);

	mainFreq.GetWindowTextA (text);
	currentInfo.mainFreq = str (text, 13, false, true);

	fskFreq.GetWindowTextA (text);
	currentInfo.fskFreq = str (text, 13, false, true);

	return currentInfo;
}

void TektronixChannelControl::setSettings (tektronixChannelOutput info)
{
	currentInfo = info;
	controlButton.SetCheck (currentInfo.control);
	onOffButton.SetCheck (currentInfo.on);
	fskButton.SetCheck (currentInfo.fsk);
	power.SetWindowTextA (cstr (currentInfo.power.expressionStr));
	mainFreq.SetWindowTextA (cstr (currentInfo.mainFreq.expressionStr));
	fskFreq.SetWindowTextA (cstr (currentInfo.fskFreq.expressionStr));
}

void TektronixChannelControl::rearrange (int width, int height, fontMap fonts)
{
	channelLabel.rearrange (width, height, fonts);
	controlButton.rearrange (width, height, fonts);
	onOffButton.rearrange (width, height, fonts);
	fskButton.rearrange (width, height, fonts);
	mainFreq.rearrange (width, height, fonts);
	power.rearrange (width, height, fonts);
	fskFreq.rearrange (width, height, fonts);
}
