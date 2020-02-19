// created by Mark O. Brown
#pragma once
#include "servoInfo.h"
#include "CustomMfcControlWrappers/myButton.h"
#include "AnalogInput/AiSystem.h"
#include "AnalogOutput/AoSystem.h"
#include "CustomMfcControlWrappers/DoubleEdit.h"
#include "ParameterSystem/ParameterSystem.h"
#include "ConfigurationSystems/Version.h"
#include "CustomMfcControlWrappers/MyListCtrl.h"
#include "ExperimentThread/Communicator.h"
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
		// THIS CLASS IS NOT COPYABLE.
		ServoManager& operator=(const ServoManager&) = delete;
		ServoManager (const ServoManager&) = delete;
		ServoManager () = default;
		
		static std::string servoTtlConfigToString (std::vector<std::pair<DioRows::which, UINT> > ttlConfig);
		static std::string servoDacConfigToString (std::vector<std::pair<UINT, double>> aoConfig);
		void initialize( POINT& pos, cToolTips& toolTips, CWnd* parent, int& id, AiSystem* ai, AoSystem* ao, 
						 DioSystem* ttls_in, ParameterSystem* globals_in );
		void handleDraw (NMHDR* pNMHDR, LRESULT* pResult);
		void setChangeVal (UINT which, double change);
		void rearrange( UINT width, UINT height, fontMap fonts );
		void runAll (Communicator& comm);
		void calibrate( servoInfo& s, UINT which );
		bool autoServo( );
		
		void handleSaveMasterConfig( std::stringstream& configStream );
		void handleOpenMasterConfig( std::stringstream& configStream, Version version );
		void setControlDisplay ( UINT which, double value );
		void handleListViewClick ( );
		void deleteServo ( );
		std::vector<servoInfo> getServoInfo ( );
	private:

		Control<CStatic> servosHeader;
		Control<CleanPush> servoButton;
		Control<CleanCheck> autoServoButton;
		void setResDisplay (UINT which, double value);
		void handleSaveMasterConfigIndvServo ( std::stringstream& configStream, servoInfo& servo );
		servoInfo handleOpenMasterConfigIndvServo ( std::stringstream& configStream, Version version );
		Control<MyListCtrl> servoList;
		std::vector<servoInfo> servos;
		void refreshListview ( );
		void addServoToListview ( servoInfo& s, UINT which );
		/*
		The manager gets pointers to the ai and ao system for hanndling the calibration process. It only gets the ttls
		to give to the ao system for changes.
		*/
		AiSystem* ai;
		AoSystem* ao;
		DioSystem* ttls;
		ParameterSystem* globals;
};

