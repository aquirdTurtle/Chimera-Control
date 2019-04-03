// created by Mark O. Brown
#include "stdafx.h"
#include "ErrDialog.h"
#include <future>
#include <boost/algorithm/string/replace.hpp>


IMPLEMENT_DYNAMIC ( ErrDialog, CDialog )

BEGIN_MESSAGE_MAP ( ErrDialog, CDialog )
	ON_WM_CTLCOLOR ( )
	ON_WM_TIMER ( )
	ON_WM_SIZE ( )
	ON_COMMAND ( IDOK, &ErrDialog::catchOk )
END_MESSAGE_MAP ( )


BOOL ErrDialog::OnInitDialog ( )
{
	SetWindowText ( dlgType == type::error ? "ERROR!" : "Notice:" );
	static HFONT font = CreateFont( 42, 0, 0, 0, FW_DONTCARE, TRUE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
									CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT ( "Old Sans Black" ) );
	RECT winRect;
	GetWindowRect ( &winRect );	
	if ( dlgType == type::error )
	{
		auto asyncbeep = std::async ( std::launch::async, [] { Beep ( 500, 500 ); } );
	}
	// replace instances of \n alone with \r\n.
	descriptionText.erase ( std::remove ( descriptionText.begin ( ), descriptionText.end ( ), '\r' ), descriptionText.end ( ) );
	boost::replace_all ( descriptionText, "\n", "\r\n" );
	description.Create ( WS_CHILD | WS_VISIBLE | ES_READONLY | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL, { 0,50,
						 winRect.right-winRect.left-10,winRect.bottom-winRect.top-100 }, this, 0 );
	description.SetWindowTextA ( descriptionText.c_str ( ) );
	header.Create ( WS_CHILD | WS_VISIBLE | ES_READONLY | ES_CENTER, { 0,0, winRect.right - winRect.left, 50 }, this, 1 );
	header.SetWindowText ( dlgType == type::error ? "ERROR!" : "Notice:" );
	header.SetFont ( CFont::FromHandle(font) );
	return TRUE;
}


HBRUSH ErrDialog::OnCtlColor ( CDC* pDC, CWnd* pWnd, UINT nCtlColor )
{
	static CBrush redBrush ( RGB ( 100, 0, 0 ) );
	static CBrush solarizedBrush ( RGB ( 253, 246, 227 ) );
	int num = pWnd->GetDlgCtrlID ( );
	pDC->SetTextColor ( dlgType == type::error ? RGB ( 255, 255, 255 ) : RGB( 101, 123, 131 ) );
	pDC->SetBkColor ( dlgType == type::error ? RGB(100,0,0) : RGB(253, 246, 227) );
	return dlgType == type::error ? redBrush : solarizedBrush;
}

void ErrDialog::catchOk ( )
{
	EndDialog ( 0 );
}
