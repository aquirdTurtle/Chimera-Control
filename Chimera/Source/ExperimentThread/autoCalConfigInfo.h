#pragma once
#include "ConfigurationSystems/profileSettings.h"
#include "expSystemRunList.h"
#include <string>


struct autoCalConfigInfo
{ 
	profileSettings prof; 
	expSystemRunList runList; 
	std::string fileName;
	std::string infoStr;  
}; 


const std::vector<autoCalConfigInfo> AUTO_CAL_LIST
{
	{
		/*prof*/ {"11x1_Atom_Loading_And_Imaging", "Atoms",
				  PROFILES_PATH + "Hotkey Experiments\\Atoms"},
		/*runlist*/ {true, true, true, false},
		/*filename*/ "BASIC_SINGLE_ATOMS",
		/*infostr*/ "Running Basic Single Atom Loading and Imaging Test...\r\n"
	},
	{
		/*prof*/ { "Mot_Size_Measurement", "MOT",
				   MOT_ROUTINES_ADDRESS },
		/*runlist*/ {false, true, false, true},
		/*filename*/ "MOT_NUMBER",
		/*infostr*/ "Running Mot Fill Calibration...\r\n"
	},
	{
		/*prof*/ {"Automated-MOT-Temperature-Measurement", 
				  "MOT", MOT_ROUTINES_ADDRESS},
		/*runlist*/ {false, true, false, true},
		/*filename*/ "MOT_TEMPERATURE",
		/*infostr*/ "Running Mot Temperature Calibration...\r\n"
	},
	{
		/*prof*/ {"Automated-PGC-Temperature-Measurement", 
				  "PGC", PGC_ROUTINES_ADDRESS},
		/*runlist*/ {false, true, false, true},
		/*filename*/ "RED_PGC_TEMPERATURE",
		/*infostr*/ "Running PGC Temperature Calibration...\r\n"
	},
	{
		/*prof*/ {"Automated-Grey-PGC-Temperature-Measurement", 
				  "PGC", PGC_ROUTINES_ADDRESS},
		/*runlist*/ {false, true, false, true},
		/*filename*/ "GREY_MOLASSES_TEMPERATURE",
		/*infostr*/ "Running Grey Molasses Temperature Calibration...\r\n"
	},
	{
		/*prof*/ {"Thermal_Radial_Sideband_Raman_Spectroscopy", "Atoms",
				  PROFILES_PATH + "Hotkey Experiments\\Atoms"},
		/*runlist*/ {true, true, true, false},
		/*filename*/ "THERMAL_RADIAL_SIDEBAND_RAMAN_SPECTROSCOPY",
		/*infostr*/ "Running Thermal Radial Raman Sideband Spectroscopy...\r\n"
	},
	{
		/*prof*/ { "3DSBC_Radial_Carrier_Raman_Spectroscopy", "Atoms",
				   PROFILES_PATH + "Hotkey Experiments\\Atoms" },
		/*runlist*/ { true, true, true, false },
		/*filename*/ "3DSBC_RADIAL_CARRIER_RAMAN_SPECTROSCOPY",
		/*infostr*/ "Running 3DSBC Radial Carrier Raman Spectroscopy...\r\n"
	},
	{
		/*prof*/ { "3DSBC_Axial_Raman_Spectroscopy", "Atoms",
				   PROFILES_PATH + "Hotkey Experiments\\Atoms" },
		/*runlist*/ { true, true, true, false },
		/*filename*/ "3DSBC_AXIAL_RAMAN_SPECTROSCOPY",
		/*infostr*/ "Running 3DSBC Axial Raman Spectroscopy...\r\n"
	},
	{
		/*prof*/ { "3DSBC_Radial_Sideband_Raman_Spectroscopy", "Atoms",
				   PROFILES_PATH + "Hotkey Experiments\\Atoms" },
		/*runlist*/ { true, true, true, false },
		/*filename*/ "3DSBC_RADIAL_SIDEBAND_RAMAN_SPECTROSCOPY",
		/*infostr*/ "Running 3DSBC Radial Raman Sideband Spectroscopy...\r\n"
	},
};
