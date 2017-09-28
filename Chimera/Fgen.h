#pragma once
#include "nifgen.h"
#include <string>

// This is my wrapper for NI_FGEN functionality. the NiawgController class owns one of these objects, any other 
// class could own a different one as well if they also need fgen.
class FgenFlume
{
	public:
		std::string getErrorMsg();
		/// wrappers around niFgen functions.

		void createWaveform( long size, ViReal64* wave );
		void writeUnNamedWaveform( ViInt32 waveID, ViInt32 mixedSampleNumber, ViReal64* wave );
		ViInt32 allocateUnNamedWaveform( ViInt32 unmixedSampleNumber );
		void clearMemory();
		void configureSoftwareTrigger( );
		void configureDigtalEdgeScriptTrigger( );
		signed short isDone();
		std::string getDeviceInfo();
		void configureOutputMode();
		void configureClockMode( ViInt32 clockMode );
		void configureMarker( ViConstString markerName, ViConstString outputLocation );
		void configureGain( ViReal64 gain );
		void configureChannels( );
		void configureSampleRate( ViReal64 sampleRate );
		void enableAnalogFilter( ViReal64 filterFrequency );
		void init( ViRsrc location, ViBoolean idQuery, ViBoolean resetDevice );

		void abortGeneration();
		void initiateGeneration();
		void configureOutputEnabled( int state );
		void errChecker( int err );
		void sendSoftwareTrigger();
		void deleteWaveform( ViConstString waveformName );
		void allocateNamedWaveform( ViConstString waveformName, ViInt32 unmixedSampleNumber );


		void writeNamedWaveform( ViConstString waveformName, ViInt32 mixedSampleNumber, ViReal64* wave );
		void resetWritePosition( );
		void writeScript( std::vector<ViChar> script );
		void deleteScript( ViConstString scriptName );
		std::string getCurrentScript();
		std::string getSoftwareTriggerName();
		std::string getExternalTriggerName();

		void setViInt32Attribute( ViAttr attributeID, ViInt32 value );
		void setViBooleanAttribute( ViAttr attribute, bool state );
		void setViStringAttribute( ViAttr atributeID, ViConstString attributeValue );
		void setAttributeViString( ViAttr attribute, ViString string );
		void setViReal64Attribute( ViAttr attribute, ViReal64 attributeVal, ViConstString channels = "" );
		ViInt32 getInt32Attribute( ViAttr attribute );
		ViInt64 getInt64Attribute( ViAttr attribute );
		ViReal64 getReal64Attribute( ViAttr attribute );
		std::string getViStringAttribute( ViAttr attribute );
		ViBoolean getViBoolAttribute( ViAttr attribute );
		ViSession getViSessionAttribute( ViAttr attribute );

	private:
		const ViConstString EXTERNAL_TRIGGER_NAME = "ScriptTrigger0";
		const ViConstString SOFTWARE_TRIGGER_NAME = "ScriptTrigger1";
		std::string currentScriptName;
		// right now these are basically all hard-set, mostly because only the niawg uses FGEN. if needed, could easily
	    // make an initialization function which initialized these for different devices.
		const ViConstString outputChannels = "0,1";
		ViSession sessionHandle;
		const ViInt32 OUTPUT_MODE = NIFGEN_VAL_OUTPUT_SCRIPT;
		// Minimum waveform size that the waveform generator can produce:
		const int MIN_WAVE_SAMPLE_SIZE = 100;
		const double MAX_CAP_TIME = 1e-3;
		const ViConstString TRIGGER_SOURCE = "PFI0";
		const ViInt32 TRIGGER_EDGE_TYPE = NIFGEN_VAL_RISING_EDGE;


};
