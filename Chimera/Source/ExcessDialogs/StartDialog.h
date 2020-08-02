// created by Mark O. Brown
#pragma once
#include "afxwin.h"
#include "CustomMfcControlWrappers/myButton.h"
#include <CustomMfcControlWrappers/Control.h>
#include <string>

class StartDialog : public CDialog
{
	public:
		using CDialog::CDialog;
		StartDialog( std::string msg, unsigned id );
		BOOL OnInitDialog ( ) override;
		void OnOK( ) override;
		void OnCancel( ) override;
		void OnSize( unsigned s, int width, int height );
		HBRUSH OnCtlColor (CDC* pDC, CWnd* pWnd, unsigned nCtlColor);
	private:
		DECLARE_DYNAMIC( StartDialog );
		DECLARE_MESSAGE_MAP( );
		std::string startMsg;
		Control<CRichEditCtrl> edit;
		Control<CleanPush> okBtn, cancelBtn;
};
