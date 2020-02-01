#pragma once
#include "afxwin.h"
#include "memory.h"
#include "GeneralImaging/memDC.h"

// This is my own class for handling RAII for the CDCs in MFC. For whatever reason MFC doesn't seem to include it's own 
// class for this, so I wrote my own tiny version.
class SmartDC
{
	public:
		SmartDC (CWnd* cwnd)
		{
			parent = cwnd;
			cdc = parent->GetDC ();
		}
		~SmartDC ()
		{
			parent->ReleaseDC (cdc);
		}
		CDC* get ()
		{
			return cdc;
		}
	private:
		// making these private disables copying. Want to disable copying to avoid extra releaseDC calls on the same cdc.  
		SmartDC (const SmartDC&);
		SmartDC& operator = (const SmartDC&);
		CWnd* parent;
		CDC* cdc;
};
