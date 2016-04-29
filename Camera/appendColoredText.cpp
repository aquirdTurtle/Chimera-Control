#include "stdafx.h"
#include "appendColoredText.h"
#include "Richedit.h"
#include "appendText.h"

int appendColoredText(std::string text, HWND richEditHandle, int richEditID, CHARFORMAT2 format, HWND ownderHandle)
{
	SendMessage(richEditHandle, EM_SETSEL, (WPARAM)GetWindowTextLength(richEditHandle), (LPARAM)GetWindowTextLength(richEditHandle));
	SendMessage(richEditHandle, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&format);
	appendText(text, richEditID, ownderHandle);

	return 0;
}
