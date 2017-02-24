
//#define MASTER_COMPUTER
#define LAPTOP_COMPUTER

#ifdef MASTER_COMPUTER
	#define DIO_SAFEMODE false
	#define DAQMX_SAFEMODE false
	#define GPIB_SAFEMODE false
	#define AGILENT_SAFEMODE false
	#define PROFILES_PATH "C:\\Documents and Settings\\Regal Lab\\My Documents\\Quantum Gas Assembly Control\\Master Profiles\\"
	#define FUNCTIONS_FOLDER_LOCATION "C:\\Documents and Settings\\Regal Lab\\My Documents\\Quantum Gas Assembly Control\\Functions\\"
	#define MASTER_CONFIGURATION_FILE_ADDRESS "C:\\Documents and Settings\\Regal Lab\\My Documents\\Quantum Gas Assembly Control\\Master Configuration"
	#define DEFAULT_SCRIPT_FOLDER_PATH "C:\\Documents and Settings\\Regal Lab\\My Documents\\Quantum Gas Assembly Control\\Default Scripts\\"
	#define LOGGING_FILE_ADDRESS "C:\\Documents and Settings\\Regal Lab\\My Documents\\Quantum Gas Assembly Control\\Master Log.txt"
	#define KEY_ADDRESS "C:\\Documents and Settings\\Regal Lab\\My Documents\\Quantum Gas Assembly Control\\key.txt"
#endif

#ifdef LAPTOP_COMPUTER
	#define DIO_SAFEMODE true
	#define DAQMX_SAFEMODE true
	#define GPIB_SAFEMODE true
	#define AGILENT_SAFEMODE true
	#define PROFILES_PATH "C:\\Users\\Mark\\Documents\\Quantum Gas Assembly Control\\Master Profiles\\"
	#define FUNCTIONS_FOLDER_LOCATION "C:\\Users\\Mark\\Documents\\Quantum Gas Assembly Control\\Functions\\"
	#define MASTER_CONFIGURATION_FILE_ADDRESS "C:\\Users\\Mark\\Documents\\Quantum Gas Assembly Control\\Master Configuration"
	#define DEFAULT_SCRIPT_FOLDER_PATH "C:\\Users\\Mark\\Documents\\Quantum Gas Assembly Control\\Default Scripts\\"
	#define LOGGING_FILE_ADDRESS "C:\\Users\\Mark\\Documents\\Quantum Gas Assembly Control\\Master Log.txt"
	#define KEY_ADDRESS "C:\\Users\\Mark\\Documents\\Quantum Gas Assembly Control\\key.txt"

#endif

#define MASTER_SCRIPT_EXTENSION ".mScript"
#define NIAWG_SCRIPT_EXTENSION ".nScript"
#define AGILENT_SCRIPT_EXTENSION ".aScript"
#define HORIZONTAL_EXTENSION ".hConfig"
#define VERTICAL_EXTENSION ".vConfig"
#define EXPERIMENT_EXTENSION ".eConfig"
#define CATEGORY_EXTENSION ".catConfig"
#define SEQUENCE_EXTENSION ".sConfig"
#define FUNCTION_EXTENSION ".func"

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

#define SYNTAX_TIMER_ID 500
#define SYNTAX_TIMER_LENGTH 1000
/// control numbers I need for message maps
#define EXPERIMENT_COMBO_ID 1004
#define CATEGORY_COMBO_ID 1005
#define ORIENTATION_COMBO_ID 1009
#define CONFIGURATION_COMBO_ID 1010
#define SEQUENCE_COMBO_ID 1013
#define IDC_GLOBAL_VARS_LISTVIEW 1015
#define IDC_CONFIG_VARS_LISTVIEW 1017
#define SET_REPETITION_ID 1018
#define TTL_HOLD 1022
#define IDC_ZERO_TTLS 1023
#define TTL_ID_BEGIN 1044
#define IDC_SEQUENCE_DISPLAY 1091
#define TTL_ID_END 1108
#define ID_DAC_SET_BUTTON 1109
#define IDC_ZERO_DACS 1110
#define ID_DAC_FIRST_EDIT 1112
#define ID_STATUS_CLEAR 1160
#define ID_ERROR_CLEAR 1163
#define EXPERIMENT_NOTES_ID 1166
#define CATEGORY_NOTES_ID 1168
#define CONFIGURATION_NOTES_ID 1170
#define IDC_SHOW_TTLS 1174
#define IDC_SHOW_DACS 1175
#define FUNCTION_COMBO_ID 1194
#define MASTER_RICH_EDIT 1196
