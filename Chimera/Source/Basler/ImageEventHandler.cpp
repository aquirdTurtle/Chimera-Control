#include "stdafx.h"
#include <Basler/ImageEventHandler.h>
#include "PrimaryWindows/IChimeraQtWindow.h"
#include <PrimaryWindows/QtBaslerWindow.h>
#include <Basler/BaslerGrabThreadWorker.h>
#include <qlogging.h>
#include <qdebug.h>

ImageEventHandler::ImageEventHandler (IChimeraQtWindow* parentHandle) : Pylon::CImageEventHandler () {
	parent = parentHandle;
}

void ImageEventHandler::OnImageGrabbed (Pylon::CInstantCamera& camera, const Pylon::CGrabResultPtr& grabResult) {
	if (worker == nullptr) {
		worker = new BaslerGrabThreadWorker;
		parent->connect ( worker, &BaslerGrabThreadWorker::newBaslerImage,
						  parent->basWin, &QtBaslerWindow::handleNewPics  );
	}
	try {
		// Image grabbed successfully?
		if (grabResult->GrabSucceeded ()) {
			const uint16_t* pImageBuffer = (uint16_t*)grabResult->GetBuffer ();
			long width = grabResult->GetWidth ();
			long vertBinNumber = grabResult->GetHeight ();
			Matrix<long> imageMatrix(vertBinNumber, width,
				std::vector<long> (pImageBuffer, pImageBuffer + width * vertBinNumber));
			emit worker->newBaslerImage (imageMatrix);
		}
		else {
			thrower ("" + str (grabResult->GetErrorCode ()) + " " + std::string (grabResult->GetErrorDescription ().c_str ()));
		}
	}
	catch (Pylon::RuntimeException&) {
		throwNested ("Error! Failed to handle image grabbing");
	}
}
