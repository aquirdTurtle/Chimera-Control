#pragma once
#include "windows.h"
#include "AoSystem.h"
#include <unordered_map>

struct aoInputStruct
{
	AoSystem* aoSys;
	cToolTips toolTips;
	AuxiliaryWindow* master;
};


/*
 * This control is an interface for extra Analog-Output settings. In particular, at the moment, the names of the 
 * different channels, and the min/max voltages allowed for each channel.
 */
class AoSettingsDialog : public CDialog
{
	public:
		DECLARE_DYNAMIC(AoSettingsDialog);
		AoSettingsDialog(aoInputStruct* inputPtr, UINT dialogResource) : CDialog(dialogResource)
		{
			input = inputPtr;
		}
		BOOL OnInitDialog();
		void handleOk();
		void handleCancel();
	private:
		DECLARE_MESSAGE_MAP()
		std::array<Control<CStatic>, 24> numberLabels;
		std::array<Control<CEdit>, 24> nameEdits;
		std::array<Control<CEdit>, 24> minValEdits;
		std::array<Control<CEdit>, 24> maxValEdits;
		std::array<Control<CStatic>, 3> dacNumberHeaders;
		std::array<Control<CStatic>, 3> dacNameHeaders;
		std::array<Control<CStatic>, 3> dacMinValHeaders;
		std::array<Control<CStatic>, 3> dacMaxValHeaders;
		int startx = 30, starty = 40;
		int width = 63;
		int height = 28;
		int id = 445;
		aoInputStruct* input;
};
