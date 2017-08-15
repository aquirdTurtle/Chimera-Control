#pragma once

#include "stdafx.h"
#include "niFgen.h"
#include <string>

// running in safemode means that the program doesn't actually try to connect to various devices. It can be used to build and debug other 
// aspects of the program.
#define MASTER_COMPUTER
//#define SPECTRE_LAPTOP
// #define DESKTOP_COMPUTER
/// File Locations
// Files for Desktop
#ifdef DESKTOP_COMPUTER
	#define PYTHON_HOME L"C:\\Users\\Mark\\Anaconda3\\"
	#define NIAWG_SAFEMODE false
	#define ANDOR_SAFEMODE true
	#define SOCKET_SAFEMODE true
	#define CONNECT_TO_ANDOR_SAFEMODE true
	const std::string PLOT_FILES_SAVE_LOCATION = "C:\\Users\\Mark\\Documents\\Quantum Gas Assembly Control\\Plotting";
	const std::string ANALYSIS_CODE_LOCATION = "C:\\\\Users\\\\Mark\\\\Documents\\\\Data-Analysis";
	const std::string LIB_PATH = "C:\\Users\\Mark\\Documents\\Quantum Gas Assembly Control\\NIAWG Wavefunction Library"
								 "\\";
	const std::string DEFAULT_SCRIPT_FOLDER_PATH = "C:\\Users\\Mark\\Documents\\Quantum Gas Assembly Control\\Default "
												   "NIAWG Scripts\\";
	const std::string EXPERIMENT_LOGGING_FILES_PATH = "C:\\Users\\Mark\\Documents\\Quantum Gas Assembly Control\\";
	const std::string CODE_LOGGING_FILES_PATH = "\\\\andor\\share\\Data and documents\\Data repository\\NIAWG Logging Files\\Code Versions\\";
	const std::string ACTUAL_CODE_FOLDER_PATH = "C:\\Users\\Mark\\Documents\\Quantum Gas Assembly Control\\Cold Atoms Control Application\\";
	const std::string PROFILES_PATH = "C:\\Users\\Mark\\Documents\\Quantum Gas Assembly Control\\Profiles\\";
#endif

#ifdef SPECTRE_LAPTOP
	#define PYTHON_HOME L"C:\\Program Files (x86)\\Anaconda3\\"
	#define NIAWG_SAFEMODE true
	#define ANDOR_SAFEMODE true
	#define SOCKET_SAFEMODE true
	#define CONNECT_TO_ANDOR_SAFEMODE true
	#define PYTHON_SAFEMODE true
	const std::string PLOT_FILES_SAVE_LOCATION = "C:\\Users\\Mark\\Documents\\Quantum-Gas-Assembly-Control\\Plotting";
	const std::string ANALYSIS_CODE_LOCATION = "C:\\\\Users\\\\Mark\\\\Documents\\\\Data-Analysis";
	const std::string LIB_PATH = "C:\\Users\\Mark\\Documents\\Quantum-Gas-Assembly-Control\\Waveforms Library\\";
	const std::string DEFAULT_SCRIPT_FOLDER_PATH = "C:\\Users\\Mark\\Documents\\Quantum-Gas-Assembly-Control\\Default Scripts\\";
	const std::string EXPERIMENT_LOGGING_FILES_PATH = "C:\\Users\\Mark\\Documents\\Quantum-Gas-Assembly-Control\\NIAWG Logging Files\\"
														"Individual Experiments\\";
	const std::string CODE_LOGGING_FILES_PATH = "\\\\andor\\share\\Data and documents\\Data repository\\NIAWG Logging Files\\Code Versions\\";
	const std::string ACTUAL_CODE_FOLDER_PATH = "C:\\Users\\Mark\\Documents\\Quantum-Gas-Assembly-Control\\Cold Atoms Control Application\\";
	const std::string PROFILES_PATH = "C:\\Users\\Mark\\Documents\\Quantum-Gas-Assembly-Control\\Profiles\\";
	const std::string DATA_SAVE_LOCATION = "C:\\Users\\Mark\\Documents\\Quantum-Gas-Assembly-Control\\Data\\";
	const std::string KEY_ORIGINAL_SAVE_LOCATION = "C:\\Users\\Mark\\Documents\\Quantum-Gas-Assembly-Control\\key.txt";
	const std::string MUSIC_LOCATION = "C:\\Users\\Mark\\Documents\\Quantum-Gas-Assembly-Control\\Camerawg\\Final Fantasy VII - Victory Fanfare [HQ].mp3";
	//
	#define DIO_SAFEMODE true
	#define DAQMX_SAFEMODE true
	#define GPIB_SAFEMODE true
	#define AGILENT_SAFEMODE true
	#define PROFILES_PATH "C:\\Users\\Mark\\Documents\\Quantum-Gas-Assembly-Control\\Master Profiles\\"
	#define FUNCTIONS_FOLDER_LOCATION "C:\\Users\\Mark\\Documents\\Quantum-Gas-Assembly-Control\\Functions\\"
	#define MASTER_CONFIGURATION_FILE_ADDRESS "C:\\Users\\Mark\\Documents\\Quantum-Gas-Assembly-Control\\Master-Configuration.txt"
	#define DEFAULT_SCRIPT_FOLDER_PATH "C:\\Users\\Mark\\Documents\\Quantum-Gas-Assembly-Control\\Default Scripts\\"
	#define LOGGING_FILE_ADDRESS "C:\\Users\\Mark\\Documents\\Quantum-Gas-Assembly-Control\\Master Log.txt"
	#define KEY_ADDRESS "C:\\Users\\Mark\\Documents\\Quantum-Gas-Assembly-Control\\Data\\key.txt"
	#define MOT_ROUTINE_ADDRESS "C:\\Users\\Mark\\Documents\\Quantum-Gas-Assembly-Control\\Master Profiles\\Hotkey Experiments\\MOT\\turnOnMot.mScript"
	#define DEBUG_OUTPUT_LOCATION "C:\\Users\\Mark\\Documents\\Quantum-Gas-Assembly-Control\\Debug-Output\\"
#endif

#ifdef MASTER_COMPUTER
	#define SOCKET_SAFEMODE true
	#define NIAWG_SAFEMODE false
	#define ANDOR_SAFEMODE false
	#define CONNECT_TO_ANDOR_SAFEMODE false
	#define PYTHON_HOME L"C:\\Program Files (x86)\Anaconda3\\"
	#define PYTHON_SAFEMODE true
	#define DIO_SAFEMODE true
	#define DAQMX_SAFEMODE false
	#define GPIB_SAFEMODE false
	#define AGILENT_SAFEMODE false

	const std::string CODE_ROOT = "C:\\Users\\Regal Lab\\Documents\\Chimera-Control\\Quantum-Gas-Assembly-Control";
	const std::string PLOT_FILES_SAVE_LOCATION = CODE_ROOT + "\\Plotting";
	const std::string ANALYSIS_CODE_LOCATION = "C:\\\\Users\\\\Mark\\\\Documents\\\\Data-Analysis";
	const std::string LIB_PATH = CODE_ROOT + "\\Waveforms Library\\WaveLibrary 350 MSpS\\";
	const std::string DEFAULT_SCRIPT_FOLDER_PATH = CODE_ROOT + "\\Default Scripts\\";
	const std::string EXPERIMENT_LOGGING_FILES_PATH = "D:\\NIAWG Logging Files\\Individual Experiments\\";
	const std::string CODE_LOGGING_FILES_PATH = "D:\\Code Versions\\";
	const std::string ACTUAL_CODE_FOLDER_PATH = CODE_ROOT + "\\Chimera\\";
	const std::string PROFILES_PATH = CODE_ROOT + "\\Profiles\\";
	const std::string DATA_SAVE_LOCATION = "J:\\Data Repository\\New Data Repository\\";
	const std::string KEY_ORIGINAL_SAVE_LOCATION = "C:\\Shared\\";
	const std::string MUSIC_LOCATION = CODE_ROOT + "\\Final Fantasy VII - Victory Fanfare [HQ].mp3";
	const std::string FUNCTIONS_FOLDER_LOCATION = CODE_ROOT + "\\Functions\\";
	const std::string MASTER_CONFIGURATION_FILE_ADDRESS = CODE_ROOT + "\\Master-Configuration.txt";
	const std::string LOGGING_FILE_ADDRESS = CODE_ROOT + "\\Master-Log.txt";
	const std::string KEY_ADDRESS = "\\\\Callisto\\Shared\\key.txt";
	const std::string MOT_ROUTINE_ADDRESS = CODE_ROOT + "\\Master Profiles\\Hotkey Experiments\\MOT\\turnOnMot.mScript";
	const std::string DEBUG_OUTPUT_LOCATION = CODE_ROOT + "\\Debug-Output\\";

#endif

// We calibrated this. // GAIN = 1.34.
#define GAIN 1.34
// GAIN = 1.34.

#define MAX_NIAWG_SIGNALS 32

// This is the minimum time (in seconds) that a correction waveform will be allowed to have. Without this, the algorithm might decide that the 1/2 of a period 
// of a waveform might be enough, even though this would probably be far below the amount of time an individual waveform is allowed to have according to the 
// NIAWG (the NIAWG has a minimum waveform size).
// Initial Value: 0.00000025 (250 ns)
#define MIN_CORRECTION_WAVEFORM_TIME 0.00000025
// this is a time in seconds.
#define MAX_CORRECTION_WAVEFORM_TIME 0.001
// this is a phase in radians.
#define CORRECTION_WAVEFORM_GOAL 0.001
// this is a phase in radians.
#define CORRECTION_WAVEFORM_ERROR_THRESHOLD 0.5
// Typically 0. We typically have this plugged into a DC Block anyways.
#define DC_OFFSET 0
// in hertz
#define NIAWG_FILTER_FREQENCY 80000000

#define HORIZONTAL_ORIENTATION "Horizontal"
#define VERTICAL_ORIENTATION "Vertical"
#define NULL_SEQUENCE "NULL SEQUENCE"

#define HORIZONTAL_EXTENSION ".hConfig"
#define VERTICAL_EXTENSION ".vConfig"
#define EXPERIMENT_EXTENSION ".eConfig"
#define CATEGORY_EXTENSION ".catConfig"
#define SEQUENCE_EXTENSION ".sConfig"

#define AGILENT_SCRIPT_EXTENSION ".aScript"
#define NIAWG_SCRIPT_EXTENSION ".nScript"

//
#define PICTURE_PALETTE_SIZE 256
#define RAW_COUNTS "Raw Counts"
#define CAMERA_PHOTONS "Camera Photons"
#define ATOM_PHOTONS "Atom Photons"


/// Agilent Parameters
// sample rate is typically 1 MS/s.
#define AGILENT_SAMPLE_RATE 1000000
#define AGILENT_FILTER_STATE "OFF"
#define AGILENT_LOAD "INF"
#define AGILENT_DEFAULT_DC "2.383 V"
#define NUMBER_OF_LIBRARY_FILES MAX_NIAWG_SIGNALS*4

/// Random other Constants
// It's PI.
#define PI 3.14159265358979323846264338327950288
// This is the length of time waited before the code decides to update your syntax coloring after a change is made.
#define SYNTAX_TIMER_LENGTH 1000

// intensity agilent address
//const char * const AGILENT_ADDRESS = "USB0::0x0957::0x2307::MY50004500::0::INSTR";
const char * const AGILENT_ADDRESS = "USB0::2391::8967::MY50004500::0::INSTR";

// Parameters that the user might want to change:
const bool CONST_POWER_OUTPUT = true;

// for floating point error... 0.2 was picked because this is the maximum power that can be outputted with 5 signals given the voltage 
// restrictions.
// TEMP: need to change back to 0.2
const double TOTAL_POWER = 0.2 - 1e-10;

// Currently bugs with the 5451 for sample rates significantly above this sample rate (350 MS/s). <---- very much T.T T.T T.T
/// IF CHANGE MAKE SURE TO CHANGE LIBRARY FILE ADDRESS !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
const long int SAMPLE_RATE = 350000000; /// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
/// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
/// This is important. If you don't change the libary file, you will be running waveforms that were compiled with a different sample rate. 
/// This would probably cause the code to throw weird errors since the number of samples in the file wouldn't match what was needed at the 
/// given sample rate.

/// Communication Parameters for communication with master computer.
#define DEFAULT_PORT "10010"
const char * const SERVER_ADDRESS = "192.168.236.1";

/// Control IDs ///
// All or near all of these #defines are specifically chosen to match the id's of corresponding controls that I need
// to watch for input. Mostly this is for message maps for the various windows mentioned below. The exact value here
// doesn't matter at all, it just matters that it matches the corresponding control ID. Around the ID definitions for
// the controls of interest, there are throw statements that stop the program early on if an ID doesn't match, so 
// you don't have to worry about these things so much.

// Main Window
#define IDC_MAIN_STATUS_BUTTON 1001
#define IDC_ERROR_STATUS_BUTTON 1004
#define IDC_DEBUG_STATUS_BUTTON 1007
#define IDC_EXPERIMENT_COMBO 1013
#define IDC_CATEGORY_COMBO 1014
#define IDC_ORIENTATION_COMBO 1018
#define IDC_CONFIGURATION_COMBO 1019
#define IDC_SEQUENCE_COMBO 1021
#define IDC_MAIN_OPTIONS_RANGE_BEGIN 1034
#define IDC_MAIN_OPTIONS_RANGE_END 1034
#define IDC_DEBUG_OPTIONS_RANGE_BEGIN 1036
#define IDC_DEBUG_OPTIONS_RANGE_END 1042
#define IDC_SHOW_TTLS 1043
#define IDC_SHOW_DACS 1044
#define IDC_SMS_TEXTING_LISTVIEW 1048
// Scripting Window
#define SYNTAX_TIMER_ID 1999
#define IDC_VERTICAL_NIAWG_FUNCTION_COMBO 2004
#define IDC_VERTICAL_NIAWG_EDIT 2005
#define IDC_HORIZONTAL_NIAWG_FUNCTION_COMBO 2010
#define IDC_HORIZONTAL_NIAWG_EDIT 2011
#define IDC_INTENSITY_FUNCTION_COMBO 2016
#define IDC_INTENSITY_EDIT 2017
#define IDC_MASTER_FUNCTION_COMBO 2022
#define IDC_MASTER_EDIT 2023

// Camera Window
#define IDC_ALERTS_BOX 3005
#define IDC_SET_ANALYSIS_LOCATIONS 3012
#define IDC_PLOTTING_LISTVIEW 3016
#define IDC_CAMERA_MODE_COMBO 3018
#define IDC_SET_EM_GAIN_BUTTON 3019
#define IDC_TRIGGER_COMBO 3022
#define IDC_SET_TEMPERATURE_BUTTON 3023
#define PICTURE_SETTINGS_ID_START 3028
#define PICTURE_SETTINGS_ID_END 3063
#define IDC_SET_IMAGE_PARAMETERS_BUTTON 3064
#define IDC_SET_REPETITONS_PER_VARIATION_BUTTON 3077
#define IDC_SET_VARIATION_NUMBER 3080
#define IDC_PICTURE_1_MIN_EDIT 3114
#define IDC_PICTURE_1_MAX_EDIT 3117
#define IDC_PICTURE_2_MIN_EDIT 3120
#define IDC_PICTURE_2_MAX_EDIT 3123
#define IDC_PICTURE_3_MIN_EDIT 3126
#define IDC_PICTURE_3_MAX_EDIT 3129
#define IDC_PICTURE_4_MIN_EDIT 3132
#define IDC_PICTURE_4_MAX_EDIT 3135

//
#define IDC_BEGINNING_DIALOG_RICH_EDIT 100
//

#define SYNTAX_TIMER_ID 500
#define SYNTAX_TIMER_LENGTH 1000

/// Device Window
//#define EXPERIMENT_COMBO_ID 4004
//#define CATEGORY_COMBO_ID 4005
//#define ORIENTATION_COMBO_ID 4009
//#define CONFIGURATION_COMBO_ID 4010
//#define SEQUENCE_COMBO_ID 4013

#define SET_REPETITION_ID 4019
#define TTL_HOLD 4005
#define IDC_ZERO_TTLS 4006
#define TTL_ID_BEGIN 4027
#define IDC_SEQUENCE_DISPLAY 4093
#define TTL_ID_END 4090
#define ID_DAC_SET_BUTTON 4092
#define IDC_ZERO_DACS 4093
#define ID_DAC_FIRST_EDIT 4094
#define ID_STATUS_CLEAR 4160
#define ID_ERROR_CLEAR 4163
#define EXPERIMENT_NOTES_ID 4203
#define CATEGORY_NOTES_ID 4205
#define CONFIGURATION_NOTES_ID 4207
//
#define IDC_TOP_BOTTOM_CHANNEL1_BUTTON 4184
#define IDC_TOP_BOTTOM_CHANNEL2_BUTTON 4185
#define IDC_TOP_BOTTOM_SYNC_BUTTON 4186
#define IDC_TOP_BOTTOM_AGILENT_COMBO 4187
#define IDC_AXIAL_UWAVE_CHANNEL1_BUTTON 4192
#define IDC_AXIAL_UWAVE_CHANNEL2_BUTTON 4193
#define IDC_AXIAL_UWAVE_SYNC_BUTTON 4194
#define IDC_AXIAL_UWAVE_AGILENT_COMBO 4195
#define IDC_FLASHING_CHANNEL1_BUTTON 4200
#define IDC_FLASHING_CHANNEL2_BUTTON 4201
#define IDC_FLASHING_SYNC_BUTTON 4202
#define IDC_FLASHING_AGILENT_COMBO 4203
#define IDC_GLOBAL_VARS_LISTVIEW 4207
#define IDC_CONFIG_VARS_LISTVIEW 4209
#define TOP_ON_OFF 4146
#define TOP_FSK 4147
#define BOTTOM_ON_OFF 4152
#define BOTTOM_FSK 4153
#define EO_ON_OFF 4164
#define EO_FSK 4165
#define AXIAL_ON_OFF 4170
#define AXIAL_FSK 4171

#define IDC_GENERAL_PLOT_TYPE 5008

#define IDC_PLOT_CREATOR_DATASET_COMBO 5010
#define IDC_PRC_PICTURE_NUMBER 5014
#define IDC_PRC_PIXEL_NUMBER 5016
#define IDC_PRC_ATOM 5017
#define IDC_PRC_NOATOM 5018
#define IDC_PRC_SHOW_ALL 5019

#define IDC_PIXELS_PER_ANALYSIS_GROUP 5029

#define IDC_PSC_CONDITION_NUMBER 5038
#define IDC_PSC_PICTURE_NUMBER 5040
#define IDC_PSC_PIXEL_NUMBER 5042
#define IDC_PSC_ATOM 5043
#define IDC_PSC_NOATOM 5044
#define IDC_PSC_SHOW_ALL 5045
#define IDC_PLOT_CREATOR_SAVE 5046
#define IDC_PLOT_CREATOR_CANCEL 5047



#define NO_FIT 12002
#define REAL_TIME_FIT 12003
#define FIT_AT_END 12004
#define GAUSSIAN_FIT 12005
#define LORENTZIAN_FIT 12006
#define SINE_FIT 12007

#define MASTER_SCRIPT_EXTENSION ".mScript"
#define NIAWG_SCRIPT_EXTENSION ".nScript"
#define AGILENT_SCRIPT_EXTENSION ".aScript"
#define HORIZONTAL_EXTENSION ".hConfig"
#define VERTICAL_EXTENSION ".vConfig"
#define EXPERIMENT_EXTENSION ".eConfig"
#define CATEGORY_EXTENSION ".catConfig"
#define SEQUENCE_EXTENSION ".sConfig"
#define FUNCTION_EXTENSION ".func"
#define PLOTTING_EXTENSION ".plot"

#define OSCILLOSCOPE_TRIGGER "C11"
#define HORIZONTAL_ORIENTATION "Horizontal"
#define VERTICAL_ORIENTATION "Vertical"

#define NULL_SEQUENCE "NULL_SEQUENCE"

#define AGILENT_SAMPLE_RATE 1000000
#define AGILENT_FILTER_STATE "OFF"
#define AGILENT_LOAD "INF"
#define AGILENT_DEFAULT_DC "2.383 V"
#define AGILENT_ADDRESS "USB0::0x0957::0x2307::MY50004500::0::INSTR"

#define RSG_ADDRESS 28
#define RSG_POWER 10
#define TEKTRONICS_AFG_1_ADDRESS 25
#define TEKTRONICS_AFG_2_ADDRESS 24






// Don't change! Or only change if you really kow what you are doing. Changing this risks overloading our RF amplifiers 
// and large-aperture AOMs. Current Value: 5
const float MAX_GAIN = 5.0; // Current Value: 5
							// Current Value: 5
// Long... array...
const char SCRIPT_INFO_TEXT[] =
"/***********************************************************************\\\r\n"
" NI 5451 ARBITRARY WAVEFORM GENERATOR INSTRUCTIONS FILE FORMATTING GUIDE\r\n"
"\\*********************************************************************** /\r\n"
"\r\n"
"\r\n"
" The instructions file consists a series of commands for the waveform, and includes commands like generating\r\n"
" constant waveforms, generating ramping waveforms, simple logic statements, repeat statements, triggers, and "
" markers.\r\n"
" \r\n"
" In each case, if a command requires input(for example, the frequency and amplitude of a waveform), the input\r\n"
" format is referenced below using angled brackets <...>. Place the input on the line below the command in the\r\n"
" format specified.\r\n"
"\r\n"
" You can use the waveform commands with or without(if you are already familiar with the format) including the\r\n"
" bracketed formatting instructions as well(e.g. \"gen 1 const\" instead of \"gen 1 const <freq> <amp> | <time>\").\r\n"
"\r\n"
" The ramping type options are currently \"lin\", \"tanh\" and \"nr\".\r\n"
"\r\n"
" The numbers indicated(e.g.gen / 3 / consts) refer to the number of different frequencies that command can take\r\n"
" as input. The other part of the command (e.g. \"freq ramp\") refer to the ramping options that the command is capable\r\n"
" of taking as input.If you want to generate 3 frequencies, and do an amplitude ramp on only one of them, then\r\n"
" use \"gen 3 amp ramps\" command, then say \"Linear\" on the frequency that you want to ramp, and \"NoRamp\" on the\r\n"
" others.\r\n"
"\r\n"
" The associated c++ code has been designed to be flexible when it comes to trailing white spaces at the ends of\r\n"
" lines and in between commands, so use whatever such formatting pleases your eyes. Do not, however, include extra\r\n"
" characters anywhere.Make sure that the command text is copied exactly, or the program will throw an error, tell\r\n"
" you that it doesn't recognize the inputed character, and exit without running the waveform.\r\n"
"\r\n"

"**********************************\r\n"
"Commands(display below settings) :\r\n"
"**********************************\r\n"
"(a)Wait Commands\r\n"
"\"wait until trigger\"\r\n"
"\"wait set # <# of samples to wait>\"\r\n"
"\r\n"
"(b)Repeat Commands\r\n"
"\"repeat set # <# of times to repeat>\"\r\n"
"\"repeat until trigger\"\r\n"
"\"repeat forever\"\r\n"
"\"end repeat\"\r\n"
"\r\n"
"(c)Logic Commands\r\n"
"\"if trigger\"\r\n"
"\"else\"\r\n"
"\"end if\"\r\n"
"\r\n"
"(d)Constant Waveforms\r\n"
"\"gen 1, const <freq> <amp> <phase (rad)>; <time> <t manage>\"\r\n"
"\"gen 2, const <freq1> <amp1> <phase1 (rad)>; <sim for 2nd>; <time> <t manage>\"\r\n"
"Etc.\r\n"
"\r\n"
"(e)Amplitude Ramps\r\n"
"\"gen 1, amp ramp <freq> <amp ramp type> <initial amp> <final amp> <phase (rad)>; <time> <t manage>\"\r\n"
"\"gen 2, amp ramp <freq1> <amp1 ramp type> <initial amp1> <final amp1> <phase1 (rad)>; <sim for 2nd>; <time> <t manage>\"\r\n"
"Etc.\r\n"
"\r\n"
"(f)frequency Ramps\r\n"
"\"gen 1, freq ramp <freq ramp type> <initial freq> <final freq> <amp> <phase (rad)>; <time> <t manage>\"\r\n"
"\"gen 2, freq ramp <freq1 ramp type> <initial freq1> <final freq1> <amp1> <phase1 (rad)>; <sim for 2nd>; <time> <t manage>\"\r\n"
"Etc.\r\n"
"\r\n"
"(g)Amplitude and Frequency Ramps\r\n"
"\"gen 1, freq & amp ramp <freq ramp type> <initial freq> <final freq> <amp ramp type> <initial amp> <final amp> <phase (rad)>; <time> <t manage>\"\r\n"
"\"gen 2, freq & amp ramp <freq1 ramp type> <initial freq1> <final freq1> <amp ramp1 type> <initial ramp1> <final ramp1> <phase1 (rad)>; <sim for 2nd>; <time> <t manage>\"\r\n"
"Etc.\r\n"
"\r\n"
"(h)Predefined Waveform\r\n"
"\"predefined waveform <vertical filename.txt> <horizontal filename.txt> <waveform type ('command' or 'raw')>\"\r\n"
"\r\n"
"(i)Predefined Script\r\n"
"\"predefined script <filename.txt>\"\r\n"
"\r\n"
"(j)Create marker event after last waveform\r\n"
"\"create marker event <samples after previous waveform to wait>\"\r\n"
"\r\n";

const std::vector<std::string> GNUPLOT_COLORS{ "lt rgb \"white\"",
												"lt rgb \"cyan\"",
												"lt rgb \"green\"",
												"lt rgb \"#DC143C\"",  // crimson
												"lt rgb \"#B8860B\"",  // dark goldenrod
												"lt rgb \"#4B0082\"",  // indigo
												"lt rgb \"blue\"",
												"lt rgb \"#FF8C00\"",  // dark orange	
												"lt rgb \"#708090\"" }; // slate grey

const std::vector<std::string> GNUPLOT_MARKERS{ "pt 5",
												"pt 7",
												"pt 9",
												"pt 11",
												"pt 13",
												"pt 15",
												"pt 1",
												"pt 3",
												"pt 6"};

const std::vector<std::string> GNUPLOT_LINETYPES{ "dt 1",
													"dt 2",
													"dt 3",
													"dt 4",
													"dt 1",
													"dt 2",
													"dt 3",
													"dt 4",
													"dt 1"};

const std::string GNUPLOT_LOCATION = "\"C:\\Program Files (x86)\\gnuplot\\bin\\gnuplot.exe\"";
