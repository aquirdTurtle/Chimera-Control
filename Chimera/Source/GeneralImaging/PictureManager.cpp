// created by Mark O. Brown
#include "stdafx.h"
#include "PictureManager.h"
#include "ConfigurationSystems/ConfigSystem.h"
#include "Andor/pictureSettingsControl.h"

PictureManager::PictureManager ( bool histOption, std::string configurationFileDelimiter, bool autoscaleDefault, 
	Qt::TransformationMode mode)
	: pictures{ {{histOption, mode}, {false, mode}, {false, mode}, {false, mode} } }, configDelim (configurationFileDelimiter),
	autoScalePictures (autoscaleDefault ){
}

void PictureManager::setSoftwareAccumulationOptions ( std::array<softwareAccumulationOption, 4> opts ){
	for ( auto picInc : range ( 4 ) ){
		pictures[ picInc ].setSoftwareAccumulationOption ( opts[ picInc ] );
	}
}

void PictureManager::drawBitmap (Matrix<long> picData, std::pair<int,int> minMax, unsigned whichPicCtrl,
	std::vector<atomGrid> grids, unsigned pictureNumber, 
	bool includingAnalysisMarkers, QPainter& painter){
	pictures[whichPicCtrl].drawBitmap ( picData, autoScalePictures, minMax.first, minMax.second, specialLessThanMin, 
		specialGreaterThanMax, grids, pictureNumber, includingAnalysisMarkers );
	if (alwaysShowGrid)	{ 
		pictures[whichPicCtrl].drawGrid (painter);
	} 
} 

Matrix<double> PictureManager::getAccumPicData (unsigned whichPicControl) {
	return pictures[whichPicControl].getAccumPicData ();
}

softwareAccumulationOption PictureManager::getSoftwareAccumulationOpt(unsigned whichPicControl) {
	return pictures[whichPicControl].getSoftwareAccumulationOption ();
}

void PictureManager::setPalletes(std::array<int, 4> palleteIds){
	for (int picInc = 0; picInc < 4; picInc++){
		if ( palleteIds[ picInc ] >= 3 ){
			errBox ( "Image Pallete ID out of range! Forcing to 0." );
			palleteIds[ picInc ] = 0;
		}
		pictures[picInc].updatePalette(palettes[palleteIds[picInc]]);
	}
}

void PictureManager::setAlwaysShowGrid(bool showOption, QPainter& painter){
	alwaysShowGrid = showOption;
	if (alwaysShowGrid){
		if (!pictures[1].isActive()){
			pictures[0].drawGrid( painter );
			return;
		}
		for (auto& pic : pictures){
			pic.drawGrid( painter );
		}
	}
}

coordinate PictureManager::getSelLocation (){
	return pictures[0].selectedLocation;
}

void PictureManager::redrawPictures( coordinate selectedLocation, std::vector<atomGrid> gridInfo, bool forceGrid, unsigned picNumber, QPainter& painter ){
	if (!pictures[1].isActive()){
		pictures[0].redrawImage();
		if (alwaysShowGrid || forceGrid ){
			pictures[0].drawGrid(painter);
		}
		return;
	}
	for (auto& pic : pictures){
		pic.redrawImage();
		if (alwaysShowGrid || forceGrid ){
			pic.drawGrid(painter);
		}
	}
}

void PictureManager::setNumberPicturesActive( int numberActive ){
	int count = 1;
	for (auto& pic : pictures){
		pic.setActive( count <= numberActive );
		count++;
	}
}

void PictureManager::setAutoScalePicturesOption(bool autoScaleOption){
	autoScalePictures = autoScaleOption;
}

void PictureManager::handleSaveConfig(ConfigStream& saveFile){
	saveFile << configDelim + "\n/*Slider Locs (Min/Max):*/\n";
	for (auto& pic : pictures){
		std::pair<unsigned, unsigned> sliderLoc = pic.getSliderLocations();
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
	unsigned count = 0;
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



void PictureManager::setSinglePicture( imageParameters imageParams){
	for (unsigned picNum = 0; picNum < 4; picNum++){
		if (picNum < 1){
			pictures[picNum].setActive(true);
		}
		else{
			pictures[picNum].setActive(false);
		}
	}
	pictures.front( ).setPictureArea( picturesLocation, picturesWidth, picturesHeight);
	auto sliderLoc = picturesLocation;
	sliderLoc.rx() += picturesWidth;
	pictures.front( ).setSliderControlLocs(sliderLoc, picturesHeight);
	setParameters( imageParams );
}

void PictureManager::resetPictureStorage(){
	for (auto& pic : pictures){
		pic.resetStorage();
	}
}

void PictureManager::setMultiplePictures( imageParameters imageParams, unsigned numberActivePics ){
	for (unsigned picNum = 0; picNum < 4; picNum++){
		if (picNum < numberActivePics){
			pictures[picNum].setActive(true);
		}
		else{
			pictures[picNum].setActive(false);
		}
	}

	QPoint loc = picturesLocation;
	auto& px = loc.rx (), & py = loc.ry ();
	// Square: width = 550, height = 440
	auto picWidth = 950;
	auto totalHeight = 220 * 4;
	auto picHeight = totalHeight / numberActivePics;
	//auto picHeight = 220;
	//int picWidth = 550;
	//int picHeight = 420;
	pictures[0].setPictureArea( loc, picWidth, picHeight );
	pictures[0].setSliderControlLocs ({ px + picWidth,py }, picHeight);
	//px += 550;
	py += picHeight + 25;
	pictures[1].setPictureArea( loc, picWidth, picHeight );
	pictures[1].setSliderControlLocs ({ px + picWidth,py }, picHeight);
	//px -= 550;
	py += picHeight + 25;
	pictures[2].setPictureArea( loc, picWidth, picHeight );
	pictures[2].setSliderControlLocs ({ px + picWidth,py }, picHeight);
	//px += 550;
	py += picHeight + 25;
	pictures[3].setPictureArea( loc, picWidth, picHeight );
	pictures[3].setSliderControlLocs ({ px + picWidth,py }, picHeight);
	setParameters( imageParams );
	for ( auto& pic : pictures ){
		//pic.setCursorValueLocations( parent );
	}
	setPalletes ({ 0,0,0,0 });
}

void PictureManager::setScaleFactor (int sf) {
	for (auto& pic : pictures) {
		pic.setPicScaleFactor (sf);
	}
}

void PictureManager::initialize( QPoint& loc, int manWidth, int manHeight, IChimeraQtWindow* widget, int scaleFactor){
	auto& px = loc.rx (), & py = loc.ry ();
	picturesLocation = loc;
	picturesWidth = manWidth;
	picturesHeight = manHeight;
	// Square: width = 550, height = 440
	auto width = 1200;
	auto height = 220;
	pictures[0].initialize( loc, width, height, widget, this, scaleFactor);
	py += height;
	pictures[1].initialize( loc, width, height, widget, this, scaleFactor);
	py += height;
	pictures[2].initialize( loc, width, height, widget, this, scaleFactor);
	py += height;
	pictures[3].initialize( loc, width, height, widget, this, scaleFactor);
	py += height;
	createPalettes ();
	for (auto& pic : pictures){
		pic.updatePalette( inferno );
	}
	// initialize to one. This matches the camera settings initialization.
	setNumberPicturesActive( 1 );
}


void PictureManager::updatePlotData ( ){
	for ( auto& pic : pictures ){
		pic.updatePlotData ( );
	}
}

unsigned PictureManager::getNumberActive( ){
	unsigned count = 0;
	for ( auto& pic : pictures ){
		if ( pic.isActive( ) ){
			count++;
		}
	}
	return count;
}

void PictureManager::drawGrids(QPainter& painter){
	for (auto& picture : pictures){
		picture.drawGrid( painter );
	}
}


void PictureManager::setParameters(imageParameters parameters){
	for (auto& picture : pictures){
		picture.recalculateGrid(parameters);
	}
}

void PictureManager::createPalettes( ){
	// greys
	palettes[2] = QVector<QRgb>{
		qRgb (255 , 255 , 255),
		qRgb (254 , 254 , 254),
		qRgb (254 , 254 , 254),
		qRgb (253 , 253 , 253),
		qRgb (253 , 253 , 253),
		qRgb (252 , 252 , 252),
		qRgb (252 , 252 , 252),
		qRgb (251 , 251 , 251),
		qRgb (251 , 251 , 251),
		qRgb (250 , 250 , 250),
		qRgb (250 , 250 , 250),
		qRgb (249 , 249 , 249),
		qRgb (249 , 249 , 249),
		qRgb (248 , 248 , 248),
		qRgb (248 , 248 , 248),
		qRgb (247 , 247 , 247),
		qRgb (247 , 247 , 247),
		qRgb (247 , 247 , 247),
		qRgb (246 , 246 , 246),
		qRgb (246 , 246 , 246),
		qRgb (245 , 245 , 245),
		qRgb (245 , 245 , 245),
		qRgb (244 , 244 , 244),
		qRgb (244 , 244 , 244),
		qRgb (243 , 243 , 243),
		qRgb (243 , 243 , 243),
		qRgb (242 , 242 , 242),
		qRgb (242 , 242 , 242),
		qRgb (241 , 241 , 241),
		qRgb (241 , 241 , 241),
		qRgb (240 , 240 , 240),
		qRgb (240 , 240 , 240),
		qRgb (239 , 239 , 239),
		qRgb (239 , 239 , 239),
		qRgb (238 , 238 , 238),
		qRgb (237 , 237 , 237),
		qRgb (237 , 237 , 237),
		qRgb (236 , 236 , 236),
		qRgb (235 , 235 , 235),
		qRgb (234 , 234 , 234),
		qRgb (234 , 234 , 234),
		qRgb (233 , 233 , 233),
		qRgb (232 , 232 , 232),
		qRgb (231 , 231 , 231),
		qRgb (231 , 231 , 231),
		qRgb (230 , 230 , 230),
		qRgb (229 , 229 , 229),
		qRgb (229 , 229 , 229),
		qRgb (228 , 228 , 228),
		qRgb (227 , 227 , 227),
		qRgb (226 , 226 , 226),
		qRgb (226 , 226 , 226),
		qRgb (225 , 225 , 225),
		qRgb (224 , 224 , 224),
		qRgb (224 , 224 , 224),
		qRgb (223 , 223 , 223),
		qRgb (222 , 222 , 222),
		qRgb (221 , 221 , 221),
		qRgb (221 , 221 , 221),
		qRgb (220 , 220 , 220),
		qRgb (219 , 219 , 219),
		qRgb (218 , 218 , 218),
		qRgb (218 , 218 , 218),
		qRgb (217 , 217 , 217),
		qRgb (216 , 216 , 216),
		qRgb (215 , 215 , 215),
		qRgb (215 , 215 , 215),
		qRgb (214 , 214 , 214),
		qRgb (213 , 213 , 213),
		qRgb (212 , 212 , 212),
		qRgb (211 , 211 , 211),
		qRgb (210 , 210 , 210),
		qRgb (209 , 209 , 209),
		qRgb (208 , 208 , 208),
		qRgb (207 , 207 , 207),
		qRgb (207 , 207 , 207),
		qRgb (206 , 206 , 206),
		qRgb (205 , 205 , 205),
		qRgb (204 , 204 , 204),
		qRgb (203 , 203 , 203),
		qRgb (202 , 202 , 202),
		qRgb (201 , 201 , 201),
		qRgb (200 , 200 , 200),
		qRgb (200 , 200 , 200),
		qRgb (199 , 199 , 199),
		qRgb (198 , 198 , 198),
		qRgb (197 , 197 , 197),
		qRgb (196 , 196 , 196),
		qRgb (195 , 195 , 195),
		qRgb (194 , 194 , 194),
		qRgb (193 , 193 , 193),
		qRgb (193 , 193 , 193),
		qRgb (192 , 192 , 192),
		qRgb (191 , 191 , 191),
		qRgb (190 , 190 , 190),
		qRgb (189 , 189 , 189),
		qRgb (188 , 188 , 188),
		qRgb (187 , 187 , 187),
		qRgb (186 , 186 , 186),
		qRgb (184 , 184 , 184),
		qRgb (183 , 183 , 183),
		qRgb (182 , 182 , 182),
		qRgb (181 , 181 , 181),
		qRgb (179 , 179 , 179),
		qRgb (178 , 178 , 178),
		qRgb (177 , 177 , 177),
		qRgb (176 , 176 , 176),
		qRgb (175 , 175 , 175),
		qRgb (173 , 173 , 173),
		qRgb (172 , 172 , 172),
		qRgb (171 , 171 , 171),
		qRgb (170 , 170 , 170),
		qRgb (168 , 168 , 168),
		qRgb (167 , 167 , 167),
		qRgb (166 , 166 , 166),
		qRgb (165 , 165 , 165),
		qRgb (164 , 164 , 164),
		qRgb (162 , 162 , 162),
		qRgb (161 , 161 , 161),
		qRgb (160 , 160 , 160),
		qRgb (159 , 159 , 159),
		qRgb (157 , 157 , 157),
		qRgb (156 , 156 , 156),
		qRgb (155 , 155 , 155),
		qRgb (154 , 154 , 154),
		qRgb (153 , 153 , 153),
		qRgb (151 , 151 , 151),
		qRgb (150 , 150 , 150),
		qRgb (149 , 149 , 149),
		qRgb (148 , 148 , 148),
		qRgb (147 , 147 , 147),
		qRgb (146 , 146 , 146),
		qRgb (145 , 145 , 145),
		qRgb (143 , 143 , 143),
		qRgb (142 , 142 , 142),
		qRgb (141 , 141 , 141),
		qRgb (140 , 140 , 140),
		qRgb (139 , 139 , 139),
		qRgb (138 , 138 , 138),
		qRgb (137 , 137 , 137),
		qRgb (136 , 136 , 136),
		qRgb (135 , 135 , 135),
		qRgb (134 , 134 , 134),
		qRgb (132 , 132 , 132),
		qRgb (131 , 131 , 131),
		qRgb (130 , 130 , 130),
		qRgb (129 , 129 , 129),
		qRgb (128 , 128 , 128),
		qRgb (127 , 127 , 127),
		qRgb (126 , 126 , 126),
		qRgb (125 , 125 , 125),
		qRgb (124 , 124 , 124),
		qRgb (123 , 123 , 123),
		qRgb (122 , 122 , 122),
		qRgb (120 , 120 , 120),
		qRgb (119 , 119 , 119),
		qRgb (118 , 118 , 118),
		qRgb (117 , 117 , 117),
		qRgb (116 , 116 , 116),
		qRgb (115 , 115 , 115),
		qRgb (114 , 114 , 114),
		qRgb (113 , 113 , 113),
		qRgb (112 , 112 , 112),
		qRgb (111 , 111 , 111),
		qRgb (110 , 110 , 110),
		qRgb (109 , 109 , 109),
		qRgb (108 , 108 , 108),
		qRgb (107 , 107 , 107),
		qRgb (106 , 106 , 106),
		qRgb (105 , 105 , 105),
		qRgb (104 , 104 , 104),
		qRgb (102 , 102 , 102),
		qRgb (101 , 101 , 101),
		qRgb (100 , 100 , 100),
		qRgb (99 , 99 , 99),
		qRgb (98 , 98 , 98),
		qRgb (97 , 97 , 97),
		qRgb (96 , 96 , 96),
		qRgb (95 , 95 , 95),
		qRgb (94 , 94 , 94),
		qRgb (93 , 93 , 93),
		qRgb (92 , 92 , 92),
		qRgb (91 , 91 , 91),
		qRgb (90 , 90 , 90),
		qRgb (89 , 89 , 89),
		qRgb (88 , 88 , 88),
		qRgb (87 , 87 , 87),
		qRgb (86 , 86 , 86),
		qRgb (85 , 85 , 85),
		qRgb (84 , 84 , 84),
		qRgb (83 , 83 , 83),
		qRgb (82 , 82 , 82),
		qRgb (80 , 80 , 80),
		qRgb (79 , 79 , 79),
		qRgb (78 , 78 , 78),
		qRgb (76 , 76 , 76),
		qRgb (75 , 75 , 75),
		qRgb (73 , 73 , 73),
		qRgb (72 , 72 , 72),
		qRgb (71 , 71 , 71),
		qRgb (69 , 69 , 69),
		qRgb (68 , 68 , 68),
		qRgb (66 , 66 , 66),
		qRgb (65 , 65 , 65),
		qRgb (64 , 64 , 64),
		qRgb (62 , 62 , 62),
		qRgb (61 , 61 , 61),
		qRgb (59 , 59 , 59),
		qRgb (58 , 58 , 58),
		qRgb (56 , 56 , 56),
		qRgb (55 , 55 , 55),
		qRgb (54 , 54 , 54),
		qRgb (52 , 52 , 52),
		qRgb (51 , 51 , 51),
		qRgb (49 , 49 , 49),
		qRgb (48 , 48 , 48),
		qRgb (47 , 47 , 47),
		qRgb (45 , 45 , 45),
		qRgb (44 , 44 , 44),
		qRgb (42 , 42 , 42),
		qRgb (41 , 41 , 41),
		qRgb (40 , 40 , 40),
		qRgb (38 , 38 , 38),
		qRgb (37 , 37 , 37),
		qRgb (35 , 35 , 35),
		qRgb (34 , 34 , 34),
		qRgb (33 , 33 , 33),
		qRgb (32 , 32 , 32),
		qRgb (31 , 31 , 31),
		qRgb (30 , 30 , 30),
		qRgb (29 , 29 , 29),
		qRgb (27 , 27 , 27),
		qRgb (26 , 26 , 26),
		qRgb (25 , 25 , 25),
		qRgb (24 , 24 , 24),
		qRgb (23 , 23 , 23),
		qRgb (22 , 22 , 22),
		qRgb (20 , 20 , 20),
		qRgb (19 , 19 , 19),
		qRgb (18 , 18 , 18),
		qRgb (17 , 17 , 17),
		qRgb (16 , 16 , 16),
		qRgb (15 , 15 , 15),
		qRgb (13 , 13 , 13),
		qRgb (12 , 12 , 12),
		qRgb (11 , 11 , 11),
		qRgb (10 , 10 , 10),
		qRgb (9 , 9 , 9),
		qRgb (8 , 8 , 8),
		qRgb (6 , 6 , 6),
		qRgb (5 , 5 , 5),
		qRgb (4 , 4 , 4),
		qRgb (3 , 3 , 3),
		qRgb (2 , 2 , 2),
		qRgb (1 , 1 , 1),
		qRgb (0 , 0 , 0),
	};
	palettes[1] = QVector<QRgb>{
		qRgb (0 , 0 , 3),
		qRgb (0 , 0 , 4),
		qRgb (0 , 0 , 6),
		qRgb (1 , 0 , 7),
		qRgb (1 , 1 , 9),
		qRgb (1 , 1 , 11),
		qRgb (2 , 1 , 14),
		qRgb (2 , 2 , 16),
		qRgb (3 , 2 , 18),
		qRgb (4 , 3 , 20),
		qRgb (4 , 3 , 22),
		qRgb (5 , 4 , 24),
		qRgb (6 , 4 , 27),
		qRgb (7 , 5 , 29),
		qRgb (8 , 6 , 31),
		qRgb (9 , 6 , 33),
		qRgb (10 , 7 , 35),
		qRgb (11 , 7 , 38),
		qRgb (13 , 8 , 40),
		qRgb (14 , 8 , 42),
		qRgb (15 , 9 , 45),
		qRgb (16 , 9 , 47),
		qRgb (18 , 10 , 50),
		qRgb (19 , 10 , 52),
		qRgb (20 , 11 , 54),
		qRgb (22 , 11 , 57),
		qRgb (23 , 11 , 59),
		qRgb (25 , 11 , 62),
		qRgb (26 , 11 , 64),
		qRgb (28 , 12 , 67),
		qRgb (29 , 12 , 69),
		qRgb (31 , 12 , 71),
		qRgb (32 , 12 , 74),
		qRgb (34 , 11 , 76),
		qRgb (36 , 11 , 78),
		qRgb (38 , 11 , 80),
		qRgb (39 , 11 , 82),
		qRgb (41 , 11 , 84),
		qRgb (43 , 10 , 86),
		qRgb (45 , 10 , 88),
		qRgb (46 , 10 , 90),
		qRgb (48 , 10 , 92),
		qRgb (50 , 9 , 93),
		qRgb (52 , 9 , 95),
		qRgb (53 , 9 , 96),
		qRgb (55 , 9 , 97),
		qRgb (57 , 9 , 98),
		qRgb (59 , 9 , 100),
		qRgb (60 , 9 , 101),
		qRgb (62 , 9 , 102),
		qRgb (64 , 9 , 102),
		qRgb (65 , 9 , 103),
		qRgb (67 , 10 , 104),
		qRgb (69 , 10 , 105),
		qRgb (70 , 10 , 105),
		qRgb (72 , 11 , 106),
		qRgb (74 , 11 , 106),
		qRgb (75 , 12 , 107),
		qRgb (77 , 12 , 107),
		qRgb (79 , 13 , 108),
		qRgb (80 , 13 , 108),
		qRgb (82 , 14 , 108),
		qRgb (83 , 14 , 109),
		qRgb (85 , 15 , 109),
		qRgb (87 , 15 , 109),
		qRgb (88 , 16 , 109),
		qRgb (90 , 17 , 109),
		qRgb (91 , 17 , 110),
		qRgb (93 , 18 , 110),
		qRgb (95 , 18 , 110),
		qRgb (96 , 19 , 110),
		qRgb (98 , 20 , 110),
		qRgb (99 , 20 , 110),
		qRgb (101 , 21 , 110),
		qRgb (102 , 21 , 110),
		qRgb (104 , 22 , 110),
		qRgb (106 , 23 , 110),
		qRgb (107 , 23 , 110),
		qRgb (109 , 24 , 110),
		qRgb (110 , 24 , 110),
		qRgb (112 , 25 , 110),
		qRgb (114 , 25 , 109),
		qRgb (115 , 26 , 109),
		qRgb (117 , 27 , 109),
		qRgb (118 , 27 , 109),
		qRgb (120 , 28 , 109),
		qRgb (122 , 28 , 109),
		qRgb (123 , 29 , 108),
		qRgb (125 , 29 , 108),
		qRgb (126 , 30 , 108),
		qRgb (128 , 31 , 107),
		qRgb (129 , 31 , 107),
		qRgb (131 , 32 , 107),
		qRgb (133 , 32 , 106),
		qRgb (134 , 33 , 106),
		qRgb (136 , 33 , 106),
		qRgb (137 , 34 , 105),
		qRgb (139 , 34 , 105),
		qRgb (141 , 35 , 105),
		qRgb (142 , 36 , 104),
		qRgb (144 , 36 , 104),
		qRgb (145 , 37 , 103),
		qRgb (147 , 37 , 103),
		qRgb (149 , 38 , 102),
		qRgb (150 , 38 , 102),
		qRgb (152 , 39 , 101),
		qRgb (153 , 40 , 100),
		qRgb (155 , 40 , 100),
		qRgb (156 , 41 , 99),
		qRgb (158 , 41 , 99),
		qRgb (160 , 42 , 98),
		qRgb (161 , 43 , 97),
		qRgb (163 , 43 , 97),
		qRgb (164 , 44 , 96),
		qRgb (166 , 44 , 95),
		qRgb (167 , 45 , 95),
		qRgb (169 , 46 , 94),
		qRgb (171 , 46 , 93),
		qRgb (172 , 47 , 92),
		qRgb (174 , 48 , 91),
		qRgb (175 , 49 , 91),
		qRgb (177 , 49 , 90),
		qRgb (178 , 50 , 89),
		qRgb (180 , 51 , 88),
		qRgb (181 , 51 , 87),
		qRgb (183 , 52 , 86),
		qRgb (184 , 53 , 86),
		qRgb (186 , 54 , 85),
		qRgb (187 , 55 , 84),
		qRgb (189 , 55 , 83),
		qRgb (190 , 56 , 82),
		qRgb (191 , 57 , 81),
		qRgb (193 , 58 , 80),
		qRgb (194 , 59 , 79),
		qRgb (196 , 60 , 78),
		qRgb (197 , 61 , 77),
		qRgb (199 , 62 , 76),
		qRgb (200 , 62 , 75),
		qRgb (201 , 63 , 74),
		qRgb (203 , 64 , 73),
		qRgb (204 , 65 , 72),
		qRgb (205 , 66 , 71),
		qRgb (207 , 68 , 70),
		qRgb (208 , 69 , 68),
		qRgb (209 , 70 , 67),
		qRgb (210 , 71 , 66),
		qRgb (212 , 72 , 65),
		qRgb (213 , 73 , 64),
		qRgb (214 , 74 , 63),
		qRgb (215 , 75 , 62),
		qRgb (217 , 77 , 61),
		qRgb (218 , 78 , 59),
		qRgb (219 , 79 , 58),
		qRgb (220 , 80 , 57),
		qRgb (221 , 82 , 56),
		qRgb (222 , 83 , 55),
		qRgb (223 , 84 , 54),
		qRgb (224 , 86 , 52),
		qRgb (226 , 87 , 51),
		qRgb (227 , 88 , 50),
		qRgb (228 , 90 , 49),
		qRgb (229 , 91 , 48),
		qRgb (230 , 92 , 46),
		qRgb (230 , 94 , 45),
		qRgb (231 , 95 , 44),
		qRgb (232 , 97 , 43),
		qRgb (233 , 98 , 42),
		qRgb (234 , 100 , 40),
		qRgb (235 , 101 , 39),
		qRgb (236 , 103 , 38),
		qRgb (237 , 104 , 37),
		qRgb (237 , 106 , 35),
		qRgb (238 , 108 , 34),
		qRgb (239 , 109 , 33),
		qRgb (240 , 111 , 31),
		qRgb (240 , 112 , 30),
		qRgb (241 , 114 , 29),
		qRgb (242 , 116 , 28),
		qRgb (242 , 117 , 26),
		qRgb (243 , 119 , 25),
		qRgb (243 , 121 , 24),
		qRgb (244 , 122 , 22),
		qRgb (245 , 124 , 21),
		qRgb (245 , 126 , 20),
		qRgb (246 , 128 , 18),
		qRgb (246 , 129 , 17),
		qRgb (247 , 131 , 16),
		qRgb (247 , 133 , 14),
		qRgb (248 , 135 , 13),
		qRgb (248 , 136 , 12),
		qRgb (248 , 138 , 11),
		qRgb (249 , 140 , 9),
		qRgb (249 , 142 , 8),
		qRgb (249 , 144 , 8),
		qRgb (250 , 145 , 7),
		qRgb (250 , 147 , 6),
		qRgb (250 , 149 , 6),
		qRgb (250 , 151 , 6),
		qRgb (251 , 153 , 6),
		qRgb (251 , 155 , 6),
		qRgb (251 , 157 , 6),
		qRgb (251 , 158 , 7),
		qRgb (251 , 160 , 7),
		qRgb (251 , 162 , 8),
		qRgb (251 , 164 , 10),
		qRgb (251 , 166 , 11),
		qRgb (251 , 168 , 13),
		qRgb (251 , 170 , 14),
		qRgb (251 , 172 , 16),
		qRgb (251 , 174 , 18),
		qRgb (251 , 176 , 20),
		qRgb (251 , 177 , 22),
		qRgb (251 , 179 , 24),
		qRgb (251 , 181 , 26),
		qRgb (251 , 183 , 28),
		qRgb (251 , 185 , 30),
		qRgb (250 , 187 , 33),
		qRgb (250 , 189 , 35),
		qRgb (250 , 191 , 37),
		qRgb (250 , 193 , 40),
		qRgb (249 , 195 , 42),
		qRgb (249 , 197 , 44),
		qRgb (249 , 199 , 47),
		qRgb (248 , 201 , 49),
		qRgb (248 , 203 , 52),
		qRgb (248 , 205 , 55),
		qRgb (247 , 207 , 58),
		qRgb (247 , 209 , 60),
		qRgb (246 , 211 , 63),
		qRgb (246 , 213 , 66),
		qRgb (245 , 215 , 69),
		qRgb (245 , 217 , 72),
		qRgb (244 , 219 , 75),
		qRgb (244 , 220 , 79),
		qRgb (243 , 222 , 82),
		qRgb (243 , 224 , 86),
		qRgb (243 , 226 , 89),
		qRgb (242 , 228 , 93),
		qRgb (242 , 230 , 96),
		qRgb (241 , 232 , 100),
		qRgb (241 , 233 , 104),
		qRgb (241 , 235 , 108),
		qRgb (241 , 237 , 112),
		qRgb (241 , 238 , 116),
		qRgb (241 , 240 , 121),
		qRgb (241 , 242 , 125),
		qRgb (242 , 243 , 129),
		qRgb (242 , 244 , 133),
		qRgb (243 , 246 , 137),
		qRgb (244 , 247 , 141),
		qRgb (245 , 248 , 145),
		qRgb (246 , 250 , 149),
		qRgb (247 , 251 , 153),
		qRgb (249 , 252 , 157),
		qRgb (250 , 253 , 160),
		qRgb (252 , 254 , 164)
	};
	palettes[0] = QVector<QRgb>{
		qRgb (10 , 0 , 12),
		qRgb (10 , 0 , 13),
		qRgb (10 , 0 , 13),
		qRgb (11 , 0 , 14),
		qRgb (11 , 1 , 14),
		qRgb (11 , 1 , 15),
		qRgb (11 , 1 , 15),
		qRgb (12 , 1 , 16),
		qRgb (12 , 2 , 16),
		qRgb (12 , 2 , 17),
		qRgb (13 , 2 , 18),
		qRgb (13 , 3 , 18),
		qRgb (13 , 3 , 19),
		qRgb (13 , 3 , 19),
		qRgb (14 , 4 , 20),
		qRgb (14 , 4 , 21),
		qRgb (14 , 4 , 21),
		qRgb (14 , 5 , 22),
		qRgb (15 , 5 , 22),
		qRgb (15 , 6 , 23),
		qRgb (15 , 6 , 24),
		qRgb (15 , 6 , 24),
		qRgb (16 , 7 , 25),
		qRgb (16 , 7 , 25),
		qRgb (16 , 7 , 26),
		qRgb (16 , 8 , 27),
		qRgb (17 , 8 , 27),
		qRgb (17 , 9 , 28),
		qRgb (17 , 9 , 29),
		qRgb (17 , 10 , 29),
		qRgb (17 , 10 , 30),
		qRgb (18 , 11 , 31),
		qRgb (18 , 11 , 31),
		qRgb (18 , 11 , 32),
		qRgb (18 , 12 , 32),
		qRgb (18 , 12 , 33),
		qRgb (18 , 13 , 34),
		qRgb (19 , 13 , 34),
		qRgb (19 , 14 , 35),
		qRgb (19 , 14 , 36),
		qRgb (19 , 15 , 36),
		qRgb (19 , 15 , 37),
		qRgb (19 , 16 , 37),
		qRgb (19 , 17 , 38),
		qRgb (20 , 17 , 39),
		qRgb (20 , 18 , 39),
		qRgb (20 , 18 , 40),
		qRgb (20 , 19 , 40),
		qRgb (20 , 19 , 41),
		qRgb (20 , 20 , 42),
		qRgb (20 , 21 , 42),
		qRgb (20 , 21 , 43),
		qRgb (20 , 22 , 43),
		qRgb (20 , 22 , 44),
		qRgb (20 , 23 , 44),
		qRgb (20 , 24 , 45),
		qRgb (21 , 24 , 46),
		qRgb (21 , 25 , 46),
		qRgb (21 , 25 , 47),
		qRgb (21 , 26 , 47),
		qRgb (21 , 27 , 48),
		qRgb (21 , 27 , 48),
		qRgb (21 , 28 , 49),
		qRgb (21 , 29 , 50),
		qRgb (21 , 29 , 50),
		qRgb (21 , 30 , 51),
		qRgb (21 , 31 , 51),
		qRgb (21 , 31 , 52),
		qRgb (21 , 32 , 52),
		qRgb (21 , 33 , 53),
		qRgb (21 , 33 , 53),
		qRgb (21 , 34 , 54),
		qRgb (21 , 35 , 54),
		qRgb (21 , 36 , 55),
		qRgb (21 , 36 , 55),
		qRgb (21 , 37 , 56),
		qRgb (21 , 38 , 56),
		qRgb (21 , 38 , 57),
		qRgb (21 , 39 , 57),
		qRgb (21 , 40 , 58),
		qRgb (21 , 41 , 58),
		qRgb (21 , 41 , 59),
		qRgb (21 , 42 , 59),
		qRgb (21 , 43 , 60),
		qRgb (21 , 44 , 60),
		qRgb (21 , 44 , 61),
		qRgb (21 , 45 , 61),
		qRgb (21 , 46 , 62),
		qRgb (20 , 47 , 62),
		qRgb (20 , 48 , 63),
		qRgb (20 , 48 , 63),
		qRgb (20 , 49 , 64),
		qRgb (20 , 50 , 64),
		qRgb (20 , 51 , 65),
		qRgb (20 , 52 , 65),
		qRgb (20 , 52 , 66),
		qRgb (20 , 53 , 66),
		qRgb (20 , 54 , 67),
		qRgb (20 , 55 , 67),
		qRgb (20 , 56 , 68),
		qRgb (20 , 57 , 68),
		qRgb (20 , 58 , 69),
		qRgb (20 , 58 , 69),
		qRgb (20 , 59 , 70),
		qRgb (20 , 60 , 70),
		qRgb (20 , 61 , 71),
		qRgb (20 , 62 , 71),
		qRgb (20 , 63 , 72),
		qRgb (20 , 64 , 72),
		qRgb (20 , 65 , 73),
		qRgb (20 , 65 , 73),
		qRgb (19 , 66 , 73),
		qRgb (19 , 67 , 74),
		qRgb (19 , 68 , 74),
		qRgb (19 , 69 , 75),
		qRgb (19 , 70 , 75),
		qRgb (19 , 71 , 76),
		qRgb (19 , 72 , 76),
		qRgb (19 , 73 , 76),
		qRgb (19 , 74 , 77),
		qRgb (19 , 75 , 77),
		qRgb (19 , 76 , 78),
		qRgb (19 , 77 , 78),
		qRgb (19 , 78 , 79),
		qRgb (19 , 79 , 79),
		qRgb (18 , 80 , 79),
		qRgb (18 , 81 , 80),
		qRgb (18 , 82 , 80),
		qRgb (18 , 83 , 80),
		qRgb (18 , 84 , 81),
		qRgb (18 , 85 , 81),
		qRgb (18 , 86 , 82),
		qRgb (18 , 87 , 82),
		qRgb (18 , 88 , 82),
		qRgb (18 , 89 , 83),
		qRgb (18 , 90 , 83),
		qRgb (18 , 91 , 83),
		qRgb (18 , 92 , 83),
		qRgb (18 , 93 , 84),
		qRgb (18 , 95 , 84),
		qRgb (18 , 96 , 84),
		qRgb (18 , 97 , 85),
		qRgb (19 , 98 , 85),
		qRgb (19 , 99 , 85),
		qRgb (19 , 100 , 85),
		qRgb (19 , 101 , 85),
		qRgb (19 , 102 , 86),
		qRgb (20 , 103 , 86),
		qRgb (20 , 105 , 86),
		qRgb (20 , 106 , 86),
		qRgb (21 , 107 , 86),
		qRgb (21 , 108 , 86),
		qRgb (22 , 109 , 87),
		qRgb (22 , 110 , 87),
		qRgb (23 , 112 , 87),
		qRgb (23 , 113 , 87),
		qRgb (24 , 114 , 87),
		qRgb (25 , 115 , 87),
		qRgb (25 , 116 , 87),
		qRgb (26 , 117 , 87),
		qRgb (27 , 119 , 87),
		qRgb (28 , 120 , 87),
		qRgb (29 , 121 , 87),
		qRgb (30 , 122 , 87),
		qRgb (31 , 123 , 87),
		qRgb (32 , 125 , 87),
		qRgb (33 , 126 , 87),
		qRgb (34 , 127 , 86),
		qRgb (35 , 128 , 86),
		qRgb (36 , 130 , 86),
		qRgb (38 , 131 , 86),
		qRgb (39 , 132 , 86),
		qRgb (40 , 133 , 85),
		qRgb (41 , 135 , 85),
		qRgb (43 , 136 , 85),
		qRgb (44 , 137 , 85),
		qRgb (46 , 138 , 84),
		qRgb (47 , 140 , 84),
		qRgb (49 , 141 , 84),
		qRgb (50 , 142 , 83),
		qRgb (52 , 143 , 83),
		qRgb (54 , 145 , 83),
		qRgb (55 , 146 , 82),
		qRgb (57 , 147 , 82),
		qRgb (59 , 148 , 81),
		qRgb (60 , 150 , 81),
		qRgb (62 , 151 , 80),
		qRgb (64 , 152 , 80),
		qRgb (66 , 153 , 79),
		qRgb (68 , 155 , 79),
		qRgb (70 , 156 , 78),
		qRgb (72 , 157 , 77),
		qRgb (74 , 158 , 77),
		qRgb (76 , 160 , 76),
		qRgb (78 , 161 , 75),
		qRgb (80 , 162 , 75),
		qRgb (82 , 163 , 74),
		qRgb (84 , 165 , 73),
		qRgb (87 , 166 , 72),
		qRgb (89 , 167 , 71),
		qRgb (91 , 168 , 71),
		qRgb (93 , 170 , 70),
		qRgb (96 , 171 , 69),
		qRgb (98 , 172 , 68),
		qRgb (101 , 173 , 67),
		qRgb (103 , 175 , 66),
		qRgb (105 , 176 , 65),
		qRgb (108 , 177 , 64),
		qRgb (110 , 178 , 63),
		qRgb (113 , 179 , 62),
		qRgb (116 , 181 , 61),
		qRgb (118 , 182 , 59),
		qRgb (121 , 183 , 58),
		qRgb (124 , 184 , 57),
		qRgb (126 , 185 , 56),
		qRgb (129 , 187 , 55),
		qRgb (132 , 188 , 53),
		qRgb (134 , 189 , 52),
		qRgb (137 , 190 , 51),
		qRgb (140 , 191 , 50),
		qRgb (143 , 192 , 48),
		qRgb (146 , 194 , 47),
		qRgb (149 , 195 , 46),
		qRgb (152 , 196 , 44),
		qRgb (155 , 197 , 43),
		qRgb (158 , 198 , 41),
		qRgb (161 , 199 , 40),
		qRgb (164 , 200 , 39),
		qRgb (167 , 201 , 37),
		qRgb (170 , 203 , 36),
		qRgb (173 , 204 , 34),
		qRgb (176 , 205 , 33),
		qRgb (179 , 206 , 32),
		qRgb (182 , 207 , 30),
		qRgb (185 , 208 , 29),
		qRgb (188 , 209 , 28),
		qRgb (192 , 210 , 27),
		qRgb (195 , 211 , 26),
		qRgb (198 , 212 , 25),
		qRgb (201 , 213 , 24),
		qRgb (204 , 214 , 24),
		qRgb (208 , 216 , 23),
		qRgb (211 , 217 , 23),
		qRgb (214 , 218 , 23),
		qRgb (217 , 219 , 23),
		qRgb (220 , 220 , 23),
		qRgb (224 , 221 , 24),
		qRgb (227 , 222 , 24),
		qRgb (230 , 223 , 25),
		qRgb (233 , 224 , 27),
		qRgb (237 , 225 , 28),
		qRgb (240 , 226 , 29),
		qRgb (243 , 227 , 31),
		qRgb (246 , 228 , 32),
		qRgb (250 , 229 , 34),
		qRgb (253 , 231 , 36)
	};
}


void PictureManager::setTransformationMode (Qt::TransformationMode mode) {
	for (auto& pic : pictures) {
		pic.setTransformationMode (mode);
	}
}

