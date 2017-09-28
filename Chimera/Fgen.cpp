#include "stdafx.h"
#include "Fgen.h"

/// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///																NI Fgen wrappers
/// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string FgenFlume::getErrorMsg()
{
	ViStatus errorStat;
	ViChar* errMsg;
	int errMsgSize = 0;
	errMsgSize = niFgen_GetError( sessionHandle, VI_NULL, 0, VI_NULL );
	if (errMsgSize == 1)
	{
		return "No NIAWG Errors.";
	}
	errMsg = (ViChar *)malloc( sizeof( ViChar ) * errMsgSize );
	niFgen_GetError( sessionHandle, &errorStat, errMsgSize, errMsg );
	std::string errStr( errMsg );
	free( errMsg );
	return errStr;
}


void FgenFlume::sendSoftwareTrigger()
{
	if (!NIAWG_SAFEMODE)
	{
		errChecker( niFgen_SendSoftwareEdgeTrigger( sessionHandle, NIFGEN_VAL_SCRIPT_TRIGGER, SOFTWARE_TRIGGER_NAME ) );
	}
}


void FgenFlume::configureGain( ViReal64 gain )
{
	if (!NIAWG_SAFEMODE)
	{
		errChecker( niFgen_ConfigureGain( sessionHandle, outputChannels, gain ) );
	}
}


void FgenFlume::configureSampleRate( ViReal64 sampleRate )
{
	if (!NIAWG_SAFEMODE)
	{
		errChecker( niFgen_ConfigureSampleRate( sessionHandle, sampleRate ) );
	}
}

std::string FgenFlume::getDeviceInfo()
{
	// Use this section of code to output some characteristics of the 5451. If you want.

	ViInt32 maximumNumberofWaveforms = 0, waveformQuantum = 0, minimumWaveformSize = 0, maximumWaveformSize = 0;

	if (!NIAWG_SAFEMODE)
	{
		errChecker( niFgen_QueryArbWfmCapabilities( sessionHandle, &maximumNumberofWaveforms, &waveformQuantum,
													&minimumWaveformSize, &maximumWaveformSize ) );
	}

	return "Max # Of Waveforms: " + str( maximumNumberofWaveforms ) + "\n"
		"Waveform Quantum: " + str( waveformQuantum ) + "\n"
		"Minimum Waveform Size: " + str( minimumWaveformSize ) + "\n"
		"Maximum Waveform Size: " + str( maximumWaveformSize ) + "\n";
}


void FgenFlume::configureChannels()
{
	if (!NIAWG_SAFEMODE)
	{
		errChecker( niFgen_ConfigureChannels( sessionHandle, outputChannels ) );
	}
}


void FgenFlume::configureMarker( ViConstString markerName, ViConstString outputLocation )
{
	if (!NIAWG_SAFEMODE)
	{
		errChecker( niFgen_ExportSignal( sessionHandle, NIFGEN_VAL_MARKER_EVENT, markerName, outputLocation ) );
	}
}


// initialize the session handle, which is a member of this class.
void FgenFlume::init( ViRsrc location, ViBoolean idQuery, ViBoolean resetDevice )
{
	if (!NIAWG_SAFEMODE)
	{
		errChecker( niFgen_init( location, idQuery, resetDevice, &sessionHandle ) );
	}
}


// I think that this is only for arbitrary waveform output mode (single) or sequence mode, neither of which I use, and so this shouldn't
// appear in my code anywhere.
void FgenFlume::createWaveform( long size, ViReal64* wave )
{
	ViInt32 waveID;
	if (!NIAWG_SAFEMODE)
	{
		errChecker( niFgen_CreateWaveformF64( sessionHandle, outputChannels, size, wave, &waveID ) );
	}
}


void FgenFlume::writeUnNamedWaveform( ViInt32 waveID, ViInt32 mixedSampleNumber, ViReal64* wave )
{
	if (!NIAWG_SAFEMODE)
	{
		errChecker( niFgen_WriteWaveform( sessionHandle, outputChannels, waveID, mixedSampleNumber, wave ) );
	}
}

// niFgen_SetNamedWaveformNextWritePosition

// put waveform into the device memory
void FgenFlume::writeNamedWaveform( ViConstString waveformName, ViInt32 mixedSampleNumber, ViReal64* wave )
{
	if (!NIAWG_SAFEMODE)
	{
		errChecker( niFgen_WriteNamedWaveformF64( sessionHandle, outputChannels, waveformName, mixedSampleNumber, wave ) );
	}
}


void FgenFlume::writeScript( std::vector<ViChar> script )
{
	std::string temp( script.begin(), script.end() );
	ViConstString constScript = temp.c_str();
	if (!NIAWG_SAFEMODE)
	{
		errChecker( niFgen_WriteScript( sessionHandle, outputChannels, constScript ) );
	}
}


void FgenFlume::resetWritePosition( )
{
	if ( !NIAWG_SAFEMODE )
	{
		errChecker( niFgen_SetNamedWaveformNextWritePosition(sessionHandle, outputChannels, "rearrangeWaveform", 
															  NIFGEN_VAL_WAVEFORM_POSITION_START, 0 ));
	}
}


void FgenFlume::deleteWaveform( ViConstString waveformName )
{
	if (!NIAWG_SAFEMODE)
	{
		errChecker( niFgen_DeleteNamedWaveform( sessionHandle, outputChannels, waveformName ) );
	}
}


void FgenFlume::deleteScript( ViConstString scriptName )
{
	if (!NIAWG_SAFEMODE)
	{
		errChecker( niFgen_DeleteScript( sessionHandle, outputChannels, scriptName ) );
	}
}


void FgenFlume::allocateNamedWaveform( ViConstString waveformName, ViInt32 unmixedSampleNumber )
{
	if (!NIAWG_SAFEMODE)
	{
		errChecker( niFgen_AllocateNamedWaveform( sessionHandle, outputChannels, waveformName, unmixedSampleNumber ) );
	}
}


ViInt32 FgenFlume::allocateUnNamedWaveform( ViInt32 unmixedSampleNumber )
{
	ViInt32 id = 0;
	if (!NIAWG_SAFEMODE)
	{
		errChecker( niFgen_AllocateWaveform( sessionHandle, outputChannels, unmixedSampleNumber, &id ) );
	}
	return id;
}


void FgenFlume::configureOutputEnabled( int state )
{
	if (!NIAWG_SAFEMODE)
	{
		errChecker( niFgen_ConfigureOutputEnabled( sessionHandle, outputChannels, state ) );
	}
}


void FgenFlume::clearMemory()
{
	if (!NIAWG_SAFEMODE)
	{
		errChecker( niFgen_ClearArbMemory( sessionHandle ) );
	}
}

std::string FgenFlume::getCurrentScript()
{
	return currentScriptName;
}

void FgenFlume::setViStringAttribute( ViAttr atributeID, ViConstString attributeValue )
{
	// I keep track of this.
	currentScriptName = attributeValue;
	if (!NIAWG_SAFEMODE)
	{
		errChecker( niFgen_SetAttributeViString( sessionHandle, outputChannels, atributeID, attributeValue ) );
	}
}

void FgenFlume::setViReal64Attribute( ViAttr attribute, ViReal64 attributeVal, ViConstString channels)
{
	// handle default value
	if ( channels = "" )
	{
		channels = outputChannels;
	}

	if ( !NIAWG_SAFEMODE )
	{
		errChecker( niFgen_SetAttributeViReal64( sessionHandle, channels, attribute, attributeVal ) );
	}
}


void FgenFlume::setViBooleanAttribute( ViAttr attribute, bool state )
{
	if (!NIAWG_SAFEMODE)
	{
		errChecker( niFgen_SetAttributeViBoolean( sessionHandle, outputChannels, attribute, state ) );
	}
}


void FgenFlume::setViInt32Attribute( ViAttr attributeID, ViInt32 value )
{
	if (!NIAWG_SAFEMODE)
	{
		errChecker( niFgen_SetAttributeViInt32( sessionHandle, outputChannels, attributeID, value ) );
	}
}


void FgenFlume::enableAnalogFilter( ViReal64 filterFrequency )
{
	if (!NIAWG_SAFEMODE)
	{
		errChecker( niFgen_EnableAnalogFilter( sessionHandle, outputChannels, filterFrequency ) );
	}
}

std::string FgenFlume::getSoftwareTriggerName()
{
	return SOFTWARE_TRIGGER_NAME;
}


std::string FgenFlume::getExternalTriggerName()
{
	return EXTERNAL_TRIGGER_NAME;
}


void FgenFlume::configureSoftwareTrigger()
{
	if (!NIAWG_SAFEMODE)
	{
		errChecker( niFgen_ConfigureSoftwareEdgeScriptTrigger( sessionHandle, SOFTWARE_TRIGGER_NAME ) );
	}
}


signed short FgenFlume::isDone()
{
	ViBoolean isDone = 0;
	if (!NIAWG_SAFEMODE)
	{
		errChecker( niFgen_IsDone( sessionHandle, &isDone ) );
	}
	return isDone;
}



void FgenFlume::configureDigtalEdgeScriptTrigger()
{
	if (!NIAWG_SAFEMODE)
	{
		errChecker( niFgen_ConfigureDigitalEdgeScriptTrigger( sessionHandle, EXTERNAL_TRIGGER_NAME, TRIGGER_SOURCE, TRIGGER_EDGE_TYPE ) );
	}
}


void FgenFlume::configureClockMode( ViInt32 clockMode )
{
	if (!NIAWG_SAFEMODE)
	{
		errChecker( niFgen_ConfigureClockMode( sessionHandle, clockMode ) );
	}
}


void FgenFlume::initiateGeneration()
{
	if (!NIAWG_SAFEMODE)
	{
		errChecker( niFgen_InitiateGeneration( sessionHandle ) );
	}
}


void FgenFlume::abortGeneration()
{
	if (!NIAWG_SAFEMODE)
	{
		errChecker( niFgen_AbortGeneration( sessionHandle ) );
	}
}


void FgenFlume::setAttributeViString( ViAttr attribute, ViString string )
{
	if (!NIAWG_SAFEMODE)
	{
		errChecker( niFgen_SetAttributeViString( sessionHandle, outputChannels, attribute, string ) );
	}
}


ViInt32 FgenFlume::getInt32Attribute( ViAttr attribute )
{
	ViInt32 value = 0;
	if (!NIAWG_SAFEMODE)
	{
		errChecker( niFgen_GetAttributeViInt32( sessionHandle, outputChannels, attribute, &value ) );
	}
	return value;
}


ViInt64 FgenFlume::getInt64Attribute( ViAttr attribute )
{
	ViInt64 value = 0;
	if (!NIAWG_SAFEMODE)
	{
		errChecker( niFgen_GetAttributeViInt64( sessionHandle, outputChannels, attribute, &value ) );
	}
	return value;
}


ViReal64 FgenFlume::getReal64Attribute( ViAttr attribute )
{
	ViReal64 value = 0;
	if (!NIAWG_SAFEMODE)
	{
		errChecker( niFgen_GetAttributeViReal64( sessionHandle, outputChannels, attribute, &value ) );
	}
	return value;
}


std::string FgenFlume::getViStringAttribute( ViAttr attribute )
{
	ViChar value[256];
	if (!NIAWG_SAFEMODE)
	{
		errChecker( niFgen_GetAttributeViString( sessionHandle, outputChannels, attribute, 256, value ) );
	}
	return str( value );
}


ViBoolean FgenFlume::getViBoolAttribute( ViAttr attribute )
{
	ViBoolean value = false;
	if (!NIAWG_SAFEMODE)
	{
		errChecker( niFgen_GetAttributeViBoolean( sessionHandle, outputChannels, attribute, &value ) );
	}
	return value;
}


ViSession FgenFlume::getViSessionAttribute( ViAttr attribute )
{
	ViSession value = 0;
	if (!NIAWG_SAFEMODE)
	{
		errChecker( niFgen_GetAttributeViSession( sessionHandle, outputChannels, attribute, &value ) );
	}
	return value;
}


void FgenFlume::configureOutputMode()
{
	if (!NIAWG_SAFEMODE)
	{
		errChecker( niFgen_ConfigureOutputMode( sessionHandle, OUTPUT_MODE ) );
	}
}

void FgenFlume::errChecker( int err )
{
	if (err < 0)
	{
		thrower( "NIAWG Error: " + getErrorMsg() + "\r\n");
	}
}
