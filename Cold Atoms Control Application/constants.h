#pragma once

#include "stdafx.h"
#include "niFgen.h"

#include <string>
// running in safemode means that the program doesn't actually try to connect to any devices. It can be used to debug other aspects of the program.
#define TWEEZER_COMPUTER_SAFEMODE true
//#define DESKTOP_COMPUTER
//#define LAB_COMPUTER
#define LAPTOP_COMPUTER

/// File Locations
// Files for Lab Computer
#ifdef LAB_COMPUTER
	const std::string LIB_PATH = "D:\\Waveforms Library\\Wavefunction Library 350 MS per second V6\\";
	const std::string DEFAULT_SCRIPT_FOLDER_PATH = "C:\\Users\\Regal Lab\\Documents\\Quantum Gas Assembly Control\\Default Scripts\\";
	const std::string EXPERIMENT_LOGGING_FILES_PATH = "\\\\andor\\share\\Data and documents\\Data repository\\NIAWG Logging Files\\Individual Experiments\\";
	const std::string CODE_LOGGING_FILES_PATH = "\\\\andor\\share\\Data and documents\\Data repository\\NIAWG Logging Files\\Code Versions\\";
	const std::string ACTUAL_CODE_FOLDER_PATH = "C:\\Users\\Regal Lab\\Documents\\Quantum Gas Assembly Control\\Cold Atoms Control Application\\";
	const std::string PROFILES_PATH = "C:\\Users\\Regal Lab\\Documents\\Quantum Gas Assembly Control\\Profiles\\";
#endif

// Files for Desktop
#ifdef DESKTOP_COMPUTER
	const std::string LIB_PATH = "C:\\Users\\Mark\\Documents\\Quantum Gas Assembly Control\\NIAWG Wavefunction Library\\";
	const std::string DEFAULT_SCRIPT_FOLDER_PATH = "C:\\Users\\Mark\\Documents\\Quantum Gas Assembly Control\\Default NIAWG Scripts\\";
	const std::string EXPERIMENT_LOGGING_FILES_PATH = "C:\\Users\\Mark\\Documents\\Quantum Gas Assembly Control\\";
	const std::string CODE_LOGGING_FILES_PATH = "\\\\andor\\share\\Data and documents\\Data repository\\NIAWG Logging Files\\Code Versions\\";
	const std::string ACTUAL_CODE_FOLDER_PATH = "C:\\Users\\Mark\\Documents\\Quantum Gas Assembly Control\\Cold Atoms Control Application\\";
	const std::string PROFILES_PATH = "C:\\Users\\Mark\\Documents\\Quantum Gas Assembly Control\\Profiles\\";
#endif

// Files for my Laptop
#ifdef LAPTOP_COMPUTER
	const std::string LIB_PATH = "C:\\Users\\Mark\\Documents\\Quantum Gas Assembly Control\\Waveforms Library\\";
	const std::string DEFAULT_SCRIPT_FOLDER_PATH = "C:\\Users\\Mark\\Documents\\Quantum Gas Assembly Control\\Default Scripts\\";
	const std::string EXPERIMENT_LOGGING_FILES_PATH = "\\\\andor\\share\\Data and documents\\Data repository\\NIAWG Logging Files\\Individual Experiments\\";
	const std::string CODE_LOGGING_FILES_PATH = "\\\\andor\\share\\Data and documents\\Data repository\\NIAWG Logging Files\\Code Versions\\";
	const std::string ACTUAL_CODE_FOLDER_PATH = "C:\\Users\\Mark\\Documents\\Quantum Gas Assembly Control\\Cold Atoms Control Application\\";
	const std::string PROFILES_PATH = "C:\\Users\\Mark\\Documents\\Quantum Gas Assembly Control\\Profiles\\";
#endif

// We calibrated this. // GAIN = 1.34.
#define GAIN 1.34
// GAIN = 1.34.
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

const char * const AGILENT_ADDRESS = "USB0::0x0957::0x2307::MY50004500::0::INSTR";

// Parameters that the user might want to change:
const bool CONST_POWER_OUTPUT = true;
// for floating point error... 0.2 was picked because this is the maximum power that can be outputted with 5 signals given the voltage restrictions.
const double TOTAL_POWER = 0.2 - 1e-10;
// Currently bugs with the 5451 for sample rates significantly above this sample rate (350 MS/s). T.T

/// IF CHANGE MAKE SURE TO CHANGE LIBRARY FILE ADDRESS !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
const long int SAMPLE_RATE = 350000000; // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
/// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
/// This is important. If you don't change the libary file, you will be running waveforms that were compiled with a different sample rate.

/// Communication Parameters
#define DEFAULT_PORT "10010"
const char * const SERVER_ADDRESS = "192.168.236.1";

// Session Parameters:
const ViInt32 OUTPUT_MODE = NIFGEN_VAL_OUTPUT_SCRIPT;

const ViRsrc NI_5451_LOCATION = "PXI1Slot2";

// Channel parameters
const ViConstString SESSION_CHANNELS = "0,1";
const ViConstString X_CHANNEL = "0";
const ViConstString Y_CHANNEL = "1";

// Minimum waveform size that the waveform generator can produce:
const int MIN_WAVE_SAMPLE_SIZE = 100;
const double MAX_CAP_TIME = 1e-3;

// Trigger Parameters:
const ViConstString TRIGGER_NAME = "ScriptTrigger0";
const ViConstString TRIGGER_SOURCE = "PFI0";
const ViInt32 TRIGGER_EDGE_TYPE = NIFGEN_VAL_RISING_EDGE;


// Contains all of of the names of the files that hold actual data file names.
const std::string WAVEFORM_NAME_FILES[20] = { "gen 1, const waveform file names.txt", "gen 2, const waveform file names.txt",
"gen 3, const waveform file names.txt", "gen 4, const waveform file names.txt", "gen 5, const waveform file names.txt",
"gen 1, amp ramp waveform file names.txt", "gen 2, amp ramp waveform file names.txt", "gen 3, amp ramp waveform file names.txt",
"gen 4, amp ramp waveform file names.txt", "gen 5, amp ramp waveform file names.txt", "gen 1, freq ramp waveform file names.txt",
"gen 2, freq ramp waveform file names.txt", "gen 3, freq ramp waveform file names.txt", "gen 4, freq ramp waveform file names.txt",
"gen 5, freq ramp waveform file names.txt", "gen 1, freq & amp ramp waveform file names.txt", "gen 2, freq & amp ramp waveform file names.txt",
"gen 3, freq & amp ramp waveform file names.txt", "gen 4, freq & amp ramp waveform file names.txt", "gen 5, freq & ""amp ramp waveform file names.txt" };

const std::string WAVEFORM_TYPE_FOLDERS[20] = { "gen1const\\", "gen2const\\", "gen3const\\", "gen4const\\", "gen5const\\",
"gen1ampramp\\", "gen2ampramp\\", "gen3ampramp\\", "gen4ampramp\\", "gen5ampramp\\",
"gen1freqramp\\", "gen2freqramp\\", "gen3freqramp\\", "gen4freqramp\\", "gen5freqramp\\",
"gen1ampfreqramp\\", "gen2ampfreqramp\\", "gen3ampfreqramp\\", "gen4ampfreqramp\\", "gen5ampfreqramp\\" };

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
" format is referenced below using angled brackets <...>.Place the input on the line below the command in the\r\n"
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
"\"gen 3, const <freq1> <amp1> <phase1 (rad)>; <sim for 2nd, 3rd>; <time> <t manage>\"\r\n"
"\"gen 4, const <freq1> <amp1> <phase1 (rad)>; <sim for 2nd, 3rd, 4th>; <time> <t manage>\"\r\n"
"\"gen 5, const <freq1> <amp1> <phase1 (rad)>; <sim for 2nd, 3rd, 4th, 5th>; <time> <t manage>\"\r\n"
"\r\n"
"(e)Amplitude Ramps\r\n"
"\"gen 1, amp ramp <freq> <amp ramp type> <initial amp> <final amp> <phase (rad)>; <time> <t manage>\"\r\n"
"\"gen 2, amp ramp <freq1> <amp1 ramp type> <initial amp1> <final amp1> <phase1 (rad)>; <sim for 2nd>; <time> <t manage>\"\r\n"
"\"gen 3, amp ramp <freq1> <amp1 ramp type> <initial amp1> <final amp1> <phase1 (rad)>; <sim for 2nd, 3rd>; <time> <t manage>\"\r\n"
"\"gen 4, amp ramp <freq1> <amp1 ramp type> <initial amp1> <final amp1> <phase1 (rad)>; <sim for 2nd, 3rd, 4th>; <time> <t manage>\"\r\n"
"\"gen 5, amp ramp <freq1> <amp1 ramp type> <initial amp1> <final amp1> <phase1 (rad)>; <sim for 2nd, 3rd, 4th, 5th>; <time> <t manage>\"\r\n"
"\r\n"
"(f)frequency Ramps\r\n"
"\"gen 1, freq ramp <freq ramp type> <initial freq> <final freq> <amp> <phase (rad)>; <time> <t manage>\"\r\n"
"\"gen 2, freq ramp <freq1 ramp type> <initial freq1> <final freq1> <amp1> <phase1 (rad)>; <sim for 2nd>; <time> <t manage>\"\r\n"
"\"gen 3, freq ramp <freq1 ramp type> <initial freq1> <final freq1> <amp1> <phase1 (rad)>; <sim for 2nd, 3rd>; <time> <t manage>\"\r\n"
"\"gen 4, freq ramp <freq1 ramp type> <initial freq1> <final freq1> <amp1> <phase1 (rad)>; <sim for 2nd, 3rd, 4th>; <time> <t manage>\"\r\n"
"\"gen 5, freq ramp <freq1 ramp type> <initial freq1> <final freq1> <amp1> <phase1 (rad)>; <sim for 2nd, 3rd, 4th, 5th>; <time> <t manage>\"\r\n"
"\r\n"
"(g)Amplitude and Frequency Ramps\r\n"
"\"gen 1, freq & amp ramp <freq ramp type> <initial freq> <final freq> <amp ramp type> <initial amp> <final amp> <phase (rad)>; <time> <t manage>\"\r\n"
"\"gen 2, freq & amp ramp <freq1 ramp type> <initial freq1> <final freq1> <amp ramp1 type> <initial ramp1> <final ramp1> <phase1 (rad)>; <sim for 2nd>; <time> <t manage>\"\r\n"
"\"gen 3, freq & amp ramp <freq1 ramp type> <initial freq1> <final freq1> <amp ramp1 type> <initial ramp1> <final ramp1> <phase1 (rad)>; <sim for 2nd, 3rd>; <time> <t manage>\"\r\n"
"\"gen 4, freq & amp ramp <freq1 ramp type> <initial freq1> <final freq1> <amp ramp1 type> <initial ramp1> <final ramp1> <phase1 (rad)>; <sim for 2nd, 3rd, 4th>; <time> <t manage>\"\r\n"
"\"gen 5, freq & amp ramp <freq1 ramp type> <initial freq1> <final freq1> <amp ramp1 type> <initial ramp1> <final ramp1> <phase1 (rad)>; <sim for 2nd, 3rd, 4th, 5th>; <time> <t manage>\"\r\n"
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

/// Defines for Windows API Handles (OTHER)
#define IDC_STATIC_INTENSITY_TITLE_HANDLE 1000
#define IDC_INTENSITY_SAVED_BUTTON 1001
#define IDC_INTENSITY_SCRIPT_EDIT 1002
#define IDC_INTENSITY_NAME_TEXT 1003
#define IDC_EXPERIMENT_TYPE_LABEL 1004
#define IDC_SYSTEM_ERROR_TEXT 1006
#define IDC_SCRIPT_ERROR_CLEAR_BUTTON 1007
#define IDC_ERROR_STATUS_TEXT_DISPLAY 1008
#define IDC_GUI_STAT_TEXT 1009
#define IDC_DEBUG_OPTION_DISPLAY_TEXT 1010
#define IDC_OUTPUT_READ_STATUS 1011
#define IDC_OUTPUT_WRITE_STATUS 1012
#define IDC_PROGRAM_INTENSITY_BOX 1013
#define IDC_EXPERMENT_CONFIGURATION_LIST_COMBO 1014
#define IDC_ORIENTATION_COMBO 1015
#define IDC_AGILENT_STAT_TEXT 1016
#define IDC_NIAWG_STAT_TEXT 1017
#define IDC_BITMAP_TEST 1018

/// Defines for Windows API Handles (NIAWG)
#define IDC_EXIT_BUTTON	101
#define IDC_MAIN_EDIT 102
#define IDC_INFO_BUTTON 103
#define IDC_OPEN_X_BUTTON 104
#define IDC_BEGIN_BUTTON 105
#define IDC_OPEN_Y_BUTTON 106
#define IDC_VAR_1_BUTTON 107
#define IDC_VAR_2_BUTTON 108
#define IDC_VAR_NAME_1_EDIT 109
#define IDC_VAR_NAME_2_EDIT 109
#define IDC_VAR_NAME_1_SAVE_BUTTON 110
#define IDC_VAR_NAME_2_SAVE_BUTTON 111
#define IDC_VAR_NAME_1_TEXT 112
#define IDC_VAR_NAME_2_TEXT 113
#define IDC_STATIC_1_TEXT 114
#define IDC_VAR_FILE_1_TEXT 115
#define IDC_VAR_FILE_2_TEXT 116
#define IDC_X_FILE_TEXT 117
#define IDC_Y_FILE_TEXT 118
#define IDC_VERTICAL_SCRIPT_EDIT 119
#define IDC_SAVE_X_INDICATOR_BUTTON 123
#define IDC_X_SCRIPT_NAME_TEXT 124
#define IDC_SYSTEM_STATUS_TEXT 126
#define IDC_VAR_NAME_3_TEXT 127
#define IDC_VAR_FILE_3_TEXT 128
#define IDC_VAR_NAME_3_EDIT 129
#define IDC_VAR_3_BUTTON 130
#define IDC_VAR_NAME_3_SAVE_BUTTON 131
#define IDC_ONLY_WRTIE_WAVEFORMS_BUTTON 132
#define IDC_ACCUMULATIONS_BUTTON 133
#define IDC_ACCUMULATIONS_EDIT 134
#define IDC_ACCUMULATIONS_TEXT 135
#define IDC_STATIC_2_TEXT 136
#define IDC_STATIC_3_TEXT 137
#define IDC_SYSTEM_DEBUG_TEXT 138
#define IDC_SCRIPT_DEBUG_CLEAR_BUTTON 139
#define IDC_SCRIPT_STATUS_CLEAR_BUTTON 140
#define IDC_STATIC_4_TEXT 141
#define IDC_STATIC_5_TEXT 142
#define IDC_CONNECT_TO_MASTER_BUTTON 143
#define IDC_RECEIVE_VAR_FILES_BUTTON 144
#define IDC_VAR_4_BUTTON 145
#define IDC_VAR_5_BUTTON 146
#define IDC_VAR_6_BUTTON 147
#define IDC_VAR_NAME_4_SAVE_BUTTON 148
#define IDC_VAR_NAME_5_SAVE_BUTTON 149
#define IDC_VAR_NAME_6_SAVE_BUTTON 150
#define IDC_VAR_NAME_4_EDIT 151
#define IDC_VAR_NAME_5_EDIT 152
#define IDC_VAR_NAME_6_EDIT 153
#define IDC_VAR_NAME_4_TEXT 154
#define IDC_VAR_NAME_5_TEXT 155
#define IDC_VAR_NAME_6_TEXT 156
#define IDC_VAR_FILE_4_TEXT 157
#define IDC_VAR_FILE_5_TEXT 158
#define IDC_VAR_FILE_6_TEXT 159
#define IDC_OUTPUT_READ_WRITE_STATUS 160
#define IDC_STATIC_6_TEXT 161
#define IDC_LOG_SCRIPT_PARAMS 162
#define IDC_HORIZONTAL_SCRIPT_EDIT 163
#define IDC_SAVE_Y_INDICATOR_BUTTON 164
#define IDC_STATIC_X_HANDLE_TEXT 165
#define IDC_STATIC_Y_HANDLE_TEXT 166
#define IDC_Y_SCRIPT_NAME_TEXT 167
#define IDC_DUMMY_BUTTON 168
#define IDC_DUMMY_NUM_EDIT 169
#define IDC_DUMMY_NUM_TEXT 170
#define IDC_DUMMY_NUM_BUTTON 171
#define IDC_SAVE_EXPERIMENT_INDICATOR_BUTTON 172
#define IDC_CONFIG_LABEL_HANDLE_TEXT 173
#define IDC_CONFIG_NAME_TEXT 174
#define IDC_VAR_NAMES_EDIT 175
#define IDC_VAR_SET_BUTTON 176
#define IDC_OUTPUT_CORR_TIME_BUTTON 177
#define IDC_COLOR_BOX 178
#define IDC_CONFIG_NOTES 179
#define IDC_NOTES_TEXT 180
#define SYNTAX_TIMER_ID 181
#define IDC_CONFIGURATION_COMBO 182
#define IDC_CATEGORY_COMBO 183
#define IDC_EXPERIMENT_COMBO 184
#define IDC_LOAD_CONFIG_BUTTON 185
#define IDC_CONFIGURATION_COMBO_LABEL 186
#define IDC_ORIENTATION_COMBO_LABEL 187
#define IDC_SUB_CONFIG_COMBO_LABEL 188
#define IDC_SAVE_CONFIGURATION_INDICATOR_BUTTON 189
#define IDC_OUTPUT_MORE_RUN_INFO 190
#define IDC_BEGINNING_DIALOG_RICH_EDIT 191
#define IDC_VERTICAL_SCRIPT_VIEW_TEXT 192
#define IDC_VERTICAL_SCRIPT_VIEW_COMBO 193
#define IDC_HORIZONTAL_SCRIPT_VIEW_TEXT 194
#define IDC_HORIZONTAL_SCRIPT_VIEW_COMBO 195
#define IDC_INTENSITY_SCRIPT_VIEW_TEXT 196
#define IDC_INTENSITY_SCRIPT_VIEW_COMBO 197
#define IDC_SEQUENCE_TEXT 198
#define IDC_SEQUENCE_COMBO 199
#define IDC_SEQUENCE_DISPLAY 200
#define IDC_EXPERIMENT_NOTES_HEADER 201
#define IDC_EXPERIMENT_NOTES 202
#define IDC_CATEGORY_NOTES_HEADER 203
#define IDC_CATEGORY_NOTES 204
#define IDC_CONFIGURATION_NOTES_HEADER 205
#define IDC_CONFIGURATION_NOTES 206
#define IDC_VARIABLES_LISTVIEW 207