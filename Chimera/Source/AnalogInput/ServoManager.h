// created by Mark O. Brown
#pragma once
#include "servoInfo.h"
#include "CustomMfcControlWrappers/myButton.h"
#include "AnalogInput/AiSystem.h"
#include "AnalogOutput/AoSystem.h"
#include "DigitalOutput/DoSystem.h"
#include "CustomMfcControlWrappers/DoubleEdit.h"
#include "ParameterSystem/ParameterSystem.h"
#include "ConfigurationSystems/Version.h"
#include "CustomMfcControlWrappers/MyListCtrl.h"
#include "AiUnits.h"
#include <QLabel>
#include <QTableWidget>
#include <QPushButton>
#include <CustomQtControls/AutoNotifyCtrls.h>
#include <PrimaryWindows/IChimeraQtWindow.h>
/*
This is a slow digital DC servo system. As far as servos go, it is very primitive, just a Proportional servo with a low
gain, as this is all that's required for DC servoing. This is not designed to be run during the experiment, it's 
designed to run in between experiments in order to maintain 

The servo system interplays between three separate systems in the code.
- It uses the analog input system in order to readbtn a power level.
- It uses the analog output system, adjusting one of the dacs, in order to change the power level. In order to use the 
	AoSystem, it also uses the DioSystem.
- It saves the result in a parameter in the global parameterSystem that can be used in scripts.

- The options in an individual servo are saved in a Servo object.
*/


class ServoManager : public IChimeraSystem {
	public:
		// THIS CLASS IS NOT COPYABLE.
		ServoManager& operator=(const ServoManager&) = delete;
		ServoManager (const ServoManager&) = delete;
		ServoManager (IChimeraQtWindow* parent);
		
		void handleContextMenu (const QPoint& pos);
		static std::string servoTtlConfigToString (std::vector<std::pair<DoRows::which, unsigned> > ttlConfig);
		static std::string servoDacConfigToString (std::vector<std::pair<unsigned, double>> aoConfig);
		void initialize( POINT& pos, IChimeraQtWindow* parent, AiSystem* ai, AoSystem* ao, DoSystem* ttls_in,
						 ParameterSystem* globals_in);
		void setChangeVal (unsigned which, double change);
		void runAll ();
		void calibrate( servoInfo& s, unsigned which );
		bool wantsExpAutoServo ();
		double convertToPower (double volt, servoInfo& si);
		void handleSaveMasterConfig( std::stringstream& configStream );
		void handleOpenMasterConfig( ConfigStream& configStream );
		void setControlDisplay (unsigned which, double value );
		std::vector<servoInfo> getServoInfo ( );
		AiUnits::which getUnitsOption ();
		void refreshListview ();
	private:
		QLabel* servosHeader;
		CQPushButton* servoButton;
		CQCheckBox* expAutoServoButton;
		CQComboBox* unitsCombo;
		QTableWidget* servoList;
		QPushButton* cancelServo;
		const std::string servoSuffix = "__sv";
		void setResDisplay (unsigned which, double value);
		void handleSaveMasterConfigIndvServo ( std::stringstream& configStream, servoInfo& servo );
		servoInfo handleOpenMasterConfigIndvServo ( ConfigStream& configStream );
		
		std::vector<servoInfo> servos;
		void addServoToListview ( servoInfo& s );
		/*
		The manager gets pointers to the ai and ao system for hanndling the calibration process. It only gets the ttls
		to give to the ao system for changes. Probably should be a better way of doing this. 
		*/
		AiSystem* ai;
		AoSystem* ao;
		DoSystem* ttls;
		ParameterSystem* globals;
};



