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
			if (input->safemode){
				Sleep (500000);
				for (auto traceNum : range (input->numTraces)) {
					unsigned numPts = 500;
					QVector<double> xData(numPts);
					QVector<double> yData (numPts);
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
				Sleep (30000);
				for (auto lineNum : range(input->numTraces)) {
					auto data = input->getCurrentTraces (lineNum);
					if (data.size () == 0) {
						emit notify ("Get current trace data failed?!");
						continue;
					}
					QVector<qreal> xdata;
					auto ymin = DBL_MAX, ymax = -DBL_MAX;
					for (auto pt : range (data.size ())) {
						xdata.push_back (pt);
						if (data[pt] < ymin) {
							ymin = data[pt];
						}
						if (data[pt] > ymax) {
							ymax = data[pt];
						}
					}
					emit newData (xdata, 0, data.size () - 1, data, ymin, ymax, lineNum);
				}
			}
		}
		catch (ChimeraError& error) { 
			emit notify (error.qtrace ());
		}
	}
}

