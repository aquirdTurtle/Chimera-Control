#include <stdafx.h>
#include <GeneralFlumes/QtSerialFlume.h>

/*
QtSerialFlume::QtSerialFlume (bool safemode_option, std::string portAddress_) : safemode(safemode_option){
	portAddress = portAddress_;
	open (portAddress);
}

void QtSerialFlume::open (std::string fileAddr) {
	port.setPortName (qstr (portAddress));
	if (!port.open (QIODevice::ReadWrite)) {
		thrower ("Failed to open QT serial flume!");
	}
}

void QtSerialFlume::close () {
	port.close ();
}

void QtSerialFlume::write (std::string msg) {
	auto ba = qstr (msg).toUtf8 ();
	port.write (ba);
}

void QtSerialFlume::resetConnection () {
	close ();
	open (portAddress);
}

std::string QtSerialFlume::read () {
	auto ba = port.read (1064);
	QString qs = QString::fromUtf8 (ba);
	return str(qs);
}

std::string QtSerialFlume::query (std::string msg) {
	write (msg);
	return read ();
}
*/
