#pragma once

#include "resource.h"

// ScriptWritingInstructionsDialog dialog

class ScriptWritingInstructionsDialog : public CDialogEx
{
	DECLARE_DYNAMIC(ScriptWritingInstructionsDialog)

	public:
		ScriptWritingInstructionsDialog(CWnd* pParent = NULL);   // standard constructor
		virtual ~ScriptWritingInstructionsDialog();

	// Dialog Data
		enum { IDD = IDD_SCRIPT_HELP_DIALOG };

	protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

		DECLARE_MESSAGE_MAP()
	public:
		afx_msg void OnEnChangeEdit2();
};
