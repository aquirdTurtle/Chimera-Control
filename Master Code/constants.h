// define this as true in order stop the code from actually calling any dio64 functions. Use, e.g. if the device is not connected.

#define DIO_SAFEMODE false
#define DAQMX_SAFEMODE false

#define PROFILES_PATH "C:\\Users\\Mark\\Documents\\Quantum Gas Assembly Control\\Master Profiles\\"
#define FUNCTIONS_FOLDER_LOCATION "C:\\Users\\Mark\\Documents\\Quantum Gas Assembly Control\\Functions\\"
#define MASTER_CONFIGURATION_FILE_ADDRESS "C:\\Users\\Mark\\Documents\\Quantum Gas Assembly Control\\Master Configuration"
#define DEFAULT_SCRIPT_FOLDER_PATH "C:\\Users\\Mark\\Documents\\Quantum Gas Assembly Control\\Default Scripts\\"
#define LOGGING_FILE_ADDRESS "C:\\Users\\Mark\\Documents\\Quantum Gas Assembly Control\\Master Log.txt"
#define KEY_ADDRESS "C:\\Users\\Mark\\Documents\\Quantum Gas Assembly Control\\key.txt"

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

#define RSG_ADDRESS 28
#define RSG_POWER 10

#define SYNTAX_TIMER_ID 500
#define SYNTAX_TIMER_LENGTH 1000
/// control numbers I need for message maps
#define IDC_SEQUENCE_DISPLAY 1092
#define TTL_ID_BEGIN 1228
#define TTL_ID_END 1311
#define TTL_HOLD 1312
#define MASTER_RICH_EDIT 1204
#define LISTVIEW_CONTROL 1320
#define SET_REPETITION_ID 1370
#define EXPERIMENT_COMBO_ID 1211
#define CATEGORY_COMBO_ID 1212
#define CONFIGURATION_COMBO_ID 1217
#define SEQUENCE_COMBO_ID 1219
#define ORIENTATION_COMBO_ID 1216
#define ID_STATUS_CLEAR 1314
#define ID_ERROR_CLEAR 1317
#define ID_DAC_FIRST_EDIT 1321
#define ID_DAC_SET_BUTTON 1369
#define CONFIGURATION_NOTES_ID 1226
#define CATEGORY_NOTES_ID 1224
#define EXPERIMENT_NOTES_ID 1222
#define FUNCTION_COMBO_ID 1205