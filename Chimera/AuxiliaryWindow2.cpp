#include "stdafx.h"
#include "AuxiliaryWindow2.h"
#include "MainWindow.h"

AuxiliaryWindow2::AuxiliaryWindow2() {

}

BOOL AuxiliaryWindow2::OnInitDialog() {
	POINT pos = { 0,0 };
	int id = 1000;
	statusBox.initialize(pos, id, this, 480, toolTips);
	return TRUE;
}
void AuxiliaryWindow2::OnCancel() {

}

void AuxiliaryWindow2::loadFriends(MainWindow* mainWin_, ScriptingWindow* scriptWin_, AndorWindow* camWin_,
						  AuxiliaryWindow* auxWin_, BaslerWindow* basWin_) {
	mainWin = mainWin_;
	scriptWin = scriptWin_;
	camWin = camWin_;
	basWin = basWin_;
	auxWin = auxWin_;
}

void AuxiliaryWindow2::OnSize(UINT nType, int cx, int cy)
{
	SetRedraw(false);
	statusBox.rearrange(cx, cy, mainWin->getFonts());
	SetRedraw();
	RedrawWindow();
}

IMPLEMENT_DYNAMIC(AuxiliaryWindow2, CDialog)


BEGIN_MESSAGE_MAP(AuxiliaryWindow2, CDialog)
	
END_MESSAGE_MAP()
