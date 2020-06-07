// created by Mark O. Brown
#include "stdafx.h"
#include "PictureManager.h"
#include "ConfigurationSystems/ProfileSystem.h"
#include "Andor/pictureSettingsControl.h"

PictureManager::PictureManager ( bool histOption, std::string configurationFileDelimiter, bool autoscaleDefault ) 
	: pictures{ { histOption, false, false, false } }, configDelim ( configurationFileDelimiter ), 
	autoScalePictures (autoscaleDefault ){
}

void PictureManager::setSoftwareAccumulationOptions ( std::array<softwareAccumulationOption, 4> opts ){
	for ( auto picInc : range ( 4 ) ){
		pictures[ picInc ].setSoftwareAccumulationOption ( opts[ picInc ] );
	}
}


void PictureManager::paint ( CRect size, CBrush* bgdBrush ){
	for ( auto& pic : pictures ){
		pic.paint ( size, bgdBrush );
	}
}


void PictureManager::drawBitmap (Matrix<long> picData, std::pair<int,int> minMax, UINT whichPicCtrl ){
	std::tuple<bool, int, int> autoScaleInfo = std::make_tuple ( autoScalePictures, minMax.first, minMax.second );
	pictures[whichPicCtrl].drawBitmap (picData, autoScaleInfo, specialLessThanMin, specialGreaterThanMax);
	if (alwaysShowGrid)	{
		pictures[whichPicCtrl].drawGrid (gridBrush);
	}
}

void PictureManager::setPalletes(std::array<int, 4> palleteIds){
	for (int picInc = 0; picInc < 4; picInc++){
		if ( palleteIds[ picInc ] > 3 ){
			errBox ( "Image Pallete ID out of range! Forcing to 0." );
			palleteIds[ picInc ] = 0;
		}
		pictures[picInc].updatePalette(palettes[palleteIds[picInc]]);
	}
}

void PictureManager::handleMouse( CPoint point ){
	for ( auto& pic : pictures ){
		pic.handleMouse( point );
	}
}

void PictureManager::setAlwaysShowGrid(bool showOption){
	alwaysShowGrid = showOption;
	if (alwaysShowGrid){
		if (!pictures[1].isActive()){
			pictures[0].drawGrid( gridBrush);
			return;
		}
		for (auto& pic : pictures){
			pic.drawGrid( gridBrush);
		}
	}
}


void PictureManager::redrawPictures( coordinate selectedLocation, std::vector<coordinate> analysisLocs,
	std::vector<atomGrid> gridInfo, bool forceGrid, UINT picNumber ){
	if (!pictures[1].isActive()){
		pictures[0].redrawImage();
		if (alwaysShowGrid || forceGrid ){
			pictures[0].drawGrid(gridBrush);
		}
		drawDongles(selectedLocation, analysisLocs, gridInfo, picNumber );
		return;
	}
	for (auto& pic : pictures){
		pic.redrawImage();
		if (alwaysShowGrid || forceGrid ){
			pic.drawGrid(gridBrush);
		}
	}
	drawDongles(selectedLocation, analysisLocs, gridInfo, picNumber );
}

/*
 *
 */
void PictureManager::drawDongles(coordinate selectedLocation, std::vector<coordinate> analysisLocs,
								  std::vector<atomGrid> grids, UINT pictureNumber, bool includingAnalysisMarkers )
{
	UINT count = 1;
	for (auto& pic : pictures){
		pic.drawCircle( selectedLocation);
		if ( includingAnalysisMarkers )	{
			pic.drawAnalysisMarkers (  analysisLocs, grids );
		}
		pic.drawPicNum( pictureNumber - getNumberActive() + count++ );
	}
}


void PictureManager::setNumberPicturesActive( int numberActive ){
	int count = 1;
	for (auto& pic : pictures){
		pic.setActive( count <= numberActive );
		count++;
	}
}

void PictureManager::handleEditChange( UINT id ){
	for (auto& pic : pictures){
		pic.handleEditChange( id );
	}
}


void PictureManager::setAutoScalePicturesOption(bool autoScaleOption){
	autoScalePictures = autoScaleOption;
}


void PictureManager::handleSaveConfig(ConfigStream& saveFile){
	saveFile << configDelim + "\n/*Slider Locs (Min/Max):*/\n";
	for (auto& pic : pictures){
		std::pair<UINT, UINT> sliderLoc = pic.getSliderLocations();
		saveFile << str(sliderLoc.first) << " " << sliderLoc.second << "\n";
	}
	saveFile << "/*Auto-Scale Pics?*/ " << autoScalePictures;
	saveFile << "\n/*Special >Max Color?*/ " << specialGreaterThanMax;
	saveFile << "\n/*Special <Min Color?*/ " << specialLessThanMin;
	saveFile << "\n/*Always Show Grid?*/ " << alwaysShowGrid;
	saveFile << "\nEND_" + configDelim + "\n";
}


void PictureManager::handleOpenConfig( ConfigStream& configFile ){
	if ( configFile.ver < Version ( "4.0" ) ){
		thrower ( "Picture Manager requires configuration file version 4.0+." );
	}
	std::array<int, 4> maxes, mins;
	for (int sliderInc = 0; sliderInc < 4; sliderInc++)	{
		configFile >> mins[sliderInc];
		configFile >> maxes[sliderInc];
	}
	configFile >> autoScalePictures >> specialGreaterThanMax >> specialLessThanMin >> alwaysShowGrid;
	UINT count = 0;
	for (auto& pic : pictures){
		pic.setSliderPositions(mins[count], maxes[count]);
		count++;
	}
	configFile.get();
}


void PictureManager::setSpecialLessThanMin(bool option){
	specialLessThanMin = option;
}


void PictureManager::setSpecialGreaterThanMax(bool option){
	specialGreaterThanMax = option;
}

void PictureManager::handleScroll(UINT nSBCode, UINT nPos, CScrollBar* scrollbar){
	if (nSBCode == SB_THUMBPOSITION || nSBCode == SB_THUMBTRACK){
		int id = scrollbar->GetDlgCtrlID();
		for (auto& control : pictures){
			control.handleScroll ( id, nPos );
			control.redrawImage ( false );
		}
	}
}

coordinate PictureManager::getClickLocation( CPoint clickLocation ){
	coordinate location;
	for (auto& pic : pictures){
		try{
			location = pic.checkClickLocation( clickLocation );
			return location;
		}
		catch(Error&){}
		// checkClickLocation throws if not found. Continue looking.
	}
	thrower ( "click location not found" );
}


void PictureManager::setSinglePicture( imageParameters imageParams){
	for (UINT picNum = 0; picNum < 4; picNum++){
		if (picNum < 1){
			pictures[picNum].setActive(true);
		}
		else{
			pictures[picNum].setActive(false);
		}
	}
	pictures.front( ).setPictureArea( picturesLocation, picturesWidth, picturesHeight);
	picturesLocation.x += picturesWidth;
	pictures.front( ).setSliderControlLocs( picturesLocation, picturesHeight);
	//pictures.front( ).setCursorValueLocations( parent );
	setParameters( imageParams );
}


void PictureManager::resetPictureStorage(){
	for (auto& pic : pictures){
		pic.resetStorage();
	}
}


void PictureManager::setMultiplePictures( imageParameters imageParams, UINT numberActivePics )
{
	for (UINT picNum = 0; picNum < 4; picNum++){
		if (picNum < numberActivePics){
			pictures[picNum].setActive(true);
		}
		else{
			pictures[picNum].setActive(false);
		}
	}

	POINT loc = picturesLocation;
	// Square: width = 550, height = 440
	auto picWidth = 1100;
	auto picHeight = 220;
	//int picWidth = 550;
	//int picHeight = 420;
	pictures[0].setPictureArea( loc, picWidth, picHeight );
	pictures[0].setSliderControlLocs ({ loc.x + picWidth,loc.y }, picHeight);
	//loc.x += 550;
	loc.y += picHeight + 25;
	pictures[1].setPictureArea( loc, picWidth, picHeight );
	pictures[1].setSliderControlLocs ({ loc.x + picWidth,loc.y }, picHeight);
	//loc.x -= 550;
	loc.y += picHeight + 25;
	pictures[2].setPictureArea( loc, picWidth, picHeight );
	pictures[2].setSliderControlLocs ({ loc.x + picWidth,loc.y }, picHeight);
	//loc.x += 550;
	loc.y += picHeight + 25;
	pictures[3].setPictureArea( loc, picWidth, picHeight );
	pictures[3].setSliderControlLocs ({ loc.x + picWidth,loc.y }, picHeight);
	setParameters( imageParams );
	for ( auto& pic : pictures ){
		//pic.setCursorValueLocations( parent );
	}
}


void PictureManager::drawBackgrounds(){
	for (auto& pic : pictures){
		pic.drawBackground();
	}
}


void PictureManager::initialize( POINT& loc, CBrush* defaultBrush, int manWidth, int manHeight,
								 IChimeraWindowWidget* widget ){
	picturesLocation = loc;
	picturesWidth = manWidth;
	picturesHeight = manHeight;
	gridBrush = defaultBrush;
	// Square: width = 550, height = 440
	auto width = 1200;
	auto height = 220;
	pictures[0].initialize( loc, width, height, widget);
	loc.y += height;
	pictures[1].initialize( loc, width, height, widget );
	loc.y += height;
	pictures[2].initialize( loc, width, height, widget );
	loc.y += height;
	pictures[3].initialize( loc, width, height, widget );
	loc.y += height;
	for (auto& pic : pictures){
		pic.updatePalette( palettes[2] );
	}
	// initialize to one. This matches the camera settings initialization.
	setNumberPicturesActive( 1 );
}


void PictureManager::updatePlotData ( ){
	for ( auto& pic : pictures ){
		pic.updatePlotData ( );
	}
}


void PictureManager::refreshBackgrounds(){
	if (!pictures[1].isActive()){
		pictures[0].drawBackground(  );
	}
	else{
		for (auto& picture : pictures){
			picture.drawBackground(  );
		}
	}
}


UINT PictureManager::getNumberActive( ){
	UINT count = 0;
	for ( auto& pic : pictures ){
		if ( pic.isActive( ) ){
			count++;
		}
	}
	return count;
}

void PictureManager::drawGrids(){
	for (auto& picture : pictures){
		picture.drawGrid(gridBrush );
	}
}


void PictureManager::setParameters(imageParameters parameters){
	for (auto& picture : pictures){
		picture.recalculateGrid(parameters);
	}
}


RECT PictureManager::getPicArea ( ){
	RECT r;
	r.left = picturesLocation.x;
	r.top = picturesLocation.y;
	r.right = r.left + picturesWidth;
	r.bottom = r.top + picturesHeight;
	return r;
}


void PictureManager::createPalettes( ){
	/*
	struct
	{
		WORD Version;
		WORD NumberEntries;
		PALETTEENTRY aEntries[256];
	} Palette = { 0x300, 256 };

	GetSystemPaletteEntries( *dc, 0, 256, Palette.aEntries );
	// this is the parula colormap from matlab. It looks nice :D
	double dark_viridis[256][3] =
	{
		// special entry
		{ 0 , 0 , 1 },
		//
		{ 0.26851 , 0.009605 , 0.335427 },
		{ 0.269944 , 0.014625 , 0.341379 },
		{ 0.271305 , 0.019942 , 0.347269 },
		{ 0.272594 , 0.025563 , 0.353093 },
		{ 0.273809 , 0.031497 , 0.358853 },
		{ 0.274952 , 0.037752 , 0.364543 },
		{ 0.276022 , 0.044167 , 0.370164 },
		{ 0.277018 , 0.050344 , 0.375715 },
		{ 0.277941 , 0.056324 , 0.381191 },
		{ 0.278791 , 0.062145 , 0.386592 },
		{ 0.279566 , 0.067836 , 0.391917 },
		{ 0.280267 , 0.073417 , 0.397163 },
		{ 0.280894 , 0.078907 , 0.402329 },
		{ 0.281446 , 0.08432 , 0.407414 },
		{ 0.281924 , 0.089666 , 0.412415 },
		{ 0.282327 , 0.094955 , 0.417331 },
		{ 0.282656 , 0.100196 , 0.42216 },
		{ 0.28291 , 0.105393 , 0.426902 },
		{ 0.283091 , 0.110553 , 0.431554 },
		{ 0.283197 , 0.11568 , 0.436115 },
		{ 0.283229 , 0.120777 , 0.440584 },
		{ 0.283187 , 0.125848 , 0.44496 },
		{ 0.283072 , 0.130895 , 0.449241 },
		{ 0.282884 , 0.13592 , 0.453427 },
		{ 0.282623 , 0.140926 , 0.457517 },
		{ 0.28229 , 0.145912 , 0.46151 },
		{ 0.281887 , 0.150881 , 0.465405 },
		{ 0.281412 , 0.155834 , 0.469201 },
		{ 0.280868 , 0.160771 , 0.472899 },
		{ 0.280255 , 0.165693 , 0.476498 },
		{ 0.279574 , 0.170599 , 0.479997 },
		{ 0.278826 , 0.17549 , 0.483397 },
		{ 0.278012 , 0.180367 , 0.486697 },
		{ 0.277134 , 0.185228 , 0.489898 },
		{ 0.276194 , 0.190074 , 0.493001 },
		{ 0.275191 , 0.194905 , 0.496005 },
		{ 0.274128 , 0.199721 , 0.498911 },
		{ 0.273006 , 0.20452 , 0.501721 },
		{ 0.271828 , 0.209303 , 0.504434 },
		{ 0.270595 , 0.214069 , 0.507052 },
		{ 0.269308 , 0.218818 , 0.509577 },
		{ 0.267968 , 0.223549 , 0.512008 },
		{ 0.26658 , 0.228262 , 0.514349 },
		{ 0.265145 , 0.232956 , 0.516599 },
		{ 0.263663 , 0.237631 , 0.518762 },
		{ 0.262138 , 0.242286 , 0.520837 },
		{ 0.260571 , 0.246922 , 0.522828 },
		{ 0.258965 , 0.251537 , 0.524736 },
		{ 0.257322 , 0.25613 , 0.526563 },
		{ 0.255645 , 0.260703 , 0.528312 },
		{ 0.253935 , 0.265254 , 0.529983 },
		{ 0.252194 , 0.269783 , 0.531579 },
		{ 0.250425 , 0.27429 , 0.533103 },
		{ 0.248629 , 0.278775 , 0.534556 },
		{ 0.246811 , 0.283237 , 0.535941 },
		{ 0.244972 , 0.287675 , 0.53726 },
		{ 0.243113 , 0.292092 , 0.538516 },
		{ 0.241237 , 0.296485 , 0.539709 },
		{ 0.239346 , 0.300855 , 0.540844 },
		{ 0.237441 , 0.305202 , 0.541921 },
		{ 0.235526 , 0.309527 , 0.542944 },
		{ 0.233603 , 0.313828 , 0.543914 },
		{ 0.231674 , 0.318106 , 0.544834 },
		{ 0.229739 , 0.322361 , 0.545706 },
		{ 0.227802 , 0.326594 , 0.546532 },
		{ 0.225863 , 0.330805 , 0.547314 },
		{ 0.223925 , 0.334994 , 0.548053 },
		{ 0.221989 , 0.339161 , 0.548752 },
		{ 0.220057 , 0.343307 , 0.549413 },
		{ 0.21813 , 0.347432 , 0.550038 },
		{ 0.21621 , 0.351535 , 0.550627 },
		{ 0.214298 , 0.355619 , 0.551184 },
		{ 0.212395 , 0.359683 , 0.55171 },
		{ 0.210503 , 0.363727 , 0.552206 },
		{ 0.208623 , 0.367752 , 0.552675 },
		{ 0.206756 , 0.371758 , 0.553117 },
		{ 0.204903 , 0.375746 , 0.553533 },
		{ 0.203063 , 0.379716 , 0.553925 },
		{ 0.201239 , 0.38367 , 0.554294 },
		{ 0.19943 , 0.387607 , 0.554642 },
		{ 0.197636 , 0.391528 , 0.554969 },
		{ 0.19586 , 0.395433 , 0.555276 },
		{ 0.1941 , 0.399323 , 0.555565 },
		{ 0.192357 , 0.403199 , 0.555836 },
		{ 0.190631 , 0.407061 , 0.556089 },
		{ 0.188923 , 0.41091 , 0.556326 },
		{ 0.187231 , 0.414746 , 0.556547 },
		{ 0.185556 , 0.41857 , 0.556753 },
		{ 0.183898 , 0.422383 , 0.556944 },
		{ 0.182256 , 0.426184 , 0.55712 },
		{ 0.180629 , 0.429975 , 0.557282 },
		{ 0.179019 , 0.433756 , 0.55743 },
		{ 0.177423 , 0.437527 , 0.557565 },
		{ 0.175841 , 0.44129 , 0.557685 },
		{ 0.174274 , 0.445044 , 0.557792 },
		{ 0.172719 , 0.448791 , 0.557885 },
		{ 0.171176 , 0.45253 , 0.557965 },
		{ 0.169646 , 0.456262 , 0.55803 },
		{ 0.168126 , 0.459988 , 0.558082 },
		{ 0.166617 , 0.463708 , 0.558119 },
		{ 0.165117 , 0.467423 , 0.558141 },
		{ 0.163625 , 0.471133 , 0.558148 },
		{ 0.162142 , 0.474838 , 0.55814 },
		{ 0.160665 , 0.47854 , 0.558115 },
		{ 0.159194 , 0.482237 , 0.558073 },
		{ 0.157729 , 0.485932 , 0.558013 },
		{ 0.15627 , 0.489624 , 0.557936 },
		{ 0.154815 , 0.493313 , 0.55784 },
		{ 0.153364 , 0.497 , 0.557724 },
		{ 0.151918 , 0.500685 , 0.557587 },
		{ 0.150476 , 0.504369 , 0.55743 },
		{ 0.149039 , 0.508051 , 0.55725 },
		{ 0.147607 , 0.511733 , 0.557049 },
		{ 0.14618 , 0.515413 , 0.556823 },
		{ 0.144759 , 0.519093 , 0.556572 },
		{ 0.143343 , 0.522773 , 0.556295 },
		{ 0.141935 , 0.526453 , 0.555991 },
		{ 0.140536 , 0.530132 , 0.555659 },
		{ 0.139147 , 0.533812 , 0.555298 },
		{ 0.13777 , 0.537492 , 0.554906 },
		{ 0.136408 , 0.541173 , 0.554483 },
		{ 0.135066 , 0.544853 , 0.554029 },
		{ 0.133743 , 0.548535 , 0.553541 },
		{ 0.132444 , 0.552216 , 0.553018 },
		{ 0.131172 , 0.555899 , 0.552459 },
		{ 0.129933 , 0.559582 , 0.551864 },
		{ 0.128729 , 0.563265 , 0.551229 },
		{ 0.127568 , 0.566949 , 0.550556 },
		{ 0.126453 , 0.570633 , 0.549841 },
		{ 0.125394 , 0.574318 , 0.549086 },
		{ 0.124395 , 0.578002 , 0.548287 },
		{ 0.123463 , 0.581687 , 0.547445 },
		{ 0.122606 , 0.585371 , 0.546557 },
		{ 0.121831 , 0.589055 , 0.545623 },
		{ 0.121148 , 0.592739 , 0.544641 },
		{ 0.120565 , 0.596422 , 0.543611 },
		{ 0.120092 , 0.600104 , 0.54253 },
		{ 0.119738 , 0.603785 , 0.5414 },
		{ 0.119512 , 0.607464 , 0.540218 },
		{ 0.119423 , 0.611141 , 0.538982 },
		{ 0.119483 , 0.614817 , 0.537692 },
		{ 0.119699 , 0.61849 , 0.536347 },
		{ 0.120081 , 0.622161 , 0.534946 },
		{ 0.120638 , 0.625828 , 0.533488 },
		{ 0.12138 , 0.629492 , 0.531973 },
		{ 0.122312 , 0.633153 , 0.530398 },
		{ 0.123444 , 0.636809 , 0.528763 },
		{ 0.12478 , 0.640461 , 0.527068 },
		{ 0.126326 , 0.644107 , 0.525311 },
		{ 0.128087 , 0.647749 , 0.523491 },
		{ 0.130067 , 0.651384 , 0.521608 },
		{ 0.132268 , 0.655014 , 0.519661 },
		{ 0.134692 , 0.658636 , 0.517649 },
		{ 0.137339 , 0.662252 , 0.515571 },
		{ 0.14021 , 0.665859 , 0.513427 },
		{ 0.143303 , 0.669459 , 0.511215 },
		{ 0.146616 , 0.67305 , 0.508936 },
		{ 0.150148 , 0.676631 , 0.506589 },
		{ 0.153894 , 0.680203 , 0.504172 },
		{ 0.157851 , 0.683765 , 0.501686 },
		{ 0.162016 , 0.687316 , 0.499129 },
		{ 0.166383 , 0.690856 , 0.496502 },
		{ 0.170948 , 0.694384 , 0.493803 },
		{ 0.175707 , 0.6979 , 0.491033 },
		{ 0.180653 , 0.701402 , 0.488189 },
		{ 0.185783 , 0.704891 , 0.485273 },
		{ 0.19109 , 0.708366 , 0.482284 },
		{ 0.196571 , 0.711827 , 0.479221 },
		{ 0.202219 , 0.715272 , 0.476084 },
		{ 0.20803 , 0.718701 , 0.472873 },
		{ 0.214 , 0.722114 , 0.469588 },
		{ 0.220124 , 0.725509 , 0.466226 },
		{ 0.226397 , 0.728888 , 0.462789 },
		{ 0.232815 , 0.732247 , 0.459277 },
		{ 0.239374 , 0.735588 , 0.455688 },
		{ 0.24607 , 0.73891 , 0.452024 },
		{ 0.252899 , 0.742211 , 0.448284 },
		{ 0.259857 , 0.745492 , 0.444467 },
		{ 0.266941 , 0.748751 , 0.440573 },
		{ 0.274149 , 0.751988 , 0.436601 },
		{ 0.281477 , 0.755203 , 0.432552 },
		{ 0.288921 , 0.758394 , 0.428426 },
		{ 0.296479 , 0.761561 , 0.424223 },
		{ 0.304148 , 0.764704 , 0.419943 },
		{ 0.311925 , 0.767822 , 0.415586 },
		{ 0.319809 , 0.770914 , 0.411152 },
		{ 0.327796 , 0.77398 , 0.40664 },
		{ 0.335885 , 0.777018 , 0.402049 },
		{ 0.344074 , 0.780029 , 0.397381 },
		{ 0.35236 , 0.783011 , 0.392636 },
		{ 0.360741 , 0.785964 , 0.387814 },
		{ 0.369214 , 0.788888 , 0.382914 },
		{ 0.377779 , 0.791781 , 0.377939 },
		{ 0.386433 , 0.794644 , 0.372886 },
		{ 0.395174 , 0.797475 , 0.367757 },
		{ 0.404001 , 0.800275 , 0.362552 },
		{ 0.412913 , 0.803041 , 0.357269 },
		{ 0.421908 , 0.805774 , 0.35191 },
		{ 0.430983 , 0.808473 , 0.346476 },
		{ 0.440137 , 0.811138 , 0.340967 },
		{ 0.449368 , 0.813768 , 0.335384 },
		{ 0.458674 , 0.816363 , 0.329727 },
		{ 0.468053 , 0.818921 , 0.323998 },
		{ 0.477504 , 0.821444 , 0.318195 },
		{ 0.487026 , 0.823929 , 0.312321 },
		{ 0.496615 , 0.826376 , 0.306377 },
		{ 0.506271 , 0.828786 , 0.300362 },
		{ 0.515992 , 0.831158 , 0.294279 },
		{ 0.525776 , 0.833491 , 0.288127 },
		{ 0.535621 , 0.835785 , 0.281908 },
		{ 0.545524 , 0.838039 , 0.275626 },
		{ 0.555484 , 0.840254 , 0.269281 },
		{ 0.565498 , 0.84243 , 0.262877 },
		{ 0.575563 , 0.844566 , 0.256415 },
		{ 0.585678 , 0.846661 , 0.249897 },
		{ 0.595839 , 0.848717 , 0.243329 },
		{ 0.606045 , 0.850733 , 0.236712 },
		{ 0.616293 , 0.852709 , 0.230052 },
		{ 0.626579 , 0.854645 , 0.223353 },
		{ 0.636902 , 0.856542 , 0.21662 },
		{ 0.647257 , 0.8584 , 0.209861 },
		{ 0.657642 , 0.860219 , 0.203082 },
		{ 0.668054 , 0.861999 , 0.196293 },
		{ 0.678489 , 0.863742 , 0.189503 },
		{ 0.688944 , 0.865448 , 0.182725 },
		{ 0.699415 , 0.867117 , 0.175971 },
		{ 0.709898 , 0.868751 , 0.169257 },
		{ 0.720391 , 0.87035 , 0.162603 },
		{ 0.730889 , 0.871916 , 0.156029 },
		{ 0.741388 , 0.873449 , 0.149561 },
		{ 0.751884 , 0.874951 , 0.143228 },
		{ 0.762373 , 0.876424 , 0.137064 },
		{ 0.772852 , 0.877868 , 0.131109 },
		{ 0.783315 , 0.879285 , 0.125405 },
		{ 0.79376 , 0.880678 , 0.120005 },
		{ 0.804182 , 0.882046 , 0.114965 },
		{ 0.814576 , 0.883393 , 0.110347 },
		{ 0.82494 , 0.88472 , 0.106217 },
		{ 0.83527 , 0.886029 , 0.102646 },
		{ 0.845561 , 0.887322 , 0.099702 },
		{ 0.85581 , 0.888601 , 0.097452 },
		{ 0.866013 , 0.889868 , 0.095953 },
		{ 0.876168 , 0.891125 , 0.09525 },
		{ 0.886271 , 0.892374 , 0.095374 },
		{ 0.89632 , 0.893616 , 0.096335 },
		{ 0.906311 , 0.894855 , 0.098125 },
		{ 0.916242 , 0.896091 , 0.100717 },
		{ 0.926106 , 0.89733 , 0.104071 },
		{ 0.935904 , 0.89857 , 0.108131 },
		{ 0.945636 , 0.899815 , 0.112838 },
		{ 0.9553 , 0.901065 , 0.118128 },
		{ 0.964894 , 0.902323 , 0.123941 },
		{ 0.974417 , 0.90359 , 0.130215 },
		{ 0.983868 , 0.904867 , 0.136897 },
		// special value
		{ 1 , 0 , 0 },
		//
	};

	UCHAR r, g, b;
	for ( UINT paletteInc : range ( PICTURE_PALETTE_SIZE ) )
	{
		// scaling it to make it a bit darker near the bottom.
		r = UCHAR ( dark_viridis[ paletteInc ][ 0 ] * ( 255.0 - 1 ) * ( 1.0 / 4 + 3.0*paletteInc / ( 4 * 255.0 ) ) );
		g = UCHAR ( dark_viridis[ paletteInc ][ 1 ] * ( 255.0 - 1 ) * ( 1.0 / 4 + 3.0*paletteInc / ( 4 * 255.0 ) ) );
		b = UCHAR ( dark_viridis[ paletteInc ][ 2 ] * ( 255.0 - 1 ) * ( 1.0 / 4 + 3.0*paletteInc / ( 4 * 255.0 ) ) );
		Palette.aEntries[ paletteInc ].peRed = LOBYTE ( r );
		Palette.aEntries[ paletteInc ].peGreen = LOBYTE ( g );
		Palette.aEntries[ paletteInc ].peBlue = LOBYTE ( b );
		Palette.aEntries[ paletteInc ].peFlags = PC_RESERVED;
	}
	palettes[0] = CreatePalette((LOGPALETTE *)&Palette );

	double redBlackBlue[256][3] =
	{
	{ 1.0, 0.0, 0.0 },
	{ 0.986928104575 , 0.0 , 0.0 },
	{ 0.97385620915 , 0.0 , 0.0 },
	{ 0.960784313725 , 0.0 , 0.0 },
	{ 0.947712418301 , 0.0 , 0.0 },
	{ 0.934640522876 , 0.0 , 0.0 },
	{ 0.921568627451 , 0.0 , 0.0 },
	{ 0.908496732026 , 0.0 , 0.0 },
	{ 0.895424836601 , 0.0 , 0.0 },
	{ 0.882352941176 , 0.0 , 0.0 },
	{ 0.869281045752 , 0.0 , 0.0 },
	{ 0.856209150327 , 0.0 , 0.0 },
	{ 0.843137254902 , 0.0 , 0.0 },
	{ 0.830065359477 , 0.0 , 0.0 },
	{ 0.816993464052 , 0.0 , 0.0 },
	{ 0.803921568627 , 0.0 , 0.0 },
	{ 0.790849673203 , 0.0 , 0.0 },
	{ 0.777777777778 , 0.0 , 0.0 },
	{ 0.764705882353 , 0.0 , 0.0 },
	{ 0.751633986928 , 0.0 , 0.0 },
	{ 0.738562091503 , 0.0 , 0.0 },
	{ 0.725490196078 , 0.0 , 0.0 },
	{ 0.712418300654 , 0.0 , 0.0 },
	{ 0.699346405229 , 0.0 , 0.0 },
	{ 0.686274509804 , 0.0 , 0.0 },
	{ 0.673202614379 , 0.0 , 0.0 },
	{ 0.662745098039 , 0.0 , 0.0 },
	{ 0.654901960784 , 0.0 , 0.0 },
	{ 0.647058823529 , 0.0 , 0.0 },
	{ 0.639215686275 , 0.0 , 0.0 },
	{ 0.63137254902 , 0.0 , 0.0 },
	{ 0.623529411765 , 0.0 , 0.0 },
	{ 0.61568627451 , 0.0 , 0.0 },
	{ 0.607843137255 , 0.0 , 0.0 },
	{ 0.6 , 0.0 , 0.0 },
	{ 0.592156862745 , 0.0 , 0.0 },
	{ 0.58431372549 , 0.0 , 0.0 },
	{ 0.576470588235 , 0.0 , 0.0 },
	{ 0.56862745098 , 0.0 , 0.0 },
	{ 0.560784313725 , 0.0 , 0.0 },
	{ 0.552941176471 , 0.0 , 0.0 },
	{ 0.545098039216 , 0.0 , 0.0 },
	{ 0.537254901961 , 0.0 , 0.0 },
	{ 0.529411764706 , 0.0 , 0.0 },
	{ 0.521568627451 , 0.0 , 0.0 },
	{ 0.513725490196 , 0.0 , 0.0 },
	{ 0.505882352941 , 0.0 , 0.0 },
	{ 0.498039215686 , 0.0 , 0.0 },
	{ 0.490196078431 , 0.0 , 0.0 },
	{ 0.482352941176 , 0.0 , 0.0 },
	{ 0.474509803922 , 0.0 , 0.0 },
	{ 0.466666666667 , 0.0 , 0.0 },
	{ 0.458823529412 , 0.0 , 0.0 },
	{ 0.450980392157 , 0.0 , 0.0 },
	{ 0.443137254902 , 0.0 , 0.0 },
	{ 0.435294117647 , 0.0 , 0.0 },
	{ 0.427450980392 , 0.0 , 0.0 },
	{ 0.419607843137 , 0.0 , 0.0 },
	{ 0.411764705882 , 0.0 , 0.0 },
	{ 0.403921568627 , 0.0 , 0.0 },
	{ 0.396078431373 , 0.0 , 0.0 },
	{ 0.388235294118 , 0.0 , 0.0 },
	{ 0.380392156863 , 0.0 , 0.0 },
	{ 0.372549019608 , 0.0 , 0.0 },
	{ 0.364705882353 , 0.0 , 0.0 },
	{ 0.356862745098 , 0.0 , 0.0 },
	{ 0.349019607843 , 0.0 , 0.0 },
	{ 0.341176470588 , 0.0 , 0.0 },
	{ 0.333333333333 , 0.0 , 0.0 },
	{ 0.325490196078 , 0.0 , 0.0 },
	{ 0.317647058824 , 0.0 , 0.0 },
	{ 0.309803921569 , 0.0 , 0.0 },
	{ 0.301960784314 , 0.0 , 0.0 },
	{ 0.294117647059 , 0.0 , 0.0 },
	{ 0.286274509804 , 0.0 , 0.0 },
	{ 0.278431372549 , 0.0 , 0.0 },
	{ 0.270588235294 , 0.0 , 0.0 },
	{ 0.264052287582 , 0.0 , 0.0 },
	{ 0.258823529412 , 0.0 , 0.0 },
	{ 0.253594771242 , 0.0 , 0.0 },
	{ 0.248366013072 , 0.0 , 0.0 },
	{ 0.243137254902 , 0.0 , 0.0 },
	{ 0.237908496732 , 0.0 , 0.0 },
	{ 0.232679738562 , 0.0 , 0.0 },
	{ 0.227450980392 , 0.0 , 0.0 },
	{ 0.222222222222 , 0.0 , 0.0 },
	{ 0.216993464052 , 0.0 , 0.0 },
	{ 0.211764705882 , 0.0 , 0.0 },
	{ 0.206535947712 , 0.0 , 0.0 },
	{ 0.201307189542 , 0.0 , 0.0 },
	{ 0.196078431373 , 0.0 , 0.0 },
	{ 0.190849673203 , 0.0 , 0.0 },
	{ 0.185620915033 , 0.0 , 0.0 },
	{ 0.180392156863 , 0.0 , 0.0 },
	{ 0.175163398693 , 0.0 , 0.0 },
	{ 0.169934640523 , 0.0 , 0.0 },
	{ 0.164705882353 , 0.0 , 0.0 },
	{ 0.159477124183 , 0.0 , 0.0 },
	{ 0.154248366013 , 0.0 , 0.0 },
	{ 0.149019607843 , 0.0 , 0.0 },
	{ 0.143790849673 , 0.0 , 0.0 },
	{ 0.138562091503 , 0.0 , 0.0 },
	{ 0.133333333333 , 0.0 , 0.0 },
	{ 0.128104575163 , 0.0 , 0.0 },
	{ 0.122875816993 , 0.0 , 0.0 },
	{ 0.117647058824 , 0.0 , 0.0 },
	{ 0.112418300654 , 0.0 , 0.0 },
	{ 0.107189542484 , 0.0 , 0.0 },
	{ 0.101960784314 , 0.0 , 0.0 },
	{ 0.0967320261438 , 0.0 , 0.0 },
	{ 0.0915032679739 , 0.0 , 0.0 },
	{ 0.0862745098039 , 0.0 , 0.0 },
	{ 0.081045751634 , 0.0 , 0.0 },
	{ 0.0758169934641 , 0.0 , 0.0 },
	{ 0.0705882352941 , 0.0 , 0.0 },
	{ 0.0653594771242 , 0.0 , 0.0 },
	{ 0.0601307189542 , 0.0 , 0.0 },
	{ 0.0549019607843 , 0.0 , 0.0 },
	{ 0.0496732026144 , 0.0 , 0.0 },
	{ 0.0444444444444 , 0.0 , 0.0 },
	{ 0.0392156862745 , 0.0 , 0.0 },
	{ 0.0339869281046 , 0.0 , 0.0 },
	{ 0.0287581699346 , 0.0 , 0.0 },
	{ 0.0235294117647 , 0.0 , 0.0 },
	{ 0.0183006535948 , 0.0 , 0.0 },
	{ 0.0130718954248 , 0.0 , 0.0 },
	{ 0.0078431372549 , 0.0 , 0.0 },
	{ 0.00261437908497 , 0.0 , 0.0 },
	{ 0.0 , 0.0 , 0.00261437908497 },
	{ 0.0 , 0.0 , 0.0078431372549 },
	{ 0.0 , 0.0 , 0.0130718954248 },
	{ 0.0 , 0.0 , 0.0183006535948 },
	{ 0.0 , 0.0 , 0.0235294117647 },
	{ 0.0 , 0.0 , 0.0287581699346 },
	{ 0.0 , 0.0 , 0.0339869281046 },
	{ 0.0 , 0.0 , 0.0392156862745 },
	{ 0.0 , 0.0 , 0.0444444444444 },
	{ 0.0 , 0.0 , 0.0496732026144 },
	{ 0.0 , 0.0 , 0.0549019607843 },
	{ 0.0 , 0.0 , 0.0601307189542 },
	{ 0.0 , 0.0 , 0.0653594771242 },
	{ 0.0 , 0.0 , 0.0705882352941 },
	{ 0.0 , 0.0 , 0.0758169934641 },
	{ 0.0 , 0.0 , 0.081045751634 },
	{ 0.0 , 0.0 , 0.0862745098039 },
	{ 0.0 , 0.0 , 0.0915032679739 },
	{ 0.0 , 0.0 , 0.0967320261438 },
	{ 0.0 , 0.0 , 0.101960784314 },
	{ 0.0 , 0.0 , 0.107189542484 },
	{ 0.0 , 0.0 , 0.112418300654 },
	{ 0.0 , 0.0 , 0.117647058824 },
	{ 0.0 , 0.0 , 0.122875816993 },
	{ 0.0 , 0.0 , 0.128104575163 },
	{ 0.0 , 0.0 , 0.133333333333 },
	{ 0.0 , 0.0 , 0.138562091503 },
	{ 0.0 , 0.0 , 0.143790849673 },
	{ 0.0 , 0.0 , 0.149019607843 },
	{ 0.0 , 0.0 , 0.154248366013 },
	{ 0.0 , 0.0 , 0.159477124183 },
	{ 0.0 , 0.0 , 0.164705882353 },
	{ 0.0 , 0.0 , 0.169934640523 },
	{ 0.0 , 0.0 , 0.175163398693 },
	{ 0.0 , 0.0 , 0.180392156863 },
	{ 0.0 , 0.0 , 0.185620915033 },
	{ 0.0 , 0.0 , 0.190849673203 },
	{ 0.0 , 0.0 , 0.196078431373 },
	{ 0.0 , 0.0 , 0.201307189542 },
	{ 0.0 , 0.0 , 0.206535947712 },
	{ 0.0 , 0.0 , 0.211764705882 },
	{ 0.0 , 0.0 , 0.216993464052 },
	{ 0.0 , 0.0 , 0.222222222222 },
	{ 0.0 , 0.0 , 0.227450980392 },
	{ 0.0 , 0.0 , 0.232679738562 },
	{ 0.0 , 0.0 , 0.237908496732 },
	{ 0.0 , 0.0 , 0.243137254902 },
	{ 0.0 , 0.0 , 0.248366013072 },
	{ 0.0 , 0.0 , 0.253594771242 },
	{ 0.0 , 0.0 , 0.258823529412 },
	{ 0.0 , 0.0 , 0.264052287582 },
	{ 0.0 , 0.0 , 0.270588235294 },
	{ 0.0 , 0.0 , 0.278431372549 },
	{ 0.0 , 0.0 , 0.286274509804 },
	{ 0.0 , 0.0 , 0.294117647059 },
	{ 0.0 , 0.0 , 0.301960784314 },
	{ 0.0 , 0.0 , 0.309803921569 },
	{ 0.0 , 0.0 , 0.317647058824 },
	{ 0.0 , 0.0 , 0.325490196078 },
	{ 0.0 , 0.0 , 0.333333333333 },
	{ 0.0 , 0.0 , 0.341176470588 },
	{ 0.0 , 0.0 , 0.349019607843 },
	{ 0.0 , 0.0 , 0.356862745098 },
	{ 0.0 , 0.0 , 0.364705882353 },
	{ 0.0 , 0.0 , 0.372549019608 },
	{ 0.0 , 0.0 , 0.380392156863 },
	{ 0.0 , 0.0 , 0.388235294118 },
	{ 0.0 , 0.0 , 0.396078431373 },
	{ 0.0 , 0.0 , 0.403921568627 },
	{ 0.0 , 0.0 , 0.411764705882 },
	{ 0.0 , 0.0 , 0.419607843137 },
	{ 0.0 , 0.0 , 0.427450980392 },
	{ 0.0 , 0.0 , 0.435294117647 },
	{ 0.0 , 0.0 , 0.443137254902 },
	{ 0.0 , 0.0 , 0.450980392157 },
	{ 0.0 , 0.0 , 0.458823529412 },
	{ 0.0 , 0.0 , 0.466666666667 },
	{ 0.0 , 0.0 , 0.474509803922 },
	{ 0.0 , 0.0 , 0.482352941176 },
	{ 0.0 , 0.0 , 0.490196078431 },
	{ 0.0 , 0.0 , 0.498039215686 },
	{ 0.0 , 0.0 , 0.505882352941 },
	{ 0.0 , 0.0 , 0.513725490196 },
	{ 0.0 , 0.0 , 0.521568627451 },
	{ 0.0 , 0.0 , 0.529411764706 },
	{ 0.0 , 0.0 , 0.537254901961 },
	{ 0.0 , 0.0 , 0.545098039216 },
	{ 0.0 , 0.0 , 0.552941176471 },
	{ 0.0 , 0.0 , 0.560784313725 },
	{ 0.0 , 0.0 , 0.56862745098 },
	{ 0.0 , 0.0 , 0.576470588235 },
	{ 0.0 , 0.0 , 0.58431372549 },
	{ 0.0 , 0.0 , 0.592156862745 },
	{ 0.0 , 0.0 , 0.6 },
	{ 0.0 , 0.0 , 0.607843137255 },
	{ 0.0 , 0.0 , 0.61568627451 },
	{ 0.0 , 0.0 , 0.623529411765 },
	{ 0.0 , 0.0 , 0.63137254902 },
	{ 0.0 , 0.0 , 0.639215686275 },
	{ 0.0 , 0.0 , 0.647058823529 },
	{ 0.0 , 0.0 , 0.654901960784 },
	{ 0.0 , 0.0 , 0.662745098039 },
	{ 0.0 , 0.0 , 0.673202614379 },
	{ 0.0 , 0.0 , 0.686274509804 },
	{ 0.0 , 0.0 , 0.699346405229 },
	{ 0.0 , 0.0 , 0.712418300654 },
	{ 0.0 , 0.0 , 0.725490196078 },
	{ 0.0 , 0.0 , 0.738562091503 },
	{ 0.0 , 0.0 , 0.751633986928 },
	{ 0.0 , 0.0 , 0.764705882353 },
	{ 0.0 , 0.0 , 0.777777777778 },
	{ 0.0 , 0.0 , 0.790849673203 },
	{ 0.0 , 0.0 , 0.803921568627 },
	{ 0.0 , 0.0 , 0.816993464052 },
	{ 0.0 , 0.0 , 0.830065359477 },
	{ 0.0 , 0.0 , 0.843137254902 },
	{ 0.0 , 0.0 , 0.856209150327 },
	{ 0.0 , 0.0 , 0.869281045752 },
	{ 0.0 , 0.0 , 0.882352941176 },
	{ 0.0 , 0.0 , 0.895424836601 },
	{ 0.0 , 0.0 , 0.908496732026 },
	{ 0.0 , 0.0 , 0.921568627451 },
	{ 0.0 , 0.0 , 0.934640522876 },
	{ 0.0 , 0.0 , 0.947712418301 },
	{ 0.0 , 0.0 , 0.960784313725 },
	{ 0.0 , 0.0 , 0.97385620915 },
	{ 0.0 , 0.0 , 0.986928104575 }
	};

	for (int paletteValueInc = 0; paletteValueInc < PICTURE_PALETTE_SIZE; paletteValueInc++)
	{
		// scaling it to make it a bit darker near the bottom.
		r = int( infernoMap[paletteValueInc][0] * 255.0 );
		g = int( infernoMap[paletteValueInc][1] * 255.0 );
		b = int( infernoMap[paletteValueInc][2] * 255.0 );
 		Palette.aEntries[paletteValueInc].peRed = LOBYTE( r );
 		Palette.aEntries[paletteValueInc].peGreen = LOBYTE( g );
 		Palette.aEntries[paletteValueInc].peBlue = LOBYTE( b );
 		Palette.aEntries[paletteValueInc].peFlags = PC_RESERVED;
	}
	palettes[1] = CreatePalette( (LOGPALETTE *)&Palette );
	///
	double blackToWhite[256][3];
	for (int paletteInc = 0; paletteInc < 256; paletteInc++)
	{
		blackToWhite[paletteInc][0] = paletteInc / 256.0;
		blackToWhite[paletteInc][1] = paletteInc / 256.0;
		blackToWhite[paletteInc][2] = paletteInc / 256.0;
	}
	// special values
	blackToWhite[0][0] = 0;
	blackToWhite[0][1] = 0;
	blackToWhite[0][2] = 1;
	// 	
	blackToWhite[255][0] = 1;
	blackToWhite[255][1] = 0;
	blackToWhite[255][2] = 0;
	for (int paletteValueInc = 0; paletteValueInc < PICTURE_PALETTE_SIZE; paletteValueInc++)
	{
		// scaling it to make it a bit darker near the bottom.
		r = int( blackToWhite[paletteValueInc][0] * 255.0 );
		g = int( blackToWhite[paletteValueInc][1] * 255.0 );
		b = int( blackToWhite[paletteValueInc][2] * 255.0 );
		Palette.aEntries[paletteValueInc].peRed = LOBYTE( r );
		Palette.aEntries[paletteValueInc].peGreen = LOBYTE( g );
		Palette.aEntries[paletteValueInc].peBlue = LOBYTE( b );
		Palette.aEntries[paletteValueInc].peFlags = PC_RESERVED;
	}
	palettes[2] = CreatePalette( (LOGPALETTE *)&Palette );

	for ( int paletteValueInc = 0; paletteValueInc < PICTURE_PALETTE_SIZE; paletteValueInc++ )
	{
		// scaling it to make it a bit darker near the bottom.
		r = int( redBlackBlue[paletteValueInc][0] * 255.0 );
		g = int( redBlackBlue[paletteValueInc][1] * 255.0 );
		b = int( redBlackBlue[paletteValueInc][2] * 255.0 );
		Palette.aEntries[paletteValueInc].peRed = LOBYTE( r );
		Palette.aEntries[paletteValueInc].peGreen = LOBYTE( g );
		Palette.aEntries[paletteValueInc].peBlue = LOBYTE( b );
		Palette.aEntries[paletteValueInc].peFlags = PC_RESERVED;
	}
	palettes[3] = CreatePalette( (LOGPALETTE *)&Palette );
	*/
}

