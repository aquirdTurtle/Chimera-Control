#include "stdafx.h"
#include "initializationUpdate.h"
#include "postMyString.h"
#include "Richedit.h"
#include "constants.h"
#include "externals.h"
#include "appendColoredText.h"
int initializationUpdate(std::string errorMessage, CHARFORMAT2 defaultFormat, CHARFORMAT2 redFormat, CHARFORMAT2 greenFormat)
{
	if (errorMessage == "DRV_SUCCESS" || errorMessage == "GOOD")
	{
		appendColoredText("Success.\r\n", eRichEditMessageBoxRichEditHandle, IDC_RICH_EDIT_MESSAGE_BOX_RICH_EDIT_ID, greenFormat, eInitializeDialogBoxHandle);
	}
	else
	{
		appendColoredText("ERROR: " + errorMessage + "\r\n", eRichEditMessageBoxRichEditHandle, IDC_RICH_EDIT_MESSAGE_BOX_RICH_EDIT_ID, redFormat, eInitializeDialogBoxHandle);
	}
	return 0;
}
