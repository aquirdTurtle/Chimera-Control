#include "stdafx.h"
#include "Debugger.h"
#include "MasterWindow.h"

void Debugger::initialize(POINT& pos, MasterWindow* master, std::vector<CToolTipCtrl*> tooltips, int& id)
{
	heading.ID = id++;
	heading.position = { pos.x, pos.y, pos.x + 480, pos.y + 20 };
	heading.Create( "DEBUGGING OPTIONS", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER, 
					heading.position, master, heading.ID );
	heading.SetFont( CFont::FromHandle( sHeadingFont ) );
	pos.y += 20;

	showTtlsButton.ID = id++;
	if ( showTtlsButton.ID != IDC_SHOW_TTLS )
	{
		throw;
	}
	showTtlsButton.position = { pos.x, pos.y, pos.x + 480, pos.y + 20 };
	showTtlsButton.Create( "Show All TTL Events", WS_CHILD | WS_VISIBLE | SS_CENTER | BS_CHECKBOX,
						   showTtlsButton.position, master, showTtlsButton.ID );
	showTtlsButton.SetFont( CFont::FromHandle( sHeadingFont ) );
	pos.y += 20;

	showDacsButton.ID = id++;
	if ( showDacsButton.ID != IDC_SHOW_DACS )
	{
		throw;
	}
	showDacsButton.position = { pos.x, pos.y, pos.x + 480, pos.y + 20 };
	showDacsButton.Create( "Show All Dac Events", WS_CHILD | WS_VISIBLE | SS_CENTER | BS_CHECKBOX,
						   showDacsButton.position, master, showDacsButton.ID );
	showDacsButton.SetFont( CFont::FromHandle( sHeadingFont ) );
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
