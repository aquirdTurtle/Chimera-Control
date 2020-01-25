// created by Mark O. Brown
#pragma once
#include "afxwin.h"
#include "Control.h"
#include <string>

class StartDialog : public CDialog
{
	public:
		using CDialog::CDialog;
		StartDialog( std::string msg, UINT id );
		BOOL OnInitDialog ( ) override;
		void OnOK( ) override;
		void OnCancel( ) override;
		void OnSize( UINT s, int width, int height );
	private:
		DECLARE_DYNAMIC( StartDialog );
		DECLARE_MESSAGE_MAP( );
		std::string startMsg;
		Control<CRichEditCtrl> edit;
		Control<CButton> okBtn, cancelBtn;
};
