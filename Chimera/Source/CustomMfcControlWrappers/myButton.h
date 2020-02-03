// created by Mark O. Brown
#pragma once
#include "afxwin.h"
#include "afxbutton.h"

/*
Just a tiny wrapper to redraw which cleans up the nasty bright white borders on the default button which conflict with 
the dark color scheme.
*/
class CleanPush : public CMFCButton
{
	virtual void DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct );
	public:
};

class CleanCheck : public CMFCButton
{
	virtual void DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct );
	public:
};
