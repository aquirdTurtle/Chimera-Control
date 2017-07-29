#include "stdafx.h"
#include "Debugger.h"
#include "MasterWindow.h"

void Debugger::rearrange(UINT width, UINT height, fontMap fonts)
{
	heading.rearrange(width, height, fonts);
	showTtlsButton.rearrange( width, height, fonts);
	showDacsButton.rearrange(width, height, fonts);
	pauseText.rearrange(width, height, fonts);
	pauseEdit.rearrange(width, height, fonts);
}


void Debugger::initialize(POINT& pos, MasterWindow* master, std::vector<CToolTipCtrl*> tooltips, int& id)
{
	heading.sPos = { pos.x, pos.y, pos.x + 480, pos.y += 20 };
	heading.Create( "DEBUGGING OPTIONS", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER, 
					heading.sPos, master, id++ );
	heading.fontType = HeadingFont;

	showTtlsButton.sPos = { pos.x, pos.y, pos.x + 480, pos.y += 20 };
	showTtlsButton.Create( "Show All TTL Events", WS_CHILD | WS_VISIBLE | SS_CENTER | BS_CHECKBOX,
						   showTtlsButton.sPos, master, id++ );
	idVerify(showTtlsButton, IDC_SHOW_TTLS);

	showDacsButton.sPos = { pos.x, pos.y, pos.x + 480, pos.y += 20 };
	showDacsButton.Create( "Show All Dac Events", WS_CHILD | WS_VISIBLE | SS_CENTER | BS_CHECKBOX,
						   showDacsButton.sPos, master, id++ );
	idVerify(showDacsButton, IDC_SHOW_DACS);
	
	pauseText.sPos = { pos.x, pos.y, pos.x + 240, pos.y + 20 };
	pauseText.Create("Pause Between Variations", WS_CHILD | WS_VISIBLE, pauseText.sPos, master, id++);

	pauseEdit.sPos = { pos.x + 240, pos.y, pos.x + 480, pos.y += 20 };
	pauseEdit.Create(WS_CHILD | WS_VISIBLE, pauseEdit.sPos, master, id++);
	pauseEdit.SetWindowTextA("0");

}


debuggingOptions Debugger::getOptions()
{
	currentOptions.showTtls = showTtlsButton.GetCheck();
	currentOptions.showDacs = showDacsButton.GetCheck();
	CString text;
	pauseEdit.GetWindowTextA(text);
	try
	{
		currentOptions.sleepTime = std::stol(str(text));
	}
	catch (std::invalid_argument& err)
	{
		thrower("ERROR: pause time failed to convert to a long!");
	}
	return currentOptions;
}


void Debugger::handlePress(UINT id)
{
	if ( id == showTtlsButton.GetDlgCtrlID() )
	{
		
	}
	else if ( id == showDacsButton.GetDlgCtrlID())
	{
		
	}
}


HBRUSH Debugger::handleColorMessage(CWnd* window, brushMap brushes, rgbMap rgbs, CDC* cDC)
{
	DWORD controlID = window->GetDlgCtrlID();

	if (controlID == showDacsButton.GetDlgCtrlID() || controlID == showTtlsButton.GetDlgCtrlID())
	{
		cDC->SetBkColor(rgbs["Medium Grey"]);
		cDC->SetTextColor(rgbs["White"]);
		return brushes["Medium Grey"];
	}
	else
	{
		return NULL;
	}
}