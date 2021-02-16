#pragma once
#include "DoRows.h"
#include "Control.h"
#include <array>
#include "PrimaryWindows/IChimeraQtWindow.h"
#include <CustomQtControls/AutoNotifyCtrls.h>

class DigitalOutput
{
	public:
		void initialize ( QPoint& pos, IChimeraQtWindow* parent );
		void initLoc ( unsigned num, DoRows::which row);
		
		void enable ( bool enabledStatus );
		void updateStatus (  );

		bool defaultStatus;
		bool getStatus ( );
		std::pair<DoRows::which, unsigned> getPosition ( );

		void set ( bool status );
		void setName ( std::string nameStr );
		
		bool holdStatus;
		void setHoldStatus ( bool stat );
		CQCheckBox* check;

	private:
		DoRows::which row;
		unsigned num;
		bool status=0;
};


class allDigitalOutputs
{
	public:
		allDigitalOutputs ( );
		DigitalOutput & operator()( unsigned num, DoRows::which row );
		static const unsigned numRows = 4;
		static const unsigned numColumns = 16;
		// here, typename tells the compiler that the return will be a type.
		typename std::array<DigitalOutput, numRows*numColumns>::iterator begin ( ) { return core.begin ( ); }
		typename std::array<DigitalOutput, numRows*numColumns>::iterator end ( ) { return core.end ( ); }

	private:
		std::array<DigitalOutput, numRows*numColumns> core;
};
