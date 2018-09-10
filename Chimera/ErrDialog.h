#pragma once
#include "Windows.h"
#include "Control.h"
#include "resource.h"

class ErrDialog : public CDialog
{
	DECLARE_DYNAMIC ( ErrDialog );

	public:

	ErrDialog ( std::string description ) : CDialog ( IDD_ERROR_DIALOG )
	{
		descriptionText = description;
	}
	BOOL OnInitDialog ( ) override;
	HBRUSH OnCtlColor ( CDC* pDC, CWnd* pWnd, UINT nCtlColor );
	void catchOk ( );
	private:
		DECLARE_MESSAGE_MAP ( );
		std::string descriptionText;
		Control<CEdit> description;
		Control<CEdit> header;
};

