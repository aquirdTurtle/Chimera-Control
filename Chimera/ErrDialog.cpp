#include "stdafx.h"
#include "ErrDialog.h"
#include <future>


IMPLEMENT_DYNAMIC ( ErrDialog, CDialog )

BEGIN_MESSAGE_MAP ( ErrDialog, CDialog )
	ON_WM_CTLCOLOR ( )
	ON_WM_TIMER ( )
	ON_WM_SIZE ( )
	ON_COMMAND ( IDOK, &ErrDialog::catchOk )
END_MESSAGE_MAP ( )


BOOL ErrDialog::OnInitDialog ( )
{
	SetWindowText ( "ERROR!" );
	static HFONT font = CreateFont( 42, 0, 0, 0, FW_DONTCARE, TRUE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
									CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT ( "Old Sans Black" ) );
	RECT winRect;
	GetWindowRect ( &winRect );
	
	auto asyncbeep = std::async ( std::launch::async, [] { Beep ( 500, 500 ); } );
	description.Create ( WS_CHILD | WS_VISIBLE | ES_READONLY | ES_MULTILINE | ES_AUTOVSCROLL, { 0,100,
						 winRect.right-winRect.left,winRect.bottom-winRect.top-25 }, this, 0 );
	description.SetWindowTextA ( descriptionText.c_str ( ) );
	header.Create ( WS_CHILD | WS_VISIBLE | ES_READONLY | ES_CENTER, { 0,0, winRect.right - winRect.left, 100 }, this, 1 );
	header.SetWindowText ( "ERROR!" );
	header.SetFont ( CFont::FromHandle(font) );
	return TRUE;
}


HBRUSH ErrDialog::OnCtlColor ( CDC* pDC, CWnd* pWnd, UINT nCtlColor )
{
	static CBrush redBrush ( RGB ( 100, 0, 0 ) );
	CBrush * result ;
	int num = pWnd->GetDlgCtrlID ( );
	pDC->SetTextColor ( RGB ( 255, 255, 255 ) );
	pDC->SetBkColor ( RGB ( 100, 0, 0 ) );
	return redBrush;
}

void ErrDialog::catchOk ( )
{
	EndDialog ( 0 );
}
