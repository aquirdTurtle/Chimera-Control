#pragma once
#include "constants.h"
#include "agilentStructures.h"

const agilentSettings UWAVE_AGILENT_SETTINGS = {
	// safemode option											
	UWAVE_SAFEMODE,
	// usb address
	UWAVE_AGILENT_USB_ADDRESS,
	// sample rate (Hz)
	10e6,
	// impedance (ohms, or inf)
	"50",
	// the output filter state (controls rounding & overshoot issues 
	// between samples)
	"NORMal",
	// Memory location, whether the device will save waveforms to 
	// the internal 64MB Memory buffer or to an external USB drive, which
	// can (obviously) have much more space.
	"INT",
	// device name (just a convenience, so that the class instance knows 
	// which device it is
	"UWave",
	// various control IDs (no need to change)
	IDC_UWAVE_CHANNEL1_BUTTON, IDC_UWAVE_CHANNEL2_BUTTON,
	IDC_UWAVE_SYNC_BUTTON, IDC_UWAVE_AGILENT_COMBO,
	IDC_UWAVE_FUNCTION_COMBO, IDC_UWAVE_EDIT, IDC_UWAVE_PROGRAM,
	IDC_UWAVE_CALIBRATION_BUTTON,
	UWAVE_AGILENT_TRIGGER_ROW, UWAVE_AGILENT_TRIGGER_NUM,
	// Configuration file delimiter, used for saving settings for this 
	// agilent.
	"MICROWAVE_AGILENT_AWG",
	// Calibration coefficients (arb length)
	{ },
	{ "output1 off", "output2 off",
	"Source1:burst:state off", 
	"Source1:Apply:Square 2 MHz, 3 VPP, 1.5 V",
	"Source1:Function:Square:DCycle 42",
	"Source1:burst:mode triggered",
	"Source1:burst:ncycles Infinity", "Source1:burst:phase 0.0",
	"Source2:burst:state off", 
	"Source2:Apply:Square 2 MHz, 6.5 VPP, 0.9 V",
	"Source2:Function:Square:DCycle 50",
	"Source2:burst:mode triggered",
	"Source2:burst:ncycles Infinity", "Source2:burst:phase 280",
	"Trigger1:Source Bus", "Trigger1:Slope Positive",
	"Trigger2:Source Bus", "Trigger2:Slope Positive",
	"Source1:burst:state on", "Source2:burst:state on",
	"output1 on", "output2 on", "*TRG" }
};
//"SOURCE" + str(channel) + ":APPLY:SQUARE " + str( info.frequency ) + " KHZ, "
// + str ( convertPowerToSetPoint ( info.amplitude, info.useCalibration, calibrationCoefficients ) ) + " VPP, "
// + str ( convertPowerToSetPoint ( info.offset, info.useCalibration, calibrationCoefficients ) ) + " V"
const agilentSettings TOP_BOTTOM_AGILENT_SETTINGS = { TOP_BOTTOM_AGILENT_SAFEMODE, TOP_BOTTOM_AGILENT_USB_ADDRESS,
1e6, "INF", "NORMal", "INT", "Top_Bottom",
IDC_TOP_BOTTOM_CHANNEL1_BUTTON, IDC_TOP_BOTTOM_CHANNEL2_BUTTON,
IDC_TOP_BOTTOM_SYNC_BUTTON, IDC_TOP_BOTTOM_AGILENT_COMBO,
IDC_TOP_BOTTOM_FUNCTION_COMBO, IDC_TOP_BOTTOM_EDIT,
IDC_TOP_BOTTOM_PROGRAM, IDC_TOP_BOTTOM_CALIBRATION_BUTTON,
TOP_BOTTOM_AGILENT_TRIGGER_ROW, TOP_BOTTOM_AGILENT_TRIGGER_NUM,
"TOP_BOTTOM_AGILENT_AWG",{ },
{ "Trigger:Source immediate", "Trigger:Slope Positive" } };

const agilentSettings AXIAL_AGILENT_SETTINGS = { AXIAL_AGILENT_SAFEMODE, AXIAL_AGILENT_USB_ADDRESS,
1e6, "INF", "NORMal", "INT", "Axial",
IDC_AXIAL_CHANNEL1_BUTTON, IDC_AXIAL_CHANNEL2_BUTTON,
IDC_AXIAL_SYNC_BUTTON, IDC_AXIAL_AGILENT_COMBO,
IDC_AXIAL_FUNCTION_COMBO, IDC_AXIAL_EDIT, IDC_AXIAL_PROGRAM,
IDC_AXIAL_CALIBRATION_BUTTON,
AXIAL_AGILENT_TRIGGER_ROW, AXIAL_AGILENT_TRIGGER_NUM,
"AXIAL_AGILENT_AWG",{ },
{ "Trigger:Source immediate", "Trigger:Slope Positive" }
};

const agilentSettings FLASHING_AGILENT_SETTINGS = { FLASHING_SAFEMODE, FLASHING_AGILENT_USB_ADDRESS,
1e6, "INF", "NORMal", "INT", "Flashing",
IDC_FLASHING_CHANNEL1_BUTTON, IDC_FLASHING_CHANNEL2_BUTTON,
IDC_FLASHING_SYNC_BUTTON, IDC_FLASHING_AGILENT_COMBO,
IDC_FLASHING_FUNCTION_COMBO, IDC_FLASHING_EDIT,
IDC_FLASHING_PROGRAM, IDC_FLASHING_CALIBRATION_BUTTON,
FLASHING_AGILENT_TRIGGER_ROW, FLASHING_AGILENT_TRIGGER_NUM,
"FLASHING_AGILENT_AWG",{ },
{ "Source1:Burst:state off", "Source1:Burst:mode: triggered",
"Source1:Burst:ncycles 1", "Source1:Burst:phase 0.0",
"Source2:Burst:state off", "Source2:Burst:mode: triggered",
"Source2:Burst:ncycles 1", "Source2:Burst:phase 280",
"Trigger:Source immediate", "Trigger:Slope Positive" }
};

const agilentSettings INTENSITY_AGILENT_SETTINGS = { INTENSITY_SAFEMODE, INTENSITY_AGILENT_USB_ADDRESS,
1e6, "INF", "NORMal", "USB", "Intensity",
IDC_INTENSITY_CHANNEL1_BUTTON, IDC_INTENSITY_CHANNEL2_BUTTON,
IDC_INTENSITY_SYNC_BUTTON, IDC_INTENSITY_AGILENT_COMBO,
IDC_INTENSITY_FUNCTION_COMBO, IDC_INTENSITY_EDIT,
IDC_INTENSITY_PROGRAM, IDC_INTENSITY_CALIBRATION_BUTTON,
INTENSITY_AGILENT_TRIGGER_ROW, INTENSITY_AGILENT_TRIGGER_NUM,
"INTENSITY_AGILENT_AWG",{ 0.000505870656651, -0.0108632090621 },
{ "Trigger:Source external", "Trigger:Slope Positive" }
};
