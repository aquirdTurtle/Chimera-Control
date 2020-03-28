#include "stdafx.h"
#include "IChimeraWindow.h"
#include "MainWindow.h"
#include "AuxiliaryWindow.h"
#include "AndorWindow.h"
#include "BaslerWindow.h"
#include "DeformableMirrorWindow.h"
#include "ScriptingWindow.h"

#include "GeneralUtilityFunctions/CommonFunctions.h"

IChimeraWindow::IChimeraWindow () : CDialog () {}

IMPLEMENT_DYNAMIC (IChimeraWindow, CDialog)

BEGIN_MESSAGE_MAP (IChimeraWindow, CDialog)
	ON_WM_CTLCOLOR ()
	ON_COMMAND_RANGE (ID_ACCELERATOR_ESC, ID_ACCELERATOR_ESC, &passCommonCommand)
	ON_COMMAND_RANGE (ID_ACCELERATOR_F5, ID_ACCELERATOR_F5, &passCommonCommand)
	ON_COMMAND_RANGE (ID_ACCELERATOR_F2, ID_ACCELERATOR_F2, &passCommonCommand)
	ON_COMMAND_RANGE (ID_ACCELERATOR_F1, ID_ACCELERATOR_F1, &passCommonCommand)
	ON_COMMAND_RANGE (MENU_ID_RANGE_BEGIN, MENU_ID_RANGE_END, &passCommonCommand)
	ON_COMMAND (IDOK, &OnEnter)
END_MESSAGE_MAP ()


HBRUSH IChimeraWindow::OnCtlColor (CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	// default colors
	switch (nCtlColor)
	{
		case CTLCOLOR_STATIC:
		{
			pDC->SetTextColor (_myRGBs["Text"]);
			pDC->SetBkColor (_myRGBs["Static-Bkgd"]);
			return *_myBrushes["Static-Bkgd"];
		}
		case CTLCOLOR_EDIT:
		{
			pDC->SetTextColor (_myRGBs["Text"]);
			pDC->SetBkColor (_myRGBs["Interactable-Bkgd"]);
			return *_myBrushes["Interactable-Bkgd"];
		}
		case CTLCOLOR_LISTBOX:
		{
			pDC->SetTextColor (_myRGBs["Text"]);
			pDC->SetBkColor (_myRGBs["Interactable-Bkgd"]);
			return *_myBrushes["Interactable-Bkgd"];
		}
		default:
			return *_myBrushes["Main-Bkgd"];
	}
}

void IChimeraWindow::passCommonCommand (UINT id)
{
	commonFunctions::handleCommonMessage (id, this);
}

void IChimeraWindow::reportErr (std::string errStr)
{
	mainWin->getComm ()->sendError (errStr);
}

void IChimeraWindow::reportStatus (std::string statusStr)
{
	mainWin->getComm ()->sendStatus (statusStr);
}

void IChimeraWindow::OnCancel ()
{
	try
	{
		passCommonCommand (ID_FILE_MY_EXIT);
	}
	catch (Error & err)
	{
		reportErr (err.trace ());
	}
}

BOOL IChimeraWindow::PreTranslateMessage (MSG* pMsg)
{
	for (auto& toolTip : toolTips)
	{
		toolTip->RelayEvent (pMsg);
	}
	return CDialog::PreTranslateMessage (pMsg);
}

void IChimeraWindow::loadFriends ( MainWindow* mainWin_, ScriptingWindow* scriptWin_, AuxiliaryWindow* auxWin_,
								   BaslerWindow* basWin_, DeformableMirrorWindow* dmWindow_, AndorWindow* andorWin_ )
{
	mainWin = mainWin_;
	scriptWin = scriptWin_;
	auxWin = auxWin_;
	basWin = basWin_;
	dmWin = dmWindow_;
	andorWin = andorWin_;
}


void IChimeraWindow::OnEnter () 
{
	errBox ("The cake is a lie.");
}
