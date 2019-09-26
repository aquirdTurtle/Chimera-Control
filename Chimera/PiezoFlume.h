#pragma once

#include <string>

class PiezoFlume
{
	public:
		PiezoFlume ( bool sMode, std::string sn);
		std::string list (  );
		std::string getDeviceInfo ( );
		void open ( );
		bool isOpen ( );
		void close ( );
		std::string getID ( );
		std::string getSerialNumber ( );
		double getLimitVoltage ( );
		void setXAxisVoltage ( double val );
		double getXAxisVoltage (  );
		void setYAxisVoltage ( double val );
		double getYAxisVoltage ( );
		void setZAxisVoltage ( double val );
		double getZAxisVoltage ( );

	private:
		const bool safemode;
		std::vector<char> comPortNumber;
		int deviceHandle=0;
		const UINT bufferSize = 256;
		typedef int ( *ftype_List )( char *serialNo );
		typedef int ( *ftype_Open )( char* serialNo, int nBaud, int timeout );
		typedef int ( *ftype_IsOpen )( char* serialNo );
		typedef int ( *ftype_Close )( int hdl );

		typedef int ( *ftype_GetId )( int hdl, char* id );

		typedef int ( *ftype_SetXAxisVoltage )( int hdl, double voltage );
		typedef int ( *ftype_GetSerialNumber)( int hdl, char* sn );
		typedef int ( *ftype_SetXAxisMinVoltage )( int hdl, double voltage );
		typedef int ( *ftype_SetXAxisMaxVoltage )( int hdl, double voltage );
		typedef int ( *ftype_GetXAxisVoltage )( int hdl, double& voltage );
		typedef int ( *ftype_GetXAxisMinVoltage )( int hdl, double& voltage );
		typedef int ( *ftype_GetXAxisMaxVoltage )( int hdl, double& voltage );

		typedef int ( *ftype_SetYAxisVoltage )( int hdl, double voltage );
		typedef int ( *ftype_SetYAxisMinVoltage )( int hdl, double voltage );
		typedef int ( *ftype_SetYAxisMaxVoltage )( int hdl, double voltage );
		typedef int ( *ftype_GetYAxisVoltage )( int hdl, double& voltage );
		typedef int ( *ftype_GetYAxisMinVoltage )( int hdl, double& voltage );
		typedef int ( *ftype_GetYAxisMaxVoltage )( int hdl, double& voltage );

		typedef int ( *ftype_SetZAxisVoltage )( int hdl, double voltage );
		typedef int ( *ftype_SetZAxisMinVoltage )( int hdl, double voltage );
		typedef int ( *ftype_SetZAxisMaxVoltage )( int hdl, double voltage );
		typedef int ( *ftype_GetZAxisVoltage )( int hdl, double& voltage );
		typedef int ( *ftype_GetZAxisMinVoltage )( int hdl, double& voltage );
		typedef int ( *ftype_GetZAxisMaxVoltage )( int hdl, double& voltage );

		typedef int ( *ftype_SetXYZAxisVoltage )( int hdl, double xVoltage, double yVoltage, double zVoltage );
		typedef int ( *ftype_GetXYZAxisVoltage )( int hdl, double& xVoltage, double& yVoltage, double& zVoltage );

		typedef int ( *ftype_SetMasterScanEnable )( int hdl, int enable );
		typedef int ( *ftype_GetMasterScanEnable )( int hdl, int& enable );
		typedef int ( *ftype_SetMasterScanVoltage )( int hdl, double voltage );
		typedef int ( *ftype_GetMasterScanVoltage )( int hdl, double& voltage );
		typedef int ( *ftype_GetLimitVoltage )( int hdl, double& voltage );
		typedef int ( *ftype_GetFriendlyName )( int hdl, char* friendName );
		typedef int ( *ftype_SetAllVoltage )( int hdl, double voltage );

		ftype_List List_raw;
		ftype_Open Open_raw;
		ftype_IsOpen IsOpen_raw;
		ftype_Close Close_raw;
		ftype_GetSerialNumber GetSerialNumber_raw;
		ftype_GetId GetId_raw;

		ftype_SetXAxisVoltage SetXAxisVoltage_raw;
		ftype_SetXAxisMaxVoltage SetXAxisMinVoltage_raw;
		ftype_SetXAxisMaxVoltage SetXAxisMaxVoltage_raw;
		ftype_GetXAxisVoltage GetXAxisVoltage_raw;
		ftype_GetXAxisMaxVoltage GetXAxisMinVoltage_raw;
		ftype_GetXAxisMaxVoltage GetXAxisMaxVoltage_raw;

		ftype_SetYAxisVoltage SetYAxisVoltage_raw;
		ftype_SetYAxisMaxVoltage SetYAxisMinVoltage_raw;
		ftype_SetYAxisMaxVoltage SetYAxisMaxVoltage_raw;
		ftype_GetYAxisVoltage GetYAxisVoltage_raw;
		ftype_GetYAxisMaxVoltage GetYAxisMinVoltage_raw;
		ftype_GetYAxisMaxVoltage GetYAxisMaxVoltage_raw;

		ftype_SetZAxisVoltage SetZAxisVoltage_raw;
		ftype_SetZAxisMaxVoltage SetZAxisMinVoltage_raw;
		ftype_SetZAxisMaxVoltage SetZAxisMaxVoltage_raw;
		ftype_GetZAxisVoltage GetZAxisVoltage_raw;
		ftype_GetZAxisMaxVoltage GetZAxisMinVoltage_raw;
		ftype_GetZAxisMaxVoltage GetZAxisMaxVoltage_raw;

		ftype_SetXYZAxisVoltage SetXYZAxisVoltage_raw;
		ftype_GetXYZAxisVoltage GetXYZAxisVoltage_raw;

		ftype_SetMasterScanEnable SetMasterScanEnable_raw;
		ftype_GetMasterScanEnable GetMasterScanEnable_raw;
		ftype_SetMasterScanVoltage SetMasterScanVoltage_raw;
		ftype_GetMasterScanVoltage GetMasterScanVoltage_raw;
		ftype_GetLimitVoltage GetLimitVoltage_raw;
		ftype_GetFriendlyName GetFriendlyName_raw;
		ftype_SetAllVoltage SetAllVoltage_raw;
};