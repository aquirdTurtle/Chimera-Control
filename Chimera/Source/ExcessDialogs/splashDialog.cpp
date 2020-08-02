// created by Mark O. Brown
#include "stdafx.h"
#include "splashDialog.h"

BEGIN_MESSAGE_MAP( splashDialog, CDialog )
	ON_WM_CTLCOLOR( )
	ON_WM_PAINT()
	// 
END_MESSAGE_MAP( )

splashDialog::splashDialog( )
{
	(colorBrush = new CBrush)->CreateSolidBrush( RGB( 0, 0, 0 ) );
}

void splashDialog::OnPaint ()
{
	stretchBmp ();
}

BOOL splashDialog::OnInitDialog ()
{
	SetWindowPos (&wndTop,0,0,0,0, SWP_NOMOVE | SWP_NOSIZE);
	ShowWindow (SW_SHOWMAXIMIZED);
	stretchBmp ();
	return 0;
}

void splashDialog::stretchBmp ()
{
	RECT rect;
	GetClientRect (&rect);
	CDC dcMemory;
	CPaintDC dc (this);
	image.LoadBitmap (IDB_BITMAP4);
	dcMemory.CreateCompatibleDC (&dc);
	dcMemory.SelectObject (&image);
	CSize size = { rect.right - rect.left, rect.bottom - rect.top };
	dc.DPtoLP (&size);
	dc.SetStretchBltMode (COLORONCOLOR);
	dc.StretchBlt (0, 0, size.cx, size.cy, &dcMemory, 0, 0, 5312, 2988, SRCCOPY);
}

HBRUSH splashDialog::OnCtlColor( CDC* pDC, CWnd* pWnd, unsigned nCtlColor )
{
	switch ( nCtlColor )
	{
		case CTLCOLOR_STATIC:
			pDC->SetBkColor( RGB( 0, 0, 0 ) );
			pDC->SetTextColor( RGB( 255, 255, 255 ) );
		default:
			return *colorBrush;
	}
}