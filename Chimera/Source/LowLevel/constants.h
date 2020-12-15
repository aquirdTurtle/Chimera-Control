// created by Mark O. Brown
#pragma once

#include "GeneralUtilityFunctions/my_str.h"
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
	constexpr auto DM_SERIAL = "25CW012#060";
	constexpr bool DM_SAFEMODE = true;
	constexpr char UW_SYSTEM_ADDRESS[] = "COM10";
	//constexpr short UW_SYSTEM_ADDRESS = 28;
	constexpr bool UW_SYSTEM_SAFEMODE = true;
	constexpr bool DOFTDI_SAFEMODE = false;
	constexpr bool DDS_SAFEMODE = false;
	constexpr bool BASLER_SAFEMODE = true;
	constexpr bool ANDOR_SAFEMODE = false;
	#ifdef _DEBUG
	constexpr bool PYTHON_SAFEMODE = true;
	#else
	constexpr bool  PYTHON_SAFEMODE = true;
	#endif
	constexpr bool DAQMX_SAFEMODE = false;
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
	constexpr auto UWAVE_AGILENT_USB_ADDRESS = "USB0::0x0957::0x2C07::MY52801397::0::INSTR";
	constexpr bool MASTER_REPUMP_SCOPE_SAFEMODE = true;
	constexpr auto MASTER_REPUMP_SCOPE_ADDRESS = "USB0::0x0699::0x03B3::C011388::0::INSTR";
	constexpr bool MOT_SCOPE_SAFEMODE = true;
	constexpr auto MOT_SCOPE_ADDRESS = "USB0::0x0699::0x0363::C100939::0::INSTR";
	constexpr bool EXPERIMENT_SCOPE_SAFEMODE = true;
	constexpr auto EXPERIMENT_SCOPE_ADDRESS = "USB0::0x0699::0x0413::C011471::INSTR";
	constexpr bool ANALOG_OUT_SAFEMODE = false;
	constexpr bool ANALOG_IN_SAFEMODE = true;

	constexpr auto CODE_ROOT = "C:\\Users\\Regal-Lab\\Code\\Chimera-Control";
	const std::string PLOT_FILES_SAVE_LOCATION = str (CODE_ROOT) + "\\Plotting";
	const std::string DATA_ANALYSIS_CODE_LOCATION = "C:\\Users\\Regal-Lab\\Code\\Data_Analysis_Control\\";
	//const std::string LIB_PATH = str (CODE_ROOT) + "\\Waveforms-Library\\WaveLibrary-320-MSpS-Gain_0p82\\";
	const std::string LIB_PATH = str (CODE_ROOT) + "\\Waveforms-Library\\WaveLibrary-320-MSpS-0.083333-PowerLevel\\";
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
	const std::string MOT_ROUTINES_ADDRESS = PROFILES_PATH + "Hotkey Experiments\\MOT";
	const std::string PGC_ROUTINES_ADDRESS = PROFILES_PATH + "Hotkey Experiments\\PGC";
	const std::string CAMERA_CAL_ROUTINE_ADDRESS = PROFILES_PATH + "Hotkey Experiments\\Camera";
	const std::string DEBUG_OUTPUT_LOCATION = str (CODE_ROOT) + "\\Debug-Output\\";
	const std::string TIMING_OUTPUT_LOCATION = DATA_SAVE_LOCATION + "\\2017\\September\\September 8\\Raw Data\\";
	// location where wave data can be outputted for analyzing with another computer.
	const std::string DM_PROFILES_LOCATION = str(CODE_ROOT) + "\\DM-Library";
	const std::string DM_FLAT_PROFILE = DM_PROFILES_LOCATION + "\\flatProfile.txt";
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

