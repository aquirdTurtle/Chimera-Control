// created by Mark O. Brown
#pragma once
#include "Windows.h"
#include "CustomMfcControlWrappers/Control.h"
#include "LowLevel/resource.h"



class ErrDialog : public CDialog
{
	DECLARE_DYNAMIC ( ErrDialog );

	public:
		enum class type
		{
			error,
			info
		};

		ErrDialog ( std::string description, type dlgType_ ) : CDialog ( IDD_ERROR_DIALOG, eMainWindowHwnd )
		{
			dlgType = dlgType_;
			descriptionText = description;
		}
		BOOL OnInitDialog ( ) override;
		HBRUSH OnCtlColor ( CDC* pDC, CWnd* pWnd, UINT nCtlColor );
		void catchOk ( );
	private:
		type dlgType;
		DECLARE_MESSAGE_MAP ( );
		std::string descriptionText;
		Control<CEdit> description;
		Control<CEdit> header;
};

