#include "stdafx.h"
#include "CameraWindow.h"
#include "commonMessages.h"
#include "CameraSettingsControl.h"

IMPLEMENT_DYNAMIC(CameraWindow, CDialog)

BEGIN_MESSAGE_MAP(CameraWindow, CDialog)
	ON_WM_CTLCOLOR()
	ON_WM_SIZE()
	ON_WM_VSCROLL()
	// menu stuff
	ON_COMMAND_RANGE(MENU_ID_RANGE_BEGIN, MENU_ID_RANGE_END, &CameraWindow::passCommonCommand)
	// 
	ON_COMMAND(IDC_SET_IMAGE_PARAMETERS_BUTTON, &CameraWindow::readImageParameters)
	ON_COMMAND(IDC_SET_EM_GAIN_BUTTON, &CameraWindow::setEmGain)
END_MESSAGE_MAP()

void CameraWindow::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* scrollbar)
{
	this->pics.handleScroll(nSBCode, nPos, scrollbar);
}
void CameraWindow::OnSize(UINT nType, int cx, int cy)
{
	AndorRunSettings settings = this->CameraSettings.getSettings();
	this->stats.reorganizeControls(settings.cameraMode, settings.triggerMode, cx, cy);
	this->CameraSettings.reorganizeControls(settings.cameraMode, settings.triggerMode, cx, cy);
	this->box.rearrange(settings.cameraMode, settings.triggerMode, cx, cy);
	this->pics.rearrange(settings.cameraMode, settings.triggerMode, cx, cy);
	this->alerts.reorganizeControls(settings.cameraMode, settings.triggerMode, cx, cy);
	return;
}

void CameraWindow::setEmGain()
{
	this->CameraSettings.setEmGain(&this->Andor, this->mainWindowFriend->getComm());
}

void CameraWindow::OnCancel()
{
	passCommonCommand(ID_FILE_MY_EXIT);
	return;
}

BOOL CameraWindow::OnInitDialog()
{
	std::array<POINT, 3> cameraPositions;
	// all of the initialization functions increment and use the id, so by the end it will be 3000 + # of controls.
	int id = 3000;
	cameraPositions[0] = { 0,0 }; 
	box.initialize(cameraPositions[0], id, this, 480);
	cameraPositions[1] = cameraPositions[2] = cameraPositions[0];
	this->CameraSettings.initialize(cameraPositions[0], cameraPositions[1], cameraPositions[2], id, this);
	alerts.initialize(cameraPositions[0], cameraPositions[1], cameraPositions[2], this, false, id);
	POINT position = { 480, 0 };
	stats.initialize(position, this, id);
	position = { 757, 0 };
	pics.initialize(position, this, id);
	// load the menu
	CMenu menu;
	menu.LoadMenu(IDR_MAIN_MENU);
	this->SetMenu(&menu);
	// final steps
	this->ShowWindow(SW_MAXIMIZE);
	return TRUE;
}

void CameraWindow::getFriends(MainWindow* mainWin, ScriptingWindow* scriptWin)
{
	this->mainWindowFriend = mainWin;
	this->scriptingWindowFriend = scriptWin;
}

void CameraWindow::redrawPictures()
{
	this->pics.refreshBackgrounds(this);
}

HBRUSH CameraWindow::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	std::unordered_map<std::string, CBrush*> brushes = mainWindowFriend->getBrushes();
	std::unordered_map<std::string, COLORREF> rgbs = mainWindowFriend->getRGB();
	switch (nCtlColor)
	{
		case CTLCOLOR_STATIC:
		{
			int num = (pWnd->GetDlgCtrlID());
			CBrush* result = this->box.handleColoring(num, pDC, brushes);
			if (result)
			{
				return *result;
			}
			else
			{
				pDC->SetTextColor(rgbs["White"]);
				pDC->SetBkColor(rgbs["Dark Grey Red"]);
				return *brushes["Dark Grey Red"];
			}
		}
		case CTLCOLOR_EDIT:
		{
			pDC->SetTextColor(rgbs["White"]);
			pDC->SetBkColor(rgbs["Dark Green"]);
			return *brushes["Dark Green"];
		}
		case CTLCOLOR_LISTBOX:
		{
			pDC->SetTextColor(rgbs["White"]);
			pDC->SetBkColor(rgbs["Dark Grey"]);
			return *brushes["Dark Grey"];
		}
		default:
		{
			return *brushes["Light Grey"];
		}
	}
}

void CameraWindow::passCommonCommand(UINT id)
{
	commonMessages::handleCommonMessage(id, this, mainWindowFriend, scriptingWindowFriend, this);
}

void CameraWindow::readImageParameters()
{
	this->redrawPictures();
	imageParameters parameters = this->CameraSettings.readImageParameters(this, this->mainWindowFriend->getComm());
	this->pics.setParameters(parameters);
	this->pics.drawGrids(this, this->mainWindowFriend->getBrushes()["White"]);
	return;
}

void CameraWindow::changeBoxColor(std::string color)
{
	this->box.changeColor(color);
	return;
}
