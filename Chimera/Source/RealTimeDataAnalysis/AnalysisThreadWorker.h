#pragma once

#include <qobject.h>
#include <string>
#include "realTimePlotterInput.h"
#include <vector>
#include <deque>
#include <Plotting/PlottingInfo.h>

class AnalysisThreadWorker : public QObject {
    Q_OBJECT

    public:
        AnalysisThreadWorker (realTimePlotterInput* input_);
        ~AnalysisThreadWorker ();

    public Q_SLOTS:
        void init ();
        void handleNewPic (atomQueue);
        void handleNewPix (PixListQueue pixlist);
        void setXpts (std::vector<double>);
    Q_SIGNALS:
        void newPlotData (std::vector<std::vector<dataPoint>>, int);

    private:
        std::vector<double> xvals;

        realTimePlotterInput* input;
        std::vector < std::vector<std::vector<dataPoint>>> dataContainers;
        //vector<vector<long>> countData( groupNum );
        //vector<vector<int> > atomPresentData( groupNum );
        // countData[gridNumber][pixel Indicator][picture number indicator] = pixelCount;
        std::vector<std::vector<std::vector<long>>> countData;
        std::vector<std::vector<std::vector<int>>> atomPresentData;
        std::vector<std::vector<std::vector<std::vector<long> > > > finalCountData;
        std::vector<std::vector<std::vector<std::pair<double, ULONG> > > > finalDataNew;
        std::vector<std::vector<std::vector<std::vector<double>>>> finalAvgs;
        // Averaged over all pixels (avgAvg is the average of averages over repetitions)
        std::vector<std::vector<std::vector<double>>> avgAvg;
        // newData[plot][dataSet][group] = true if new data so change some vector sizes.
        std::vector<std::vector<std::vector<bool> > > newData;
        std::vector<std::vector<std::vector<std::deque<double>>>> finalHistData, finalErrorBars, finalXVals;
        //vector<vector<vector<std::deque<double>>>> finalHistData (allPlots.size ());
        std::vector<std::vector<std::vector<std::map<int, std::pair<int, ULONG>>>>> histogramData;
        unsigned noAtomsCounter = 0, atomCounterTotal = 0, currentThreadPictureNumber = 1, plotNumberCount = 0;
        unsigned currentPixPictureNum = 1;
        std::vector<PlottingInfo> allPlots;
};

