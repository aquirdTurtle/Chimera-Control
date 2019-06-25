// created by Mark O. Brown
#include "stdafx.h"
#include "MyListCtrl.h"


void MyListCtrl::InsertColumn ( int col, std::string txt )
{
	InsertColumn ( col, txt, lastWidth );
}


void MyListCtrl::InsertColumn ( int col, std::string txt, int width )
{
	if ( width == -1 )
	{
		width = lastWidth;
	}
	lastWidth = width;
	LV_COLUMN c = { 0 };
	c.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	c.pszText = (LPSTR)txt.c_str();
	c.cx = width;
	CListCtrl::InsertColumn ( col, &c );
}

/*
Inserts a blank row (full of "___") at the end of the listview.
*/
void MyListCtrl::insertBlankRow ( )
{
	auto colCount = GetHeaderCtrl ( )->GetItemCount ( );
	auto rowCount = GetItemCount ( );
	InsertItem ( "___", rowCount, 0 );
	for ( int itemInc = 1; itemInc < colCount; itemInc++ )
	{
		SetItem ( "___", rowCount, itemInc );
	}
}


void MyListCtrl::InsertItem ( std::string txt, int item, int subitem )
{
	LVITEM i = { 0 };
	i.mask = LVIF_TEXT;
	i.cchTextMax = 256;
	i.pszText = (LPSTR)txt.c_str();
	i.iItem = item;
	i.iSubItem = subitem;
	CListCtrl::InsertItem ( &i );
}

void MyListCtrl::SetItem ( std::string txt, int item, int subitem )
{
	LVITEM i = { 0 };
	i.mask = LVIF_TEXT;
	i.cchTextMax = 256;
	i.pszText = (LPSTR) txt.c_str ( );
	i.iItem = item;
	i.iSubItem = subitem;
	CListCtrl::SetItem ( &i );
}

