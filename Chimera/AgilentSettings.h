#pragma once
#include "constants.h"
#include "agilentStructures.h"

const agilentSettings UWAVE_AGILENT_SETTINGS = {
	// safemode option											
	UWAVE_SAFEMODE,
	// usb address
	UWAVE_AGILENT_USB_ADDRESS,
	// sample rate in hertz
	1e6,
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
	"Source1:burst:state off",  "Source2:burst:state off",
	"output1:load INF", "output2:load INF", 
	"SOURCE1:FUNC:ARB:FILTER Normal", "SOURCE2:FUNC:ARB:FILTER Normal" }
};

const agilentSettings TOP_BOTTOM_AGILENT_SETTINGS = { TOP_BOTTOM_AGILENT_SAFEMODE, TOP_BOTTOM_AGILENT_USB_ADDRESS,
 10e6, "INT", "Top_Bottom",
IDC_TOP_BOTTOM_CHANNEL1_BUTTON, IDC_TOP_BOTTOM_CHANNEL2_BUTTON,
IDC_TOP_BOTTOM_SYNC_BUTTON, IDC_TOP_BOTTOM_AGILENT_COMBO,
IDC_TOP_BOTTOM_FUNCTION_COMBO, IDC_TOP_BOTTOM_EDIT,
IDC_TOP_BOTTOM_PROGRAM, IDC_TOP_BOTTOM_CALIBRATION_BUTTON,
TOP_BOTTOM_AGILENT_TRIGGER_ROW, TOP_BOTTOM_AGILENT_TRIGGER_NUM,
"TOP_BOTTOM_AGILENT_AWG", { 8.01540821e-03, 3.43755634e-03, - 6.90550963e-05, 9.22870663e-07,
- 6.98004384e-09, 2.95332870e-11, - 6.25086083e-14, 2.61003740e-17,
1.29537083e-19, - 1.62637099e-22, - 5.22144118e-26, 5.41402046e-29,
- 1.72312386e-32, 1.69330554e-34, 2.33870036e-37, 4.52615707e-40,
- 1.38848265e-42, 1.09787051e-45, - 8.39836130e-49, - 1.59321641e-50,
2.31490207e-53, 2.24825783e-56, - 1.62818064e-59, - 5.99609844e-62,
4.87130426e-65 },
{ "Trigger:Source immediate", "Trigger:Slope Positive", "output1:load INF", "output2:load INF",
   "SOURCE1:FUNC:ARB:FILTER Normal", "SOURCE2:FUNC:ARB:FILTER Normal" } };

const agilentSettings AXIAL_AGILENT_SETTINGS = { AXIAL_AGILENT_SAFEMODE, AXIAL_AGILENT_USB_ADDRESS,
1e6, "INT", "Axial",
IDC_AXIAL_CHANNEL1_BUTTON, IDC_AXIAL_CHANNEL2_BUTTON,
IDC_AXIAL_SYNC_BUTTON, IDC_AXIAL_AGILENT_COMBO,
IDC_AXIAL_FUNCTION_COMBO, IDC_AXIAL_EDIT, IDC_AXIAL_PROGRAM,
IDC_AXIAL_CALIBRATION_BUTTON,
AXIAL_AGILENT_TRIGGER_ROW, AXIAL_AGILENT_TRIGGER_NUM,
"AXIAL_AGILENT_AWG",{ },
{ "Trigger:Source immediate", "Trigger:Slope Positive", "output1:load INF", "output2:load INF",
  "SOURCE1:FUNC:ARB:FILTER Normal", "SOURCE2:FUNC:ARB:FILTER Normal" }
};

const agilentSettings FLASHING_AGILENT_SETTINGS = { FLASHING_SAFEMODE, FLASHING_AGILENT_USB_ADDRESS,
1e6, "INT", "Flashing",
IDC_FLASHING_CHANNEL1_BUTTON, IDC_FLASHING_CHANNEL2_BUTTON,
IDC_FLASHING_SYNC_BUTTON, IDC_FLASHING_AGILENT_COMBO,
IDC_FLASHING_FUNCTION_COMBO, IDC_FLASHING_EDIT,
IDC_FLASHING_PROGRAM, IDC_FLASHING_CALIBRATION_BUTTON,
FLASHING_AGILENT_TRIGGER_ROW, FLASHING_AGILENT_TRIGGER_NUM,
"FLASHING_AGILENT_AWG",{ },
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
"output1 on", "output2 on", "*TRG", "output1:load INF", "output2:load INF", 
"SOURCE1:FUNC:ARB:FILTER Normal", "SOURCE2:FUNC:ARB:FILTER Normal" }
}; 


const agilentSettings INTENSITY_AGILENT_SETTINGS = { INTENSITY_SAFEMODE, INTENSITY_AGILENT_USB_ADDRESS,
													1e6, "USB", "Intensity",
													IDC_INTENSITY_CHANNEL1_BUTTON, IDC_INTENSITY_CHANNEL2_BUTTON,
													IDC_INTENSITY_SYNC_BUTTON, IDC_INTENSITY_AGILENT_COMBO,
													IDC_INTENSITY_FUNCTION_COMBO, IDC_INTENSITY_EDIT,
													IDC_INTENSITY_PROGRAM, IDC_INTENSITY_CALIBRATION_BUTTON,
													INTENSITY_AGILENT_TRIGGER_ROW, INTENSITY_AGILENT_TRIGGER_NUM, 
													"INTENSITY_AGILENT_AWG",{ 0.000505870656651, -0.0108632090621 },
													{ "Trigger:Source external", "Trigger:Slope Positive", 
													  "output1:load INF", "output2:load INF",
													  "SOURCE1:FUNC:ARB:FILTER Normal", "SOURCE2:FUNC:ARB:FILTER Normal" }
};
