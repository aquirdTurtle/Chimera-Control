#pragma once
#include "ConfigurationSystems/profileSettings.h"
#include "expSystemRunList.h"
#include <string>

struct autoCalConfigInfo { 
	profileSettings prof; 
	std::string fileName;
	std::string infoStr;  
}; 

const std::vector<autoCalConfigInfo> AUTO_CAL_LIST {
	{
		/*prof*/ {"7x1_Atom_Loading_And_Imaging", "Atoms", PROFILES_PATH + "Standard_Calibrations\\Atoms"},
		/*filename*/ "BASIC_SINGLE_ATOMS",
		/*infostr*/ "Running Basic Single Atom Loading and Imaging Test...\r\n"
	},
	{
		/*prof*/ { "Mot_Size_Measurement", "MOT", MOT_ROUTINES_ADDRESS },
		/*filename*/ "MOT_NUMBER",
		/*infostr*/ "Running Mot Fill Calibration...\r\n"
	},
	{
		/*prof*/ {"Automated-MOT-Temperature-Measurement", "MOT", MOT_ROUTINES_ADDRESS},
		/*filename*/ "MOT_TEMPERATURE",
		/*infostr*/ "Running Mot Temperature Calibration...\r\n"
	},
	{
		/*prof*/ {"Automated-PGC-Temperature-Measurement", "PGC", PGC_ROUTINES_ADDRESS},
		/*filename*/ "RED_PGC_TEMPERATURE",
		/*infostr*/ "Running PGC Temperature Calibration...\r\n"
	},
	{
		/*prof*/ {"Automated-Grey-PGC-Temperature-Measurement", "PGC", PGC_ROUTINES_ADDRESS},
		/*filename*/ "GREY_MOLASSES_TEMPERATURE",
		/*infostr*/ "Running Grey Molasses Temperature Calibration...\r\n"
	},
	{
		/*prof*/ {"GPulse_Thermal_Top_Sideband_Raman_Spectroscopy", "Atoms", PROFILES_PATH + "Standard_Calibrations\\Atoms"},
		/*filename*/ "THERMAL_TOP_SIDEBAND_RAMAN_SPECTROSCOPY",
		/*infostr*/ "Running Thermal Top Raman Sideband Spectroscopy...\r\n"
	},
	{
		/*prof*/ { "3DGSBC_Carrier_Calibration", "Atoms", PROFILES_PATH + "Standard_Calibrations\\Atoms" },
		/*filename*/ "3DSBC_TOP_CARRIER_RAMAN_SPECTROSCOPY",
		/*infostr*/ "Running 3DSBC Top Carrier Raman Spectroscopy...\r\n"
	},
	{
		/*prof*/ { "3DGSBC_Axial_Sideband_Spectroscopy", "Atoms", PROFILES_PATH + "Standard_Calibrations\\Atoms" },
		/*filename*/ "3DSBC_AXIAL_RAMAN_SPECTROSCOPY",
		/*infostr*/ "Running 3DSBC Axial Raman Spectroscopy...\r\n"
	},
	{
		/*prof*/ { "3DGSBC_Top_Sideband_Spectroscopy", "Atoms", PROFILES_PATH + "Standard_Calibrations\\Atoms" },
		/*filename*/ "3DSBC_TOP_SIDEBAND_RAMAN_SPECTROSCOPY",
		/*infostr*/ "Running 3DSBC Top Raman Sideband Spectroscopy...\r\n"
	},
	{
		/*prof*/ { "Depth_Measurement_Deep", "Atoms", PROFILES_PATH + "Standard_Calibrations\\Atoms" },
		/*filename*/ "DEPTH_MEASUREMENT_DEEP",
		/*infostr*/ "Running Deep Depth Measurement...\r\n"
	},
	{
		/*prof*/ { "Depth_Measurement_Shallow", "Atoms", PROFILES_PATH + "Standard_Calibrations\\Atoms" },
		/*filename*/ "DEPTH_MEASUREMENT_SHALLOW",
		/*infostr*/ "Running Shallow Depth Measurement...\r\n"
	},
	{		
		/*prof*/ { "Lifetime_Measurement", "Atoms", PROFILES_PATH + "Standard_Calibrations\\Atoms" },
		/*filename*/ "LIFETIME_MEASUREMENT",
		/*infostr*/ "Running Lifetime Measurement...\r\n"
	},
	{
		/*prof*/ { "StandardTunneling", "Atoms", PROFILES_PATH + "Standard_Calibrations\\Atoms" },
		/*filename*/ "StandardTunneling",
		/*infostr*/ "Running Tunneling bias scan...\r\n"
	}
};
