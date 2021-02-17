#include "stdafx.h"
#include "IDeviceCore.h"
#include <ExperimentThread/ExpThreadWorker.h>

void IDeviceCore::notify(statusMsg msg, ExpThreadWorker* threadworker) {
	if (threadworker != nullptr) {
		msg.systemDelim = qstr(getDelim());
		emit threadworker->notification(msg);
	}
}