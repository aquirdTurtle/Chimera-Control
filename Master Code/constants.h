// define one (and only one) of the following:
//#define MASTER_COMPUTER
//#define LAPTOP_COMPUTER
#define HP_LAPTOP

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
	#define KEY_ADDRESS "\\\\Callisto\\Shared\\key.txt"
	#define MOT_ROUTINE_ADDRESS "C:\\Documents and Settings\\Regal Lab\\My Documents\\Quantum Gas Assembly Control\\Master Profiles\\Hotkey Experiments\\MOT\\turnOnMot.mScript"
	#define DEBUG_OUTPUT_LOCATION "C:\\Documents and Settings\\Regal Lab\\My Documents\\Quantum Gas Assembly Control\\Debug-Output\\"
#endif


#ifdef LAPTOP_COMPUTER
	#define DIO_SAFEMODE true
	#define DAQMX_SAFEMODE true
	#define GPIB_SAFEMODE true
	#define AGILENT_SAFEMODE false
	#define PROFILES_PATH "C:\\Users\\Mark\\Documents\\Quantum Gas Assembly Control\\Master Profiles\\"
	#define FUNCTIONS_FOLDER_LOCATION "C:\\Users\\Mark\\Documents\\Quantum Gas Assembly Control\\Functions\\"
	#define MASTER_CONFIGURATION_FILE_ADDRESS "C:\\Users\\Mark\\Documents\\Quantum Gas Assembly Control\\Master Configuration"
	#define DEFAULT_SCRIPT_FOLDER_PATH "C:\\Users\\Mark\\Documents\\Quantum Gas Assembly Control\\Default Scripts\\"
	#define LOGGING_FILE_ADDRESS "C:\\Users\\Mark\\Documents\\Quantum Gas Assembly Control\\Master Log.txt"
	#define KEY_ADDRESS "C:\\Users\\Mark\\Documents\\Quantum Gas Assembly Control\\key.txt"
	#define MOT_ROUTINE_ADDRESS "C:\\Users\\Mark\\Documents\\Quantum-Gas-Assembly-Control\\Master Profiles\\turnOnMot.mScript"
#endif

#ifdef HP_LAPTOP
	#define DIO_SAFEMODE true
	#define DAQMX_SAFEMODE true
	#define GPIB_SAFEMODE true
	#define AGILENT_SAFEMODE true
	#define PROFILES_PATH "C:\\Users\\Mark\\Documents\\Quantum-Gas-Assembly-Control\\Master Profiles\\"
	#define FUNCTIONS_FOLDER_LOCATION "C:\\Users\\Mark\\Documents\\Quantum-Gas-Assembly-Control\\Functions\\"
	#define MASTER_CONFIGURATION_FILE_ADDRESS "C:\\Users\\Mark\\Documents\\Quantum-Gas-Assembly-Control\\Master Configuration"
	#define DEFAULT_SCRIPT_FOLDER_PATH "C:\\Users\\Mark\\Documents\\Quantum-Gas-Assembly-Control\\Default Scripts\\"
	#define LOGGING_FILE_ADDRESS "C:\\Users\\Mark\\Documents\\Quantum-Gas-Assembly-Control\\Master Log.txt"
	#define KEY_ADDRESS "C:\\Users\\Mark\\Documents\\Quantum-Gas-Assembly-Control\\key.txt"
	#define MOT_ROUTINE_ADDRESS "C:\\Users\\Mark\\Documents\\Quantum-Gas-Assembly-Control\\Master Profiles\\Hotkey Experiments\\MOT\\turnOnMot.mScript"
	#define DEBUG_OUTPUT_LOCATION "C:\\Users\\Mark\\Documents\\Quantum-Gas-Assembly-Control\\Debug-Output\\"
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
#define ID_DAC_FIRST_EDIT 1111
#define ID_STATUS_CLEAR 1161
#define ID_ERROR_CLEAR 1164
#define EXPERIMENT_NOTES_ID 1203
#define CATEGORY_NOTES_ID 1205
#define CONFIGURATION_NOTES_ID 1207
#define IDC_SHOW_TTLS 1211
#define IDC_SHOW_DACS 1212
#define IDC_TOP_BOTTOM_CHANNEL1_BUTTON 1223
#define IDC_TOP_BOTTOM_CHANNEL2_BUTTON 1224
#define IDC_TOP_BOTTOM_SYNC_BUTTON 1225
#define IDC_TOP_BOTTOM_AGILENT_COMBO 1226
#define IDC_AXIAL_UWAVE_CHANNEL1_BUTTON 1215
#define IDC_AXIAL_UWAVE_CHANNEL2_BUTTON 1216
#define IDC_AXIAL_UWAVE_SYNC_BUTTON 1217
#define IDC_AXIAL_UWAVE_AGILENT_COMBO 1218
#define IDC_FLASHING_CHANNEL1_BUTTON 1231
#define IDC_FLASHING_CHANNEL2_BUTTON 1232
#define IDC_FLASHING_SYNC_BUTTON 1233
#define IDC_FLASHING_AGILENT_COMBO 1234

#define FUNCTION_COMBO_ID 1241
#define MASTER_RICH_EDIT 1242

#define AXIAL_ON_OFF 1192
#define BOTTOM_ON_OFF 1174
#define TOP_ON_OFF 1168
#define EO_ON_OFF 1186

#define AXIAL_FSK 1193
#define BOTTOM_FSK 1175
#define TOP_FSK 1169
#define EO_FSK 1187

