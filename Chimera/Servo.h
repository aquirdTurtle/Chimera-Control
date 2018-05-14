#pragma once
#include "Control.h"
#include "DoubleEdit.h"
#include "myButton.h"
#include "Version.h"
/*
This is really just a gui and settings control, the servoManager does the actual work with servoing to avoid 
proliferation of pointers to the AoSystem, etc.
*/
class Servo
{
	public:
		void initialize( POINT& pos, cToolTips& toolTips, CWnd* parent, int& id, std::string name,
						 UINT aiNum, UINT aoNum );
		void rearrange( UINT width, UINT height, fontMap fonts );
		void setControlDisp( double val );
		double getSetPoint( );
		UINT getAiInputChannel( );
		UINT getAoControlChannel( );
		bool servoed = false;
		bool isActive( );
		std::string servoName;
		void handleSaveMasterConfig( std::stringstream& configStream );
		void handleOpenMasterConfig( std::stringstream& configStream, Version version );
		//void handleNewMasterConfig( );
	private:
		// this variable keeps track of whether this servo was successfully servoed.
		UINT aiInputChannel;
		UINT aoControlChannel;
		Control<CStatic> servoNameDisp;
		Control<DoubleEdit> setPointEdit;
		Control<CleanCheck> activeCheck;
		Control<CStatic> aiInputDisp;
		Control<CStatic> aoOutputDisp;
		Control<CStatic> controlValueDisp;
};
