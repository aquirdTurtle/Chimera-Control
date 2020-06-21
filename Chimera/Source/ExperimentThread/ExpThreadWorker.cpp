#include "stdafx.h"
#include "ExpThreadWorker.h"
#include <ExperimentThread/ExperimentThreadManager.h>

ExpThreadWorker::ExpThreadWorker (ExperimentThreadInput* input_){
	inputr = input_;
}

ExpThreadWorker::~ExpThreadWorker () {
}

void ExpThreadWorker::process (){
	inputr->workerThread = this;
	ExperimentThreadManager::experimentThreadProcedure((void*)inputr);
	emit mainProcessFinish ();
}
