#pragma once
#include "windows.h"
#include "DacSystem.h"
#include <unordered_map>


struct dacInputStruct
{
	DacSystem* dacs;
	std::vector<CToolTipCtrl*> toolTips;
	MasterWindow* master;
};


class DacSettingsDialog : public CDialog
{
	public:
		DECLARE_DYNAMIC(DacSettingsDialog);
		DacSettingsDialog(dacInputStruct* inputPtr, UINT dialogResource) : CDialog(dialogResource)
		{
			input = inputPtr;
		}
		//using CDialog::CDialog;

		BOOL OnInitDialog();
		void handleOk();
		void handleCancel();
	private:
		DECLARE_MESSAGE_MAP()
		
		//dacInputStruct input;
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
		int startID = 445;
		dacInputStruct* input;
};

INT_PTR CALLBACK viewAndChangeDAC_NamesProcedure(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);