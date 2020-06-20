#pragma once
#include <array>
#include <vector>

struct PhotoDetectorCalibrationInfo
{
	std::vector<double> atPdCalCoeff;
	std::vector<double> atAtomsCalCoeff;
};

// {0,1} means there's no calibration, it will just convert to power = 0 + 1 * volt
// here each row is for a given photodetector, and the leftmost column is to convert the voltage
// to the power at the photodetector itself while the second column is to convert the voltage
// to the power approximately seen by the atoms. Typically the two calibrations are identical
// up to a ratio which was measured.
// Powers are in mW, voltages are in volts.  Order of coefficients is in order of power of volts 
// they are to be multiplied by (i.e. 1st is multiplied by volts^0 i.e it's the constant offset.)
const std::array<PhotoDetectorCalibrationInfo, 8> AI_SYSTEM_CAL = { {
	{{0,1},{0,1}},	
	{{0.00039159, 0.09603802},{0.00231692, 0.56822492}},
	{{-0.00020468, 0.00733892},{-0.00226454,0.08119654}},
	{{-1.80307e-5,1.2433e-2},{-9.5305e-5,6.57155e-2}},
	{{0,1},{0,1}},
	{{0,1},{0,1}},
	{{0,1},{0,1}},
	{{0,1},{0,1}}} };
//old calibration for pd1, replaced 6/1/2020 {{-0.00011377,0.0999863},{-0.00085577, 0.75207086}},
