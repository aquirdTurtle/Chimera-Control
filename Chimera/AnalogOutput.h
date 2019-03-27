#pragma once
#include "control.h"
#include "AoStructures.h"


class AnalogOutput
{
	public:
		AnalogOutput ( );
		void initialize ( POINT& pos, CWnd* parent, int id, cToolTips& toolTips, int whichDac );
		void handleEdit ( bool roundToDacPrecision );
		void updateEdit ( bool roundToDacPrecision );
		static double roundToDacResolution ( double num );
		void rearrange ( UINT width, UINT height, fontMap fonts );
		double getVal ( bool useDefault );
		double setVal ( );
		void setNote ( std::string note, cToolTips& toolTips, CWnd* master );
		AoInfo info;
		void setEditColorState ( int state );
		void setName ( std::string name, cToolTips& toolTips, CWnd* master );
		HBRUSH handleColorMessage ( int id, CWnd* window, CDC* cDC );
		void shade ( );
		void unshade ( );
		void disable ( );
		bool handleArrow ( CWnd* focus, bool up );
	private:
		UINT dacNum;
		Control<CEdit> edit;
		Control<CStatic> label;
};