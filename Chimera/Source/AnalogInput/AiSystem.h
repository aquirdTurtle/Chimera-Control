// created by Mark O. Brown
#pragma once
#include "Control.h"
#include "CustomMfcControlWrappers/myButton.h"
#include "LowLevel/constants.h"
#include "AnalogInput/AiSettings.h"
#include "GeneralObjects/IDeviceCore.h"
#include "CustomMfcControlWrappers/DoubleEdit.h"
#include "CustomMfcControlWrappers/UintEdit.h"
#include "ConfigurationSystems/Version.h"
#include "Scripts/ScriptStream.h"
#include "AnalogOutput/DaqMxFlume.h"
#include "ConfigurationSystems/ConfigStream.h"
#include "PrimaryWindows/IChimeraWindowWidget.h"
#include <QLabel.h>
#include <CustomQtControls/AutoNotifyCtrls.h>
#include "afxwin.h"
#include "nidaqmx2.h"
#include <array>


/*
 * This is a interface for taking analog input data through an NI card that uses DAQmx. These cards are generally 
 * somewhat flexible, but right now I only use it to readbtn and record voltage values from Analog inputs.
 */
class AiSystem : public IDeviceCore
{
	public:
		// THIS CLASS IS NOT COPYABLE.
		AiSystem& operator=(const AiSystem&) = delete;
		AiSystem (const AiSystem&) = delete;

		AiSystem( );
		AiSettings getAiSettings ();
		void initDaqmx( );
		void initialize( POINT& loc, IChimeraWindowWidget* parent );
		void refreshDisplays( );
		void rearrange( int width, int height, fontMap fonts );
		void refreshCurrentValues( );
		std::array<float64, NUMBER_AI_CHANNELS> getSingleSnapArray( UINT n_to_avg );
		std::vector<float64> getSingleSnap( UINT n_to_avg );
		double getSingleChannelValue( UINT chan, UINT n_to_avg );
		void armAquisition( UINT numSnapshots );
		void getAquisitionData( );
		std::vector<float64> getCurrentValues( );
		bool wantsQueryBetweenVariations( );
		bool wantsContinuousQuery( );
		std::string getSystemStatus( );
		void setAiSettings (AiSettings settings);
		AiSettings getSettingsFromConfig (ConfigStream& file);
		void handleSaveConfig (ConfigStream& file);
		const std::string configDelim{ "AI-SYSTEM" };
		std::string getDelim () { return configDelim; }
		void programVariation (UINT variation, std::vector<parameterType>& params) {};
		void calculateVariations (std::vector<parameterType>& params, ExpThreadWorker* threadworker) {};
		void loadExpSettings (ConfigStream& stream) {};

		void logSettings (DataLogger& log);
		void normalFinish () {};
		void errorFinish () {};
	private:
		QLabel* title;
		std::array<QLabel*, NUMBER_AI_CHANNELS> voltDisplays;
		std::array<QLabel*, NUMBER_AI_CHANNELS> dacLabels;
		CQPushButton* getValuesButton;
		CQCheckBox* continuousQueryCheck;
		CQCheckBox* queryBetweenVariations;

		CQLineEdit* continuousInterval;
		QLabel* continuousIntervalLabel;

		CQLineEdit* avgNumberEdit;
		QLabel* avgNumberLabel;
		// float64 should just be a double type.
		std::array<float64, NUMBER_AI_CHANNELS> currentValues;
		std::vector<float64> aquisitionData;
		TaskHandle analogInTask0 = NULL;
		DaqMxFlume daqmx;
		const std::string boardName = "dev2";
};



