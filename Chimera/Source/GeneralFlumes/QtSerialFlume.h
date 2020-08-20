#pragma once
/*
#include <QSerialPort>

class QtSerialFlume {
public:
	// THIS CLASS IS NOT COPYABLE.
	QtSerialFlume& operator=(const QtSerialFlume&) = delete;
	QtSerialFlume (const QtSerialFlume&) = delete;

	QtSerialFlume (bool safemode_option, std::string portAddress);
	void open (std::string fileAddr);
	void close ();
	void write (std::string msg);
	void resetConnection ();
	std::string read ();
	std::string query (std::string msg);

private:
	const bool safemode;
	QSerialPort port;
	std::string portAddress;
};
*/