#include "Servo.h"
#include "myButton.h"
#include "AiSystem.h"
#include "AoSystem.h"


class ServoManager
{
	public:
		void initialize( POINT& pos, cToolTips& toolTips, CWnd* parent, int& id, AiSystem* ai, AoSystem* ao );
		void rearrange( UINT width, UINT height, fontMap fonts );
		void calibrateAll( );
		void calibrate( Servo& s );
	private:
		Control<CStatic> header;
		Control<CleanButton> calibrateButton;
		Servo sidemotServo;
		Servo diagMotServo;
		/*
		The manager gets pointers to the ai and ao system for hanndling the calibration process.
		*/
		AiSystem* ai;
		AoSystem* ao;
};

