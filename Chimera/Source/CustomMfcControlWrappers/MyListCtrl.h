// created by Mark O. Brown
#pragma once 
#include "afxwin.h"
#include <string>

/*
Some small wrappers to make these a little easier to work with.
*/
class MyListCtrl : public CListCtrl
{
	public:
		void InsertColumn ( int col, std::string txt );
		void InsertColumn ( int col, std::string txt, int width );
		void InsertItem ( std::string txt, int item, int subitem );
		void SetItem( std::string txt, int item, int subitem );
		void insertBlankRow ( );
	private:
		int lastWidth = 0;
};

