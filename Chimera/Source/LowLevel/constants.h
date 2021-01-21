// created by Mark O. Brown
#pragma once

#include "Agilent/agilentStructures.h"
#include "Microwave/WindFreakFlume.h"
#include "Microwave/RsgFlume.h"
#include "Microwave/microwaveSettings.h"
#include "GeneralUtilityFunctions/my_str.h"
#include "Piezo/PiezoType.h"
#include <string>
#include <vector>
#include <array>
#include <pylon/PylonIncludes.h>
#include <pylon/usb/BaslerUsbInstantCamera.h>
#include <pylon/1394/Basler1394InstantCamera.h>

// running in safemode means that the program doesn't actually try to connect to physical devices. Generally, it will 
// follow the exact coding logic, but at the exact point where it would normally communicate with a device, it will 
// instead simply skip this step. It might generate example data where useful / necessary (e.g. after querying a
// camera system for a picture). It can be used to build and debug other aspects of the program, such as the gui, 
// coding logic, etc.
 
#define MASTER_COMPUTER
//#define MARKS_LAPTOP 
//#define ANALYSIS_COMPUTER 
//#define HOME_DESKTOP
//#define TEST_PC

	#ifdef MASTER_COMPUTER
	using MICROWAVE_FLUME = RsgFlume;
	constexpr microwaveDevice MICROWAVE_SYSTEM_DEVICE_TYPE = microwaveDevice::WindFreak;
	constexpr auto DM_SERIAL = "25CW012#060";
	constexpr bool DM_SAFEMODE = true;
	constexpr char UW_SYSTEM_ADDRESS[] = "COM10";
	//constexpr short UW_SYSTEM_ADDRESS = 28;
	constexpr bool UW_SYSTEM_SAFEMODE = false;
	constexpr bool DOFTDI_SAFEMODE = false;
	constexpr bool DDS_SAFEMODE = false;
	constexpr bool BASLER_SAFEMODE = false;
	constexpr bool NIAWG_SAFEMODE = false;
	constexpr bool ANDOR_SAFEMODE = false;
	#ifdef _DEBUG
	constexpr bool PYTHON_SAFEMODE = true;
	#else
	constexpr bool  PYTHON_SAFEMODE = true;
	#endif
	constexpr bool DAQMX_SAFEMODE = false;
	constexpr bool TOP_BOTTOM_TEK_SAFEMODE = true;
	constexpr auto TOP_BOTTOM_TEK_USB_ADDRESS = "USB0::0x0699::0x0343::C021681::0::INSTR";
	constexpr bool EO_AXIAL_TEK_SAFEMODE = false;
	constexpr auto EO_AXIAL_TEK_USB_ADDRESS = "USB0::0x0699::0x034C::C010386::0::INSTR";
	constexpr bool TOP_BOTTOM_AGILENT_SAFEMODE = false;
	constexpr auto TOP_BOTTOM_AGILENT_USB_ADDRESS = "USB0::0x0957::0x2307::MY50002574::0::INSTR";
	constexpr bool AXIAL_AGILENT_SAFEMODE = false;
	constexpr auto AXIAL_AGILENT_USB_ADDRESS = "USB0::0x0957::0x2C07::MY52810615::0::INSTR";
	constexpr bool INTENSITY_SAFEMODE = false;
	constexpr auto INTENSITY_AGILENT_USB_ADDRESS = "USB0::0x0957::0x2307::MY50004500::0::INSTR";
	constexpr bool FLASHING_SAFEMODE = false;
	constexpr auto FLASHING_AGILENT_USB_ADDRESS = "USB0::0x0957::0x2307::MY50003003::0::INSTR";
	constexpr bool UWAVE_SAFEMODE = true;
	constexpr auto UWAVE_AGILENT_USB_ADDRESS = "USB0::0x0957::0x2C07::MY52801397::0::INSTR";
	constexpr bool MASTER_REPUMP_SCOPE_SAFEMODE = false;
	constexpr auto MASTER_REPUMP_SCOPE_ADDRESS = "USB0::0x0699::0x03B3::C011388::0::INSTR";
	constexpr bool MOT_SCOPE_SAFEMODE = false;
	constexpr auto MOT_SCOPE_ADDRESS = "USB0::0x0699::0x0363::C100939::0::INSTR";
	constexpr bool EXPERIMENT_SCOPE_SAFEMODE = true;
	constexpr auto EXPERIMENT_SCOPE_ADDRESS = "USB0::0x0699::0x0413::C011471::INSTR";
	constexpr bool ANALOG_OUT_SAFEMODE = false;
	constexpr bool ANALOG_IN_SAFEMODE = false;

	constexpr auto CODE_ROOT = "C:\\Users\\Regal-Lab\\Code\\Chimera-Control";
	const std::string PLOT_FILES_SAVE_LOCATION = str (CODE_ROOT) + "\\Plotting";
	const std::string DATA_ANALYSIS_CODE_LOCATION = "C:\\Users\\Regal-Lab\\Code\\Data_Analysis_Control\\";
	//const std::string LIB_PATH = str (CODE_ROOT) + "\\Waveforms-Library\\WaveLibrary-320-MSpS-Gain_0p82\\";
	const std::string LIB_PATH = str (CODE_ROOT) + "\\Waveforms-Library\\WaveLibrary-320-MSpS-New\\";
	const std::string DEFAULT_SCRIPT_FOLDER_PATH = str (CODE_ROOT) + "\\Default-Scripts\\";
	const std::string CODE_LOGGING_FILES_PATH = "D:\\Code Versions\\";
	const std::string ACTUAL_CODE_FOLDER_PATH = str (CODE_ROOT) + "\\Chimera\\";
	const std::string PROFILES_PATH = str (CODE_ROOT) + "\\Profiles\\";
	const std::string DATA_SAVE_LOCATION = "J:\\Data Repository\\New Data Repository\\";
	const std::string KEY_ORIGINAL_SAVE_LOCATION = "C:\\Shared\\";
	const std::string MUSIC_LOCATION = str (CODE_ROOT) + "\\Final Fantasy VII - Victory Fanfare [HQ].mp3";
	const std::string FUNCTIONS_FOLDER_LOCATION = str (CODE_ROOT) + "\\Functions\\";
	const std::string MASTER_CONFIGURATION_FILE_ADDRESS (str (CODE_ROOT) + "\\Master-Configuration.txt");
	const std::string LOGGING_FILE_ADDRESS = str (CODE_ROOT) + "\\Master-Log.txt";
	const std::string MOT_ROUTINES_ADDRESS = PROFILES_PATH + "Standard_Calibrations\\MOT";
	const std::string PGC_ROUTINES_ADDRESS = PROFILES_PATH + "Standard_Calibrations\\PGC";
	const std::string CAMERA_CAL_ROUTINE_ADDRESS = PROFILES_PATH + "Standard_Calibrations\\Camera";
	const std::string DEBUG_OUTPUT_LOCATION = str (CODE_ROOT) + "\\Debug-Output\\";
	const std::string TIMING_OUTPUT_LOCATION = DATA_SAVE_LOCATION + "\\2017\\September\\September 8\\Raw Data\\";
	const std::string RAMP_LOCATION = str (CODE_ROOT) + "\\Ramp_Files\\";
	// location where wave data can be outputted for analyzing with another computer.
	const std::string NIAWG_WAVEFORM_OUTPUT_LOCATION = DATA_SAVE_LOCATION + "2017\\September\\September 7\\Raw Data\\";
	const piezoSetupInfo PIEZO_1_INFO = { PiezoType::B, "COM5", "PIEZO_CONTROLLER_1", true, false };
	const piezoSetupInfo PIEZO_2_INFO = { PiezoType::B, "COM4", "PIEZO_CONTROLLER_2", true, false };
	const piezoSetupInfo PIEZO_3_INFO = { PiezoType::A, "COM11", "PIEZO_CONTROLLER_3", true, false };
	const piezoSetupInfo IMG_PIEZO_INFO = { PiezoType::B, "COM8", "IMAGING_PIEZO", false, true };

	const std::string DM_PROFILES_LOCATION = str(CODE_ROOT) + "\\DM-Library";
	const std::string DM_FLAT_PROFILE = DM_PROFILES_LOCATION + "\\flatProfile.txt";
#endif

/// File Locations and safemode options
#ifdef TEST_PC
	#define DDS_SAFEMODE true
	#define BASLER_SAFEMODE true
	#define PYTHON_HOME L"C:\\ProgramData\\Python37\\"
	#define NIAWG_SAFEMODE true
	#define ANDOR_SAFEMODE true
	#define PYTHON_SAFEMODE true
	#define VIEWPOINT_SAFEMODE true
	#define DIOFTDI_SAFEMODE false
	#define ANALOG_OUT_SAFEMODE true
	#define ANALOG_IN_SAFEMODE true
	#define RSG_SAFEMODE true
	#define TOP_BOTTOM_TEK_SAFEMODE true
	#define TOP_BOTTOM_TEK_USB_ADDRESS "USB0::0x0699::0x0343::C021681::0::INSTR"
	#define EO_AXIAL_TEK_SAFEMODE true
	#define EO_AXIAL_TEK_USB_ADDRESS "USB0::0x0699::0x034C::C010386::0::INSTR"
	#define TOP_BOTTOM_AGILENT_SAFEMODE true
	#define TOP_BOTTOM_AGILENT_USB_ADDRESS "USB0::0x0957::0x2307::MY50002574::0::INSTR"
	#define AXIAL_AGILENT_SAFEMODE true
	#define AXIAL_AGILENT_USB_ADDRESS "USB0::0x0957::0x2C07::MY52810615::0::INSTR"
	#define INTENSITY_SAFEMODE true
	#define INTENSITY_AGILENT_USB_ADDRESS "USB0::0x0957::0x2307::MY50004500::0::INSTR"
	#define FLASHING_SAFEMODE true
	#define FLASHING_AGILENT_USB_ADDRESS "USB0::0x0957::0x2307::MY50003003::0::INSTR"
	#define UWAVE_SAFEMODE true
	//#define UWAVE_AGILENT_USB_ADDRESS "USB0::0x0957::0x2C07::MY52801397::0::INSTR"
	#define UWAVE_AGILENT_USB_ADDRESS "USB0::2391::11271::MY52801397::0::INSTR"

	#define MASTER_REPUMP_SCOPE_SAFEMODE true
	#define MASTER_REPUMP_SCOPE_ADDRESS "USB0::0x0957::0x2C07::MY52801397::0::INSTR"
	#define MOT_SCOPE_SAFEMODE true
	#define MOT_SCOPE_ADDRESS			"USB0::0x0957::0x2C07::MY52801397::0::INSTR"

	const std::string PROJECT_LOCATION = "C:\\Users\\Max\\64bitControlCode\\";
	const std::string PYTHON_CODE_LOCATION = "C:/Users/Max/64bitControlCode/Chimera";
	const std::string DATA_ANALYSIS_CODE_LOCATION = "C:\\Users\\Mark-Brown\\Code\\Data_Analysis_Code\\";
	// same as debug output location but with forward slashes for ease of use in python
	const std::string PYTHON_INPUT_LOCATION = "C:/Users/Max/64bitControlCode/Debug-Output/";
	const std::string PLOT_FILES_SAVE_LOCATION = PROJECT_LOCATION + "Plotting";
	const std::string LIB_PATH = PROJECT_LOCATION + "Waveforms-Library\\dummyLib\\";
	const std::string DEFAULT_SCRIPT_FOLDER_PATH = PROJECT_LOCATION + "Default-Scripts\\";
	const std::string PROFILES_PATH = PROJECT_LOCATION + "Profiles\\";
	const std::string DATA_SAVE_LOCATION = PROJECT_LOCATION + "Data\\";
	const std::string MUSIC_LOCATION = PROJECT_LOCATION + "Camerawg\\Final Fantasy VII - Victory Fanfare [HQ].mp3";
	const std::string FUNCTIONS_FOLDER_LOCATION = PROJECT_LOCATION + "Functions\\";
	const std::string MASTER_CONFIGURATION_FILE_ADDRESS = PROJECT_LOCATION + "Master-Configuration.txt";
	const std::string MOT_ROUTINES_ADDRESS = PROJECT_LOCATION + "Profiles\\Hotkey Experiments\\MOT";
	const std::string PGC_ROUTINES_ADDRESS = PROJECT_LOCATION + "Profiles\\Hotkey Experiments\\PGC";
	const std::string CAMERA_CAL_ROUTINE_ADDRESS = PROJECT_LOCATION + "Profiles\\Hotkey Experiments\\Camera";
	const std::string DEBUG_OUTPUT_LOCATION = PROJECT_LOCATION + "Debug-Output\\";
	const std::string TIMING_OUTPUT_LOCATION = PROJECT_LOCATION + "\\Data\\";
	const std::string NIAWG_WAVEFORM_OUTPUT_LOCATION = TIMING_OUTPUT_LOCATION;
#endif
	/// File Locations and safemode options
#ifdef HOME_DESKTOP
	using MICROWAVE_FLUME = WindFreakFlume;
	constexpr microwaveDevice MICROWAVE_SYSTEM_DEVICE_TYPE = microwaveDevice::WindFreak;
	constexpr auto DM_SERIAL = "25CW012#060";
	constexpr bool DM_SAFEMODE = true;
	constexpr char UW_SYSTEM_ADDRESS[] = "COM7";
	constexpr bool AUTO_CALIBRATE = false;
	constexpr bool DOFTDI_SAFEMODE = true;
	constexpr bool DDS_SAFEMODE = true;
	constexpr bool BASLER_SAFEMODE = true;
	constexpr wchar_t* PYTHON_HOME = L"C:\\ProgramData\\Anaconda3\\";
	constexpr bool NIAWG_SAFEMODE = true;
	constexpr bool ANDOR_SAFEMODE = true;
	constexpr bool PYTHON_SAFEMODE = true;
	constexpr bool VIEWPOINT_SAFEMODE = true;
	constexpr bool ANALOG_OUT_SAFEMODE = true;
	constexpr bool ANALOG_IN_SAFEMODE = true;
	constexpr bool UW_SYSTEM_SAFEMODE = true;
	constexpr bool TOP_BOTTOM_TEK_SAFEMODE = true;
	constexpr auto TOP_BOTTOM_TEK_USB_ADDRESS = "USB0::0x0699::0x0343::C021681::0::INSTR";
	constexpr bool EO_AXIAL_TEK_SAFEMODE = true;
	constexpr auto EO_AXIAL_TEK_USB_ADDRESS = "USB0::0x0699::0x034C::C010386::0::INSTR";
	constexpr bool TOP_BOTTOM_AGILENT_SAFEMODE = true;
	constexpr auto TOP_BOTTOM_AGILENT_USB_ADDRESS = "USB0::0x0957::0x2307::MY50002574::0::INSTR";
	constexpr bool AXIAL_AGILENT_SAFEMODE = true;
	constexpr auto AXIAL_AGILENT_USB_ADDRESS = "USB0::0x0957::0x2C07::MY52810615::0::INSTR";
	constexpr bool INTENSITY_SAFEMODE = true;
	constexpr auto INTENSITY_AGILENT_USB_ADDRESS = "USB0::0x0957::0x2307::MY50004500::0::INSTR";
	constexpr bool FLASHING_SAFEMODE = true;
	constexpr auto FLASHING_AGILENT_USB_ADDRESS = "USB0::0x0957::0x2307::MY50003003::0::INSTR";
	constexpr bool UWAVE_SAFEMODE = true;
	//#define UWAVE_AGILENT_USB_ADDRESS "USB0::0x0957::0x2C07::MY52801397::0::INSTR"
	constexpr auto UWAVE_AGILENT_USB_ADDRESS = "USB0::2391::11271::MY52801397::0::INSTR";

	constexpr bool MASTER_REPUMP_SCOPE_SAFEMODE = true;
	constexpr auto MASTER_REPUMP_SCOPE_ADDRESS = "USB0::0x0957::0x2C07::MY52801397::0::INSTR";
	constexpr bool MOT_SCOPE_SAFEMODE = true;
	constexpr auto MOT_SCOPE_ADDRESS = "USB0::0x0957::0x2C07::MY52801397::0::INSTR";

	const std::string PROJECT_LOCATION = "C:\\Users\\Mark-Brown\\Code\\Chimera-Control\\";
	const std::string DM_PROFILES_LOCATION = PROJECT_LOCATION + "DM-Library";
	const std::string DM_FLAT_PROFILE = DM_PROFILES_LOCATION + +"\\25CW012#060_CLOSED_LOOP_COMMANDS.txt";
	const std::string PYTHON_CODE_LOCATION = "C:/Users/Mark-Brown/Code/Chimera-Control/Chimera";
	const std::string DATA_ANALYSIS_CODE_LOCATION = "C:\\Users\\Mark-Brown\\Code\\Data_Analysis_Code\\";
	// same as debug output location but with forward slashes for ease of use in python
	const std::string PYTHON_INPUT_LOCATION = "C:/Users/Mark-Brown/Code/Chimera-Control/Debug-Output/";
	const std::string PLOT_FILES_SAVE_LOCATION = PROJECT_LOCATION + "Plotting";
	const std::string LIB_PATH = PROJECT_LOCATION + "Waveforms-Library\\dummyLib\\";
	const std::string DEFAULT_SCRIPT_FOLDER_PATH = PROJECT_LOCATION + "Default-Scripts\\";
	const std::string PROFILES_PATH = PROJECT_LOCATION + "Profiles\\";
	const std::string DATA_SAVE_LOCATION = PROJECT_LOCATION + "Data\\";
	const std::string MUSIC_LOCATION = PROJECT_LOCATION + "Camerawg\\Final Fantasy VII - Victory Fanfare [HQ].mp3";
	const std::string FUNCTIONS_FOLDER_LOCATION = PROJECT_LOCATION + "Functions\\";
	const std::string MASTER_CONFIGURATION_FILE_ADDRESS = PROJECT_LOCATION + "Master-Configuration.txt";
	const std::string MOT_ROUTINES_ADDRESS = PROJECT_LOCATION + "Profiles\\Hotkey Experiments\\MOT";
	const std::string PGC_ROUTINES_ADDRESS = PROJECT_LOCATION + "Profiles\\Hotkey Experiments\\PGC";
	const std::string CAMERA_CAL_ROUTINE_ADDRESS = PROJECT_LOCATION + "Profiles\\Hotkey Experiments\\Camera";
	const std::string DEBUG_OUTPUT_LOCATION = PROJECT_LOCATION + "Debug-Output\\";
	const std::string TIMING_OUTPUT_LOCATION = PROJECT_LOCATION + "\\Data\\";
	const std::string NIAWG_WAVEFORM_OUTPUT_LOCATION = TIMING_OUTPUT_LOCATION;
	const piezoSetupInfo PIEZO_1_INFO = { PiezoType::NONE, "COM5", "PIEZO_CONTROLLER_1" };
	const piezoSetupInfo PIEZO_2_INFO = { PiezoType::NONE, "COM4", "PIEZO_CONTROLLER_2" };
#endif

/// File Locations and safemode options
#ifdef MARKS_LAPTOP
	using MICROWAVE_FLUME = WindFreakFlume;
	constexpr microwaveDevice MICROWAVE_SYSTEM_DEVICE_TYPE = microwaveDevice::WindFreak;
	constexpr auto DM_SERIAL = "25CW012#060";
	constexpr bool DM_SAFEMODE = true;
	constexpr char UW_SYSTEM_ADDRESS[] = "COM7";
	constexpr bool AUTO_CALIBRATE = false;
	constexpr bool DOFTDI_SAFEMODE = true;
	constexpr bool DDS_SAFEMODE = true;
	constexpr bool BASLER_SAFEMODE = true;
	constexpr auto PYTHON_HOME = "C:\\ProgramData\\Anaconda3\\";
	constexpr bool NIAWG_SAFEMODE = true;
	constexpr bool ANDOR_SAFEMODE = true;
	constexpr bool PYTHON_SAFEMODE = true;
	constexpr bool VIEWPOINT_SAFEMODE = true;
	constexpr bool ANALOG_OUT_SAFEMODE = true;
	constexpr bool ANALOG_IN_SAFEMODE = true;
	constexpr bool UW_SYSTEM_SAFEMODE = true;
	constexpr bool TOP_BOTTOM_TEK_SAFEMODE = true;
	constexpr auto TOP_BOTTOM_TEK_USB_ADDRESS = "USB0::0x0699::0x0343::C021681::0::INSTR";
	constexpr bool EO_AXIAL_TEK_SAFEMODE = true;
	constexpr auto EO_AXIAL_TEK_USB_ADDRESS = "USB0::0x0699::0x034C::C010386::0::INSTR";
	constexpr bool TOP_BOTTOM_AGILENT_SAFEMODE = true;
	constexpr auto TOP_BOTTOM_AGILENT_USB_ADDRESS = "USB0::0x0957::0x2307::MY50002574::0::INSTR";
	constexpr bool AXIAL_AGILENT_SAFEMODE = true;
	constexpr auto AXIAL_AGILENT_USB_ADDRESS = "USB0::0x0957::0x2C07::MY52810615::0::INSTR";
	constexpr bool INTENSITY_SAFEMODE = true;
	constexpr auto INTENSITY_AGILENT_USB_ADDRESS = "USB0::0x0957::0x2307::MY50004500::0::INSTR";
	constexpr bool FLASHING_SAFEMODE = true;
	constexpr auto FLASHING_AGILENT_USB_ADDRESS = "USB0::0x0957::0x2307::MY50003003::0::INSTR";
	constexpr bool UWAVE_SAFEMODE = true;
	//#define UWAVE_AGILENT_USB_ADDRESS "USB0::0x0957::0x2C07::MY52801397::0::INSTR"
	constexpr auto UWAVE_AGILENT_USB_ADDRESS = "USB0::2391::11271::MY52801397::0::INSTR";

	constexpr bool MASTER_REPUMP_SCOPE_SAFEMODE = true;
	constexpr auto MASTER_REPUMP_SCOPE_ADDRESS = "USB0::0x0957::0x2C07::MY52801397::0::INSTR";
	constexpr bool MOT_SCOPE_SAFEMODE = true;
	constexpr auto MOT_SCOPE_ADDRESS = "USB0::0x0957::0x2C07::MY52801397::0::INSTR";
	const std::string PROJECT_LOCATION = "C:\\Users\\Mark-Brown\\Code\\Chimera-Control\\";
	const std::string DM_PROFILES_LOCATION = PROJECT_LOCATION + "DM-Library";
	const std::string DM_FLAT_PROFILE = DM_PROFILES_LOCATION + +"\\25CW012#060_CLOSED_LOOP_COMMANDS.txt";
	const std::string PYTHON_CODE_LOCATION = "C:/Users/Mark-Brown/Code/Chimera-Control/Chimera";
	const std::string DATA_ANALYSIS_CODE_LOCATION = "C:\\Users\\Mark-Brown\\Code\\Data_Analysis_Code\\";
	// same as debug output location but with forward slashes for ease of use in python
	const std::string PYTHON_INPUT_LOCATION = "C:/Users/Mark-Brown/Code/Chimera-Control/Debug-Output/";
	const std::string PLOT_FILES_SAVE_LOCATION = PROJECT_LOCATION + "Plotting";
	const std::string LIB_PATH = PROJECT_LOCATION + "Waveforms-Library\\dummyLib\\";
	const std::string DEFAULT_SCRIPT_FOLDER_PATH = PROJECT_LOCATION + "Default-Scripts\\";
	const std::string PROFILES_PATH = PROJECT_LOCATION + "Profiles\\";
	const std::string DATA_SAVE_LOCATION = PROJECT_LOCATION + "Data\\";
	const std::string MUSIC_LOCATION = PROJECT_LOCATION + "Camerawg\\Final Fantasy VII - Victory Fanfare [HQ].mp3";
	const std::string FUNCTIONS_FOLDER_LOCATION = PROJECT_LOCATION + "Functions\\";
	const std::string MASTER_CONFIGURATION_FILE_ADDRESS = PROJECT_LOCATION + "Master-Configuration.txt";
	const std::string MOT_ROUTINES_ADDRESS = PROJECT_LOCATION + "Profiles\\Hotkey Experiments\\MOT";
	const std::string PGC_ROUTINES_ADDRESS = PROJECT_LOCATION + "Profiles\\Hotkey Experiments\\PGC";
	const std::string CAMERA_CAL_ROUTINE_ADDRESS = PROJECT_LOCATION + "Profiles\\Hotkey Experiments\\Camera";
	const std::string DEBUG_OUTPUT_LOCATION = PROJECT_LOCATION + "Debug-Output\\";
	const std::string TIMING_OUTPUT_LOCATION = PROJECT_LOCATION + "\\Data\\";
	const std::string NIAWG_WAVEFORM_OUTPUT_LOCATION = TIMING_OUTPUT_LOCATION;
	const piezoSetupInfo PIEZO_1_INFO = { PiezoType::NONE, "COM5", "PIEZO_CONTROLLER_1" };
	const piezoSetupInfo PIEZO_2_INFO = { PiezoType::NONE, "COM4", "PIEZO_CONTROLLER_2" };
#endif
	

#ifdef ANALYSIS_COMPUTER
	#define DIOFTDI_SAFEMODE true
	const PiezoType PIEZO_1_TYPE = PiezoType::NONE;
	const PiezoType PIEZO_2_TYPE = PiezoType::NONE;
	#define DDS_SAFEMODE true
	#define BASLER_SAFEMODE true
	#define PYTHON_HOME L"C:\\ProgramData\\Anaconda3\\"
	#define NIAWG_SAFEMODE true
	#define ANDOR_SAFEMODE true
	#define PYTHON_SAFEMODE true
	#define VIEWPOINT_SAFEMODE true
	#define ANALOG_OUT_SAFEMODE true
	#define ANALOG_IN_SAFEMODE true
	#define RSG_SAFEMODE true
	#define TOP_BOTTOM_TEK_SAFEMODE true
	#define TOP_BOTTOM_TEK_USB_ADDRESS "USB0::0x0699::0x0343::C021681::0::INSTR"
	#define EO_AXIAL_TEK_SAFEMODE true
	#define EO_AXIAL_TEK_USB_ADDRESS "USB0::0x0699::0x034C::C010386::0::INSTR"
	#define TOP_BOTTOM_AGILENT_SAFEMODE true
	#define TOP_BOTTOM_AGILENT_USB_ADDRESS "USB0::0x0957::0x2307::MY50002574::0::INSTR"
	#define AXIAL_AGILENT_SAFEMODE true
	#define AXIAL_AGILENT_USB_ADDRESS "USB0::0x0957::0x2C07::MY52810615::0::INSTR"
	#define INTENSITY_SAFEMODE true
	#define INTENSITY_AGILENT_USB_ADDRESS "USB0::0x0957::0x2307::MY50004500::0::INSTR"
	#define FLASHING_SAFEMODE true
	#define FLASHING_AGILENT_USB_ADDRESS "USB0::0x0957::0x2307::MY50003003::0::INSTR"
	#define UWAVE_SAFEMODE true
	#define UWAVE_AGILENT_USB_ADDRESS "USB0::0x0957::0x2C07::MY52801397::0::INSTR"
	#define MASTER_REPUMP_SCOPE_SAFEMODE true
	#define MASTER_REPUMP_SCOPE_ADDRESS "USB0::0x0957::0x2C07::MY52801397::0::INSTR"
	#define MOT_SCOPE_SAFEMODE true
	#define MOT_SCOPE_ADDRESS			"USB0::0x0957::0x2C07::MY52801397::0::INSTR"
	const std::string DATA_ANALYSIS_CODE_LOCATION = "C:\\Users\\Regal-Lab\\Code\\Data_Analysis_Code\\";
	const std::string PYTHON_CODE_LOCATION = "C:/Users/Regal-Lab/Code/Chimera-Control/Chimera";
	const std::string PROJECT_LOCATION = "C:\\Users\\Regal-Lab\\Code\\Chimera-Control\\";
	//const std::string PYTHON_CODE_LOCATION = PROJECT_LOCATION + "\\Chimera";
	// same as debug output location but with forward slashes for ease of use in python
	const std::string PYTHON_INPUT_LOCATION = PROJECT_LOCATION + "\\Debug-Output\\";
	const std::string PLOT_FILES_SAVE_LOCATION = PROJECT_LOCATION + "Plotting";
	const std::string LIB_PATH = PROJECT_LOCATION + "Waveforms-Library\\dummyLib\\";
	const std::string DEFAULT_SCRIPT_FOLDER_PATH = PROJECT_LOCATION + "Default-Scripts\\";
	const std::string PROFILES_PATH = PROJECT_LOCATION + "Profiles\\";
	const std::string DATA_SAVE_LOCATION = PROJECT_LOCATION + "Data\\";
	const std::string MUSIC_LOCATION = PROJECT_LOCATION + "Camerawg\\Final Fantasy VII - Victory Fanfare [HQ].mp3";
	const std::string FUNCTIONS_FOLDER_LOCATION = PROJECT_LOCATION + "Functions\\";
	const std::string MASTER_CONFIGURATION_FILE_ADDRESS = PROJECT_LOCATION + "Master-Configuration.txt";
	const std::string MOT_ROUTINES_ADDRESS = PROJECT_LOCATION + "Profiles\\Hotkey Experiments\\MOT";
	const std::string PGC_ROUTINES_ADDRESS = PROJECT_LOCATION + "Profiles\\Hotkey Experiments\\PGC";
	const std::string CAMERA_CAL_ROUTINE_ADDRESS = PROJECT_LOCATION + "Profiles\\Hotkey Experiments\\Camera";
	const std::string DEBUG_OUTPUT_LOCATION = PROJECT_LOCATION + "Debug-Output\\";
	const std::string TIMING_OUTPUT_LOCATION = PROJECT_LOCATION + "\\Data\\";
	const std::string NIAWG_WAVEFORM_OUTPUT_LOCATION = TIMING_OUTPUT_LOCATION;
#endif

#define FIREWIRE_CAMERA
// The code compiles fairly differently for Firewire (1384) cameras vs. USB cameras.
#ifdef FIREWIRE_CAMERA 
	typedef Pylon::CBasler1394InstantCamera cameraType;
	typedef Pylon::CBasler1394GrabResultPtr grabPtr;
	namespace cameraParams = Basler_IIDC1394CameraParams;
#elif defined USB_CAMERA
	typedef Pylon::CBaslerUsbInstantCamera cameraType;
	typedef Pylon::CBaslerUsbGrabResultPtr grabPtr;
	namespace cameraParams = Basler_UsbCameraParams;
#endif
/// Random other Constants
// It's PI.
constexpr double PI = 3.14159265358979323846264338327950288;

