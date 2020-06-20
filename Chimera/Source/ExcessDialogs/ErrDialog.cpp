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
	if ( dlgType == type::error )
	{
		//auto asyncbeep = std::async ( std::launch::async, [] { Beep ( 500, 500 ); } );
	}
	// replace instances of \n alone with \r\n.
	descriptionText.erase ( std::remove ( descriptionText.begin ( ), descriptionText.end ( ), '\r' ), descriptionText.end ( ) );
	boost::replace_all ( descriptionText, "\n", "\r\n" );
	RECT rect;
	GetClientRect (&rect);
	description.sPos = { 0, 50, rect.right, rect.bottom - 50 };
	description.Create ( WS_CHILD | WS_VISIBLE | ES_READONLY | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL, description.sPos, this, 0 );
	description.SetWindowTextA ( descriptionText.c_str ( ) );
	header.Create ( WS_CHILD | WS_VISIBLE | ES_READONLY | ES_CENTER, { 0,0, rect.right - rect.left, 50 }, this, 1 );
	header.SetWindowText ( dlgType == type::error ? "ERROR!" : "Notice:" );
	header.SetFont ( CFont::FromHandle(font) );
	okBtn.sPos = { 0, rect.bottom-50, rect.right, rect.bottom};
	okBtn.Create ("OK", NORM_PUSH_OPTIONS, okBtn.sPos, this, IDOK);
	return TRUE;
}

HBRUSH ErrDialog::OnCtlColor ( CDC* pDC, CWnd* pWnd, UINT nCtlColor )
{
	static CBrush redBrush ( RGB ( 100, 0, 0 ) );
	pDC->SetTextColor ( dlgType == type::error ? RGB ( 255, 255, 255 ) : _myRGBs["Text"]);
	pDC->SetBkColor ( dlgType == type::error ? RGB(100,0,0) : _myRGBs["Main-Bkgd"]);
	return dlgType == type::error ? redBrush : *_myBrushes["Main-Bkgd"];
}

void ErrDialog::catchOk ( )
{
	EndDialog ( 0 );
}
