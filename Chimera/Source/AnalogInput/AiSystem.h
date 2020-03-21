// created by Mark O. Brown
#pragma once
#include "Control.h"
#include "CustomMfcControlWrappers/myButton.h"
#include "LowLevel/constants.h"
#include "AnalogInput/AiSettings.h"
#include "CustomMfcControlWrappers/DoubleEdit.h"
#include "CustomMfcControlWrappers/UintEdit.h"
#include "ConfigurationSystems/Version.h"
#include "afxwin.h"
#include "nidaqmx2.h"
#include <array>
#include "AnalogOutput/DaqMxFlume.h"


/*
 * This is a interface for taking analog input data through an NI card that uses DAQmx. These cards are generally 
 * somewhat flexible, but right now I only use it to read and record voltage values from Analog inputs.
 */
class AiSystem
{
	public:
		// THIS CLASS IS NOT COPYABLE.
		AiSystem& operator=(const AiSystem&) = delete;
		AiSystem (const AiSystem&) = delete;

		AiSystem( );
		AiSettings getAiSettings ();
		void initDaqmx( );
		void initialize( POINT& loc, CWnd* parent, int& id );
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
		static AiSettings getAiSettingsFromConfig (std::ifstream& file, Version ver);
		void AiSystem::handleSaveConfig (std::ofstream& file);
		const std::string configDelim{ "AI-SYSTEM" };
	private:
		Control<CStatic> title;
		std::array<Control<CStatic>, NUMBER_AI_CHANNELS> voltDisplays;
		std::array<Control<CStatic>, NUMBER_AI_CHANNELS> dacLabels;
		Control<CleanPush> getValuesButton;
		Control<CleanCheck> continuousQueryCheck;
		Control<CleanCheck> queryBetweenVariations;

		Control<DoubleEdit> continuousInterval;
		Control<CStatic> continuousIntervalLabel;

		Control<UintEdit> avgNumberEdit;
		Control<CStatic> avgNumberLabel;
		// float64 should just be a double type.
		std::array<float64, NUMBER_AI_CHANNELS> currentValues;
		std::vector<float64> aquisitionData;
		TaskHandle analogInTask0 = NULL;
		DaqMxFlume daqmx;
		const std::string boardName = "dev2";
};



