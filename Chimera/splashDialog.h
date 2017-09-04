#pragma once
#include "afxwin.h"
class splashDialog : public CDialog
{
	public:
	splashDialog( );
	HBRUSH OnCtlColor( CDC* pDC, CWnd* pWnd, UINT nCtlColor );
	
	private:
	CBrush* colorBrush;
	DECLARE_MESSAGE_MAP( )

};

