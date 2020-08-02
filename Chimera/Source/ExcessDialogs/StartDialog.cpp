// created by Mark O. Brown
#include "stdafx.h"
#include "StartDialog.h"
#include <sstream>


StartDialog::StartDialog( std::string msg, unsigned id ) : CDialog::CDialog( id )
{
	startMsg = msg;
}

IMPLEMENT_DYNAMIC( StartDialog, CDialog )

BEGIN_MESSAGE_MAP( StartDialog, CDialog )
	ON_WM_CTLCOLOR ()
	ON_WM_SIZE( )
	ON_COMMAND( IDOK, &StartDialog::OnOK )
	ON_COMMAND( IDCANCEL, &StartDialog::OnCancel )
END_MESSAGE_MAP( )

void StartDialog::OnSize(unsigned s, int width, int height )
{
}

void StartDialog::OnOK()
{
	EndDialog( TRUE );
}

void StartDialog::OnCancel( )
{
	EndDialog( FALSE );
}

BOOL StartDialog::OnInitDialog( )
{
	RECT r;
	GetClientRect( &r );
	edit.sPos = { 0, 0, r.right, r.bottom-30};
	edit.Create( WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL	| ES_READONLY, edit.sPos, this, NULL );
	edit.SetWindowTextA( startMsg.c_str() );
	edit.SetBackgroundColor (false, _myRGBs["Main-Bkgd"]);

	okBtn.sPos = { 0, r.bottom - 30, r.right/2, r.bottom };
	okBtn.Create( "OK", NORM_PUSH_OPTIONS, okBtn.sPos, this, IDOK );
	cancelBtn.sPos = { r.right/2, r.bottom - 30,  r.right, r.bottom };
	cancelBtn.Create( "CANCEL", NORM_PUSH_OPTIONS, cancelBtn.sPos, this, IDCANCEL );
	std::stringstream streamObj( startMsg );
	std::string line;
	DWORD start = 0, end = 0;
	std::size_t prev, pos;
	std::string word, analysisWord;
	CHARFORMAT syntaxFormat = { 0 };
	syntaxFormat.cbSize = sizeof( CHARFORMAT );
	syntaxFormat.dwMask = CFM_COLOR;
	while ( std::getline( streamObj, line ) )
	{
		syntaxFormat.crTextColor = _myRGBs["Text"];
		DWORD lineStartCoordingate = start;
		edit.SetSel (lineStartCoordingate, std::string::npos);
		edit.SetSelectionCharFormat (syntaxFormat);
		prev = 0;
		while ( (pos = line.find_first_of( " \t\r\n", prev )) != std::string::npos )
		{
			end = lineStartCoordingate + pos;
			word = line.substr( prev, pos - prev + 1 );
			analysisWord = word.substr( 0, word.length( ) - 1 );
			if ( analysisWord == "TRUE" || analysisWord == "SAVED" )
			{
				syntaxFormat.crTextColor = RGB( 0, 150, 0 );
				edit.SetSel( start, end );
				edit.SetSelectionCharFormat( syntaxFormat );
			}
			else if ( analysisWord == "FALSE" || analysisWord == "NOT" )
			{
				syntaxFormat.crTextColor = RGB( 255, 0, 0 );
				edit.SetSel( start, end );
				edit.SetSelectionCharFormat( syntaxFormat );
			}
			end++;
			start = end;
			prev = pos + 1;
		}
		if ( prev < line.length( ) )
		{
			word = line.substr( prev, std::string::npos );
			if ( analysisWord == "TRUE" || analysisWord == "SAVED" )
			{
				syntaxFormat.crTextColor = RGB( 0, 150, 0 );
				edit.SetSel( start, end );
				edit.SetSelectionCharFormat( syntaxFormat );
			}
			else if ( analysisWord == "FALSE" || analysisWord == "NOT" )
			{
				syntaxFormat.crTextColor = RGB( 255, 0, 0 );
				edit.SetSel( start, end );
				edit.SetSelectionCharFormat( syntaxFormat );
			}
			end++;
			start = end;
		}
	}
	SetFocus();
	return TRUE;
}

HBRUSH StartDialog::OnCtlColor (CDC* pDC, CWnd* pWnd, unsigned nCtlColor)
{
	pDC->SetTextColor (_myRGBs["Text"]);
	pDC->SetBkColor (_myRGBs["Main-Bkgd"]);
	return *_myBrushes["Main-Bkgd"];
}
