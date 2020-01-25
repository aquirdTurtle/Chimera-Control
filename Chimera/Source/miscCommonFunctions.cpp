// created by Mark O. Brown
#include "stdafx.h"
#include "miscCommonFunctions.h"
#include "my_str.h"
#include <Windows.h>
#include <string>


// for mfc edits
void appendText(std::string newText, CEdit& edit)
{
	// get the initial text length
	int nLength = edit.GetWindowTextLength();
	// put the selection at the end of text
	edit.SetSel(nLength, nLength);
	// replace the selection
	edit.ReplaceSel(cstr(newText));
	edit.LineScroll(INT_MAX);
}

// for mfc rich edits
void appendText(std::string newText, Control<CRichEditCtrl>& edit)
{
	// get the initial text length
	int nLength = edit.GetWindowTextLength();
	// put the selection at the end of text
	edit.SetSel(nLength, nLength);
	// replace the selection
	edit.ReplaceSel(cstr(newText));
	edit.SetFocus();
	nLength = edit.GetWindowTextLength();
	// put the selection at the end of text
	edit.SetSel(nLength, nLength);
	//	edit.LineScroll(INT_MAX);
}


ULONG getNextFileIndex( std::string fileBase, std::string ext )
{
	// find the first data file that hasn't been already written, starting with fileBase1.h5
	ULONG fileNum = 1;
	// The while condition here check if file exists. No idea how this actually works.
	struct stat statBuffer;
	// figure out the next file number
	while ( (stat( cstr( fileBase + str( fileNum ) + ext ),
				   &statBuffer ) == 0) )
	{
		fileNum++;
	}
	return fileNum;
}
