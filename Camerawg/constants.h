#pragma once

#include "stdafx.h"
#include "niFgen.h"
#include <string>

// running in safemode means that the program doesn't actually try to connect to various devices. It can be used to build and debug other 
// aspects of the program.
#define NIAWG_SAFEMODE false
#define ANDOR_SAFEMODE true
#define LAB_COMPUTER

/// File Locations
// Files for Lab Computer
#ifdef LAB_COMPUTER
	#define SOCKET_SAFEMODE false
	#define NIAWG_SAFEMODE false
	#define ANDOR_SAFEMODE true
	#define CONNECT_TO_ANDOR_SAFEMODE false
	#define PYTHON_HOME L"C:\\ProgramData\\Anaconda3\\"
	const std::string PLOT_FILES_SAVE_LOCATION = "C:\\Users\\Regal Lab\\Documents\\Quantum Gas Assembly Control\\Plotting\\";
    const std::string ANALYSIS_CODE_LOCATION = "C:\\\\Users\\\\Mark\\\\Documents\\\\Data-Analysis";
	const std::string LIB_PATH = "D:\\Waveforms Library\\Wavefunction Library 350 MS per second V6\\";
	const std::string DEFAULT_SCRIPT_FOLDER_PATH = "C:\\Users\\Regal Lab\\Documents\\Quantum Gas Assembly Control\\Default Scripts\\";
	const std::string EXPERIMENT_LOGGING_FILES_PATH = "\\\\andor\\share\\Data and documents\\Data repository\\NIAWG Logging Files\\Individual Experiments\\";
	const std::string CODE_LOGGING_FILES_PATH = "\\\\andor\\share\\Data and documents\\Data repository\\NIAWG Logging Files\\Code Versions\\";
	const std::string ACTUAL_CODE_FOLDER_PATH = "C:\\Users\\Regal Lab\\Documents\\Quantum Gas Assembly Control\\Camerawg\\";
	const std::string PROFILES_PATH = "C:\\Users\\Regal Lab\\Documents\\Quantum Gas Assembly Control\\Profiles\\";
#endif

// Files for Desktop
#ifdef DESKTOP_COMPUTER
	#define PYTHON_HOME L"C:\\Users\\Mark\\Anaconda3\\"
	#define NIAWG_SAFEMODE false
	#define ANDOR_SAFEMODE true
	#define SOCKET_SAFEMODE true
	#define CONNECT_TO_ANDOR_SAFEMODE true
	const std::string PLOT_FILES_SAVE_LOCATION = "C:\\Users\\Mark\\Documents\\Quantum Gas Assembly Control\\Plotting\\";
	const std::string ANALYSIS_CODE_LOCATION = "C:\\\\Users\\\\Mark\\\\Documents\\\\Data-Analysis";
	const std::string LIB_PATH = "C:\\Users\\Mark\\Documents\\Quantum Gas Assembly Control\\NIAWG Wavefunction Library\\";
	const std::string DEFAULT_SCRIPT_FOLDER_PATH = "C:\\Users\\Mark\\Documents\\Quantum Gas Assembly Control\\Default NIAWG Scripts\\";
	const std::string EXPERIMENT_LOGGING_FILES_PATH = "C:\\Users\\Mark\\Documents\\Quantum Gas Assembly Control\\";
	const std::string CODE_LOGGING_FILES_PATH = "\\\\andor\\share\\Data and documents\\Data repository\\NIAWG Logging Files\\Code Versions\\";
	const std::string ACTUAL_CODE_FOLDER_PATH = "C:\\Users\\Mark\\Documents\\Quantum Gas Assembly Control\\Cold Atoms Control Application\\";
	const std::string PROFILES_PATH = "C:\\Users\\Mark\\Documents\\Quantum Gas Assembly Control\\Profiles\\";
#endif

// Files for my Laptop
#ifdef LAPTOP_COMPUTER
	#define PYTHON_HOME L"C:\\Users\\Mark\\Anaconda3\\"
	#define NIAWG_SAFEMODE false
	#define ANDOR_SAFEMODE true
	#define SOCKET_SAFEMODE true
	#define CONNECT_TO_ANDOR_SAFEMODE true
	const std::string PLOT_FILES_SAVE_LOCATION = "C:\\Users\\Mark\\Documents\\Quantum Gas Assembly Control\\Plotting\\";
	const std::string ANALYSIS_CODE_LOCATION = "C:\\\\Users\\\\Mark\\\\Documents\\\\Data-Analysis";
	const std::string LIB_PATH = "C:\\Users\\Mark\\Documents\\Quantum Gas Assembly Control\\Waveforms Library\\";
	const std::string DEFAULT_SCRIPT_FOLDER_PATH = "C:\\Users\\Mark\\Documents\\Quantum Gas Assembly Control\\Default Scripts\\";
	const std::string EXPERIMENT_LOGGING_FILES_PATH = "\\\\andor\\share\\Data and documents\\Data repository\\NIAWG Logging Files\\"
		"Individual Experiments\\";
	const std::string CODE_LOGGING_FILES_PATH = "\\\\andor\\share\\Data and documents\\Data repository\\NIAWG Logging Files\\Code Versions\\";
	const std::string ACTUAL_CODE_FOLDER_PATH = "C:\\Users\\Mark\\Documents\\Quantum Gas Assembly Control\\Cold Atoms Control Application\\";
	const std::string PROFILES_PATH = "C:\\Users\\Mark\\Documents\\Quantum Gas Assembly Control\\Profiles\\";
#endif

#ifdef MASTER_COMPUTER
	#define SOCKET_SAFEMODE true
	#define NIAWG_SAFEMODE true
	#define ANDOR_SAFEMODE true
	#define CONNECT_TO_ANDOR_SAFEMODE true
	#define PYTHON_HOME L"C:\\ProgramData\\Anaconda3\\"
	const std::string PLOT_FILES_SAVE_LOCATION = "C:\\Users\\Regal Lab\\Documents\\Quantum-Gas-Assembly-Control\\Plotting\\";
	const std::string ANALYSIS_CODE_LOCATION = "C:\\\\Users\\\\Mark\\\\Documents\\\\Data-Analysis";
	const std::string LIB_PATH = "C:\\Users\\Regal Lab\\Documents\\Quantum-Gas-Assembly-Control\\Waveforms Library\\Waveform Library 350 "
		"MS per second V6\\";
	const std::string DEFAULT_SCRIPT_FOLDER_PATH = "C:\\Users\\Regal Lab\\Documents\\Quantum-Gas-Assembly-Control\\Default Scripts\\";
	const std::string EXPERIMENT_LOGGING_FILES_PATH = "\\\\andor\\share\\Data and documents\\Data repository\\NIAWG Logging Files\\"
		"Individual Experiments\\";
	const std::string CODE_LOGGING_FILES_PATH = "\\\\andor\\share\\Data and documents\\Data repository\\NIAWG Logging Files\\Code "
		"Versions\\";
	const std::string ACTUAL_CODE_FOLDER_PATH = "C:\\Users\\Regal Lab\\Documents\\Quantum-Gas-Assembly-Control\\Camerawg\\";
	const std::string PROFILES_PATH = "C:\\Users\\Regal Lab\\Documents\\Quantum-Gas-Assembly-Control\\Profiles\\";
#endif


// We calibrated this. // GAIN = 1.34.
#define GAIN 1.34
// GAIN = 1.34.

#define MAX_NIAWG_SIGNALS 10

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
#define NUMBER_OF_LIBRARY_FILES 20

/// Random other Constants
// It's PI.
#define PI 3.14159265358979323846264338327950288
// This is the length of time waited before the code decides to update your syntax coloring after a change is made.
#define SYNTAX_TIMER_LENGTH 1000

// intensity agilent address
const char * const AGILENT_ADDRESS = "USB0::0x0957::0x2307::MY50004500::0::INSTR";

// Parameters that the user might want to change:
const bool CONST_POWER_OUTPUT = true;

// for floating point error... 0.2 was picked because this is the maximum power that can be outputted with 5 signals given the voltage 
// restrictions.
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
// 
// Main Window
#define IDC_MAIN_STATUS_BUTTON 1001
#define IDC_ERROR_STATUS_BUTTON 1004
#define IDC_DEBUG_STATUS_BUTTON 1007
#define IDC_EXPERIMENT_COMBO 1013
#define IDC_CATEGORY_COMBO 1014
#define IDC_ORIENTATION_COMBO 1018
#define IDC_CONFIGURATION_COMBO 1019
#define IDC_SEQUENCE_COMBO 1021
#define IDC_VARIABLES_LISTVIEW 1031
#define IDC_MAIN_OPTIONS_RANGE_BEGIN 1033
#define IDC_MAIN_OPTIONS_RANGE_END 1035
#define IDC_DEBUG_OPTIONS_RANGE_BEGIN 1037
#define IDC_DEBUG_OPTIONS_RANGE_END 1043

// Scripting Window
#define SYNTAX_TIMER_ID 1999
#define IDC_VERTICAL_SCRIPT_COMBO 2003
#define IDC_VERTICAL_SCRIPT_EDIT 2004
#define IDC_HORIZONTAL_SCRIPT_COMBO 2008
#define IDC_HORIZONTAL_SCRIPT_EDIT 2009
#define IDC_AGILENT_SCRIPT_COMBO 2013
#define IDC_AGILENT_SCRIPT_EDIT 2014

// Camera Window
#define IDC_ALERTS_BOX 3004
#define IDC_PLOTTING_LISTVIEW 3016

#define IDC_CAMERA_MODE_COMBO 3018
#define IDC_SET_EM_GAIN_BUTTON 3019
#define IDC_TRIGGER_COMBO 3023
#define IDC_SET_TEMPERATURE_BUTTON 3024
#define PICTURE_SETTINGS_ID_START 3028
#define PICTURE_SETTINGS_ID_END 3063
#define IDC_SET_IMAGE_PARAMETERS_BUTTON 3064

#define IDC_PICTURE_1_MIN_EDIT 3110
#define IDC_PICTURE_1_MAX_EDIT 3113
#define IDC_PICTURE_2_MIN_EDIT 3116
#define IDC_PICTURE_2_MAX_EDIT 3119
#define IDC_PICTURE_3_MIN_EDIT 3122
#define IDC_PICTURE_3_MAX_EDIT 3125
#define IDC_PICTURE_4_MIN_EDIT 3128
#define IDC_PICTURE_4_MAX_EDIT 3131
//
#define IDC_BEGINNING_DIALOG_RICH_EDIT 100
//

// Don't change! Or only change if you really kow what you are doing. Current Value: 5
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

