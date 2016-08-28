#pragma once
#include "stdafx.h"

class MainWindow : public CDialog
{
	using CDialog::CDialog;
	DECLARE_DYNAMIC(MainWindow);
	public:
		BOOL OnInitDialog() override;
	private:
		DECLARE_MESSAGE_MAP();
};
