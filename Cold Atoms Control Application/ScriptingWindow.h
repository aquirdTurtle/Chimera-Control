#pragma once

#include "stdafx.h"

class ScriptingWindow : public CDialog
{
	using CDialog::CDialog;
	DECLARE_DYNAMIC(ScriptingWindow);

	public:
		BOOL OnInitDialog() override;
	private:
		DECLARE_MESSAGE_MAP();
};
