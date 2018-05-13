#include "Control.h"
#include "DoubleEdit.h"


class Servo
{
	public:
		void initialize( POINT& pos, cToolTips& toolTips, CWnd* parent, int& id, std::string name,
						 UINT aiNum, UINT aoNum );
		void rearrange( UINT width, UINT height, fontMap fonts );
		void calibrate( );
		double getSetPoint( );
		UINT getAiInputChannel( );
		UINT getAoControlChannel( );
	private:
		UINT aiInputChannel;
		UINT aoControlChannel;
		Control<CStatic> servoNameDisp;
		Control<CStatic> servoValueLabel;
		Control<DoubleEdit> servoValueEdit;
		Control<CStatic> aiInputDisp;
		Control<CStatic> aoOutputDisp;
		Control<CStatic> controlValueLabel;
		Control<CStatic> controlValueDisp;
};
