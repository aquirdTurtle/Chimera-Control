#pragma once
#include "Windows.h"

INT_PTR  CALLBACK beginningSettingsDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

class StartDialog : public CDialog
{
	public:
		using CDialog::CDialog;
		StartDialog( std::string msg, UINT id );
		BOOL OnInitDialog( );
		void OnOK( );
		void OnCancel( );
		void OnSize( UINT s, int width, int height );
	private:
		DECLARE_DYNAMIC( StartDialog );
		DECLARE_MESSAGE_MAP( );
		std::string startMsg;
		Control<CRichEditCtrl> edit;
		Control<CButton> okBtn, cancelBtn;
};
