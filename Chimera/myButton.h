#pragma once
#include "afxwin.h"

/*
Just a tiny wrapper to redraw which cleans up the nasty bright white borders on the default button which conflict with 
the dark color scheme.
*/
class CleanButton : public CMFCButton
{
	virtual void DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct );
};

class CleanCheck : public CMFCButton
{
	virtual void DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct );
};
