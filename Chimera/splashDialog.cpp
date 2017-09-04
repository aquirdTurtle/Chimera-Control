#include "stdafx.h"
#include "splashDialog.h"

BEGIN_MESSAGE_MAP( splashDialog, CDialog )
	ON_WM_CTLCOLOR( )
	// 
END_MESSAGE_MAP( )

splashDialog::splashDialog( )
{
	(colorBrush = new CBrush)->CreateSolidBrush( RGB( 0, 0, 0 ) );
}

HBRUSH splashDialog::OnCtlColor( CDC* pDC, CWnd* pWnd, UINT nCtlColor )
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