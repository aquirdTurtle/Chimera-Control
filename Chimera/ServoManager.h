#pragma once
#include "Servo.h"
#include "myButton.h"
#include "AiSystem.h"
#include "AoSystem.h"
#include "DoubleEdit.h"
#include "ParameterSystem.h"
#include "Version.h"

/*
This is a slow digital DC servo system. As far as servos go, it is very primitive, just a Proportional servo with a low
gain, as this is all that's required for DC servoing. This is not designed to be run during the experiment, it's 
designed to run in between experiments in order to maintain 

The servo system interplays between three separate systems in the code.
- It uses the analog input system in order to read a power level.
- It uses the analog output system, adjusting one of the dacs, in order to change the power level. In order to use the 
	AoSystem, it also uses the DioSystem.
- (TODO) It saves the result in a parameter in the global parameterSystem that can be used in scripts.

- The options in an individual servo are saved in a Servo object.
*/
class ServoManager
{
	public:
		void initialize( POINT& pos, cToolTips& toolTips, CWnd* parent, int& id, AiSystem* ai, AoSystem* ao, 
						 DioSystem* ttls_in, ParameterSystem* globals_in );
		void rearrange( UINT width, UINT height, fontMap fonts );
		void runAll( );
		void calibrate( Servo& s );
		bool autoServo( );
		void handleSaveMasterConfig( std::stringstream& configStream );
		void handleOpenMasterConfig( std::stringstream& configStream, Version version );
		//void handleNewMasterConfig( std::stringstream& configStream );
	private:
		Control<CStatic> servosHeader;
		Control<CleanButton> servoButton;
		Control<CleanCheck> autoServoButton;

		Control<CStatic> toleranceLabel;
		Control<DoubleEdit> toleranceEdit;
		Control<CStatic> attemptLimitLabel;
		Control<DoubleEdit> attemptLimitEdit;

		Control<CStatic> nameHeader;
		Control<CStatic> activeHeader;
		Control<CStatic> valueHeader;
		Control<CStatic> aiNumberHeader;
		Control<CStatic> aoNumberHeader;
		Control<CStatic> controlHeader;

		const UINT numServos = 3;
		Servo sidemotServo;
		Servo diagMotServo;
		Servo d1Servo;
		/*
		The manager gets pointers to the ai and ao system for hanndling the calibration process. It only gets the ttls
		to give to the ao system for changes.
		*/
		AiSystem* ai;
		AoSystem* ao;
		DioSystem* ttls;
		ParameterSystem* globals;
};

