#include "stdafx.h"
#include <RealTimeDataAnalysis/AtomCruncherWorker.h>
#include <RealTimeDataAnalysis/atomCruncherInput.h>

CruncherThreadWorker::CruncherThreadWorker (atomCruncherInput* input_) : input(input_) {
}

CruncherThreadWorker::~CruncherThreadWorker () {
}

void CruncherThreadWorker::init () {
	auto gridSize = input->grids.size ();
	if (gridSize == 0) {
		return;
	}
	monitoredPixelIndecies.resize(gridSize);
	// preparing for the crunching
	for (auto gridInc : range (gridSize)) {
		auto& grid = input->grids[gridInc];
		for (auto columnInc : range (grid.width)) {
			for (auto rowInc : range (grid.height)) {
				unsigned long pixelRowInverted = grid.topLeftCorner.row + rowInc * grid.pixelSpacing;
				unsigned long pixelColumn = grid.topLeftCorner.column + columnInc * grid.pixelSpacing;
				unsigned long pixelRow = input->imageDims.height () - pixelRowInverted - 1;

				if (pixelRow >= input->imageDims.height () || pixelColumn >= input->imageDims.width ()) {
					emit error ("atom grid appears to include pixels outside the image frame! Not allowed, seen by atom "
						"cruncher thread");
					return;
				}
				int index = ((input->imageDims.height () - 1 - pixelRow) * input->imageDims.width () + pixelColumn);
				if (index >= input->imageDims.width () * input->imageDims.height ()) {
					emit error ("Math error! Somehow, the pixel indexes appear within bounds, but the calculated index"
								" is larger than the image is!  (A low level bug, this shouldn't happen)");
					return;
				}
				monitoredPixelIndecies[gridInc].push_back (index);
			}
		}
	}
	for (auto picThresholds : input->thresholds) {
		if (picThresholds.size () != 1 && picThresholds.size () != input->grids[0].numAtoms ()) {
			emit error (cstr ("the list of thresholds isn't size 1 (constant) or the size of the number of atoms in the "
				"first grid! Size is " + str (picThresholds.size ()) + "and grid size is " +
				str (input->grids[0].numAtoms ())));
			return;
		}
	}
	imageCount = 0;
}


void CruncherThreadWorker::handleImage (NormalImage image){
	// loop watching the image queue.
	// if no images wait until images. Should probably change to be event based, but want this to be fast...
	//if (imageCount % 2 == 0) {
	//	input->catchPicTime->push_back (chronoClock::now ());
	//}
	// tempImagePixels[grid][pixel]; only contains the counts for the pixels being monitored.
	PixListQueue tempImagePixels (input->grids.size ());
	// tempAtomArray[grid][pixel]; only contains the boolean true/false of whether an atom passed a threshold or not. 
	atomQueue tempAtomArray (input->grids.size ());
	for (auto gridInc : range (input->grids.size ())) {
		tempAtomArray[gridInc].image = std::vector<bool> (monitoredPixelIndecies[gridInc].size ());
		tempImagePixels[gridInc].image = std::vector<long> (monitoredPixelIndecies[gridInc].size ());
	}
	for (auto gridInc : range (input->grids.size ())) {
		unsigned count = 0;
		{
			tempImagePixels[gridInc].picNum = image.picNum;
			tempAtomArray[gridInc].picNum = image.picNum;
			///*** Deal with 1st element entirely first, as this is important for the rearranger thread and the 
			/// load-skip both of which are very time-sensitive.
			for (auto pixelIndex : monitoredPixelIndecies[gridInc]) {
				if (pixelIndex > image.image.size ()) {
					emit error (cstr ("Monitored pixel indecies included pixel out of image?!?! pixel: " + str (pixelIndex)
						+ ", size: " + str (image.image.size ())));
					// should I return here?
				}
				else {
					tempImagePixels[gridInc].image[count++] = image.image.data[pixelIndex];
				}
			}
		}
		count = 0;
		for (auto& pix : tempImagePixels[gridInc].image) {
			auto& picThresholds = input->thresholds[imageCount % input->picsPerRep];
			if (pix >= picThresholds[count % picThresholds.size ()]) {
				tempAtomArray[gridInc].image[count] = true;
			}
			count++;
		}
		// explicitly deal with the rearranger thread and load skip as soon as possible, these are time-critical.
		if (gridInc == 0) {
			// if last picture of repetition, check for loadskip condition.
			if (imageCount % input->picsPerRep == input->picsPerRep - 1) {
				unsigned numAtoms = std::accumulate (tempAtomArray[0].image.begin (), tempAtomArray[0].image.end (), 0);
				*input->skipNext = (numAtoms >= input->atomThresholdForSkip);
			}
		}
	}
	emit atomArray (tempAtomArray);
	emit pixArray (tempImagePixels);
	imageCount++;
}
