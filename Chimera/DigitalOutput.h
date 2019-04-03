#pragma once
#include "DioRows.h"
#include "Control.h"
#include <array>

class DigitalOutput
{
	public:
		void initialize ( POINT& pos, CWnd* parent, UINT id, cToolTips& toolTips );
		void initLoc ( UINT num, DioRows::which row);
		void shade ( bool shadeStat );
		
		void enable ( bool enabledStatus );
		void rearrange ( int width, int height, fontMap fonts );
		void updateStatus (  );

		bool defaultStatus;
		bool getShadeStatus ( );
		bool getStatus ( );
		std::string getName ( );
		std::pair<DioRows::which, UINT> getPosition ( );

		void set ( bool status );
		void setName ( std::string nameStr, cToolTips& toolTips, CWnd* parent );
		
		HBRUSH handleColorMessage ( UINT controlID, CWnd* window, CDC* cDC );
		int getCheckID ( );
		bool holdStatus;
		void setHoldStatus ( bool stat );
	private:
		Control<CButton> check;
		DioRows::which row;
		UINT num;
		bool status;
		bool shadeStatus;		
		std::string name;
};


class allDigitalOutputs
{
	public:
		allDigitalOutputs ( );
		DigitalOutput & operator()( UINT num, DioRows::which row );
		static const UINT numRows = 4;
		static const UINT numColumns = 16;
		// here, typename tells the compiler that the return will be a type.
		typename std::array<DigitalOutput, numRows*numColumns>::iterator begin ( ) { return core.begin ( ); }
		typename std::array<DigitalOutput, numRows*numColumns>::iterator end ( ) { return core.end ( ); }

	private:
		std::array<DigitalOutput, numRows*numColumns> core;
};
