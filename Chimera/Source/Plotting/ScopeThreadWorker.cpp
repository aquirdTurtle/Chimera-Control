#include "stdafx.h"
#include <Plotting/ScopeThreadWorker.h>
#include <Plotting/ScopeViewer.h>

ScopeThreadWorker::ScopeThreadWorker (ScopeViewer* input_) {
	input = input_;
}

void ScopeThreadWorker::process (){
	// this thread continuously requests new info from the scopes. The input is just a pointer to the scope object.
	while (true){
		try { 
			Sleep (5000);
			if (true /*safemode*/){
				for (auto traceNum : range (input->numTraces)) {
					unsigned numPts = 5000;
					QVector<double> xData(numPts);
					QVector<double> yData (numPts);
					//std::vector<double>* xdata  = new std::vector<double> (numPts);
					//std::vector<double>* ydata = new std::vector<double> (numPts);
					auto min = DBL_MAX;
					auto max = -DBL_MAX;
					unsigned count = 0;
					for (auto dpn : range(yData.size())) {
						xData[dpn] = count++;
						auto& dp = yData[dpn];
						dp = (rand () % 1000) / 1000.0;
						min = dp < min ? dp : min;
						max = dp > max ? dp : max;
					}
					emit newData (xData, 0, count - 1, yData, min, max, traceNum);
				}
			}
			else {
				// get data from scope...
			}
			//input->refreshData ();
		}
		catch (Error&) { /* ??? */ }
	}
}

