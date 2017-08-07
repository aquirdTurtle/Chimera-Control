#pragma once
#include "windows.h"
#include "TtlSystem.h"
#include <unordered_map>

struct ttlInputStruct
{
	TtlSystem* ttls;
	std::vector<CToolTipCtrl*> toolTips;
	DeviceWindow* master;
};

class TtlSettingsDialog : public CDialog
{
	public:
		TtlSettingsDialog(ttlInputStruct* inputPtr, UINT dialogResource);
		void handleOk();
		void handleCancel();
		BOOL OnInitDialog();

	private:
		DECLARE_MESSAGE_MAP();
		ttlInputStruct* input;
		std::array<Control<CStatic>, 16> numberlabels;
		std::array<Control<CStatic>, 4> rowLabels;
		std::array<std::array<Control<CEdit>, 16>, 4> edits;
};
