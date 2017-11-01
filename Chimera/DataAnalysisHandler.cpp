
#include "stdafx.h"

#include "DataAnalysisHandler.h"
#include "Control.h"
#include "CameraWindow.h"
#include "ProfileSystem.h"
#include "PlotDesignerDialog.h"
#include "realTimePlotterInput.h"
#include <numeric>
#include <boost/tuple/tuple.hpp>
#include <map>


using std::vector;

void DataAnalysisControl::initialize( cameraPositions& pos, int& id, CWnd* parent, cToolTips& tooltips, 
									  int isTriggerModeSensitive, rgbMap rgbs )
{
	header.seriesPos = { pos.seriesPos.x,  pos.seriesPos.y,  pos.seriesPos.x + 480, pos.seriesPos.y += 25 };
	header.videoPos = { pos.videoPos.x, pos.videoPos.y, pos.videoPos.x + 480, pos.videoPos.y += 25 };
	header.amPos = { pos.amPos.x,   pos.amPos.y,   pos.amPos.x + 480, pos.amPos.y += 25 };
	header.triggerModeSensitive = isTriggerModeSensitive;
	header.Create("DATA ANALYSIS", NORM_HEADER_OPTIONS, header.seriesPos, parent, id++);
	header.fontType = HeadingFont;
	pos.seriesPos.y += 25;
	pos.videoPos.y += 25;
	pos.amPos.y += 25;

	/// Data analysis stuffs.
	//
	currentDataSetNumberText.seriesPos = { pos.seriesPos.x, pos.seriesPos.y, pos.seriesPos.x + 400, pos.seriesPos.y + 25 };
	currentDataSetNumberText.amPos = { pos.amPos.x, pos.amPos.y, pos.amPos.x + 400, pos.amPos.y + 25 };
	currentDataSetNumberText.videoPos = { -1,-1,-1,-1 };
	currentDataSetNumberText.triggerModeSensitive = isTriggerModeSensitive;
	currentDataSetNumberText.Create( "Most Recent Data Set #:", NORM_STATIC_OPTIONS, currentDataSetNumberText.seriesPos, 
									 parent, id++);
	currentDataSetNumberDisp.seriesPos = { pos.seriesPos.x + 400, pos.seriesPos.y, pos.seriesPos.x + 480, pos.seriesPos.y + 25 };
	currentDataSetNumberDisp.amPos = { pos.amPos.x + 400, pos.amPos.y, pos.amPos.x + 480, pos.amPos.y + 25 };
	currentDataSetNumberDisp.videoPos = { -1,-1,-1,-1 };
	currentDataSetNumberDisp.triggerModeSensitive = isTriggerModeSensitive;
	currentDataSetNumberDisp.Create("?", NORM_STATIC_OPTIONS, currentDataSetNumberDisp.seriesPos, parent, id++);
	pos.seriesPos.y += 25;
	pos.amPos.y += 25;

	// Atom Grid Settings
	gridHeader.seriesPos = { pos.seriesPos.x, pos.seriesPos.y, pos.seriesPos.x + 480, pos.seriesPos.y += 25 };
	gridHeader.amPos = { pos.amPos.x, pos.amPos.y, pos.amPos.x + 480, pos.amPos.y += 25 };
	gridHeader.videoPos = { -1,-1,-1,-1 };
	gridHeader.triggerModeSensitive = isTriggerModeSensitive;
	gridHeader.Create( "Atom Grid Settings", NORM_STATIC_OPTIONS, gridHeader.seriesPos, parent, id++ );
	//
	setGridCorner.seriesPos = { pos.seriesPos.x, pos.seriesPos.y, pos.seriesPos.x + 240, pos.seriesPos.y + 25 };
	setGridCorner.amPos = { pos.amPos.x, pos.amPos.y, pos.amPos.x + 240, pos.amPos.y + 25 };
	setGridCorner.videoPos = { -1,-1,-1,-1 };
	setGridCorner.triggerModeSensitive = isTriggerModeSensitive;
	setGridCorner.Create( "Set Grid Top-Left Corner", NORM_CWND_OPTIONS | BS_PUSHLIKE | BS_CHECKBOX,
						  setGridCorner.seriesPos, parent, IDC_SET_GRID_CORNER );
	//
	gridSpacingText.seriesPos = { pos.seriesPos.x + 240, pos.seriesPos.y, pos.seriesPos.x + 360, pos.seriesPos.y + 25 };
	gridSpacingText.amPos = { pos.amPos.x + 240, pos.amPos.y, pos.amPos.x + 360, pos.amPos.y + 25 };
	gridSpacingText.videoPos = { -1,-1,-1,-1 };
	gridSpacingText.triggerModeSensitive = isTriggerModeSensitive;
	gridSpacingText.Create("Pixel Spacing", NORM_STATIC_OPTIONS, gridSpacingText.seriesPos, parent, id++ );

	gridSpacing.seriesPos = { pos.seriesPos.x + 360, pos.seriesPos.y, pos.seriesPos.x + 480, pos.seriesPos.y += 25 };
	gridSpacing.amPos = { pos.amPos.x + 360, pos.amPos.y, pos.amPos.x + 480, pos.amPos.y += 25 };
	gridSpacing.videoPos = { -1,-1,-1,-1 };
	gridSpacing.triggerModeSensitive = isTriggerModeSensitive;
	gridSpacing.Create( NORM_EDIT_OPTIONS, gridSpacing.seriesPos, parent, id++ );

	gridWidthText.seriesPos = { pos.seriesPos.x, pos.seriesPos.y, pos.seriesPos.x + 120, pos.seriesPos.y + 25 };
	gridWidthText.amPos = { pos.amPos.x, pos.amPos.y, pos.amPos.x + 120, pos.amPos.y + 25 };
	gridWidthText.videoPos = { -1,-1,-1,-1 };
	gridWidthText.triggerModeSensitive = isTriggerModeSensitive;
	gridWidthText.Create( "Width", NORM_STATIC_OPTIONS, gridWidthText.seriesPos, parent, id++ );

	gridWidth.seriesPos = { pos.seriesPos.x + 120, pos.seriesPos.y, pos.seriesPos.x + 240, pos.seriesPos.y + 25 };
	gridWidth.amPos = { pos.amPos.x + 120, pos.amPos.y, pos.amPos.x + 240, pos.amPos.y + 25 };
	gridWidth.videoPos = { -1,-1,-1,-1 };
	gridWidth.triggerModeSensitive = isTriggerModeSensitive;
	gridWidth.Create( NORM_EDIT_OPTIONS, gridWidth.seriesPos, parent, id++ );

	gridHeightText.seriesPos = { pos.seriesPos.x + 240, pos.seriesPos.y, pos.seriesPos.x + 360, pos.seriesPos.y + 25 };
	gridHeightText.amPos = { pos.amPos.x + 240, pos.amPos.y, pos.amPos.x + 360, pos.amPos.y + 25 };
	gridHeightText.videoPos = { -1,-1,-1,-1 };
	gridHeightText.triggerModeSensitive = isTriggerModeSensitive;
	gridHeightText.Create( "Height", NORM_STATIC_OPTIONS, gridHeightText.seriesPos, parent, id++ );

	gridHeight.seriesPos = { pos.seriesPos.x + 360, pos.seriesPos.y, pos.seriesPos.x + 480, pos.seriesPos.y += 25 };
	gridHeight.amPos = { pos.amPos.x + 360, pos.amPos.y, pos.amPos.x + 480, pos.amPos.y += 25 };
	gridHeight.videoPos = { -1,-1,-1,-1 };
	gridHeight.triggerModeSensitive = isTriggerModeSensitive;
	gridHeight.Create( NORM_EDIT_OPTIONS, gridHeight.seriesPos, parent, id++ );
	// 
	manualSetAnalysisLocsButton.seriesPos = { pos.seriesPos.x, pos.seriesPos.y,
		pos.seriesPos.x + 480, pos.seriesPos.y + 25 };
	manualSetAnalysisLocsButton.amPos = { pos.amPos.x, pos.amPos.y,
		pos.amPos.x + 480, pos.amPos.y + 25 };
	manualSetAnalysisLocsButton.videoPos = { -1,-1,-1,-1 };
	manualSetAnalysisLocsButton.triggerModeSensitive = isTriggerModeSensitive;
	manualSetAnalysisLocsButton.Create("Manually Set AutoAnalysis Points", NORM_CWND_OPTIONS | BS_PUSHLIKE | BS_CHECKBOX,
									  manualSetAnalysisLocsButton.seriesPos, parent, IDC_SET_ANALYSIS_LOCATIONS );
	manualSetAnalysisLocsButton.EnableWindow( false );
	pos.seriesPos.y += 25;
	pos.amPos.y += 25;
	/// PLOTTING FREQUENCY CONTROLS
	// Set Plotting Frequency
	updateFrequencyLabel1.seriesPos = { pos.seriesPos.x,  pos.seriesPos.y,  pos.seriesPos.x + 150, pos.seriesPos.y + 25 };
	updateFrequencyLabel1.videoPos = { pos.videoPos.x, pos.videoPos.y, pos.videoPos.x + 150, pos.videoPos.y + 25 };
	updateFrequencyLabel1.amPos = { pos.amPos.x,   pos.amPos.y,   pos.amPos.x + 150, pos.amPos.y + 25 };
	updateFrequencyLabel1.triggerModeSensitive = isTriggerModeSensitive;
	updateFrequencyLabel1.Create("Update plots every (", NORM_STATIC_OPTIONS, updateFrequencyLabel1.seriesPos, parent, 
								  id++);
	// Plotting Frequency Edit
	updateFrequencyEdit.seriesPos = { pos.seriesPos.x + 150, pos.seriesPos.y,pos.seriesPos.x + 200, pos.seriesPos.y + 25 };
	updateFrequencyEdit.videoPos = { pos.videoPos.x + 150, pos.videoPos.y, pos.videoPos.x + 200, pos.videoPos.y + 25 };
	updateFrequencyEdit.amPos = { pos.amPos.x + 150, pos.amPos.y, pos.amPos.x + 200, pos.amPos.y + 25 };
	updateFrequencyEdit.triggerModeSensitive = isTriggerModeSensitive;
	updateFrequencyEdit.Create( NORM_EDIT_OPTIONS, updateFrequencyEdit.seriesPos, parent, id++);
	updateFrequency = 5;
	updateFrequencyEdit.SetWindowTextA("5");
	// end of that statement
	updateFrequencyLabel2.seriesPos = { pos.seriesPos.x + 200,  pos.seriesPos.y,  pos.seriesPos.x + 480, pos.seriesPos.y += 25 };
	updateFrequencyLabel2.videoPos = { pos.videoPos.x + 200, pos.videoPos.y, pos.videoPos.x + 480, pos.videoPos.y += 25 };
	updateFrequencyLabel2.amPos = { pos.amPos.x + 200,   pos.amPos.y,   pos.amPos.x + 480, pos.amPos.y += 25 };
	updateFrequencyLabel2.triggerModeSensitive = isTriggerModeSensitive;
	updateFrequencyLabel2.Create(") repetitions.", NORM_STATIC_OPTIONS, updateFrequencyLabel2.seriesPos, parent, id++);
	/// the listview
	plotListview.seriesPos = { pos.seriesPos.x,   pos.seriesPos.y,  pos.seriesPos.x + 480,  pos.seriesPos.y += 150 };
	plotListview.videoPos = { pos.videoPos.x, pos.videoPos.y, pos.videoPos.x + 480, pos.videoPos.y += 150 };
	plotListview.amPos = { pos.amPos.x,     pos.amPos.y,   pos.amPos.x + 480,   pos.amPos.y += 150 };
	plotListview.triggerModeSensitive = isTriggerModeSensitive;
	plotListview.Create( NORM_LISTVIEW_OPTIONS, plotListview.seriesPos, parent, IDC_PLOTTING_LISTVIEW );
	// initialize the listview
	LV_COLUMN listViewDefaultCollumn;
	// Zero Members
	memset(&listViewDefaultCollumn, 0, sizeof(listViewDefaultCollumn));
	listViewDefaultCollumn.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	listViewDefaultCollumn.pszText = "Name";
	// width between each coloum
	listViewDefaultCollumn.cx = 0x82;
	// Inserting Collumbs as much as we want
	plotListview.InsertColumn(0, &listViewDefaultCollumn);
	listViewDefaultCollumn.pszText = "Active?";
	plotListview.InsertColumn(1, &listViewDefaultCollumn);
	listViewDefaultCollumn.pszText = "View Details?";
	plotListview.InsertColumn(1, &listViewDefaultCollumn);
	listViewDefaultCollumn.pszText = "Edit?";
	plotListview.InsertColumn(1, &listViewDefaultCollumn);

	plotListview.SetBkColor( rgbs["Solarized Base02"]);
	plotListview.SetTextBkColor( rgbs["Solarized Base02"] );
	plotListview.SetTextColor( rgbs["Solarized Green"] );
	//
	reloadListView();
}


ULONG DataAnalysisControl::getPlotFreq( )
{
	CString txt;
	updateFrequencyEdit.GetWindowText(txt);
	try
	{
		updateFrequency = std::stol( str( txt ) );
	}
	catch ( std::invalid_argument& )
	{
		thrower( "ERROR: Failed to convert plotting update frequency to an integer! text was: " + str( txt ) );
	}
	return updateFrequency;
}


void DataAnalysisControl::handleOpenConfig( std::ifstream& file, int versionMajor, int versionMinor )
{
	ProfileSystem::checkDelimiterLine( file, "BEGIN_DATA_ANALYSIS" );
	file >> currentGrid.topLeftCorner.row;
	file >> currentGrid.topLeftCorner.column;
	file >> currentGrid.width;
	gridWidth.SetWindowTextA( cstr(currentGrid.width) );
	file >> currentGrid.height;
	gridHeight.SetWindowTextA( cstr( currentGrid.height ) );
	file >> currentGrid.pixelSpacing;
	gridSpacing.SetWindowTextA( cstr( currentGrid.pixelSpacing ) );
	if ( (versionMajor == 2 && versionMinor > 7) || versionMajor > 2 )
	{
		ProfileSystem::checkDelimiterLine( file, "BEGIN_ACTIVE_PLOTS" );
		UINT numPlots = 0;
		file >> numPlots;
		file.get( );
		vector<std::string> activePlotNames;
		for ( auto pltInc : range( numPlots ) )
		{
			std::string tmp;
			std::getline( file, tmp );
			activePlotNames.push_back( tmp );
		}
		UINT counter = 0;
		for ( auto& pltInfo : allPlots )
		{
			LVITEM listViewItem;
			memset( &listViewItem, 0, sizeof( listViewItem ) );
			listViewItem.mask = LVIF_TEXT;
			listViewItem.cchTextMax = 256;
			listViewItem.iItem = counter;
			listViewItem.iSubItem = 3;
			bool found = false;
			for ( auto activePlt : activePlotNames )
			{
				if ( activePlt == pltInfo.name )
				{
					pltInfo.isActive = true;
					listViewItem.pszText = "YES";
					plotListview.SetItem( &listViewItem );
					found = true;
					break;
				}
			}
			if ( !found )
			{
				pltInfo.isActive = false;
				listViewItem.pszText = "NO";
				plotListview.SetItem( &listViewItem );
			}
			counter++;
		}
		ProfileSystem::checkDelimiterLine( file, "END_ACTIVE_PLOTS" );
	}
	ProfileSystem::checkDelimiterLine( file, "END_DATA_ANALYSIS" );
}


void DataAnalysisControl::handleNewConfig( std::ofstream& file )
{
	file << "BEGIN_DATA_ANALYSIS\n";
	file << 0 << " " << 0 << "\n";
	file << 0 << " " << 0 << " " << 0 << "\n";
	file << "BEGIN_ACTIVE_PLOTS\n";
	file << "0\n";
	file << "END_ACTIVE_PLOTS\n";
	file << "END_DATA_ANALYSIS\n";
}


void DataAnalysisControl::handleSaveConfig( std::ofstream& file )
{
	file << "BEGIN_DATA_ANALYSIS\n";
	file << currentGrid.topLeftCorner.row << " " << currentGrid.topLeftCorner.column << "\n";
	file << currentGrid.width << " " << currentGrid.height << " " << currentGrid.pixelSpacing << "\n";
	file << "BEGIN_ACTIVE_PLOTS\n";
	UINT activeCount = 0;
	for ( auto miniPlot : allPlots )
	{
		if ( miniPlot.isActive )
		{
			activeCount++;
		}
	}
	file << activeCount << "\n";
	for ( auto miniPlot : allPlots )
	{
		if ( miniPlot.isActive )
		{
			file << miniPlot.name << "\n";
		}
	}
	file << "END_ACTIVE_PLOTS\n";
	file << "END_DATA_ANALYSIS\n";


}



unsigned __stdcall DataAnalysisControl::plotterProcedure(void* voidInput)
{
	realTimePlotterInput* input = (realTimePlotterInput*)voidInput;
	// make vector of plot information classes. 
	vector<PlottingInfo> allPlots;
	/// open files
	for (auto plotInc : range(input->plotInfo.size()))
	{
		std::string tempFileName = PLOT_FILES_SAVE_LOCATION + "\\" + input->plotInfo[plotInc].name +  "."
			+ PLOTTING_EXTENSION;
		allPlots.push_back(PlottingInfo::PlottingInfo(tempFileName));
		allPlots[plotInc].setGroups(input->analysisLocations);
	}
	if (allPlots.size() == 0)
	{
		// no plots to run so just quit.
		return 0;
	}
	/// check pictures per experiment
	for (auto plotInc : range(allPlots.size()))
	{
		if (allPlots[0].getPicNumber() != allPlots[plotInc].getPicNumber())
		{
			errBox("ERROR: Number of pictures per experiment don't match between plots. The plotting thread will "
					"close.");
			return 0;
		}
	}
	UINT groupNum = input->atomGridInfo.height * input->atomGridInfo.width;
	int totalNumberOfPixels = 0;
	int numberOfLossDataPixels = 0;
	
	/// figure out which pixels need any data
	for (auto plotInc :range(allPlots.size()))
	{
		for (auto pixelInc :range(allPlots[plotInc].getPixelNumber()))
		{
			for (auto groupInc :range(groupNum))
			{
				bool alreadyExists = false;
				/*
				// allPlots[plotInc].getPixelLocation(pixelInc, groupInc, row, collumn);
				for (UINT savedPixelInc = 0; savedPixelInc < pixelDataType.size(); savedPixelInc++)
				{
					// figure out if it already exists
					if (allPlots[plotInc].getPlotType() == "Atoms")
					{
						std::array<UINT, 3> testArray = { { row, collumn, 1 } };
						if (pixelDataType[savedPixelInc] == testArray)
						{
							alreadyExists = true;
							allPlots[plotInc].setPixelIndex(pixelInc, groupInc, savedPixelInc);
						}
					}
					else
					{
						std::array<UINT, 3> testArray1 = { { row, collumn, 1 } }, testArray2 = { { row, collumn, 0 } };
						if (pixelDataType[savedPixelInc] == testArray1 || pixelDataType[savedPixelInc] == testArray2)
						{
							alreadyExists = true;
							allPlots[plotInc].setPixelIndex(pixelInc, groupInc, savedPixelInc);
						}
					}
				}
				// if doesn't already exist, add it.
				if (!alreadyExists)
				{
					if (allPlots[plotInc].getPlotType() == "Atoms")
					{
						// in this case I need atom data
						pixelDataType.push_back({ row, collumn, 1 });
						allPlots[plotInc].setPixelIndex(pixelInc, groupInc, totalNumberOfPixels);
						totalNumberOfPixels++;
					}
					else
					{
						// in this case I need atom data
						pixelDataType.push_back({ row, collumn, 0 });
						allPlots[plotInc].setPixelIndex(pixelInc, groupInc, totalNumberOfPixels);
						totalNumberOfPixels++;
						numberOfLossDataPixels++;
					}
				}
				*/
			}
		}
	}
	// 
	if (totalNumberOfPixels == 0)
	{
		// no locations selected for analysis; quit.
		// return 0;
	}
	/// Initialize Arrays for data. (using std::vector above)
	// thinking about making these experiment-picture sized and resetting after getting the needed data out of them.
	// pixelData[pixel Indicator][picture number indicator] = pixelCount;
	vector<vector<long>> countData( groupNum );
	// atomPresentData[pixelIndicator][picture number] = true if atom present, false if atom not present;
	vector<vector<int> > atomPresentData( groupNum );
	// finalData[plot][dataset][group][repetitionNumber];
	vector<vector<vector<vector<long> > > > finalCountData( allPlots.size( ) );
	vector<vector<vector<std::pair<double, ULONG> > > > finalDataNew( allPlots.size( ) );
	vector<variationData> finalAvgs( allPlots.size( ) ), finalErrorBars( allPlots.size( ) ), 
		finalXVals( allPlots.size( ) );
	// Averaged over all pixels (avgAvg is the average of averages over repetitions)
	vector<avgData> avgAvg( allPlots.size( )), avgErrBar( allPlots.size( )), avgXVals( allPlots.size( ));
	// newData[plot][dataSet][group] = true if new data so change some vector sizes.
	vector<vector<vector<bool> > > newData( allPlots.size( ));
	vector<vector<vector<std::deque<double>>>> finalHistData( allPlots.size( ) );
	vector<vector<vector<std::map<int, std::pair<int, ULONG>>>>> histogramData( allPlots.size( ));


	// much resizing...
	for (auto plotInc : range(allPlots.size()))
	{
		UINT datasetNumber = allPlots[plotInc].getDataSetNumber( );
		histogramData[plotInc].resize( datasetNumber );
		finalCountData[plotInc].resize( datasetNumber );
		finalDataNew[plotInc].resize( datasetNumber );
		finalAvgs[plotInc].resize( datasetNumber );
		finalErrorBars[plotInc].resize( datasetNumber );
		finalXVals[plotInc].resize( datasetNumber );
		avgAvg[plotInc].resize( datasetNumber );
		avgErrBar[plotInc].resize( datasetNumber );
		avgXVals[plotInc].resize( datasetNumber );
		newData[plotInc].resize( datasetNumber );
		for (auto dataSetInc : range(allPlots[plotInc].getDataSetNumber()))
		{
			histogramData[plotInc][dataSetInc].resize( groupNum );
			finalCountData[plotInc][dataSetInc].resize( groupNum );
			finalAvgs[plotInc][dataSetInc].resize( groupNum );
			finalDataNew[plotInc][dataSetInc].resize( groupNum );
			finalErrorBars[plotInc][dataSetInc].resize( groupNum );
			finalXVals[plotInc][dataSetInc].resize( groupNum );
			newData[plotInc][dataSetInc].resize( groupNum, true );
		}
	}

	UINT noAtomsCounter = 0, atomCounterTotal = 0, currentThreadPictureNumber = 1, plotNumberCount = 0;
	// this keeps track of whether a "slow" message has been sent to the main window yet. Only want to send msg once.
	bool plotIsSlowStatus = false;
	/// Start loop waiting for plots
	while ((*input->active || (input->atomQueue->size() > 0)) && (!*input->aborting))
	{
		// if no image, continue.
		if (input->atomQueue->size() == 0 || input->atomQueue[0].size() == 0)
		{
			continue;
		}
		std::vector<long> tempImage;
		if ( input->needsCounts )
		{
			std::lock_guard<std::mutex> locker( *input->plotLock );
			if ( input->imageQueue->size( ) == 0 )
			{
				continue;
			}
			tempImage = input->imageQueue->front( );
			if ( tempImage.size( ) == 0 )
			{
				continue;
			}
		}
		if (input->atomQueue->size() > 2 && plotIsSlowStatus == false)
		{
			//PostMessage(eCameraWindowHandle, ePlottingIsSlowMessage, 0, 0);
			plotIsSlowStatus = true;
		}
		else if (input->atomQueue->size() == 1 && plotIsSlowStatus == true)
		{
			// the plotting has caught up, reset this.
			plotIsSlowStatus = false;
		}
		if (input->needsCounts)
		{
			/// for all pixels... gather count information
			UINT count = 0;
			std::lock_guard<std::mutex> locker( *input->plotLock );
			for ( auto row : range( input->atomGridInfo.width ) )
			{
				for ( auto column : range( input->atomGridInfo.height ) )
				{
					countData[count].push_back( tempImage[count] );
					count++;
				}
			}
		}
		/// get all the atom data problem
		bool thereIsAtLeastOneAtom = false;
		for (UINT pixelInc = 0; pixelInc < groupNum; pixelInc++)
		{
			if (input->atomQueue->at(0)[pixelInc])
			{
				thereIsAtLeastOneAtom = true;
				atomPresentData[pixelInc].push_back(1);
			}
			else
			{
				atomPresentData[pixelInc].push_back(0);
			}
		}
		if ( thereIsAtLeastOneAtom )
		{
			noAtomsCounter = 0;
		}
		else
		{
			noAtomsCounter++;
		}
		if (noAtomsCounter >= input->alertThreshold && input->wantAlerts )
		{
			input->comm->sendNoAtomsAlert( );
		}
		/// check if have enough data to plot
		if (currentThreadPictureNumber % allPlots[0].getPicNumber() != 0)
		{
			// In this case, not enough data to plot a point yet, but I've just analyzed a pic, so remove that pic.
			std::lock_guard<std::mutex> locker(*input->plotLock);
			if ( input->needsCounts && input->imageQueue->size( ) > 0 )
			{
				input->imageQueue->erase( input->imageQueue->begin( ) );
			}
			if ( input->atomQueue->size( ) > 0 )
			{
				input->atomQueue->erase( input->atomQueue->begin( ) );
			}
			// increment the thread's accumulation Number.
			currentThreadPictureNumber++;
			// wait for next picture.
			continue;
		}
		plotNumberCount++;
		// for every plot...
		for ( auto plotI : range( allPlots.size( ) ) )
		{
			/// Check Post-Selection Conditions
			// initialize this vector to all true. 
			// statisfiesPsc[dataSetI][groupI] = true or false
			vector<vector<bool> > satisfiesPsc( allPlots[plotI].getDataSetNumber( ), vector<bool>( groupNum, true ) );
			determineWhichPscsSatisfied( allPlots[plotI], groupNum, atomPresentData, satisfiesPsc );
			// split into one of two big subroutines. The handling here is encapsulated into functions mostly just for 
			// organization purposes.
			if ( allPlots[plotI].getPlotType( ) == "Atoms" )
			{
				DataAnalysisControl::handlePlotAtoms( input, allPlots[plotI], currentThreadPictureNumber,
													  finalDataNew[plotI],
													  finalAvgs[plotI], finalErrorBars[plotI], finalXVals[plotI],
													  avgAvg[plotI], avgErrBar[plotI], avgXVals[plotI],
													  newData[plotI], satisfiesPsc, plotI, countData,
													  plotNumberCount, atomPresentData );
			}
			else if ( allPlots[plotI].getPlotType( ) == "Pixel Counts" )
			{
				DataAnalysisControl::handlePlotCounts( input, allPlots[plotI], currentThreadPictureNumber,
													   finalCountData[plotI], finalAvgs[plotI], finalErrorBars[plotI],
													   finalXVals[plotI], avgAvg[plotI], avgErrBar[plotI],
													   avgXVals[plotI], newData[plotI], satisfiesPsc, plotI, countData,
													   plotNumberCount, atomPresentData );
			}
			else if ( allPlots[plotI].getPlotType( ) == "Pixel Count Histograms" )
			{
				DataAnalysisControl::handlePlotHist( input, allPlots[plotI], plotI, countData, finalHistData[plotI],
													 satisfiesPsc, plotNumberCount, histogramData[plotI] );
			}
		}
		/// clear data
		// all pixels being recorded, not pixels in a data set.
		for (auto pixelI : range(groupNum))
		{
			countData[pixelI].clear();
			atomPresentData[pixelI].clear();
		}
		// finally, remove the data from the queue.
		std::lock_guard<std::mutex> locker(*input->plotLock);
		if (input->needsCounts)
		{
			// delete the first entry of the Queue which has just been handled.
			if ( input->imageQueue->size( ) != 0 )
			{
				input->imageQueue->erase( input->imageQueue->begin( ) );
			}
		}
		input->atomQueue->erase(input->atomQueue->begin());
		currentThreadPictureNumber++;
	}
	return 0;
}


void DataAnalysisControl::determineWhichPscsSatisfied( PlottingInfo& info, UINT groupSize, 
													   vector<vector<int>> atomPresentData, 
													   vector<vector<bool>>& pscSatisfied)
{
	// There's got to be a better way to iterate through these guys...
	for ( auto dataSetI : range( info.getDataSetNumber( ) ) )
	{
		for ( auto groupI : range( groupSize ) )
		{
			for ( auto conditionI : range( info.getConditionNumber( ) ) )
			{
				for ( auto pixelI : range( info.getPixelNumber( ) ) )
				{
					for ( auto picI : range( info.getPicNumber( ) ) )
					{
						// test if condition exists
						int condition = info.getPostSelectionCondition( dataSetI, conditionI, pixelI, picI );
						if ( condition == 0 )
						{
							continue;
						}
						if ( condition == 1 && atomPresentData[groupI][picI] != 1 )
						{
							pscSatisfied[dataSetI][groupI] = false;
						}
						else if ( condition == -1 && atomPresentData[groupI][picI] != 0 )
						{
							pscSatisfied[dataSetI][groupI] = false;
						}
					}
				}
			}
		}
	}
}


void DataAnalysisControl::handlePlotAtoms( realTimePlotterInput* input, PlottingInfo plotInfo, UINT pictureNumber,
										   vector<vector<std::pair<double, ULONG>> >& finData,
										   variationData& finAvgs, variationData& finErrs, variationData& finX,
										   avgData& avgAvgs, avgData& avgErrs, avgData& avgX,
										   vector<vector<bool> >& needNewData,
										   vector<vector<bool>>& pscSatisfied, int plotNumber,
										   vector<vector<long>>& countData, int plotNumberCount,
										   vector<vector<int> > atomPresent )
{
	UINT groupNum = input->atomGridInfo.width * input->atomGridInfo.height;
	if (pictureNumber % input->picsPerVariation == plotInfo.getPicNumber())
	{
		// first pic of new variation, so need to update x vals.
		finData = vector<vector<std::pair<double, ULONG>>>(  plotInfo.getDataSetNumber( ), 
															vector<std::pair<double, ULONG>>( groupNum, { 0,0 } ) );
		needNewData = vector<vector<bool>>( plotInfo.getDataSetNumber( ), vector<bool>( groupNum, true ) );
	}
	/// Check Data Conditions
	for (auto dataSetI : range(plotInfo.getDataSetNumber()))
	{
		for (auto groupI :range(groupNum))
		{
			if (pscSatisfied[dataSetI][groupI] == false)
			{
				// no new data.
				continue;
			}
			bool dataVal = true;
			for (auto pixelI : range(plotInfo.getPixelNumber()))
			{
				for (auto picI : range(plotInfo.getPicNumber()))
				{
					// check if there is a condition at all
					int truthCondition = plotInfo.getResultCondition(dataSetI, pixelI, picI);
					if (truthCondition == 0)
					{
						continue;
					}
					int pixel = groupI;
					if (truthCondition == 1 && atomPresent[pixel][picI] != 1)
					{
						dataVal = false;
					}
					else if (truthCondition == 0 && atomPresent[groupI][picI] != 0)
					{
						dataVal = false;
					}
				}
			}
			finData[dataSetI][groupI].first += dataVal;
			finData[dataSetI][groupI].second++;
			// then the size of the containers gets updated every time.
			if (plotInfo.getXAxis() == "Running Average")
			{
				needNewData[dataSetI][groupI] = true;
			}
		}
	}
	// Core data structures have been updated. return if not time for a plot update yet.
	if ( plotNumberCount % input->plottingFrequency != 0 )
	{
		return;
	}
	/// Calculate averages and standard devations for Data sets AND groups...
	for ( auto dataSetI : range(plotInfo.getDataSetNumber( )))
	{
		UINT maxSize = 0;
		// for each pixel
		for ( auto groupI : range( groupNum))
		{
			// check if first picture of set
			// ??????????????????????
			if ( pictureNumber % input->plottingFrequency != 0 )
			{
				continue;
			}
			if ( needNewData[dataSetI][groupI] == true )
			{
				finAvgs[dataSetI][groupI].resize( finAvgs[dataSetI][groupI].size( ) + 1 );
				if ( plotInfo.getXAxis( ) == "Running Average" )
				{
					finX[dataSetI][groupI].resize( finX[dataSetI][groupI].size( ) + 1 );
					if ( !(finData[dataSetI][groupI].second >= input->numberOfRunsToAverage ) )
					{
						finX[dataSetI][groupI].back( ) = (std::accumulate( finX[dataSetI][groupI].begin( ),
																		   finX[dataSetI][groupI].end( ), 0.0 )
														   + finData[dataSetI][groupI].second)
							/ finData[dataSetI][groupI].second;
					}
				}
				else
				{
					finErrs[dataSetI][groupI].resize( finErrs[dataSetI][groupI].size( ) + 1 );
					finX[dataSetI][groupI].push_back( input->key[(pictureNumber - 1) / input->picsPerVariation] );
				}
				// set the flag to not do this again before this array gets reset at beginning of the next accumulation stack.
				needNewData[dataSetI][groupI] = false;
			}
			// calculate new data points
			if ( plotInfo.getXAxis( ) == "Running Average" )
			{
				if ( finData[dataSetI][groupI].second >= input->numberOfRunsToAverage )
				{
					double sum = 1;
					// need to change if going to get running average working again.
					double mean = sum / input->numberOfRunsToAverage;
					finAvgs[dataSetI][groupI].back( ) = mean;
					finX[dataSetI][groupI].back( ) = (std::accumulate( finX[dataSetI][groupI].end( )
																	   - input->numberOfRunsToAverage + 1,
																	   finX[dataSetI][groupI].end( ), 0.0 )
													   + finData[dataSetI][groupI].second ) / input->numberOfRunsToAverage;
					input->plotter->send( "set xrange [" + str( finX[dataSetI][groupI][0] - 1 ) + ":"
										  + str( finX[dataSetI][groupI].back( ) + 1 ) + "]" );
				}
			}
			else
			{
				double mean = finData[dataSetI][groupI].first / finData[dataSetI][groupI].second;
				double error = mean * ( 1 - mean ) / std::sqrt( finData[dataSetI][groupI].second );
				finAvgs[dataSetI][groupI].back( ) = mean;
				finErrs[dataSetI][groupI].back( ) = error;
			}
			if ( finAvgs[dataSetI][groupI].size( ) > maxSize )
			{
				maxSize = finAvgs[dataSetI][groupI].size( );
			}
		}
		/// calculate averages
		if ( maxSize > avgAvgs[dataSetI].size( ) )
		{
			//  resize the objects for new data.
			avgAvgs[dataSetI].resize( maxSize );
			avgErrs[dataSetI].resize( maxSize );
			avgX[dataSetI].resize( maxSize );
			avgX[dataSetI].back( ) = input->key[(pictureNumber - 1) / input->picsPerVariation];
		}
		double avgAvgVal = 0, avgErrsVal = 0;
		std::pair<double, ULONG> allDataTempNew(0,0);
		for ( auto data : finData[dataSetI] )
		{
			allDataTempNew.first += data.first;
			allDataTempNew.second += data.second;
		}
		double mean = allDataTempNew.first / allDataTempNew.second;
		double error = mean * ( 1 - mean ) / std::sqrt( allDataTempNew.second );
		avgAvgs[dataSetI].back( ) = mean;
		avgErrs[dataSetI].back( ) = error;
	}
	/// General Plotting Options
	input->plotter->send("set terminal wxt " + str(plotNumber) + " title \"" + plotInfo.getTitle() 
						  + "\" noraise background rgb 'black'");
	input->plotter->send("set format y \"%.1f\"");
	// set x range.
	double xRangeMin = *std::min_element(input->key.begin(), input->key.end());
	double xRangeMax = *std::max_element(input->key.begin(), input->key.end());
	double xrange = xRangeMax - xRangeMin;
	if ( xrange == 0 )
	{
		// zero range causes issues, but happens if you have only 1 data point.
		xrange++;
	}
	xRangeMin -= xrange / input->key.size();
	xRangeMax += xrange / input->key.size();
	input->plotter->send("set xrange [" + str(xRangeMin) + ":" + str(xRangeMax) + "]");
	input->plotter->send("set grid ytics lc rgb \"#bbbbbb\" lw 1 lt 0");
	input->plotter->send("set grid xtics lc rgb \"#bbbbbb\" lw 1 lt 0");
	input->plotter->send("set yrange [0:1]");
	input->plotter->send("set title \"" + plotInfo.getTitle() + "\" tc rgb 'white'");
	input->plotter->send("set xlabel \"Key Value\" tc rgb 'white'");
	input->plotter->send("set ylabel \"" + plotInfo.getYLabel() + "\" tc rgb 'white'");
	input->plotter->send("set border lc rgb 'white'");
	input->plotter->send("set key tc rgb 'white' outside");
	/// FITTING
	for (UINT dataSetI = 0; dataSetI < plotInfo.getDataSetNumber(); dataSetI++)
	{
		if (plotInfo.whenToFit(dataSetI) == REAL_TIME_FIT 
			|| (plotInfo.whenToFit(dataSetI) == FIT_AT_END && pictureNumber == input->picsPerVariation))
		{
			for (UINT groupInc = 0; groupInc < groupNum; groupInc++)
			{
				std::string fitNum = str( groupNum * dataSetI + groupInc);
				// in this case, fitting.
				switch (plotInfo.getFitOption(dataSetI))
				{
					// the to_string argument in each case is a unique number indicating the fit given the data set and group. I need
					// to keep track of each fit separately so that I can plot them all later. 
					case GAUSSIAN_FIT:
					{
						input->plotter->send("f" + fitNum + "(x) = A" + fitNum + " * exp(-(x - B" + fitNum 
											  + ")**2 / (2 * C" + fitNum + "))");
						input->plotter->send("A" + fitNum + " = 1");
						input->plotter->send("B" + fitNum + " = " + str(finX[dataSetI][groupInc].size() / 2.0));
						input->plotter->send("C" + fitNum + " = 1");
						input->plotter->send( "fit f" + fitNum + "(x) '-' using 1:2 via A" + fitNum + ", B" + fitNum 
											  + ", C" + fitNum );
						break;
					}
					case LORENTZIAN_FIT:
					{
						input->plotter->send( "f" + fitNum + "(x) = (A" + fitNum + " / (2 * 3.14159265359)) / ((x - B" 
											  + fitNum + ")**2 + (A" + fitNum + " / 2)**2)" );
						input->plotter->send( "A" + fitNum + " = 1" );
						input->plotter->send( "B" + fitNum + " = " + str( finX[dataSetI][groupInc].size( ) / 2.0 ) );
						input->plotter->send( "fit f" + fitNum + "(x) '-' using 1:2 via A" + fitNum + ", B" + fitNum );
						break;
					}
					case SINE_FIT:
					{
						input->plotter->send( "f" + fitNum + "(x) = A" + fitNum + " * sin(B" + fitNum + " * x + C" 
											  + fitNum + ") * exp( - D" + fitNum + " * x)" );
						input->plotter->send( "A" + fitNum + " = 1" );
						input->plotter->send( "B" + fitNum + " = 1" );
						input->plotter->send( "C" + fitNum + " = 1" );
						input->plotter->send( "D" + fitNum + " = 1" );
						input->plotter->send( "fit f" + fitNum + "(x) '-' using 1:2 via A" + fitNum + ", B" + fitNum + ", C" + fitNum + ", D" + fitNum );
						break;
					}
					default:
					{
						errBox( "Coding Error: Bad Fit option!" );
					}
					input->plotter->sendData( finX[dataSetI][groupInc], finAvgs[dataSetI][groupInc] );
				}
			}
		}
	}
	/// ////////////////////////////
	/// SEND PLOT COMMANDS AND DATA

	/// send plot commands
	std::string gnuplotPlotCmd = "plot";
	if ( plotInfo.getXAxis( ) == "Running Average" )
	{
		for ( auto dataSetI : range(plotInfo.getDataSetNumber( )))
		{
			for ( auto groupI : range(groupNum) )
			{
				UINT markerNumber = dataSetI % GNUPLOT_MARKERS.size( );
				UINT colorSpacing = 256 / finAvgs[dataSetI].size( );
				std::string colorText = "\" lt rgb \"#" + GIST_RAINBOW[colorSpacing * groupI] + "\"";
				gnuplotPlotCmd += " '-' using 1:2 " + colorText + " " + GNUPLOT_MARKERS[markerNumber] + " title \"G"
								  + str( groupI + 1 ) + " " + plotInfo.getLegendText( dataSetI ) + "\",";
				if ( plotInfo.whenToFit( dataSetI ) == REAL_TIME_FIT
					 || (plotInfo.whenToFit( dataSetI ) == FIT_AT_END
						  && pictureNumber == input->picsPerVariation) )
				{
					std::string fitNum = str( groupNum* dataSetI + groupI );
					std::string plotString = "fit" + str( groupNum * dataSetI + groupI ) + "= ";
					switch ( plotInfo.getFitOption( dataSetI ) )
					{
						case GAUSSIAN_FIT:
						{
							plotString += "sprintf(\"%.3f * exp{/Symbol \\\\173}-(x - %.3f)^2 / (2 * %.3f){/Symbol \\\\175}\", A" + fitNum + ", B" + fitNum + ", C"
								+ fitNum + ")\n";
							break;
						}
						case LORENTZIAN_FIT:
						{

							plotString += "sprintf(\"(%.3f / (2 * Pi)) / ((x - %.3f)^2 + ( %.3f / 2)^2)\", A" + fitNum + ", B" + fitNum
								+ ", A" + fitNum + ")\n";
							break;
						}
						case SINE_FIT:
						{
							plotString += "sprintf(\"%.3f * sin{/Symbol \\\\173}%.3f * x + %.3f{/Symbol \\\\175} * exp{/Symbol \\\\173} - %.3f * x {/Symbol \\\\175}\", A" + fitNum + ", B" + fitNum
								+ ", C" + fitNum + ", D" + fitNum + ")\n";
							break;
						}
						default:
						{
							errBox( "Coding Error: Bad Fit option!" );
						}
					}
					input->plotter->send( plotString );
					UINT lineTypeNumber = dataSetI % GNUPLOT_LINETYPES.size();
					UINT colorSpacing = 256 / finAvgs[dataSetI].size( );
					std::string colorText = "\" lt rgb \"#" + GIST_RAINBOW[colorSpacing * groupI] + "\"";
					gnuplotPlotCmd += " f" + fitNum + "(x) title fit" + str( groupI ) + " " + colorText
									  + " " + GNUPLOT_LINETYPES[lineTypeNumber] + ",";
				}
			}
		}
		for ( auto dataSetI : range(plotInfo.getDataSetNumber( )) )
		{
			for ( auto groupI : range(groupNum))
			{
				if ( finData[dataSetI][groupI].second >= input->numberOfRunsToAverage )
				{
					UINT colorSpacing = 256 / finAvgs[dataSetI].size( );
					std::string colorText = "\" lt rgb \"#" + GIST_RAINBOW[colorSpacing * groupI] + "\"";
					gnuplotPlotCmd += " '-' using 1:2 " + colorText + " with lines title \"G"
						+ str( groupI + 1 ) + " " + plotInfo.getLegendText( dataSetI ) + "\",";
				}
			}
		}
	}
	else 
	{ 
		// average each variations
		for ( auto dataSetI : range(plotInfo.getDataSetNumber( )) )
		{
			for ( auto groupI : range(finAvgs[dataSetI].size( )) )
			{
				// handle color stuffs, should prob make this a function
				std::stringstream hexStream;
				hexStream << std::hex << int((1 - 1.0 / sqrt(finAvgs[dataSetI].size( ))) * 255);
				std::string alpha = hexStream.str( );
				if ( alpha.size( ) < 2)
				{
					// This shouldn't happen...
					alpha = "00";
				}
				else
				{
					alpha = alpha.substr( alpha.size( ) - 2 );
				}
				UINT colorSpacing = 256 / finAvgs[dataSetI].size( );
				UINT markerNumber = dataSetI % GNUPLOT_MARKERS.size( );
				std::string colorText = "\" lt rgb \"#" + alpha + GIST_RAINBOW[colorSpacing * groupI] + "\"";
				gnuplotPlotCmd += " '-' using 1:2:3 with yerrorbars title \"G" + str(groupI + 1) + " "
									+ plotInfo.getLegendText(dataSetI) + colorText + " " 
									+ GNUPLOT_MARKERS[markerNumber] + " pointsize 0.5,";

				if (plotInfo.whenToFit(dataSetI) == REAL_TIME_FIT
					|| (plotInfo.whenToFit(dataSetI) == FIT_AT_END && pictureNumber == input->picsPerVariation))
				{
					std::string fitNum = str( groupNum * dataSetI + groupI);
					std::string plotString = "fit" + fitNum + "= ";
					switch (plotInfo.getFitOption(dataSetI))
					{
						case GAUSSIAN_FIT:
						{
							plotString += "sprintf(\"%.3f * exp{/Symbol \\\\173}-(x - %.3f)^2 / (2 * %.3f){/Symbol \\\\175}\", A" + fitNum + ", B" + fitNum + ", C" + fitNum + ")\n";
							break;
						}
						case LORENTZIAN_FIT:
						{
							plotString += "sprintf(\"(%.3f / (2 * Pi)) / ((x - %.3f)^2 + (%.3f / 2)^2)\", A" + fitNum + ", B" + fitNum + ", A" + fitNum + ")\n";
							break;
						}
						case SINE_FIT:
						{
							plotString += "sprintf(\"%.3f * sin{/Symbol \\\\173}%.3f * x + %.3f{/Symbol \\\\175} * exp{/Symbol \\\\173} - %.3f * x {/Symbol \\\\175}\", A" + fitNum + ", B" + fitNum + ", C" + fitNum + ", D" + fitNum + ")\n";
							break;
						}
						default:
						{
							errBox("Coding Error: Bad Fit option!");
						}
					}
					input->plotter->send(plotString);
					UINT lineTypeNumber = dataSetI % GNUPLOT_LINETYPES.size( );
					UINT colorSpacing = 256 / finAvgs[dataSetI].size( );
					std::string colorText = "\" lt rgb \"#" + GIST_RAINBOW[colorSpacing * groupI] + "\"";
					gnuplotPlotCmd += " f" + fitNum + "(x) title fit" + fitNum + " " + colorText + " " 
									  + GNUPLOT_LINETYPES[dataSetI] + ",";
				}
			}
			// add the average line
			gnuplotPlotCmd += " '-' using 1:2:3 with yerrorbars title \"Average\" lt rgb \"#FFFFFF\" pt 5 pointsize 0.5,";
		}
	}
	std::string error;
	input->plotter->send(gnuplotPlotCmd);
	/// SEND DATA
	if (plotInfo.getXAxis() == "Running Average")
	{
		for (auto dataSetI : range(plotInfo.getDataSetNumber()))
		{
			for (auto groupI : range( groupNum))
			{
				// ???
				//input->plotter->sendData(finAvgs[dataSetI][groupI], finData[dataSetI][groupI]);
			}
			for (auto groupI : range( groupNum) )
			{
				if (finData[dataSetI][groupI].second >= input->numberOfRunsToAverage)
				{
					input->plotter->sendData(finX[dataSetI][groupI], finAvgs[dataSetI][groupI]);
				}
			}
		}
	}
	else
	{
		for ( auto dataSetI : range( plotInfo.getDataSetNumber()))
		{
			for (auto groupI : range(finAvgs[dataSetI].size()))
			{
				input->plotter->sendData(finX[dataSetI][groupI], finAvgs[dataSetI][groupI], finErrs[dataSetI][groupI]);
			}
			input->plotter->sendData( avgX[dataSetI], avgAvgs[dataSetI], avgErrs[dataSetI] );
		}
	}
}


/// using std::vector above
void DataAnalysisControl::handlePlotCounts( realTimePlotterInput* input, PlottingInfo plotInfo, UINT pictureNumber,
												   vector<vector<vector<long> > >& finData,
												   variationData& finAvgs, variationData& finErrs, variationData& finX,
												   avgData& avgAvgs, avgData& avgErrs, avgData& avgX,
												   vector<vector<bool> >& needNewData,
												   vector<vector<bool>>& pscSatisfied, int plotNumber,
												   vector<vector<long>>& countData, int plotNumberCount,
												   vector<vector<int> > atomPresent )
{
	// will eventually be passed in as arg
	vector<vector<std::pair<double, ULONG>> > finDataNew;
	//
	UINT groupNum = input->atomGridInfo.width * input->atomGridInfo.height;
	if ( pictureNumber % input->picsPerVariation == plotInfo.getPicNumber( ) )
	{
		// first pic of new variation, so need to update x vals.
		finData = vector<vector<vector<long>>>( plotInfo.getDataSetNumber( ),
															   vector<vector<long>>( groupNum ) );
		finDataNew = vector<vector<std::pair<double, ULONG>>>(
			plotInfo.getDataSetNumber( ), vector<std::pair<double, ULONG>>( groupNum, { 0,0 } ) );
		needNewData = vector<vector<bool>>( plotInfo.getDataSetNumber( ), vector<bool>( groupNum, true ) );
	}
	/// Check Data Conditions
	for ( auto dataSetI : range( plotInfo.getDataSetNumber( ) ) )
	{
		for ( auto groupI : range( groupNum ) )
		{
			if ( pscSatisfied[dataSetI][groupI] == false )
			{
				continue;
			}
			UINT pixel, picture;
			plotInfo.getDataCountsLocation( dataSetI, pixel, picture );
			finData[dataSetI][groupI].push_back( countData[groupI][picture] );
		}
	}
	// Core data structures have been updated. return if not time for a plot update yet.
	if ( plotNumberCount % input->plottingFrequency != 0 )
	{
		return;
	}
	/// Calculate averages and standard devations for Data sets AND groups...
	for ( auto dataSetI : range( plotInfo.getDataSetNumber( ) ) )
	{
		UINT maxSize = 0;
		// for each pixel
		for ( auto groupI : range( groupNum ) )
		{
			// check if first picture of set
			// ??????????????????????
			if ( pictureNumber % input->plottingFrequency != 0 )
			{
				continue;
			}
			if ( needNewData[dataSetI][groupI] == true )
			{
				finAvgs[dataSetI][groupI].resize( finAvgs[dataSetI][groupI].size( ) + 1 );
				// integer division here.
				if ( plotInfo.getXAxis( ) == "Running Average" )
				{
					finX[dataSetI][groupI].resize( finX[dataSetI][groupI].size( ) + 1 );
					if ( !(finData[dataSetI][groupI].size( ) >= input->numberOfRunsToAverage) )
					{
						finX[dataSetI][groupI].back( ) = (std::accumulate( finX[dataSetI][groupI].begin( ),
																		   finX[dataSetI][groupI].end( ), 0.0 )
														   + finData[dataSetI][groupI].size( ))
							/ finData[dataSetI][groupI].size( );
					}
				}
				else
				{
					finErrs[dataSetI][groupI].resize( finErrs[dataSetI][groupI].size( ) + 1 );
					finX[dataSetI][groupI].push_back( input->key[(pictureNumber - 1) / input->picsPerVariation] );
				}
				// set the flag to not do this again before this array gets reset at beginning of the next accumulation stack.
				needNewData[dataSetI][groupI] = false;
			}
			// calculate new data points
			if ( plotInfo.getXAxis( ) == "Running Average" )
			{
				if ( finData[dataSetI][groupI].size( ) >= input->numberOfRunsToAverage )
				{
					double sum = std::accumulate( finData[dataSetI][groupI].end( ) - input->numberOfRunsToAverage,
												  finData[dataSetI][groupI].end( ), 0.0 );
					double mean = sum / input->numberOfRunsToAverage;
					finAvgs[dataSetI][groupI].back( ) = mean;
					finX[dataSetI][groupI].back( ) = (std::accumulate( finX[dataSetI][groupI].end( )
																	   - input->numberOfRunsToAverage + 1,
																	   finX[dataSetI][groupI].end( ), 0.0 )
													   + finData[dataSetI][groupI].size( )) / input->numberOfRunsToAverage;
					input->plotter->send( "set xrange [" + str( finX[dataSetI][groupI][0] - 1 ) + ":"
										  + str( finX[dataSetI][groupI].back( ) + 1 ) + "]" );
				}
			}
			else
			{
				double sum = std::accumulate( finData[dataSetI][groupI].begin( ), finData[dataSetI][groupI].end( ), 0.0 );
				double mean = sum / finData[dataSetI][groupI].size( );
				double squaredSum = std::inner_product( finData[dataSetI][groupI].begin( ), finData[dataSetI][groupI].end( ),
														finData[dataSetI][groupI].begin( ), 0.0 );
				double error = ((double)std::sqrt( squaredSum / finData[dataSetI][groupI].size( ) - mean * mean ))
					/ std::sqrt( finData[dataSetI][groupI].size( ) );

				double meanNew = finDataNew[dataSetI][groupI].first / finDataNew[dataSetI][groupI].second;
				double errorNew = meanNew * (1 - meanNew) / std::sqrt( finDataNew[dataSetI][groupI].second );

				finAvgs[dataSetI][groupI].back( ) = mean;
				finErrs[dataSetI][groupI].back( ) = error;
			}
			if ( finAvgs[dataSetI][groupI].size( ) > maxSize )
			{
				maxSize = finAvgs[dataSetI][groupI].size( );
			}
		}
		/// calculate averages
		if ( maxSize > avgAvgs[dataSetI].size( ) )
		{
			//  resize the objects for new data.
			avgAvgs[dataSetI].resize( maxSize );
			avgErrs[dataSetI].resize( maxSize );
			avgX[dataSetI].resize( maxSize );
			avgX[dataSetI].back( ) = input->key[(pictureNumber - 1) / input->picsPerVariation];
		}
		double avgAvgVal = 0, avgErrsVal = 0;
		std::vector<long> allDataTemp;
		std::pair<double, ULONG> allDataTempNew( 0, 0 );
		for ( auto data : finData[dataSetI] )
		{
			allDataTemp.insert( allDataTemp.begin( ), data.begin( ), data.end( ) );
			//
			std::pair<double, ULONG> fromNewFinData;
			allDataTempNew.first += fromNewFinData.first;
			allDataTempNew.second += fromNewFinData.second;
		}

		double sum = std::accumulate( allDataTemp.begin( ), allDataTemp.end( ), 0.0 );
		double mean = sum / allDataTemp.size( );
		double squaredSum = std::inner_product( allDataTemp.begin( ), allDataTemp.end( ),
												allDataTemp.begin( ), 0.0 );
		double error = ((double)std::sqrt( squaredSum / allDataTemp.size( ) - mean * mean ))
			/ std::sqrt( allDataTemp.size( ) );

		double meanNew = allDataTempNew.first / allDataTempNew.second;
		double errorNew = meanNew * (1 - meanNew) / std::sqrt( allDataTempNew.second );
		avgAvgs[dataSetI].back( ) = mean;
		avgErrs[dataSetI].back( ) = error;
	}

	/// General Plotting Options
	input->plotter->send( "set terminal wxt " + str( plotNumber ) + " title \"" + plotInfo.getTitle( )
						  + "\" noraise background rgb 'black'" );
	input->plotter->send( "set format y \"%.1f\"" );
	// counts can be arbitrarily large.
	input->plotter->send( "set autoscale y" );
	input->plotter->send( "set title \"" + plotInfo.getTitle( ) + "\" tc rgb 'white'" );
	input->plotter->send( "set xlabel \"Key Value\" tc rgb 'white'" );
	input->plotter->send( "set ylabel \"" + plotInfo.getYLabel( ) + "\" tc rgb 'white'" );
	input->plotter->send( "set border lc rgb 'white'" );
	input->plotter->send( "set key tc rgb 'white' outside" );
	/// FITTING
	for ( UINT dataSetI = 0; dataSetI < plotInfo.getDataSetNumber( ); dataSetI++ )
	{
		if ( plotInfo.whenToFit( dataSetI ) == REAL_TIME_FIT
			 || (plotInfo.whenToFit( dataSetI ) == FIT_AT_END && pictureNumber == input->picsPerVariation) )
		{
			for ( UINT groupInc = 0; groupInc < groupNum; groupInc++ )
			{
				std::string fitNum = str( groupNum * dataSetI + groupInc );
				// in this case, fitting.
				switch ( plotInfo.getFitOption( dataSetI ) )
				{
					// the to_string argument in each case is a unique number indicating the fit given the data set and group. I need
					// to keep track of each fit separately so that I can plot them all later. 
					case GAUSSIAN_FIT:
					{
						input->plotter->send( "f" + fitNum + "(x) = A" + fitNum + " * exp(-(x - B" + fitNum
											  + ")**2 / (2 * C" + fitNum + "))" );
						input->plotter->send( "A" + fitNum + " = 1" );
						input->plotter->send( "B" + fitNum + " = " + str( finX[dataSetI][groupInc].size( ) / 2.0 ) );
						input->plotter->send( "C" + fitNum + " = 1" );
						input->plotter->send( "fit f" + fitNum + "(x) '-' using 1:2 via A" + fitNum + ", B" + fitNum
											  + ", C" + fitNum );
						break;
					}
					case LORENTZIAN_FIT:
					{
						input->plotter->send( "f" + fitNum + "(x) = (A" + fitNum + " / (2 * 3.14159265359)) / ((x - B" + fitNum + ")**2 + (A" + fitNum + " / 2)**2)" );
						input->plotter->send( "A" + fitNum + " = 1" );
						input->plotter->send( "B" + fitNum + " = " + str( finX[dataSetI][groupInc].size( ) / 2.0 ) );
						input->plotter->send( "fit f" + fitNum + "(x) '-' using 1:2 via A" + fitNum + ", B" + fitNum );
						break;
					}
					case SINE_FIT:
					{
						input->plotter->send( "f" + fitNum + "(x) = A" + fitNum + " * sin(B" + fitNum + " * x + C" + fitNum + ") * exp( - D" + fitNum + " * x)" );
						input->plotter->send( "A" + fitNum + " = 1" );
						input->plotter->send( "B" + fitNum + " = 1" );
						input->plotter->send( "C" + fitNum + " = 1" );
						input->plotter->send( "D" + fitNum + " = 1" );
						input->plotter->send( "fit f" + fitNum + "(x) '-' using 1:2 via A" + fitNum + ", B" + fitNum + ", C" + fitNum + ", D" + fitNum );
						break;
					}
					default:
					{
						errBox( "Coding Error: Bad Fit option!" );
					}
					input->plotter->sendData( finX[dataSetI][groupInc], finAvgs[dataSetI][groupInc] );
				}
			}
		}
	}
	/// ////////////////////////////
	/// SEND PLOT COMMANDS AND DATA

	/// send plot commands
	std::string gnuplotPlotCmd = "plot";
	if ( plotInfo.getXAxis( ) == "Running Average" )
	{
		for ( auto dataSetI : range( plotInfo.getDataSetNumber( ) ) )
		{
			for ( auto groupI : range( groupNum ) )
			{
				UINT markerNumber = dataSetI % GNUPLOT_MARKERS.size( );
				UINT colorSpacing = 256 / finAvgs[dataSetI].size( );
				std::string colorText = "\" lt rgb \"#" + GIST_RAINBOW[colorSpacing * groupI] + "\"";
				gnuplotPlotCmd += " '-' using 1:2 " + colorText + " " + GNUPLOT_MARKERS[markerNumber] + " title \"G"
					+ str( groupI + 1 ) + " " + plotInfo.getLegendText( dataSetI ) + "\",";
				if ( plotInfo.whenToFit( dataSetI ) == REAL_TIME_FIT
					 || (plotInfo.whenToFit( dataSetI ) == FIT_AT_END
						  && pictureNumber == input->picsPerVariation) )
				{
					std::string fitNum = str( groupNum* dataSetI + groupI );
					std::string plotString = "fit" + str( groupNum * dataSetI + groupI ) + "= ";
					switch ( plotInfo.getFitOption( dataSetI ) )
					{
					case GAUSSIAN_FIT:
					{
						plotString += "sprintf(\"%.3f * exp{/Symbol \\\\173}-(x - %.3f)^2 / (2 * %.3f){/Symbol \\\\175}\", A" + fitNum + ", B" + fitNum + ", C"
							+ fitNum + ")\n";
						break;
					}
					case LORENTZIAN_FIT:
					{

						plotString += "sprintf(\"(%.3f / (2 * Pi)) / ((x - %.3f)^2 + ( %.3f / 2)^2)\", A" + fitNum + ", B" + fitNum
							+ ", A" + fitNum + ")\n";
						break;
					}
					case SINE_FIT:
					{
						plotString += "sprintf(\"%.3f * sin{/Symbol \\\\173}%.3f * x + %.3f{/Symbol \\\\175} * exp{/Symbol \\\\173} - %.3f * x {/Symbol \\\\175}\", A" + fitNum + ", B" + fitNum
							+ ", C" + fitNum + ", D" + fitNum + ")\n";
						break;
					}
					default:
					{
						errBox( "Coding Error: Bad Fit option!" );
					}
					}
					input->plotter->send( plotString );
					UINT lineTypeNumber = dataSetI % GNUPLOT_LINETYPES.size( );
					UINT colorSpacing = 256 / finAvgs[dataSetI].size( );
					std::string colorText = "\" lt rgb \"#" + GIST_RAINBOW[colorSpacing * groupI] + "\"";
					gnuplotPlotCmd += " f" + fitNum + "(x) title fit" + str( groupI ) + " " + colorText
						+ " " + GNUPLOT_LINETYPES[lineTypeNumber] + ",";
				}
			}
		}
		for ( auto dataSetI : range( plotInfo.getDataSetNumber( ) ) )
		{
			for ( auto groupI : range( groupNum ) )
			{
				if ( finData[dataSetI][groupI].size( ) >= input->numberOfRunsToAverage )
				{
					UINT colorSpacing = 256 / finAvgs[dataSetI].size( );
					std::string colorText = "\" lt rgb \"#" + GIST_RAINBOW[colorSpacing * groupI] + "\"";
					gnuplotPlotCmd += " '-' using 1:2 " + colorText + " with lines title \"G"
						+ str( groupI + 1 ) + " " + plotInfo.getLegendText( dataSetI ) + "\",";
				}
			}
		}
	}
	else // average each variations
	{
		for ( auto dataSetI : range( plotInfo.getDataSetNumber( ) ) )
		{
			for ( auto groupI : range( finAvgs[dataSetI].size( ) ) )
			{
				// handle color stuffs, should prob make this a function
				std::stringstream hexStream;
				hexStream << std::hex << int( (1 - 1.0 / sqrt( finAvgs[dataSetI].size( ) )) * 255 );
				std::string alpha = hexStream.str( );
				if ( alpha.size( ) < 2 )
				{
					// This shouldn't happen...
					alpha = "00";
				}
				else
				{
					alpha = alpha.substr( alpha.size( ) - 2 );
				}
				UINT colorSpacing = 256 / finAvgs[dataSetI].size( );
				UINT markerNumber = dataSetI % GNUPLOT_MARKERS.size( );
				std::string colorText = "\" lt rgb \"#" + alpha + GIST_RAINBOW[colorSpacing * groupI] + "\"";
				gnuplotPlotCmd += " '-' using 1:2:3 with yerrorbars title \"G" + str( groupI + 1 ) + " "
					+ plotInfo.getLegendText( dataSetI ) + colorText + " "
					+ GNUPLOT_MARKERS[markerNumber] + " pointsize 0.5,";

				if ( plotInfo.whenToFit( dataSetI ) == REAL_TIME_FIT
					 || (plotInfo.whenToFit( dataSetI ) == FIT_AT_END && pictureNumber == input->picsPerVariation) )
				{
					std::string fitNum = str( groupNum * dataSetI + groupI );
					std::string plotString = "fit" + fitNum + "= ";
					switch ( plotInfo.getFitOption( dataSetI ) )
					{
						case GAUSSIAN_FIT:
						{
							plotString += "sprintf(\"%.3f * exp{/Symbol \\\\173}-(x - %.3f)^2 / (2 * %.3f){/Symbol \\\\175}\", A" + fitNum + ", B" + fitNum + ", C" + fitNum + ")\n";
							break;
						}
						case LORENTZIAN_FIT:
						{
							plotString += "sprintf(\"(%.3f / (2 * Pi)) / ((x - %.3f)^2 + (%.3f / 2)^2)\", A" + fitNum + ", B" + fitNum + ", A" + fitNum + ")\n";
							break;
						}
						case SINE_FIT:
						{
							plotString += "sprintf(\"%.3f * sin{/Symbol \\\\173}%.3f * x + %.3f{/Symbol \\\\175} * exp{/Symbol \\\\173} - %.3f * x {/Symbol \\\\175}\", A" + fitNum + ", B" + fitNum + ", C" + fitNum + ", D" + fitNum + ")\n";
							break;
						}
						default:
						{
							errBox( "Coding Error: Bad Fit option!" );
						}
					}
					input->plotter->send( plotString );
					UINT lineTypeNumber = dataSetI % GNUPLOT_LINETYPES.size( );
					UINT colorSpacing = 256 / finAvgs[dataSetI].size( );
					std::string colorText = "\" lt rgb \"#" + GIST_RAINBOW[colorSpacing * groupI] + "\"";
					gnuplotPlotCmd += " f" + fitNum + "(x) title fit" + fitNum + " " + colorText + " "
						+ GNUPLOT_LINETYPES[dataSetI] + ",";
				}
			}
			// add the average line
			gnuplotPlotCmd += " '-' using 1:2:3 with yerrorbars title \"Average\" lt rgb \"#FFFFFF\" pt 5 pointsize 0.5,";
		}
	}
	std::string error;
	input->plotter->send( gnuplotPlotCmd );
	/// SEND DATA
	if ( plotInfo.getXAxis( ) == "Running Average" )
	{
		for ( auto dataSetI : range( plotInfo.getDataSetNumber( ) ) )
		{
			for ( auto groupI : range( groupNum ) )
			{
				input->plotter->sendData( finAvgs[dataSetI][groupI], finData[dataSetI][groupI] );
			}
			for ( auto groupI : range( groupNum ) )
			{
				if ( finData[dataSetI][groupI].size( ) >= input->numberOfRunsToAverage )
				{
					input->plotter->sendData( finX[dataSetI][groupI], finAvgs[dataSetI][groupI] );
				}
			}
		}
	}
	else
	{
		for ( auto dataSetI : range( plotInfo.getDataSetNumber( ) ) )
		{
			for ( auto groupI : range( finAvgs[dataSetI].size( ) ) )
			{
				input->plotter->sendData( finX[dataSetI][groupI], finAvgs[dataSetI][groupI], finErrs[dataSetI][groupI] );
			}
			input->plotter->sendData( avgX[dataSetI], avgAvgs[dataSetI], avgErrs[dataSetI] );
		}
	}
}

// using vector = std::vector
void DataAnalysisControl::handlePlotHist( realTimePlotterInput* input, PlottingInfo plotInfo, UINT plotNumber,
										  vector<vector<long>> countData,
										  vector<vector<std::deque<double>>>& finData,
										  vector<vector<bool>>pscSatisfied, int plotNumberCount,
										  vector<vector<std::map<int, std::pair<int, ULONG>>>>& histData)
{
	/// options are fundamentally different for histograms.
	UINT groupNum = input->atomGridInfo.width * input->atomGridInfo.height;
	// load pixel counts into data array pixelData
	for ( auto dataSetI : range(plotInfo.getDataSetNumber( )) )
	{
		for ( auto groupI : range( groupNum ) )
		{
			if ( pscSatisfied[dataSetI][groupI] == false )
			{
				continue;
			}
			double binWidth = plotInfo.getDataSetHistBinWidth( dataSetI );
			int binNum = std::round(double(countData[groupI].back( )) / binWidth);
			if ( histData[dataSetI][groupI].find( binNum ) == histData[dataSetI][groupI].end( ) )
			{
				histData[dataSetI][groupI][binNum] = { binNum * binWidth, 1 };
			}
			else
			{
				histData[dataSetI][groupI][binNum].second++;
			}
		}
	}
	// Core data structures have been updated. return if not time for an update yet.
	if ( plotNumberCount % input->plottingFrequency != 0 )
	{
		return;
	}
	// is redundant to re-set these things each re-plot, but I'm not sure there's a better way.
	input->plotter->send( "set terminal wxt " + str( plotNumber ) + " title \"" + plotInfo.getTitle( ) 
						  + "\" noraise background rgb 'black'" );
	input->plotter->send( "set title \"" + plotInfo.getTitle( ) + "\" tc rgb '#FFFFFF'" );
	input->plotter->send( "set xlabel \"Counts\" tc rgb '#FFFFFF'" );
	input->plotter->send( "set ylabel \"" + plotInfo.getYLabel( ) + "\" tc rgb '#FFFFFF'" );
	input->plotter->send( "set border lc rgb '#FFFFFF'" );
	input->plotter->send( "set key tc rgb '#FFFFFF' outside" );
	input->plotter->send( "set title \"" + plotInfo.getTitle( ) + "\"" );
	input->plotter->send( "set format y \"%.1f\"" );
	input->plotter->send( "set style fill" );
	input->plotter->send( "set autoscale x" );
	input->plotter->send( "set yrange [0:*]" );
	input->plotter->send( "set xlabel \"Count #\"" );
	input->plotter->send( "set ylabel \"Occurrences\"" );
	double spaceFactor = 1;
	// this is fixed. in principle the bin width, set later, can vary, which at the moment would result in some odd 
	// plots.
	double boxWidth = spaceFactor * plotInfo.getDataSetHistBinWidth( 0 );
	input->plotter->send( "set boxwidth " + str( boxWidth ) );
	input->plotter->send( "set style fill solid 1" );
	// leave 0.2 pixels worth of space in between the bins.
	std::string gnuCommand = "plot";
	int totalDataSetNum = plotInfo.getDataSetNumber( );
	for ( auto dataSetI : range(plotInfo.getDataSetNumber( )) )
	{
		for ( auto groupI : range( groupNum ) )
		{
			std::stringstream hexStream;
			hexStream << std::hex << int( (1 - 1.0 / sqrt( histData[dataSetI].size( ) )) * 255 );
			std::string alpha = hexStream.str( );
			alpha = ((alpha.size( ) < 2) ? alpha = "00" : alpha = alpha.substr( alpha.size( ) - 2 ));
			UINT markerNumber = dataSetI % GNUPLOT_MARKERS.size( );
			// long command that makes hist correctly.
			UINT colorSpacing = 256 / histData[dataSetI].size( );
			std::string colorText = "\" lt rgb \"#" + alpha + GIST_RAINBOW[colorSpacing * groupI] + "\"";
			std::string newHistCmd = (" '-' with boxes title \"G " + str( groupI + 1 ) + " " 
				+ plotInfo.getLegendText( dataSetI ) + " " + colorText + " " + GNUPLOT_MARKERS[markerNumber] + ",");
			gnuCommand += newHistCmd;
		}
	}

	input->plotter->send( gnuCommand );
	for ( auto dataSetI : range(plotInfo.getDataSetNumber( )) )
	{
		for ( auto groupI : range( input->atomGridInfo.width * input->atomGridInfo.height ) )
		{
			vector<int> locations;
			vector<ULONG> values;
			for ( const auto& elem : histData[dataSetI][groupI] )
			{
				locations.push_back( elem.second.first );
				values.push_back( elem.second.second );
			}
			input->plotter->sendData( locations, values );
		}
	}
}


atomGrid DataAnalysisControl::getAtomGrid( )
{
	// update the current grid by load stuff from edits before returning.
	if ( currentGrid.topLeftCorner == coordinate( 0, 0 ) )
	{
		// don't try updating, it's not used.
		return currentGrid;
	}
	CString txt;
	try
	{
		gridSpacing.GetWindowTextA( txt );
		currentGrid.pixelSpacing = std::stol( str( txt ) );
		gridWidth.GetWindowTextA( txt );
		currentGrid.width = std::stol( str( txt ) );
		gridHeight.GetWindowTextA( txt );
		currentGrid.height = std::stol( str( txt ) );
	}
	catch ( std::invalid_argument& )
	{
		thrower( "ERROR: Please make sure all atom grid parameters are convertible to integers!" );
	}

	return currentGrid;
}


void DataAnalysisControl::fillPlotThreadInput(realTimePlotterInput* input)
{
	vector<tinyPlotInfo> usedPlots;
	input->plotInfo.clear();

	for (auto plt : allPlots)
	{
		if (plt.isActive)
		{
			input->plotInfo.push_back(plt);
		}
	}

	input->analysisLocations = getAnalysisLocs();
	input->atomGridInfo = getAtomGrid( );

	input->plottingFrequency = updateFrequency;

	// as I fill the input, also check this, which is necessary info for plotting.
	threadNeedsCounts = false;
	for (auto plt : input->plotInfo)
	{
		PlottingInfo info(PLOT_FILES_SAVE_LOCATION + "\\" + plt.name + "." + PLOTTING_EXTENSION);
		if (info.getPlotType() != "Atoms")
		{
			threadNeedsCounts = true;
		}
	}
	
	input->needsCounts = threadNeedsCounts;
}


void DataAnalysisControl::rearrange(std::string cameraMode, std::string trigMode, int width, int height, fontMap fonts)
{
	updateFrequencyLabel1.rearrange(cameraMode, trigMode, width, height, fonts);
	updateFrequencyLabel2.rearrange(cameraMode, trigMode, width, height, fonts);
	updateFrequencyEdit.rearrange(cameraMode, trigMode, width, height, fonts);
	header.rearrange(cameraMode, trigMode, width, height, fonts);
	plotListview.rearrange(cameraMode, trigMode, width, height, fonts);
	currentDataSetNumberDisp.rearrange( cameraMode, trigMode, width, height, fonts );
	currentDataSetNumberText.rearrange( cameraMode, trigMode, width, height, fonts );
	manualSetAnalysisLocsButton.rearrange( cameraMode, trigMode, width, height, fonts );

	gridHeader.rearrange( cameraMode, trigMode, width, height, fonts );
	setGridCorner.rearrange( cameraMode, trigMode, width, height, fonts );
	gridSpacingText.rearrange( cameraMode, trigMode, width, height, fonts );
	gridSpacing.rearrange( cameraMode, trigMode, width, height, fonts );
	gridWidthText.rearrange( cameraMode, trigMode, width, height, fonts );
	gridWidth.rearrange( cameraMode, trigMode, width, height, fonts );
	gridHeightText.rearrange( cameraMode, trigMode, width, height, fonts );
	gridHeight.rearrange( cameraMode, trigMode, width, height, fonts );
}


vector<std::string> DataAnalysisControl::getActivePlotList()
{
	vector<std::string> list;
	for ( auto plot : allPlots )
	{
		if ( plot.isActive ) 
		{
			list.push_back( plot.name );
		}
	}
	return list;
}




bool DataAnalysisControl::getLocationSettingStatus()
{
	return currentlySettingAnalysisLocations;
}


void DataAnalysisControl::updateDataSetNumberEdit( int number )
{
	if ( number > 0 )
	{
		currentDataSetNumberDisp.SetWindowTextA( cstr( number ) );
	}
	else
	{
		currentDataSetNumberDisp.SetWindowTextA( "None" );
	}
}

void DataAnalysisControl::analyze( std::string date, long runNumber, long accumulations, 
								   EmbeddedPythonHandler* pyHandler, Communicator* comm )
{
	// Note: python is initialized in the constructor for the data handler object. 
	// Get information to send to the python script from inputParam
	//pyHandler->runDataAnalysis( date, runNumber, accumulations, atomLocations );
}


bool DataAnalysisControl::buttonClicked()
{
	return (manualSetAnalysisLocsButton.GetCheck() || setGridCorner.GetCheck());

}


void DataAnalysisControl::onCornerButtonPushed( )
{
	if ( setGridCorner.GetCheck( ) )
	{
		// if pressed currently, then upress it.
		setGridCorner.SetCheck( 0 );
		setGridCorner.SetWindowTextA( "Set Grid Top-Left Corner" );
		currentlySettingGridCorner = false;
	}
	else
	{
		// else press it.
		atomLocations.clear( );
		currentGrid.topLeftCorner = { 0,0 };
		setGridCorner.SetCheck( 1 );
		setGridCorner.SetWindowTextA( "Right-Click Top-Left Corner of New Grid Location" );
		currentlySettingGridCorner = true;
	}
}


// handles the pressing of the analysis points button.
// TODO: handle different cases where single locations or pairs of locations are being analyzed. 
void DataAnalysisControl::onManualButtonPushed()
{	
	if ( manualSetAnalysisLocsButton.GetCheck() )
	{
		// if pressed currently, then upress it.
		manualSetAnalysisLocsButton.SetCheck(0);
		manualSetAnalysisLocsButton.SetWindowTextA( "Set Analysis Points" );
		currentlySettingAnalysisLocations = false;
	}
	else
	{
		// else press it.
		atomLocations.clear();
		currentGrid.topLeftCorner = { 0,0 };
		manualSetAnalysisLocsButton.SetCheck( 1 );
		manualSetAnalysisLocsButton.SetWindowTextA( "Right-Click Relevant Points and Reclick" );
		currentlySettingAnalysisLocations = true;

	}
}


void DataAnalysisControl::handlePictureClick( coordinate location )
{
	if ( setGridCorner.GetCheck( ) )
	{
		currentGrid.topLeftCorner = location;
		//setGridCorner.SetCheck( false );
		onCornerButtonPushed( );
	}
	else if ( manualSetAnalysisLocsButton.GetCheck( ) )
	{
		bool exists = false;
		for ( UINT locInc = 0; locInc < atomLocations.size( ); locInc++ )
		{
			if ( location.row == atomLocations[locInc].row &&  location.column == atomLocations[locInc].column )
			{
				exists = true;
			}
		}
		if ( !exists )
		{
			atomLocations.push_back( location );
		}
	}
}


vector<coordinate> DataAnalysisControl::getAnalysisLocs()
{
	return atomLocations;
}


void DataAnalysisControl::clearAtomLocations()
{
	atomLocations.clear();
}


void DataAnalysisControl::handleDoubleClick(fontMap* fonts, UINT currentPicsPerRepetition)
{
	POINT cursorPos;
	GetCursorPos(&cursorPos);
	plotListview.ScreenToClient(&cursorPos);
	int subitemIndicator;
	LVHITTESTINFO myItemInfo;
	memset(&myItemInfo, 0, sizeof(LVHITTESTINFO));
	myItemInfo.pt = cursorPos;
	int itemIndicator;
	plotListview.SubItemHitTest(&myItemInfo);
	itemIndicator = myItemInfo.iItem;
	subitemIndicator = myItemInfo.iSubItem;
	if (itemIndicator == -1)
	{
		// user didn't click in an item.
		return;
	}

	LVITEM listViewItem;
	memset(&listViewItem, 0, sizeof(listViewItem));
	listViewItem.mask = LVIF_TEXT;
	listViewItem.cchTextMax = 256;
	listViewItem.iItem = itemIndicator;
	if (itemIndicator == allPlots.size())
	{
		// open plot creator.
		PlotDesignerDialog dlg(fonts, currentPicsPerRepetition);
		dlg.DoModal();
		reloadListView();
		return;
	}
	/// Handle different subitem clicks
	switch (subitemIndicator)
	{
		case 0:
		{
			//... clearly I haven't actually implemented anything here.
			// prompt for a name
			std::string newName;
			if (newName == "")
			{
				// probably canceled.
				break;
			}
			// rename the file 
			// ...
			// update the screen
			listViewItem.iItem = itemIndicator;
			listViewItem.iSubItem = subitemIndicator;
			listViewItem.pszText = (LPSTR)cstr(newName);
			plotListview.SetItem(&listViewItem);
			break;
		}
		case 1:
		{
			// edit existing plot file using the plot designer.
			try
			{
				PlotDesignerDialog dlg(fonts, PLOT_FILES_SAVE_LOCATION + "\\" + allPlots[itemIndicator].name + "." 
										+ PLOTTING_EXTENSION);
				dlg.DoModal();
			}
			catch (Error& err)
			{
				errBox(err.what());
			}
			reloadListView();
			break;
		}
		case 2:
		{
			/// view plot settings.
			try
			{
				infoBox(PlottingInfo::getAllSettingsStringFromFile(PLOT_FILES_SAVE_LOCATION + "\\" 
																	+ allPlots[itemIndicator].name + "." 
																	+ PLOTTING_EXTENSION));
			}
			catch (Error& err)
			{
				errBox(err.what());
			}
			break;
		}
		case 3:
		{
			/// toggle active
			listViewItem.iItem = itemIndicator;
			listViewItem.iSubItem = subitemIndicator;

			if (allPlots[itemIndicator].isActive)
			{
				allPlots[itemIndicator].isActive = false;
				listViewItem.pszText = "NO";
			}
			else
			{
				allPlots[itemIndicator].isActive = true;
				listViewItem.pszText = "YES";
			}
			plotListview.SetItem( &listViewItem );
			break;
		}
	}
}


void DataAnalysisControl::reloadListView()
{
	vector<std::string> names = ProfileSystem::searchForFiles(PLOT_FILES_SAVE_LOCATION, str("*.") + PLOTTING_EXTENSION);
	plotListview.DeleteAllItems();
	allPlots.clear();
	for (auto item : range(names.size()))
	{
		LVITEM listViewItem;
		memset(&listViewItem, 0, sizeof(listViewItem));
		listViewItem.mask = LVIF_TEXT; 
		listViewItem.cchTextMax = 256; 
		listViewItem.iItem = item;
		//listViewItem.pszText = (LPSTR)cstr(names[item]);
		listViewItem.pszText = (LPSTR)names[item].c_str();
		plotListview.InsertItem(&listViewItem);
		listViewItem.iSubItem = 3;
		listViewItem.pszText = "NO";
		plotListview.SetItem(&listViewItem);
		tinyPlotInfo tempInfo;
		tempInfo.name = names[item];
		tempInfo.isActive = false;
		allPlots.push_back(tempInfo);
	}
	// Make First Blank row.
	LVITEM listViewDefaultItem;
	memset(&listViewDefaultItem, 0, sizeof(listViewDefaultItem));
	// Text Style
	listViewDefaultItem.mask = LVIF_TEXT;
	// Max size of test
	listViewDefaultItem.cchTextMax = 256;
	listViewDefaultItem.pszText = "___";
	// choose item  
	listViewDefaultItem.iItem = names.size();
	// Put in first coluom
	listViewDefaultItem.iSubItem = 0;
	plotListview.InsertItem(&listViewDefaultItem);
	// Add SubItems in a loop
	for (int itemInc = 1; itemInc <= 3; itemInc++)
	{
		listViewDefaultItem.iSubItem = itemInc;
		// Enter text to SubItems
		plotListview.SetItem(&listViewDefaultItem);
	}
}


void DataAnalysisControl::handleRClick()
{
	// delete...
	/// get the item and subitem
	POINT cursorPos;
	GetCursorPos(&cursorPos);
	plotListview.ScreenToClient(&cursorPos);
	int subitemIndicator = plotListview.HitTest(cursorPos);
	LVHITTESTINFO myItemInfo;
	memset(&myItemInfo, 0, sizeof(LVHITTESTINFO));
	myItemInfo.pt = cursorPos;
	int itemIndicator = plotListview.SubItemHitTest(&myItemInfo);
	if (itemIndicator == -1 || itemIndicator == allPlots.size())
	{
		// user didn't click in a deletable item.
		return;
	}
	int answer = promptBox("Delete Plot " + allPlots[itemIndicator].name + "?", MB_YESNO);
	if (answer == IDYES)
	{
		int result = DeleteFile(cstr(PLOT_FILES_SAVE_LOCATION + "\\" + allPlots[itemIndicator].name + "." 
									  + PLOTTING_EXTENSION));
		if (!result)
		{
			errBox("Failed to delete script file! Error code: " + str(GetLastError()));
			return;
		}
		plotListview.DeleteItem(itemIndicator);
		allPlots.erase(allPlots.begin() + itemIndicator);
	}
}

