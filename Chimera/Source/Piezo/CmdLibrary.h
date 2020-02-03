#pragma once

#ifdef __cplusplus
#ifdef MDT69XB_COMMAND_LIB_EXPORTS
#define COMMANDLIB_API extern "C" __declspec( dllexport )
#else
#define COMMANDLIB_API extern "C" __declspec( dllimport )
#endif
#else
#define COMMANDLIB_API 
#endif


/// <summary>
/// list all the possible port on this computer.
/// </summary>
/// <param name="serialNo">port list returned string include serial number and device descriptor, separated by comma</param>
/// <returns>non-negative number: number of device in the list; negative number: failed.</returns>
COMMANDLIB_API int List(unsigned char *serialNo);

/// <summary>
///  open port function.
/// </summary>
/// <param name="serialNo">serial number of the device to be opened, use GetPorts function to get exist list first.</param>
/// <param name="nBaud">bit per second of port</param>
/// <param name="timeout">set timeout value in (s)</param>
/// <returns> non-negative number: hdl number returned Successfully; negative number: failed.</returns>
COMMANDLIB_API int Open(unsigned char* serialNo, int nBaud, int timeout);

/// <summary>
/// check opened status of port
/// </summary>
/// <param name="serialNo">serial number of the device to be checked.</param>
/// <returns> 0: port is not opened; 1: port is opened.</returns>
COMMANDLIB_API int IsOpen(unsigned char* serialNo);

/// <summary>
/// close current opened port
/// </summary>
/// <param name="hdl">handle of port.</param>
/// <returns> 0: Success; negative number: failed.</returns>
COMMANDLIB_API int Close(int hdl);

/// <summary>
/// <p>Read string from device through opened port.</p>
/// <p>make sure the port was opened Successful before call this function.</p>
/// </summary>
/// <param name="hdl">handle of port.</param>
/// <param name="b">returned string buffer</param>
/// <param name="limit">
/// <p>ABS(limit): max length value of b buffer. </p>
/// <p>SIGN(limit) == 1 : wait RX event until time out value expired;</p>
/// <p>SIGN(limit) == -1: INFINITE wait event untill RX has data;</p>
/// </param>
/// <returns>non-negative number: size of actual read data in byte; negative number: failed.</returns>
COMMANDLIB_API int Read(int hdl, unsigned char *b, int limit);

/// <summary>
/// <p>Write string to device through opened port.</p>
/// <p>make sure the port was opened Successful before call this function.</p>
/// </summary>
/// <param name="hdl">handle of port.</param>
/// <param name="b">input string</param>
/// <param name="size">size of string to be written.</param>
/// <returns>non-negative number: number of bytes written; negative number: failed.</returns>
COMMANDLIB_API int Write(int hdl, unsigned char *b, int size);

/// <summary>
/// <p>set command to device according to protocol in manual.</p>
/// <p>make sure the port was opened Successful before call this function.</p>
/// <p>make sure this is the correct device by checking the ID string before call this function.</p>
/// </summary>
/// <param name="hdl">handle of port.</param>
/// <param name="c">input command string</param>
/// <param name="size">lenth of input command string (<255)</param>
/// <returns>
/// <p>0: Success; negative number: failed.</p>
/// <p>0xEA: CMD_NOT_DEFINED;</p>
/// <p>0xEB: time out;</p>
/// <p>0xED: invalid string buffer;</p>
/// </returns>
COMMANDLIB_API int Set(int hdl, unsigned char *c, int size);

/// <summary>
/// <p>set command to device according to protocol in manual and get the return string.</p>
/// <p>make sure the port was opened Successful before call this function.</p>
/// <p>make sure this is the correct device by checking the ID string before call this function.</p>
/// </summary>
/// <param name="hdl">handle of port.</param>
/// <param name="c">input command string (<255)</param>
/// <param name="d">output string (<255)</param>
/// <returns>
/// <p>0: Success; negative number: failed.</p>
/// <p>0xEA: CMD_NOT_DEFINED;</p>
/// <p>0xEB: time out;</p>
/// <p>0xED: invalid string buffer;</p>
/// </returns>
COMMANDLIB_API int Get(int hdl, unsigned char *c, char *d);

/// <summary>
/// set time out value for read or write process.
/// </summary>
/// <param name="hdl">handle of port.</param>
/// <param name="time">time out value</param>
/// <returns>
/// <p>0: Success; negative number: failed.</p>
/// </returns>
COMMANDLIB_API int SetTimeout(int hdl, int time);

/// <summary>
/// Purge the RX and TX buffer on port.
/// </summary>
/// <param name="hdl">handle of port.</param>
/// <param name="flag">
/// <p>FT_PURGE_RX: 0x01</p>
/// <p>FT_PURGE_TX: 0x02</p>
///</param>
/// <returns> 0: Success; negative number: failed.</returns>
COMMANDLIB_API int Purge(int hdl, int flag);

/// <summary>
/// <p>Get the product header and firmware version</p>
/// <p>make sure the port was opened Successful before call this function.</p>
/// <p>make sure this is the correct device by checking the ID string before call this function.</p>
/// </summary>
/// <param name="hdl">handle of port.</param>
/// <param name="id">model number and firmware version</param>
/// <returns>
/// <p>0: Success; negative number: failed.</p>
/// </returns>
COMMANDLIB_API int GetId(int hdl, unsigned char* id);

/// <summary>
/// <p>get the serial number of device.</p>
/// <p>make sure the port was opened Successful before call this function.</p>
/// <p>make sure this is the correct device by checking the ID string before call this function.</p>
/// </summary>
/// <param name="hdl">handle of port.</param>
/// <param name="sn">serial number of device.</param>
/// <returns>
/// <p>0: Success; negative number: failed.</p>
/// </returns>
COMMANDLIB_API int GetSerialNumber(int hdl, unsigned char* sn);

/// <summary>
/// <p>Restores default factory settings.</p>
/// <p>make sure the port was opened Successful before call this function.</p>
/// <p>make sure this is the correct device by checking the ID string before call this function.</p>
/// </summary>
/// <param name="hdl">handle of port.</param>
/// <returns>
/// <p>0: Success; negative number: failed.</p>
/// </returns>
COMMANDLIB_API int RestoreDefaultSettings(int hdl);

/// <summary>
/// <p>Get output voltage limit setting.</p>
/// <p>make sure the port was opened Successful before call this function.</p>
/// <p>make sure this is the correct device by checking the ID string before call this function.</p>
/// </summary>
/// <param name="hdl">handle of port.</param>
/// <param name="voltage">output voltage limit setting.</param>
/// <returns>
/// <p>0: Success; negative number: failed.</p>
/// </returns>
COMMANDLIB_API int GetLimitVoltage(int hdl, double * voltage);

/// <summary>
/// <p>Get the display intensity.</p>
/// <p>make sure the port was opened Successful before call this function.</p>
/// <p>make sure this is the correct device by checking the ID string before call this function.</p>
/// </summary>
/// <param name="hdl">handle of port.</param>
/// <param name="intensity">intensity of display pannel.</param>
/// <returns>
/// <p>0: Success; negative number: failed.</p>
/// </returns>
COMMANDLIB_API int GetDisplayIntensity(int hdl, int * intensity);

/// <summary>
/// <p>Set the Display Intensity (0-15).</p>
/// <p>make sure the port was opened Successful before call this function.</p>
/// <p>make sure this is the correct device by checking the ID string before call this function.</p>
/// </summary>
/// <param name="hdl">handle of port.</param>
/// <param name="intensity">target intensity range:(1~15)</param>
/// <returns>
/// <p>0: Success; negative number: failed.</p>
/// </returns>
COMMANDLIB_API int SetDisplayIntensity(int hdl, int intensity);

/// <summary>
/// <p>Set all outputs to desired voltage.</p>
/// <p>make sure the port was opened Successful before call this function.</p>
/// <p>make sure this is the correct device by checking the ID string before call this function.</p>
/// </summary>
/// <param name="hdl">handle of port.</param>
/// <param name="voltage">target voltage range:(0 ~ limit voltage)</param>
/// <returns>
/// <p>0: Success; negative number: failed.</p>
/// </returns>
COMMANDLIB_API int SetAllVoltage(int hdl, double voltage);

/// <summary>
/// <p>Get the state of the Master Scan enable.</p>
/// <p>make sure the port was opened Successful before call this function.</p>
/// <p>make sure this is the correct device by checking the ID string before call this function.</p>
/// </summary>
/// <param name="hdl">handle of port.</param>
/// <param name="state">current master scan state.(1-enable,0-disable)</param>
/// <returns>
/// <p>0: Success; negative number: failed.</p>
/// </returns>
COMMANDLIB_API int GetMasterScanEnable(int hdl, int * state);

/// <summary>
/// <p>Set Master Scan mode.</p>
/// <p>make sure the port was opened Successful before call this function.</p>
/// <p>make sure this is the correct device by checking the ID string before call this function.</p>
/// </summary>
/// <param name="hdl">handle of port.</param>
/// <param name="state">target state of master scan:(1-enable,0-disable)</param>
/// <returns>
/// <p>0: Success; negative number: failed.</p>
/// </returns>
COMMANDLIB_API int SetMasterScanEnable(int hdl, int state);

/// <summary>
/// <p>Get the master scan voltage.</p>
/// <p>make sure the port was opened Successful before call this function.</p>
/// <p>make sure this is the correct device by checking the ID string before call this function.</p>
/// </summary>
/// <param name="hdl">handle of port.</param>
/// <param name="voltage">current master scan voltage</param>
/// <returns>
/// <p>0: Success; negative number: failed.</p>
/// </returns>
COMMANDLIB_API int GetMasterScanVoltage(int hdl, double * voltage);

/// <summary>
/// <p>Set a master scan voltage that adds to the x, y, and z axis voltages.</p>
/// <p>make sure the port was opened Successful before call this function.</p>
/// <p>make sure this is the correct device by checking the ID string before call this function.</p>
/// </summary>
/// <param name="hdl">handle of port.</param>
/// <param name="voltage">target voltage range:(0 ~ limit voltage)</param>
/// <returns>
/// <p>0: Success; negative number: failed.</p>
/// </returns>
COMMANDLIB_API int SetMasterScanVoltage(int hdl, double voltage);

/// <summary>
/// <p>Get the X axis output voltage.</p>
/// <p>make sure the port was opened Successful before call this function.</p>
/// <p>make sure this is the correct device by checking the ID string before call this function.</p>
/// </summary>
/// <param name="hdl">handle of port.</param>
/// <param name="voltage">current x-axis output voltage.</param>
/// <returns>
/// <p>0: Success; negative number: failed.</p>
/// </returns>
COMMANDLIB_API int GetXAxisVoltage(int hdl, double * voltage);

/// <summary>
/// <p>Set the output voltage for the X axis.</p>
/// <p>make sure the port was opened Successful before call this function.</p>
/// <p>make sure this is the correct device by checking the ID string before call this function.</p>
/// </summary>
/// <param name="hdl">handle of port.</param>
/// <param name="voltage">target voltage range:(0 ~ limit voltage)</param>
/// <returns>
/// <p>0: Success; negative number: failed.</p>
/// </returns>
COMMANDLIB_API int SetXAxisVoltage(int hdl, double voltage);

/// <summary>
/// <p>Get the Y axis output voltage.</p>
/// <p>make sure the port was opened Successful before call this function.</p>
/// <p>make sure this is the correct device by checking the ID string before call this function.</p>
/// </summary>
/// <param name="hdl">handle of port.</param>
/// <param name="voltage">current y-axis output voltage.</param>
/// <returns>
/// <p>0: Success; negative number: failed.</p>
/// </returns>
COMMANDLIB_API int GetYAxisVoltage(int hdl, double * voltage);

/// <summary>
/// <p>Set the output voltage for the Y axis.</p>
/// <p>make sure the port was opened Successful before call this function.</p>
/// <p>make sure this is the correct device by checking the ID string before call this function.</p>
/// </summary>
/// <param name="hdl">handle of port.</param>
/// <param name="voltage">target voltage range:(0 ~ limit voltage)</param>
/// <returns>
/// <p>0: Success; negative number: failed.</p>
/// </returns>
COMMANDLIB_API int SetYAxisVoltage(int hdl, double voltage);

/// <summary>
/// <p>Get the Z axis output voltage.</p>
/// <p>make sure the port was opened Successful before call this function.</p>
/// <p>make sure this is the correct device by checking the ID string before call this function.</p>
/// </summary>
/// <param name="hdl">handle of port.</param>
/// <param name="voltage">current z-axis output voltage.</param>
/// <returns>
/// <p>0: Success; negative number: failed.</p>
/// </returns>
COMMANDLIB_API int GetZAxisVoltage(int hdl, double * voltage);

/// <summary>
/// <p>Set the output voltage for the Z axis.</p>
/// <p>make sure the port was opened Successful before call this function.</p>
/// <p>make sure this is the correct device by checking the ID string before call this function.</p>
/// </summary>
/// <param name="hdl">handle of port.</param>
/// <param name="voltage">target voltage range:(0 ~ limit voltage)</param>
/// <returns>
/// <p>0: Success; negative number: failed.</p>
/// </returns>
COMMANDLIB_API int SetZAxisVoltage(int hdl, double voltage);

/// <summary>
/// <p>Get the minimum output voltage limit for X axis.</p>
/// <p>make sure the port was opened Successful before call this function.</p>
/// <p>make sure this is the correct device by checking the ID string before call this function.</p>
/// </summary>
/// <param name="hdl">handle of port.</param>
/// <param name="voltage">current x-axis min output voltage.</param>
/// <returns>
/// <p>0: Success; negative number: failed.</p>
/// </returns>
COMMANDLIB_API int GetXAxisMinVoltage(int hdl, double * voltage);

/// <summary>
/// <p>Set the minimum output voltage limit for X axis.</p>
/// <p>make sure the port was opened Successful before call this function.</p>
/// <p>make sure this is the correct device by checking the ID string before call this function.</p>
/// </summary>
/// <param name="hdl">handle of port.</param>
/// <param name="voltage">target voltage range:(0 ~ limit voltage)</param>
/// <returns>
/// <p>0: Success; negative number: failed.</p>
/// </returns>
COMMANDLIB_API int SetXAxisMinVoltage(int hdl, double voltage);

/// <summary>
/// <p>Get the minimum output voltage limit for Y axis.</p>
/// <p>make sure the port was opened Successful before call this function.</p>
/// <p>make sure this is the correct device by checking the ID string before call this function.</p>
/// </summary>
/// <param name="hdl">handle of port.</param>
/// <param name="voltage">current y-axis min output voltage.</param>
/// <returns>
/// <p>0: Success; negative number: failed.</p>
/// </returns>
COMMANDLIB_API int GetYAxisMinVoltage(int hdl, double * voltage);

/// <summary>
/// <p>Set the minimum output voltage limit for Y axis.</p>
/// <p>make sure the port was opened Successful before call this function.</p>
/// <p>make sure this is the correct device by checking the ID string before call this function.</p>
/// </summary>
/// <param name="hdl">handle of port.</param>
/// <param name="voltage">target voltage range:(0 ~ limit voltage)</param>
/// <returns>
/// <p>0: Success; negative number: failed.</p>
/// </returns>
COMMANDLIB_API int SetYAxisMinVoltage(int hdl, double voltage);

/// <summary>
/// <p>Get the minimum output voltage limit for Z axis.</p>
/// <p>make sure the port was opened Successful before call this function.</p>
/// <p>make sure this is the correct device by checking the ID string before call this function.</p>
/// </summary>
/// <param name="hdl">handle of port.</param>
/// <param name="voltage">current min z-axis output voltage.</param>
/// <returns>
/// <p>0: Success; negative number: failed.</p>
/// </returns>
COMMANDLIB_API int GetZAxisMinVoltage(int hdl, double * voltage);

/// <summary>
/// <p>Set the minimum output voltage limit for Z axis.</p>
/// <p>make sure the port was opened Successful before call this function.</p>
/// <p>make sure this is the correct device by checking the ID string before call this function.</p>
/// </summary>
/// <param name="hdl">handle of port.</param>
/// <param name="voltage">target voltage range:(0 ~ limit voltage)</param>
/// <returns>
/// <p>0: Success; negative number: failed.</p>
/// </returns>
COMMANDLIB_API int SetZAxisMinVoltage(int hdl, double voltage);

/// <summary>
/// <p>Get the maximum output voltage limit for X axis.</p>
/// <p>make sure the port was opened Successful before call this function.</p>
/// <p>make sure this is the correct device by checking the ID string before call this function.</p>
/// </summary>
/// <param name="hdl">handle of port.</param>
/// <param name="voltage">current x-axis max output voltage.</param>
/// <returns>
/// <p>0: Success; negative number: failed.</p>
/// </returns>
COMMANDLIB_API int GetXAxisMaxVoltage(int hdl, double * voltage);

/// <summary>
/// <p>Set the maximum output voltage limit for X axis.</p>
/// <p>make sure the port was opened Successful before call this function.</p>
/// <p>make sure this is the correct device by checking the ID string before call this function.</p>
/// </summary>
/// <param name="hdl">handle of port.</param>
/// <param name="voltage">target voltage range:(0 ~ limit voltage)</param>
/// <returns>
/// <p>0: Success; negative number: failed.</p>
/// </returns>
COMMANDLIB_API int SetXAxisMaxVoltage(int hdl, double voltage);

/// <summary>
/// <p>Get the maximum output voltage limit for Y axis.</p>
/// <p>make sure the port was opened Successful before call this function.</p>
/// <p>make sure this is the correct device by checking the ID string before call this function.</p>
/// </summary>
/// <param name="hdl">handle of port.</param>
/// <param name="voltage">current y-axis max output voltage.</param>
/// <returns>
/// <p>0: Success; negative number: failed.</p>
/// </returns>
COMMANDLIB_API int GetYAxisMaxVoltage(int hdl, double * voltage);

/// <summary>
/// <p>Set the maximum output voltage limit for Y axis.</p>
/// <p>make sure the port was opened Successful before call this function.</p>
/// <p>make sure this is the correct device by checking the ID string before call this function.</p>
/// </summary>
/// <param name="hdl">handle of port.</param>
/// <param name="voltage">target voltage range:(0 ~ limit voltage)</param>
/// <returns>
/// <p>0: Success; negative number: failed.</p>
/// </returns>
COMMANDLIB_API int SetYAxisMaxVoltage(int hdl, double voltage);

/// <summary>
/// <p>Get the maximum output voltage limit for Z axis.</p>
/// <p>make sure the port was opened Successful before call this function.</p>
/// <p>make sure this is the correct device by checking the ID string before call this function.</p>
/// </summary>
/// <param name="hdl">handle of port.</param>
/// <param name="voltage">current max z-axis output voltage.</param>
/// <returns>
/// <p>0: Success; negative number: failed.</p>
/// </returns>
COMMANDLIB_API int GetZAxisMaxVoltage(int hdl, double * voltage);

/// <summary>
/// <p>Set the maximum output voltage limit for Z axis.</p>
/// <p>make sure the port was opened Successful before call this function.</p>
/// <p>make sure this is the correct device by checking the ID string before call this function.</p>
/// </summary>
/// <param name="hdl">handle of port.</param>
/// <param name="voltage">target voltage range:(0 ~ limit voltage)</param>
/// <returns>
/// <p>0: Success; negative number: failed.</p>
/// </returns>
COMMANDLIB_API int SetZAxisMaxVoltage(int hdl, double voltage);

/// <summary>
/// <p>Get the current step resolution.</p>
/// <p>make sure the port was opened Successful before call this function.</p>
/// <p>make sure this is the correct device by checking the ID string before call this function.</p>
/// </summary>
/// <param name="hdl">handle of port.</param>
/// <param name="step">current step solution.</param>
/// <returns>
/// <p>0: Success; negative number: failed.</p>
/// </returns>
COMMANDLIB_API int GetVoltageAdjustmentResolution(int hdl, int * step);

/// <summary>
/// <p>Set the step resolution when using up/down arrow keys.</p>
/// <p>make sure the port was opened Successful before call this function.</p>
/// <p>make sure this is the correct device by checking the ID string before call this function.</p>
/// </summary>
/// <param name="hdl">handle of port.</param>
/// <param name="step">target step range:(1 ~ 1000)</param>
/// <returns>
/// <p>0: Success; negative number: failed.</p>
/// </returns>
COMMANDLIB_API int SetVoltageAdjustmentResolution(int hdl, int step);

/// <summary>
/// <p>Get the x,y,z axis output voltages.</p>
/// <p>make sure the port was opened Successful before call this function.</p>
/// <p>make sure this is the correct device by checking the ID string before call this function.</p>
/// </summary>
/// <param name="hdl">handle of port.</param>
/// <param name="xVoltage">current x axis output voltage.</param>
/// <param name="yVoltage">current y axis output voltage.</param>
/// <param name="zVoltage">current z axis output voltage.</param>
/// <returns>
/// <p>0: Success; negative number: failed.</p>
/// </returns>
COMMANDLIB_API int GetXYZAxisVoltage(int hdl, double * xVoltage, double * yVoltage, double * zVoltage);

/// <summary>
/// <p>Set the x,y,z axis output voltages.</p>
/// <p>make sure the port was opened Successful before call this function.</p>
/// <p>make sure this is the correct device by checking the ID string before call this function.</p>
/// </summary>
/// <param name="hdl">handle of port.</param>
/// <param name="xVoltage">target x axis voltage range:(0 ~ limit voltage)</param>
/// <param name="yVoltage">target y axis voltage range:(0 ~ limit voltage)</param>
/// <param name="zVoltage">target z axis voltage range:(0 ~ limit voltage)</param>
/// <returns>
/// <p>0: Success; negative number: failed.</p>
/// </returns>
COMMANDLIB_API int SetXYZAxisVoltage(int hdl, double xVoltage, double yVoltage, double zVoltage);

