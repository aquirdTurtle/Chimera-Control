// created by Mark O. Brown
#pragma once
#include "afxwin.h"
class splashDialog : public CDialog
{
	public:
	splashDialog( );
	HBRUSH OnCtlColor( CDC* pDC, CWnd* pWnd, UINT nCtlColor );
	BOOL OnInitDialog () override;
	void stretchBmp ();
	private:
	CBrush* colorBrush;
	void OnPaint ();
	DECLARE_MESSAGE_MAP( )
	CBitmap image;
};

