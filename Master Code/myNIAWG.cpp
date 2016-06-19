#include "stdafx.h"
#include "myNIAWG.h"
namespace myNIAWG
{
	bool loadDefault()
	{
		/*
		// This is what was orirignally used here. 
		if (!TWEEZER_COMPUTER_SAFEMODE)
		{
		/// Set the default accordingly
		if (myNIAWG::NIAWG_CheckWindowsError(niFgen_ConfigureOutputEnabled(eSessionHandle, SESSION_CHANNELS, VI_FALSE)))
		{
		return true;
		}
		// Officially stop trying to generate anything.
		if (myNIAWG::NIAWG_CheckWindowsError(niFgen_AbortGeneration(eSessionHandle)))
		{
		return true;
		}
		// clear the memory
		if (myNIAWG::NIAWG_CheckWindowsError(niFgen_ClearArbMemory(eSessionHandle)))
		{
		return true;
		}
		}
		ViInt32 waveID;
		if (currentProfileSettings.orientation == HORIZONTAL_ORIENTATION)
		{
		if (!TWEEZER_COMPUTER_SAFEMODE)
		{
		// create waveform (necessary?)
		if (myNIAWG::NIAWG_CheckWindowsError(niFgen_CreateWaveformF64(eSessionHandle, SESSION_CHANNELS, eDefault_hConfigMixedSize, eDefault_hConfigMixedWaveform, &waveID)))
		{
		return true;
		}
		// allocate waveform into the device memory
		if (myNIAWG::NIAWG_CheckWindowsError(niFgen_AllocateNamedWaveform(eSessionHandle, SESSION_CHANNELS, eDefault_hConfigWaveformName.c_str(), eDefault_hConfigMixedSize / 2)))
		{
		return true;
		}
		// write named waveform. on the device. Now the device knows what "waveform0" refers to when it sees it in the script.
		if (myNIAWG::NIAWG_CheckWindowsError(niFgen_WriteNamedWaveformF64(eSessionHandle, SESSION_CHANNELS, eDefault_hConfigWaveformName.c_str(), eDefault_hConfigMixedSize, eDefault_hConfigMixedWaveform)))
		{
		return true;
		}
		// rewrite the script. default_hConfigScript should still be valid.
		if (myNIAWG::NIAWG_CheckWindowsError(niFgen_WriteScript(eSessionHandle, SESSION_CHANNELS, eDefault_hConfigScript)))
		{
		return true;
		}
		// start generic waveform to maintain power output to AOM.
		if (myNIAWG::NIAWG_CheckWindowsError(niFgen_ConfigureOutputEnabled(eSessionHandle, SESSION_CHANNELS, VI_TRUE)))
		{
		return true;
		}
		if (myNIAWG::NIAWG_CheckWindowsError(niFgen_SetAttributeViString(eSessionHandle, SESSION_CHANNELS, NIFGEN_ATTR_SCRIPT_TO_GENERATE, "DefaultHConfigScript")))
		{
		return true;
		}
		}
		eCurrentScript = "DefaultHConfigScript";
		}
		else if (currentProfileSettings.orientation == VERTICAL_ORIENTATION)
		{
		if (!TWEEZER_COMPUTER_SAFEMODE)
		{
		// create waveform (necessary?)
		if (myNIAWG::NIAWG_CheckWindowsError(niFgen_CreateWaveformF64(eSessionHandle, SESSION_CHANNELS, eDefault_vConfigMixedSize, eDefault_vConfigMixedWaveform, &waveID)))
		{
		return true;
		}
		// allocate waveform into the device memory
		if (myNIAWG::NIAWG_CheckWindowsError(niFgen_AllocateNamedWaveform(eSessionHandle, SESSION_CHANNELS, eDefault_vConfigWaveformName.c_str(), eDefault_vConfigMixedSize / 2)))
		{
		return true;
		}
		// write named waveform. on the device. Now the device knows what "waveform0" refers to when it sees it in the script.
		if (myNIAWG::NIAWG_CheckWindowsError(niFgen_WriteNamedWaveformF64(eSessionHandle, SESSION_CHANNELS, eDefault_vConfigWaveformName.c_str(), eDefault_vConfigMixedSize, eDefault_vConfigMixedWaveform)))
		{
		return true;
		}
		// rewrite the script. default_hConfigScript should still be valid.
		if (myNIAWG::NIAWG_CheckWindowsError(niFgen_WriteScript(eSessionHandle, SESSION_CHANNELS, eDefault_vConfigScript)))
		{
		return true;
		}
		// start generic waveform to maintain power output to AOM.
		if (myNIAWG::NIAWG_CheckWindowsError(niFgen_ConfigureOutputEnabled(eSessionHandle, SESSION_CHANNELS, VI_TRUE)))
		{
		return true;
		}
		if (myNIAWG::NIAWG_CheckWindowsError(niFgen_SetAttributeViString(eSessionHandle, SESSION_CHANNELS, NIFGEN_ATTR_SCRIPT_TO_GENERATE, "DefaultVConfigScript")))
		{
		return true;
		}
		}
		eCurrentScript = "DefaultVConfigScript";
		}
		if (!TWEEZER_COMPUTER_SAFEMODE)
		{
		// Initiate Generation.
		if (myNIAWG::NIAWG_CheckWindowsError(niFgen_InitiateGeneration(eSessionHandle)))
		{
		return true;
		}
		}
		*/
		return true;
	}
}
