// created by Mark O. Brown
#include "stdafx.h"
#include "PlottingInfo.h"
#include "ConfigurationSystems/ConfigSystem.h"
#include <sstream>
#include <boost/lexical_cast.hpp>

PlottingInfo::PlottingInfo(unsigned picNumber){
	// initialize things.
	generalPlotType = "Atoms";
	title = "";
	yLabel = "";
	fileName = "";
	dataSets.clear();
	dataSets.resize(1);
	dataSets[0].resetPixelNumber( 1 ); 
	dataSets[0].resetPictureNumber(picNumber);	
	currentPixelNumber = 1;
	currentConditionNumber = 1;
	numberOfPictures = picNumber;
	xAxis = "";
}


PlottingInfo::PlottingInfo(std::string fileName){
	loadPlottingInfoFromFile(fileName);
}


analysisGroupLocation& PlottingInfo::groupInfo( unsigned pixelNumber, unsigned pixelSet ){
	if ( pixelNumber > analysisGroups.size( ) ){
		thrower ( "ERROR: pixel number out of range in analysis group access!" );
	}
	if ( pixelSet > analysisGroups[pixelNumber].size( ) ){
		thrower ( "ERROR: pixel set out of range in analysis group access!" );
	}
	return analysisGroups[pixelNumber][pixelSet];
}

analysisGroupLocation  PlottingInfo::groupInfo( unsigned pixelNumber, unsigned pixelSet ) const{
	if ( pixelNumber > analysisGroups.size( ) ){
		thrower ( "ERROR: pixel number out of range in analysis group access!" );
	}
	if ( pixelSet > analysisGroups[pixelNumber].size( ) ){
		thrower ( "ERROR: pixel set out of range in analysis group access!" );
	}
	return analysisGroups[pixelNumber][pixelSet];
}


void PlottingInfo::changeTitle(std::string newTitle){
	title = newTitle;
}

std::string PlottingInfo::getPrcSettingsString(){
	std::string allConditionsString = "\r\nAll Result Conditions:\r\n=========================\r\n";
	allConditionsString += "Data Set #, picture #, pixel #:\r\n";
	for (unsigned dataSetInc = 0; dataSetInc < getDataSetNumber(); dataSetInc++){
		for (unsigned pictureInc = 0; pictureInc < getPicNumber(); pictureInc++){
			for (unsigned pixelInc = 0; pixelInc < getPixelNumber(); pixelInc++){
				int currentValue = getResultCondition(dataSetInc, pixelInc, pictureInc);
				allConditionsString += str( dataSetInc + 1 ) + ", " + str( pictureInc + 1 ) + ", " + str( pixelInc + 1 )
					+ " = ";
				if (currentValue == 1){
					allConditionsString += " Atom Present";
				}
				else if (currentValue == -1){
					allConditionsString += " Atom Not Present";
				}
				else{
					allConditionsString += " No Condition";
				}
				allConditionsString += "\r\n";
			}
		}
		allConditionsString += "=====\r\n";
	}
	return allConditionsString;
}


std::string PlottingInfo::getPscSettingsString(){
	std::string allConditionsString = "\r\nAll Current Post-Selection Conditions:\r\n=========================\r\n";
	allConditionsString += "condition #, Data Set #, picture #, pixel #:\r\n";
	for (unsigned conditionInc = 0; conditionInc < getConditionNumber(); conditionInc++){
		for (unsigned dataSetInc = 0; dataSetInc < getDataSetNumber(); dataSetInc++){
			for (unsigned pictureInc = 0; pictureInc < getPicNumber(); pictureInc++){
				for (unsigned pixelInc = 0; pixelInc < getPixelNumber(); pixelInc++){
					allConditionsString += str( conditionInc + 1 ) + ", " + str( dataSetInc + 1 ) + ", "
						+ str( pictureInc + 1 ) + ", " + str( pixelInc + 1 ) + " = ";
					int currentValue = getPostSelectionCondition(dataSetInc, conditionInc, pixelInc, pictureInc);
					if (currentValue == 1){
						allConditionsString += " Atom Present";
					}
					else if (currentValue == -1){
						allConditionsString += " Atom Not Present";
					}
					else{
						allConditionsString += " No Condition";
					}
					allConditionsString += "\r\n";
				}
			}
			allConditionsString += "=====\r\n";
		}
	}
	return allConditionsString;
}


void PlottingInfo::changeYLabel(std::string newYLabel){
	yLabel = newYLabel;
}


void PlottingInfo::changeFileName(std::string newFileName) {
	fileName = newFileName;
}


void PlottingInfo::changeGeneralPlotType(std::string newPlotType){
	generalPlotType = newPlotType;
}


void PlottingInfo::changeXAxis(std::string newXAxis){
	xAxis = newXAxis;
}


void PlottingInfo::addGroup(){
	for (unsigned pixelInc = 0; pixelInc < currentPixelNumber; pixelInc++){
		analysisGroups[pixelInc].resize(analysisGroups[pixelInc].size() + 1);
	}
}


void PlottingInfo::setGroupLocation( unsigned pixel, unsigned analysisSet, unsigned row, unsigned column){
	groupInfo( pixel, analysisSet ).row = row;
	groupInfo( pixel, analysisSet ).col = column;
}


void PlottingInfo::addPixel(){
	// change all the data set structures.
	currentPixelNumber++;
	analysisGroups.resize(analysisGroups.size() + 1);
	analysisGroups[analysisGroups.size() - 1].resize(analysisGroups[0].size());
	for (auto& dset : dataSets){
		dset.addPixel(numberOfPictures);
	}
}

void PlottingInfo::removePixel(){
	// make sure there is a pixel to remove.
	if (currentPixelNumber < 2){
		thrower ("ERROR: Something tried to remove the last pixel!");
	}
	// change all the data set structures.
	currentPixelNumber--;
	for (auto& dset : dataSets)	{
		dset.removePixel();
	}
}



// Change all structures that depend on the number of data sets.
void PlottingInfo::addDataSet(){	
	// initialize new data set with 1 condition and the right number of pixels and pictures.
	dataSets.resize(dataSets.size() + 1);
	dataSets[dataSets.size() - 1].initialize(currentConditionNumber, currentPixelNumber, numberOfPictures);
}


void PlottingInfo::removeDataSet(){
	// make sure there is a data set to remove.
	if (dataSets.size() < 2){
		thrower ("ERROR: Something tried to remove the last data set");
	}	
	dataSets.resize(dataSets.size() - 1);
}

// change all structures that depend on the number of pictures. 
void PlottingInfo::addPicture(){
	numberOfPictures++;
	for (auto& dset : dataSets){
		dset.addPicture();
	}
}


void PlottingInfo::removePicture(){
	if (numberOfPictures < 2){
		thrower ("ERROR: Something tried to remove the last picture!");
	}
	numberOfPictures--;
	for (auto& dset : dataSets){
		dset.removePicture();
	}
}


void PlottingInfo::setPostSelCondition( unsigned dataSetNumber, unsigned conditionNumber, unsigned pixel, unsigned picture,
											  unsigned postSelectionCondition){
	dataSets[dataSetNumber].setPostSelectionCondition(conditionNumber, pixel, picture, postSelectionCondition);
}

void PlottingInfo::setResultCondition( unsigned dataSetNumber, unsigned pixel, unsigned picture, 
										unsigned positiveResultCondition){
	dataSets[dataSetNumber].setResultCondition(pixel, picture, positiveResultCondition);
}

// stores the info in the row and column arguments
void PlottingInfo::getPixelLocation( unsigned pixel, unsigned analysisSet, unsigned& row, unsigned& column){
	row = groupInfo(pixel, analysisSet).row;
	column = groupInfo( pixel, analysisSet ).col;
}

void PlottingInfo::removeAnalysisSet(){
	// always at least one pixel...

	// make sure tat there is an analysis set to remove.
	for (unsigned pixelInc = 0; pixelInc < currentPixelNumber; pixelInc++){
		if (analysisGroups[pixelInc].size() < 2){
			thrower ("ERROR: Something tried to remove the last analysis group!");
		}
		analysisGroups[pixelInc].resize(analysisGroups[pixelInc].size() - 1);
	}
}

unsigned PlottingInfo::getPicNumberFromFile(std::string fileAddress){
	// load the file completely then readbtn the picture number.
	PlottingInfo temp(fileAddress);
	return temp.getPicNumber();
}


unsigned PlottingInfo::getResultCondition( unsigned dataSetNumber, unsigned pixel, unsigned picture){
	return dataSets[dataSetNumber].getPositiveResultCondition(pixel, picture);
}

unsigned PlottingInfo::getPostSelectionCondition( unsigned dataSetNumber, unsigned conditionNumber, unsigned pixel,		
												  unsigned picture){
	return dataSets[dataSetNumber].getPostSelectionCondition(conditionNumber, pixel, picture);
}

void PlottingInfo::addPostSelectionCondition(){
	for (auto& dset : dataSets ){
		// the currentPixelNumber is used as a vector index, the number of pictures is used as a resize size.
		dset.addPostSelectionCondition(currentPixelNumber, numberOfPictures);
	}
	currentConditionNumber++;
}


void PlottingInfo::removePostSelectionCondition(){
	for ( auto& dset : dataSets ){
		dset.removePostSelectionCondition();
	}
	currentConditionNumber--;
}


std::string PlottingInfo::getPlotType(){
	return generalPlotType;
}

std::string PlottingInfo::getXAxis(){
	return xAxis;
}

void PlottingInfo::setDataCountsLocation( unsigned dataSet, unsigned pixel, unsigned picture){
	if (dataSet < dataSets.size() && !(dataSet < 0)){
		dataSets[dataSet].setDataCountsLocation(currentPixelNumber, numberOfPictures, pixel, picture);
	}
	else{
		thrower ("ERROR: tried to set data counts location for data set that hasn't been assigned yet. dataSet = " 
				+ str(dataSet));
	}
}


void PlottingInfo::setPlotData( unsigned dataSet, bool plotData){
	if (dataSet >= dataSets.size() || dataSet < 0){
		thrower ( "ERROR: tried to set \"Plot this data\" value for data set that hasn't been assigned. dataSet = "
			     + str(dataSet));
	}
	dataSets[dataSet].setPlotThisData(plotData);
}


void PlottingInfo::setDataSetHistBinWidth( unsigned dataSet, unsigned width ){
	if ( dataSet >= dataSets.size( ) || dataSet < 0 ){
		thrower ( "ERROR: tried to set Histogram Bin Width value for data set that hasn't been assigned. dataSet = "
				 + str( dataSet ) );
	}
	dataSets[dataSet].setHistBinWidth( width );
}


unsigned PlottingInfo::getDataSetHistBinWidth( unsigned dataSet ){
	if ( dataSet >= dataSets.size( ) || dataSet < 0 ){
		thrower ( "ERROR: tried to get histogram bin width value for data set that hasn't been assigned. dataSet = "
				 + str( dataSet ) );
	}
	return dataSets[dataSet].getHistBinWidth( );
}

bool PlottingInfo::getPlotThisDataValue( unsigned dataSet){
	if (dataSet >= dataSets.size() || dataSet < 0){
		thrower ("ERROR: tried to get \"Plot this data\" value for data set that hasn't been assigned. dataSet = "
				+ str(dataSet));
	}
	return dataSets[dataSet].getPlotThisDataValue();
}


void PlottingInfo::getDataCountsLocation( unsigned dataSet, unsigned& pixel, unsigned& picture){
	dataSets[dataSet].getDataCountsLocation(pixel, picture);
}


void PlottingInfo::changeLegendText( unsigned dataSet, std::string newLegend){
	if (dataSet >= dataSets.size() || dataSet < 0){
		thrower ( "ERROR: attempted to set dataset legend for data set that hadn't been allocated. dataset = " 
			     + str(dataSet));
	}
	dataSets[dataSet].changeLegendText(newLegend);
}


std::string PlottingInfo::getLegendText( unsigned dataSet){
	if (dataSet >= dataSets.size())	{
		thrower ( "ERROR: attempted to get dataset legend for data set that hadn't been allocated. dataset = " 
			     + str(dataSet));
	}
	return dataSets[dataSet].getLegendText();
}

std::string PlottingInfo::getAllSettingsString(){
	std::string message = "All Plotting Parameters (" + fileName + ")\r\n=======================================\r\n\r\n";
	message += "Plot Title: " + title + "\r\n";
	message += "Plot Type: " + generalPlotType + "\r\n";
	message += "Y Label: " + yLabel + "\r\n";
	message += "X Axis Type: " + xAxis + "\r\n";
	message += "File Name: " + fileName + "\r\n";
	message += getPrcSettingsString();
	message += getPscSettingsString();
	return message;
}


std::string PlottingInfo::getAllSettingsStringFromFile(std::string fileAddress){
	PlottingInfo temp(fileAddress);
	return temp.getAllSettingsString();
}

void PlottingInfo::savePlotInfo(){
	std::string completeAddress = PLOT_FILES_SAVE_LOCATION + "\\" + fileName + ".plot";
	std::fstream saveFile(completeAddress, std::fstream::out);
	if (!saveFile.is_open()){
		thrower ("Couldn't open file at + " + PLOT_FILES_SAVE_LOCATION + "\\" + fileName + ".plot!");
	}
	std::string message;
	message += "Version: " + str( versionMajor ) + "." + str( versionMinor ) + "\n";
	message += title + "\n";
	message += generalPlotType + "\n";
	message += yLabel + "\n";
	message += xAxis + "\n";
	message += fileName + "\n";

	message += str(dataSets.size()) + "\n";
	message += str(currentConditionNumber) + "\n";
	message += str(currentPixelNumber) + "\n";
	message += str(numberOfPictures) + "\n";

	message += "POSITIVE_RESULT_BEGIN\n";
	for (auto& dset : dataSets)	{
		message += "DATA_SET_BEGIN\n";
		for (unsigned pictureInc = 0; pictureInc < numberOfPictures; pictureInc++){
			message += "PICTURE_BEGIN\n";
			for (unsigned pixelInc = 0; pixelInc < currentPixelNumber; pixelInc++){
				message += str(dset.getPositiveResultCondition(pixelInc, pictureInc)) + "\n";
			}
			message += "PICTURE_END\n";
		}
		message += "DATA_SET_END\n";
	}
	message += "POSITIVE_RESULT_END\n";
	message += "POST_SELECTION_BEGIN\n";
	for (unsigned conditionInc = 0; conditionInc < currentConditionNumber; conditionInc++){
		message += "CONDITION_BEGIN\n";
		for ( auto& dset : dataSets ){
			message += "DATA_SET_BEGIN\n";
			for (unsigned pictureInc = 0; pictureInc < numberOfPictures; pictureInc++){
				message += "PICTURE_BEGIN\n";
				for (unsigned pixelInc = 0; pixelInc < currentPixelNumber; pixelInc++){
					message += str(dset.getPostSelectionCondition(conditionInc, pixelInc, pictureInc)) + "\n";
				}
				message += "PICTURE_END\n";
			}
			message += "DATA_SET_END\n";
		}
		message += "CONDITION_END\n";
	}
	message += "POST_SELECTION_END\n";

	// legends
	message += "LEGENDS_BEGIN\n";
	for (unsigned dataSetInc = 0; dataSetInc < dataSets.size(); dataSetInc++){
		message += getLegendText(dataSetInc) + "\n";
	}
	message += "LEGENDS_END\n";

	// data count locations
	message += "PLOT_COUNTS_LOCATIONS_BEGIN\n";
	if ( getPlotType( ) != "Atoms" ){
		for ( unsigned dataSetInc = 0; dataSetInc < dataSets.size( ); dataSetInc++ ){
			unsigned pixel, picture;
			getDataCountsLocation( dataSetInc, pixel, picture );
			message += str( pixel ) + " " + str( picture ) + "\n";
		}
	}
	message += "PLOT_COUNTS_LOCATIONS_END\n";
	
	message += "FITTING_OPTIONS_BEGIN\n";
	for ( auto& dset : dataSets ){
		message += str(dset.getFitType()) + " " + str(dset.getWhenToFit()) + "\n";
	}
	message += "FITTING_OPTIONS_END\n";	
	message += "HIST_OPTIONS_BEGIN\n";
	message += str( dataSets.size()) + "\n";
	for ( auto& dset : dataSets ){
		message += str( dset.getHistBinWidth() )+ "\n";
	}
	message += "HIST_OPTIONS_END\n";

	saveFile << message;
}


void PlottingInfo::loadPlottingInfoFromFile(std::string fileLocation){
	ConfigStream plotStream (fileLocation, true);
	// this string will hold headers in this file temporarily and check to make sure they are correct.
	std::string versionStr;
	plotStream >> versionStr >> versionStr;
	double version;
	try{
		version = boost::lexical_cast<double>( versionStr );
	}
	catch ( boost::bad_lexical_cast& ){
		throwNested ( "ERROR: Version string failed to convert to double while opening configuration!" );
	}
	int versionMajor = int( version );
	int versionMinor;
	double tempDouble = std::round( (version - versionMajor) * 1e6 );
	while ( true ){
		double res = fabs( double( double( tempDouble / 10 ) - int( tempDouble / 10 ) ) );
		if ( res < 1e-6 ){
			break;
		}
		tempDouble /= 10;
	}
	versionMinor = int(std::round( tempDouble ));
	plotStream.get( );
	getline(plotStream, title);
	getline(plotStream, generalPlotType);
	getline(plotStream, yLabel);
	getline(plotStream, xAxis);
	getline(plotStream, fileName);

	// Data Set Number
	std::string testString;
	getline(plotStream, testString);
	try{
		int tempDataSetNumber = boost::lexical_cast<int>(testString);
		resetDataSetNumber(tempDataSetNumber);
	}
	catch ( boost::bad_lexical_cast&){
		throwNested ("ERROR: Couldn't read data set number from file. The data set string was " + testString);
	}
	// Condition Number
	getline(plotStream, testString);
	try	{
		currentConditionNumber = boost::lexical_cast<int>(testString);
		resetConditionNumber(currentConditionNumber);
	}
	catch ( boost::bad_lexical_cast&){
		throwNested ("ERROR: Couldn't read post-selection number from file. The post-selection string was " + testString);
	}
	// Pixel Number
	getline(plotStream, testString);
	// Picture Number
	std::string testString2;
	getline(plotStream, testString2);
	try	{
		numberOfPictures = boost::lexical_cast<int>(testString2);
		resetPictureNumber(numberOfPictures);
	}
	catch ( boost::bad_lexical_cast&){
		throwNested ("ERROR: Couldn't read Picture number from file. The picture string was " + testString);
	}
	try	{
		currentPixelNumber = boost::lexical_cast<int>(testString);
		resetPixelNumber(currentPixelNumber);
	}
	catch ( boost::bad_lexical_cast&){
		throwNested ("ERROR: Couldn't read pixel number from file. The pixel string was " + testString);
	}
	/// Analys pixels
	ConfigSystem::checkDelimiterLine(plotStream, str("POSITIVE_RESULT_BEGIN"));
	unsigned dataSetCount = 0;
	// data set loop
	while (!ConfigSystem::checkDelimiterLine(plotStream, "DATA_SET_BEGIN", str("POSITIVE_RESULT_END") ) ){
		// picture loop
		int pictureCount = 0;
		while ( !ConfigSystem::checkDelimiterLine(plotStream, "PICTURE_BEGIN", str("DATA_SET_END") ) ){			
			// pixel loop
			int pixelCount = 0;
			while (true){
				plotStream >> testString;
				if (testString == "PICTURE_END"){
					break;
				}
				int tempTruthCondition;
				try	{
					tempTruthCondition = boost::lexical_cast<int>(testString);
				}
				catch ( boost::bad_lexical_cast&){
					throwNested ("Truth condition failed to evaluate to an integer. The truth condition string was"
							+ testString);
				}
				if (tempTruthCondition != -1 && tempTruthCondition != 0 && tempTruthCondition != 1){
					thrower ("Truth condition was not one of the valid options: -1, 0, or 1. The truth condition was" 
									+ str(tempTruthCondition));
				}
				setResultCondition(dataSetCount, pixelCount, pictureCount, tempTruthCondition);
				//(int dataSetNumber, int pixel, int picture, int trueConditionValue)
				pixelCount++;
			}
			if (pixelCount != currentPixelNumber){
				thrower ("Number of pixels the truth condition was set for doesn't match the number of pixels"
						" reported earlier in the file.");
			}
			pictureCount++;
		}
		if (pictureCount != numberOfPictures){
			thrower ("Number of pictures the truth condition was set for doesn't match the number of"
					   " pictures reported earlier in the file.");
		}
		dataSetCount++;
	}
	if (dataSetCount != dataSets.size()){
		thrower ( "Number of data sets the truth condition was set for doesn't match the number of data sets "
			     "reported earlier in the file.");
	}

	ConfigSystem::checkDelimiterLine(plotStream, "POST_SELECTION_BEGIN" );
	// condition loop
	int conditionCount = 0;
	while (!ConfigSystem::checkDelimiterLine(plotStream, "CONDITION_BEGIN", str("POST_SELECTION_END") ) ){
		// data set loop
		int dataSetCount = 0;
		while ( !ConfigSystem::checkDelimiterLine(plotStream, "DATA_SET_BEGIN", str("CONDITION_END") ) )	{
			// picture loop
			int pictureCount = 0;
			while ( !ConfigSystem::checkDelimiterLine(plotStream, "PICTURE_BEGIN", str("DATA_SET_END") ) ){
				// pixel loop
				int pixelCount = 0;
				while (true){
					plotStream >> testString;
					if (testString == "PICTURE_END"){
						break;
					}
					int tempPostSelectionCondition;

					try {
						tempPostSelectionCondition = boost::lexical_cast<int>(testString);
					}
					catch ( boost::bad_lexical_cast&){
						throwNested ("truth condition failed to evaluate to an integer. The truth condition string was" + testString);
					}
					if (tempPostSelectionCondition != -1 && tempPostSelectionCondition != 0 && tempPostSelectionCondition != 1){
						thrower ("truth condition was not one of the valid options: -1, 0, or 1. The truth condition was"
								+ str(tempPostSelectionCondition));
					}
					setPostSelCondition(dataSetCount, conditionCount, pixelCount, pictureCount, tempPostSelectionCondition);
					pixelCount++;
				}
				if (pixelCount != currentPixelNumber){
					thrower ( "number of pixels the truth condition was set for doesn't match the number of pixels"
						     " reported earlier in the file.");
				}
				pictureCount++;
			}
			if (pictureCount != numberOfPictures){
				thrower ( "number of pictures the truth condition was set for doesn't match the number of pictures"
					     " reported earlier in the file.");
			}
			dataSetCount++;
		}
		if (dataSetCount != dataSets.size()){
			thrower ( "Number of data sets the truth condition was set for doesn't match the number of data sets "
				     "reported earlier in the file.");
		}
		conditionCount++;
	}
	if (conditionCount != currentConditionNumber){
		thrower ( "number of post selection conditions that were set doesn't match the number of conditions "
			     "reported earlier in the file.");
	}
	
	// get legends
	ConfigSystem::checkDelimiterLine(plotStream, "LEGENDS_BEGIN" );
	plotStream.get( );
	dataSetCount = 0;
	while (true){
		getline(plotStream, testString);
		if (testString == "LEGENDS_END"){
			break;
		}
		changeLegendText(dataSetCount, testString);
		dataSetCount++;
	}
	if (dataSetCount != dataSets.size()){
		thrower ("the number of data sets that legends were read for doesn't match the number of data sets "
			   "reported earlier in the file.");
	}
	// get counts locations
	ConfigSystem::checkDelimiterLine(plotStream, "PLOT_COUNTS_LOCATIONS_BEGIN" );
	dataSetCount = 0;
	while (true){
		std::string pixelStr, pictureStr;
		int pixel, picture;
		plotStream >> pixelStr;
		if ( pixelStr == "PLOT_COUNTS_LOCATIONS_END" ){
			break;
		}
		plotStream >> pictureStr;
		try{
			pixel = boost::lexical_cast<int>(pixelStr);
		}
		catch ( boost::bad_lexical_cast&){
			throwNested ("ERROR: pixel listed in file did not convert to integer correctly. pixel string was" + pixelStr);
		}
		try{
			picture = boost::lexical_cast<int>(pictureStr);
		}
		catch ( boost::bad_lexical_cast&){
			throwNested ("ERROR: picture listed in file did not convert to integer correctly. picture string was" + pictureStr);
		}
		setDataCountsLocation(dataSetCount, pixel, picture);
		dataSetCount++;
	}

	if (generalPlotType == "Pixel Count Histograms" || generalPlotType == "Pixel Counts"){
		if (dataSetCount != dataSets.size()){
			thrower ("the number of data sets that counts locations were read for doesn't match the number of "
				   "data sets reported earlier in the file.");
		}
	}
	else{
		if (dataSetCount > 0){
			thrower ("There were counts plotting locations listed in the file despite the plot type not being"
					" pixel count histograms or pixel counts.");
		}
	}
	ConfigSystem::checkDelimiterLine(plotStream, "FITTING_OPTIONS_BEGIN" );
	plotStream.get( );
	dataSetCount = 0;
	while (true){
		getline(plotStream, testString);
		if (testString == "FITTING_OPTIONS_END"){
			break;
		}
		std::string fitOptionStr, whenToFitStr;
		int fitOption, whenToFit;
		std::stringstream tempStream;
		tempStream << testString;
		tempStream >> fitOptionStr;
		try{
			fitOption = boost::lexical_cast<int>(fitOptionStr);
		}
		catch ( boost::bad_lexical_cast&){
			throwNested ("ERROR: fit option listed in file did not convert to integer correctly. fit option string was" + fitOptionStr);
		}
		tempStream >> whenToFitStr;
		try{
			whenToFit = boost::lexical_cast<int>(whenToFitStr);
		}
		catch ( boost::bad_lexical_cast&){
			throwNested ("ERROR: When to Fit option listed in file did not convert to integer correctly. when to fit string "
				   "was" + whenToFitStr);
		}
		dataSets[dataSetCount].setFitType(fitOption);
		dataSets[dataSetCount].setWhenToFit(whenToFit);
		dataSetCount++;
	}
	if (dataSetCount > dataSets.size()){
		thrower ("ERROR: more fit options than data sets!");
	}

	ConfigSystem::checkDelimiterLine(plotStream, "HIST_OPTIONS_BEGIN" );
	plotStream.get( );
	std::string tmpStr;
	plotStream >> tmpStr;
	unsigned dsetNum = 0;
	try{
		dsetNum = boost::lexical_cast<unsigned long>( tmpStr );
	}
	catch ( boost::bad_lexical_cast& ){
		throwNested ( "expected number of data sets in plot file while loading hist options, instead found: " + tmpStr );
	}
	if ( dsetNum != dataSets.size( ) ){
		thrower ( "data set number in file while opening hist settings doesn't match number from earlier in file." );
	}
	for ( auto& dset : dataSets ){
		plotStream >> tmpStr;
		unsigned width;
		try{
			width = boost::lexical_cast<unsigned long>( tmpStr );
		}
		catch ( boost::bad_lexical_cast& ){
			throwNested ( "ERROR: failed to convert histogram bin width to an unsigned int! string was: " + tmpStr );
		}
		dset.setHistBinWidth( width );
	}
	ConfigSystem::checkDelimiterLine(plotStream, "HIST_OPTIONS_END" );
}


void PlottingInfo::resetNumberOfAnalysisGroups( unsigned setNumber){
	for (unsigned pixelInc = 0; pixelInc < currentPixelNumber; pixelInc++){
		analysisGroups[pixelInc].clear();
		analysisGroups[pixelInc].resize(setNumber);
	}
}

void PlottingInfo::resetPixelNumber( unsigned pixelNumber){
	currentPixelNumber = pixelNumber;
	analysisGroups.resize(pixelNumber);
	for (unsigned dataSetInc = 0; dataSetInc < dataSets.size(); dataSetInc++){
		dataSets[dataSetInc].resetPixelNumber(pixelNumber);
		dataSets[dataSetInc].resetPictureNumber(numberOfPictures);
	}
}

void PlottingInfo::resetPictureNumber( unsigned pictureNumber){
	numberOfPictures = pictureNumber;
	for (unsigned dataSetInc = 0; dataSetInc < dataSets.size(); dataSetInc++){
		dataSets[dataSetInc].resetPictureNumber(pictureNumber);
	}
}

void PlottingInfo::resetConditionNumber( unsigned conditionNumber){
	currentConditionNumber = conditionNumber;
	for (unsigned dataSetInc = 0; dataSetInc < dataSets.size(); dataSetInc++){
		dataSets[dataSetInc].resetPostSelectionConditionNumber(conditionNumber);
	}
}


void PlottingInfo::resetDataSetNumber( unsigned dataSetNumber){
	dataSets.clear();
	dataSets.resize(dataSetNumber);
}


void PlottingInfo::clear(){
	title = "";
	yLabel = "";
	analysisGroups.clear();
	dataSets.clear();
	fileName = "";
	numberOfPictures = 0;
	currentPixelNumber = 0;
	currentConditionNumber = 0;
	// these are invalid values... should probably default to some default valid values?
	xAxis = "";
	generalPlotType = "";
}

std::string PlottingInfo::getTitle(){
	return title;
}

std::string PlottingInfo::getYLabel(){
	return yLabel;
}

std::string PlottingInfo::getFileName(){
	return fileName;
}

unsigned PlottingInfo::getPixelNumber(){
	return currentPixelNumber;
}

unsigned PlottingInfo::getPicNumber(){
	return numberOfPictures;
}

unsigned PlottingInfo::getConditionNumber(){
	return currentConditionNumber;
}


unsigned PlottingInfo::getPixelGroupNumber(){
	if (analysisGroups.size() == 0){
		thrower ( "ERROR: tried to get group number when no data sets!" );
	}
	return analysisGroups[0].size();
}


unsigned PlottingInfo::getDataSetNumber(){
	return dataSets.size();
}


void PlottingInfo::setPixelIndex( unsigned pixel, unsigned group, unsigned index){
	groupInfo( pixel, group ).pixelIndex = index;
}


unsigned PlottingInfo::getPixelIndex( unsigned pixel, unsigned group){
	return groupInfo( pixel, group ).pixelIndex;
}


void PlottingInfo::setFitOption( unsigned dataSet, unsigned fitType){
	dataSets[dataSet].setFitType(fitType);
}


void PlottingInfo::setWhenToFit( unsigned dataSet, unsigned whenToFit){
	dataSets[dataSet].setWhenToFit(whenToFit);
}


unsigned PlottingInfo::getFitOption( unsigned dataSet){
	return dataSets[dataSet].getFitType();
}


unsigned PlottingInfo::whenToFit( unsigned dataSet){
	return dataSets[dataSet].getWhenToFit();
}


std::vector<std::pair<unsigned, unsigned>> PlottingInfo::getAllPixelLocations(){
	std::vector<std::pair<unsigned, unsigned>> allUniqueLocations;
	for (unsigned pixelInc = 0; pixelInc < analysisGroups.size(); pixelInc++){
		for (unsigned groupInc = 0; groupInc < analysisGroups[pixelInc].size(); groupInc++){
			std::pair<unsigned, unsigned> tempLocation;
			tempLocation.first = groupInfo( pixelInc, groupInc ).row;
			tempLocation.second = groupInfo( pixelInc, groupInc ).col;
			bool alreadyExists = false;
			for (unsigned uniqueLocationInc = 0; uniqueLocationInc < allUniqueLocations.size(); uniqueLocationInc++){
				if (allUniqueLocations[uniqueLocationInc].first == tempLocation.first 
					 && allUniqueLocations[uniqueLocationInc].second == tempLocation.second){
					alreadyExists = true;
				}
			}
			if (!alreadyExists){
				allUniqueLocations.push_back(tempLocation);
			}
		}
	}
	return allUniqueLocations;
}


void PlottingInfo::setGroups(std::vector<coordinate> locations){
	if (locations.size() % currentPixelNumber != 0){
		thrower ( "ERROR: One of your real-time plots was expecting a multiple of " + str(currentPixelNumber) + " pixels to analyze, but you "
			     "selected " + str(locations.size()) + " pixels.");
	}
	unsigned locationInc = 0;
	for (unsigned groupInc = 0; groupInc < locations.size() / currentPixelNumber; groupInc++){
		addGroup();
		for (unsigned pixelInc = 0; pixelInc < currentPixelNumber; pixelInc++){
			setGroupLocation(pixelInc, groupInc, locations[locationInc].column+1, locations[locationInc].row+1);
			locationInc++;
		}
	}
}

