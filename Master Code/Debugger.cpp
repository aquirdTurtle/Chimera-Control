#include "stdafx.h"
#include "Debugger.h"
#include "MasterWindow.h"

void Debugger::rearrange(UINT width, UINT height, fontMap fonts)
{
	heading.rearrange(width, height, fonts);
	showTtlsButton.rearrange( width, height, fonts);
	showDacsButton.rearrange(width, height, fonts);
}

void Debugger::initialize(POINT& pos, MasterWindow* master, std::vector<CToolTipCtrl*> tooltips, int& id)
{
	heading.ID = id++;
	heading.sPos = { pos.x, pos.y, pos.x + 480, pos.y + 20 };
	heading.Create( "DEBUGGING OPTIONS", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER, 
					heading.sPos, master, heading.ID );
	heading.fontType = Heading;
	pos.y += 20;

	showTtlsButton.ID = id++;
	if ( showTtlsButton.ID != IDC_SHOW_TTLS )
	{
		throw;
	}
	showTtlsButton.sPos = { pos.x, pos.y, pos.x + 480, pos.y + 20 };
	showTtlsButton.Create( "Show All TTL Events", WS_CHILD | WS_VISIBLE | SS_CENTER | BS_CHECKBOX,
						   showTtlsButton.sPos, master, showTtlsButton.ID );
	showTtlsButton.fontType = Normal;
	pos.y += 20;

	showDacsButton.ID = id++;
	if ( showDacsButton.ID != IDC_SHOW_DACS )
	{
		throw;
	}
	showDacsButton.sPos = { pos.x, pos.y, pos.x + 480, pos.y + 20 };
	showDacsButton.Create( "Show All Dac Events", WS_CHILD | WS_VISIBLE | SS_CENTER | BS_CHECKBOX,
						   showDacsButton.sPos, master, showDacsButton.ID );
	showDacsButton.fontType = Normal;
	pos.y += 20;
}

debuggingOptions Debugger::getOptions()
{
	return this->currentOptions;
}

void Debugger::handlePress(UINT id)
{
	if ( id == this->showTtlsButton.ID )
	{
		if ( showTtlsButton.GetCheck() )
		{
			currentOptions.showTtls = true;
		}
		else
		{
			currentOptions.showTtls = false;
		}
	}
	else if ( id == this->showDacsButton.ID )
	{
		if ( showDacsButton.GetCheck() )
		{
			currentOptions.showDacs = true;
		}
		else
		{
			currentOptions.showDacs = false;
		}
	}
	return;
}
