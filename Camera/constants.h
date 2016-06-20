#pragma once
#include "stdafx.h"
#include "windows.h"
#include "ATMCD32D.H"
#include <string>
#include <vector>

#define ANDOR_SAFEMODE true

// Some Default Settings
#define DEFAULT_CAMERA_TEMPERATURE 0;
// for andor:


// extra slashes here because python needs to process the string as well, so extra escape characters...
/*
#define PYTHON_HOME L"C:\\Users\\Regal Lab\\Anaconda3\\"
const std::string ANALYSIS_CODE_LOCATION = "C:\\\\Users\\\\Regal Lab\\\\Documents\\\\My Data Analysis";
const std::string SAVE_BASE_ADDRESS("E:\\share\\Data and documents\\Data repository\\");
const std::string PLOT_FILES_SAVE_LOCATION = "C:\\Users\\Regal Lab\\Documents\\Quantum Gas Assembly Code\\Plotting\\";
const std::string KEY_FILE_LOCATION = "E:\\share\\Data and documents\\Data repository\\Key_file\\";
const std::string CAMERA_CONFIGURATION_FILES_LOCATION = "C:\\Users\\Regal Lab\\Documents\\Quantum Gas Assembly Code\\Camera Configurations\\";
*/
// for my laptop:
// extra slashes here because python needs to process the string as well, so extra escape characters...
#define PYTHON_HOME L"C:\\Users\\Mark\\Anaconda3\\"
const std::string ANALYSIS_CODE_LOCATION = "C:\\\\Users\\\\Mark\\\\Documents\\\\My Data Analysis";
const std::string SAVE_BASE_ADDRESS = "C:\\Users\\Mark\\Documents\\Quantum Gas Assembly Control\\Data\\Camera Data\\";
const std::string PLOT_FILES_SAVE_LOCATION = "C:\\Users\\Mark\\Documents\\Quantum Gas Assembly Control\\Plotting\\";
const std::string KEY_FILE_LOCATION = "C:\\Users\\Mark\\Documents\\Quantum Gas Assembly Control\\Data\\Data Keys\\";
const std::string CAMERA_CONFIGURATION_FILES_LOCATION = "C:\\Users\\Mark\\Documents\\Quantum Gas Assembly Control\\Camera Configuration Files\\";
//
const std::vector<std::string> GNUPLOT_COLORS{"lt rgb \"black\"",
											 "lt rgb \"blue\"",
											 "lt rgb \"green\"",  
											 "lt rgb \"#DC143C\"",  // crimson
											 "lt rgb \"#B8860B\"",  // dark goldenrod
											 "lt rgb \"#4B0082\"",  // indigo
											 "lt rgb \"cyan\"",    
											 "lt rgb \"#FF8C00\"",  // dark orange	
											 "lt rgb \"#708090\""}; // slate grey

const std::vector<std::string> GNUPLOT_MARKERS{"pt 5",
											   "pt 7",
											   "pt 9",
											   "pt 11",
											   "pt 13",
											   "pt 15",
											   "pt 1"};

const std::vector<std::string> GNUPLOT_LINETYPES{"dt 1",
												 "dt 2",
												 "dt 3",
												 "dt 4"};

#define UPDATE_TEXT 12000
#define CHANGE_FORMAT 12001
#define NO_FIT 12002
#define REAL_TIME_FIT 12003
#define FIT_AT_END 12004
#define GAUSSIAN_FIT 12005
#define LORENTZIAN_FIT 12006
#define SINE_FIT 12007


// ID values for Windows API Objects
#define IDC_PB_START_BUTTON 7000
#define IDC_PB_ABORT_BUTTON 7001
#define IDC_PB_CLOSE_BUTTON 7002
#define IDC_SET_TEMP_BUTTON 7003
#define IDC_PB_COOLER_OFF_BUTTON 7004
#define ID_TEMPERATURE_TIMER 200
#define ID_STATUS_TIMER 100
#define IDC_STATUS_EDIT 7005
#define NUM_PALETTE_COLORS 256
#define IDC_INCREMENT_FILE_OPTION_BUTTON 7006
#define IDC_PIXEL_1X_EDIT 7007
#define IDC_PIXEL_2X_EDIT 7008
#define IDC_PIXEL_1_DISPLAY 7009
#define IDC_PIXEL_2_DISPLAY 7010
#define IDC_PIXEL_1Y_EDIT 7011
#define IDC_PIXEL_2Y_EDIT 7012
#define IDC_TRIGGER_COMBO 7013

#define IDC_TEMP_EDIT 7015
#define IDC_REPETITIONS_PER_VARIATION_EDIT 7016
#define IDC_KINETIC_CYCLE_TIME_EDIT 7017
#define IDC_IMG_LEFT_EDIT 7018
#define IDC_IMG_RIGHT_EDIT 7019
#define IDC_IMAGE_BOTTOM_EDIT 7020
#define IDC_IMAGE_TOP_EDIT 7021
#define IDC_HOR_BIN_EDIT 7022
#define IDC_VERTICAL_BIN_EDIT 7023
#define IDC_EXPOSURE_DISP 7024
#define IDC_TRIGGER_DISP 7025
#define IDC_STATUS_DISP 7026
#define IDC_CURRENT_TEMP_DISP 7027
// missing #s
#define IDC_KINETIC_CYLCE_TIME_TEXT 7034
#define IDC_SCAN_NUMBER_TEXT 7035
#define IDC_TEMP_DISP_EDIT 7036
#define IDC_SET_EXPOSURE_BUTTON 7037
#define IDC_SET_IMAGE_PARAMS_BUTTON 7038
#define IDC_IMG_LEFT_DISP 7039
#define IDC_IMG_RIGHT_DISP 7040
#define IDC_HOR_BIN_DISP 7041
#define IDC_IMG_TOP_DISP 7042
#define IDC_IMG_BOTTOM_DISP 7043
#define IDC_VERTICAL_BIN_DISP 7044
#define IDC_SET_KINETIC_CYCLE_TIME_BUTTON 7052
#define IDC_KINETIC_CYCLE_TIME_DISP 7053
#define IDC_SET_REPETITONS_PER_VARIATION_BUTTON 7054
#define IDC_REPETITIONS_PER_VARIATION_DISP 7055
#define IDC_SET_DETECTION_THRESHOLD_BUTTON 7056
#define IDC_DETECTION_THRESHOLD_EDIT_HANDLE 7057
#define IDC_DETECTION_THRESHOLD_DISP_HANDLE 7058
#define IDC_ACCUMULATION_NUM_DISP 7063
#define IDC_MAX_COUNT_DISP 7064
#define IDC_MIN_COUNT_DISP 7065
#define IDC_SET_TUNNELING_PAIRS_BUTTON 7067
#define IDC_TUNNELING_PAIRS_EDIT_HANDLE 7068
#define IDC_TUNNELING_PAIRS_DISP_HANDLE 7069
#define IDC_SET_VARIATION_NUMBER 7070
#define IDC_VARIATION_EDIT 7071
#define IDC_VARIATION_DISP_HANDLE 7072
#define IDC_CLEAR_STATUS_BUTTON 7074
#define IDC_SET_PICTURES_PER_REPITITION_BUTTON 7075
#define IDC_PICTURES_PER_REPETITION_DISP 7076
#define IDC_PICTURES_PER_REPETITION_EDIT 7077
#define IDC_RICH_EDIT_MESSAGE_BOX_RICH_EDIT_ID 7078
#define IDC_RICH_EDIT_OK_CANCEL_MESSAGE_BOX_RICH_EDIT_ID 7079
#define IDC_ERROR_EDIT 7080
#define IDC_CAMERA_MODE_COMBO 7081
#define IDC_SET_ACCUMULATION_TIME_BUTTON 7082
#define IDC_ACCUMULATION_TIME_EDIT 7083
#define IDC_ACCUMULATION_TIME_DISP 7084
#define IDC_SET_ACCUMULATION_NUMBER_BUTTON 7085
#define IDC_ACCUMULATION_NUMBER_EDIT 7086
#define IDC_ACCUMULATION_NUMBER_DISP 7087
#define IDC_RUNNING_AVERAGE_BUTTON 7088
#define IDC_RUNNING_AVERAGE_BOX 7089
#define IDC_RUNNING_AVERAGE_DISP 7090
#define IDC_RUNNING_AVERAGE_EDIT 7091
#define IDC_PLOTTING_FREQUENCY_BUTTON 7092
#define IDC_PLOTTING_FREQUENCY_EDIT 7093
#define IDC_PLOTTING_FREQUENCY_DISP 7094
#define IDC_PLOTTING_COMBO 7095
#define IDC_PLOTTING_TEXT 7096
#define IDC_CURRENT_PLOTS_TEXT 7097
#define IDC_CURRENT_PLOTS_COMBO 7098
#define IDC_SERIES_PROGRESS_BAR 7099
#define IDC_MIN_SLIDER_1 7100
#define IDC_MAX_SLIDER_1 7101
#define IDC_MIN_SLIDER_1_TEXT 7102
#define IDC_MAX_SLIDER_1_TEXT 7103
#define IDC_MIN_SLIDER_1_NUMBER_EDIT 7104
#define IDC_MAX_SLIDER_1_NUMBER_EDIT 7105
#define IDC_SUBSERIES_PROGRESS_BAR 7106
#define IDC_MIN_SLIDER_2 7107
#define IDC_MAX_SLIDER_2 7108
#define IDC_MIN_SLIDER_2_TEXT 7109
#define IDC_MAX_SLIDER_2_TEXT 7110
#define IDC_MIN_SLIDER_2_NUMBER_EDIT 7111
#define IDC_MAX_SLIDER_2_NUMBER_EDIT 7112
#define IDC_MIN_SLIDER_3 7112
#define IDC_MAX_SLIDER_3 7113
#define IDC_MIN_SLIDER_3_TEXT 7114
#define IDC_MAX_SLIDER_3_TEXT 7115
#define IDC_MIN_SLIDER_3_NUMBER_EDIT 7116
#define IDC_MAX_SLIDER_3_NUMBER_EDIT 7117
#define IDC_MIN_SLIDER_4 7118
#define IDC_MAX_SLIDER_4 7119
#define IDC_MIN_SLIDER_4_TEXT 7120
#define IDC_MAX_SLIDER_4_TEXT 7121
#define IDC_MIN_SLIDER_4_NUMBER_EDIT 7122
#define IDC_MAX_SLIDER_4_NUMBER_EDIT 7123
#define IDC_SELECTION_DISP 7124
#define IDC_PICTURE_DISP 7125
#define IDC_PICTURE_1_DISP 7126
#define IDC_PICTURE_2_DISP 7127
#define IDC_PICTURE_3_DISP 7128
#define IDC_PICTURE_4_DISP 7129
#define IDC_MAX_COUNT_1_DISP 7130
#define IDC_MAX_COUNT_2_DISP 7131
#define IDC_MAX_COUNT_3_DISP 7132
#define IDC_MAX_COUNT_4_DISP 7133
#define IDC_MIN_COUNT_1_DISP 7134
#define IDC_MIN_COUNT_2_DISP 7135
#define IDC_MIN_COUNT_3_DISP 7136
#define IDC_MIN_COUNT_4_DISP 7137
#define IDC_SELECTION_1_DISP 7138
#define IDC_SELECTION_2_DISP 7139
#define IDC_SELECTION_3_DISP 7140
#define IDC_SELECTION_4_DISP 7141
#define IDC_EM_GAIN_MODE_BOX 7142
#define IDC_EXPOSURE_1_EDIT 7143
#define IDC_EXPOSURE_2_EDIT 7144
#define IDC_EXPOSURE_3_EDIT 7145
#define IDC_EM_GAIN_MODE_EDIT 7146
#define IDC_EM_GAIN_MODE_DISP 7147
#define IDC_SET_EM_GAIN_MODE 7148
#define IDC_ERROR_CLEAR_BUTTON 7149
#define IDC_TEMP_OFF_BUTTON 7150
#define ID_GREEN 7151
#define ID_RED 7152
#define IDC_CONFIGURATION_COMBO 7153
#define IDC_TIME_DISPLAY 7154
#define ID_BLUE 7155
#define IDC_PEOPLE_LIST_VIEW 7156
#define IDC_MOST_RECENT_DATA_SET_NUMBER 7157
#define IDC_EXPOSURE_4_EDIT 7158
#define IDC_AUTOANALYZE_CHECKBOX 7159
#define IDC_DATA_OUTPUT_NAME_COMBO 7160
#define IDC_DATA_AUTOANALYSIS_COMBO 7161
#define IDC_SET_ANALYSIS_LOCATION 7162
#define IDC_ANALYZE_MOST_RECENT 7163
#define IDC_DATA_OUTPUT_NAME_DETAILS_COMBO 7164
