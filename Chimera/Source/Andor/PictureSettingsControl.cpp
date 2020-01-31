// created by Mark O. Brown

#include "stdafx.h"
#include "PictureSettingsControl.h"
#include "Andor/Andor.h"
#include "Andor/CameraSettingsControl.h"
#include "PrimaryWindows/AndorWindow.h"
#include "Commctrl.h"
#include <boost/lexical_cast.hpp>


void PictureSettingsControl::initialize( cameraPositions& pos, CWnd* parent, int& id )
{
	// introducing things row by row
	/// Set Picture Options
	UINT count = 0;
	/// Picture Numbers
	pictureLabel.setPositions ( pos, 0, 0, 100, 20, false, false, true );
	pictureLabel.Create( "Picture #:", NORM_STATIC_OPTIONS, pictureLabel.seriesPos, parent, 
						 PICTURE_SETTINGS_ID_START + count++ );
	pictureLabel.fontType = fontTypes::SmallFont;

	for ( auto picInc : range(4) )
	{
		pictureNumbers[ picInc ].setPositions ( pos, 100 + 95 * picInc, 0, 95, 20, picInc == 3, false, true );
		pictureNumbers[picInc].Create( cstr( picInc + 1 ), NORM_STATIC_OPTIONS, pictureNumbers[picInc].seriesPos, 
									   parent, PICTURE_SETTINGS_ID_START + count++ );
	}

	/// Total picture number
	totalPicNumberLabel.setPositions ( pos, 0, 0, 100, 20, false, false, true );
	totalPicNumberLabel.Create( "Total Picture #", NORM_STATIC_OPTIONS, totalPicNumberLabel.seriesPos, parent,
								PICTURE_SETTINGS_ID_START + count++ );
	totalPicNumberLabel.fontType = fontTypes::SmallFont;
	for ( auto picInc : range ( 4 ) )
	{
		totalNumberChoice[ picInc ].setPositions ( pos, 100 + 95 * picInc, 0, 95, 20, picInc == 3, false, true );
		totalNumberChoice[ picInc ].Create ( "", ( picInc == 0 ) ? ( NORM_RADIO_OPTIONS | WS_GROUP ) : NORM_RADIO_OPTIONS,
											 totalNumberChoice[ picInc ].seriesPos, parent, PICTURE_SETTINGS_ID_START + count++ );
		totalNumberChoice[ picInc ].SetCheck ( picInc == 0 );
	}
	/// Exposure Times
	exposureLabel.setPositions ( pos, 0, 0, 100, 20, false, false, true );
	exposureLabel.Create( "Exposure (ms):", NORM_STATIC_OPTIONS, exposureLabel.seriesPos, parent, 
						  PICTURE_SETTINGS_ID_START + count++ );
	exposureLabel.fontType = fontTypes::SmallFont;

	for ( auto picInc : range(4) )
	{
		exposureEdits[ picInc ].setPositions ( pos, 100 + 95 * picInc, 0, 95, 20, picInc == 3, false, true );
		exposureEdits[picInc].Create( NORM_EDIT_OPTIONS, exposureEdits[picInc].seriesPos, parent,
									  PICTURE_SETTINGS_ID_START + count++ );
	}
	setUnofficialExposures ( std::vector<float> ( 4, 10 / 1000.0f ) );

	/// Thresholds
	thresholdLabel.setPositions ( pos, 0, 0, 100, 20, false, false, true );
	thresholdLabel.Create( "Threshold (cts)", NORM_STATIC_OPTIONS, thresholdLabel.seriesPos, parent,
						   PICTURE_SETTINGS_ID_START + count++ );
	thresholdLabel.fontType = fontTypes::SmallFont;
	for ( auto picInc : range(4) )
	{
		thresholdEdits[ picInc ].setPositions ( pos, 100 + 95 * picInc, 0, 95, 20, picInc == 3, false, true );
		thresholdEdits[picInc].Create( NORM_EDIT_OPTIONS | ES_AUTOHSCROLL, thresholdEdits[picInc].seriesPos, parent,
									   PICTURE_SETTINGS_ID_START + count++ );
		thresholdEdits[picInc].SetWindowTextA( "100" );
		settings.thresholds[ picInc ] = { 100 };
	}
	/// colormaps
	colormapLabel.setPositions ( pos, 0, 0, 100, 20, false, false, true );
	colormapLabel.Create( "Colormap", NORM_STATIC_OPTIONS, colormapLabel.seriesPos, parent,
						  PICTURE_SETTINGS_ID_START + count++ );
	colormapLabel.fontType = fontTypes::SmallFont;
	for ( auto picInc : range(4) )
	{
		colormapCombos[ picInc ].setPositions ( pos, 100 + 95 * picInc, 0, 95, 100, false, false, true, false );
		colormapCombos[picInc].Create( NORM_COMBO_OPTIONS, colormapCombos[picInc].seriesPos, parent, 
									   PICTURE_SETTINGS_ID_START + count++ );
		colormapCombos[picInc].fontType = fontTypes::SmallFont;
		colormapCombos[picInc].AddString( "Dark Viridis" );
		colormapCombos[picInc].AddString( "Inferno" );
		colormapCombos[picInc].AddString( "Black & White" );
		colormapCombos[picInc].AddString( "Red-Black-Blue" );
		colormapCombos[picInc].SetCurSel( 0 );
		settings.colors[picInc] = 2;
	}
	pos.seriesPos.y += 20;
	pos.amPos.y += 20;
	pos.videoPos.y += 20;
	/// display types
	displayTypeLabel.setPositions ( pos, 0, 0, 100, 20, false, false, true);
	displayTypeLabel.Create( "Display-Type:", NORM_STATIC_OPTIONS, displayTypeLabel.seriesPos, parent,
						     PICTURE_SETTINGS_ID_START + count++ );
	displayTypeLabel.fontType = fontTypes::SmallFont;
	for ( auto picInc : range ( 4 ) )
	{
		displayTypeCombos[ picInc ].setPositions ( pos, 100 + 95 * picInc, 0, 95, 100, false, false, true, false );
		displayTypeCombos[ picInc ].Create ( NORM_COMBO_OPTIONS, colormapCombos[ picInc ].seriesPos, parent,
											 PICTURE_SETTINGS_ID_START + count++ );
		displayTypeCombos[ picInc ].fontType = fontTypes::SmallFont;
		displayTypeCombos[ picInc ].AddString ( "Normal" );
		displayTypeCombos[ picInc ].AddString ( "Dif: 1" );
		displayTypeCombos[ picInc ].AddString ( "Dif: 2" );
		displayTypeCombos[ picInc ].AddString ( "Dif: 3" );
		displayTypeCombos[ picInc ].AddString ( "Dif: 4" );
		displayTypeCombos[ picInc ].SetCurSel ( 0 );
		settings.colors[ picInc ] = 2;
	}
	pos.seriesPos.y += 20;
	pos.amPos.y += 20;
	pos.videoPos.y += 20;

	/// software accumulation mode	
	softwareAccumulationLabel.setPositions ( pos, 0, 0, 100, 20, false, false, true );
	softwareAccumulationLabel.Create ( "Software Accum:", NORM_STATIC_OPTIONS, softwareAccumulationLabel.seriesPos,
									   parent, PICTURE_SETTINGS_ID_START + count++ );
	softwareAccumulationLabel.fontType = fontTypes::SmallFont;
	for ( auto picInc : range ( 4 ) )
	{
		softwareAccumulateAll[ picInc ].setPositions ( pos, 100 + 95 * picInc, 0, 65, 20, false, false, true );
		softwareAccumulateAll[ picInc ].Create ( "All?", NORM_CHECK_OPTIONS, softwareAccumulateAll[ picInc ].seriesPos, parent,
												 PICTURE_SETTINGS_ID_START + count++ );
		softwareAccumulateAll[ picInc ].SetCheck ( 0 );
		softwareAccumulateAll[ picInc ].fontType = fontTypes::SmallFont;

		softwareAccumulateNum[ picInc ].setPositions ( pos, 165 + 95 * picInc, 0, 30, 20, picInc==3, false, true );
		softwareAccumulateNum[ picInc ].Create ( NORM_EDIT_OPTIONS, softwareAccumulateNum[ picInc ].seriesPos, parent,
												 PICTURE_SETTINGS_ID_START + count++ );
		softwareAccumulateNum[ picInc ].SetWindowTextA ( "1" );
		softwareAccumulateNum[ picInc ].fontType = fontTypes::SmallFont;
	}
	//
	setPictureControlEnabled (0, true);
	setPictureControlEnabled (1, false);
	setPictureControlEnabled (2, false);
	setPictureControlEnabled (3, false);
}


std::array<displayTypeOption, 4> PictureSettingsControl::getDisplayTypeOptions( )
{
	std::array<displayTypeOption, 4> options;
	UINT counter = 0;
	for ( auto& combo : displayTypeCombos )
	{
		auto sel = combo.GetCurSel( );
		if ( sel < 0 || sel > 4 )
		{
			thrower ( "Invalid selection in display type combo???" );
		}
		options[counter].isDiff = sel != 0;
		options[counter].whichPicForDif = sel;
		counter++;
	}
	return options;
}

void PictureSettingsControl::handleNewConfig( std::ofstream& newFile )
{
	newFile << "PICTURE_SETTINGS\n";
	for ( auto color : settings.colors )
	{
		newFile << 0 << " ";
	}
	newFile << "\n";
	for ( auto threshold : settings.thresholds )
	{
		newFile << 200 << " ";
	}
	newFile << "\n";
	newFile << "END_PICTURE_SETTINGS\n";
}

std::array<std::string, 4> PictureSettingsControl::getThresholdStrings()
{
	std::array<std::string, 4> res;
	// grab the thresholds
	for ( int thresholdInc = 0; thresholdInc < 4; thresholdInc++ )
	{
		auto& picThresholds = settings.thresholds[ thresholdInc ];
		picThresholds.resize ( 1 );
		CString textEdit;
		thresholdEdits[ thresholdInc ].GetWindowTextA ( textEdit );
		res[ thresholdInc ] = textEdit;
	}
	return res;
}

void PictureSettingsControl::handleSaveConfig(std::ofstream& saveFile)
{
	saveFile << "PICTURE_SETTINGS\n";
	for (auto color : settings.colors)
	{
		saveFile << color << " ";
	}
	saveFile << "\n";
	for (auto threshold : getThresholdStrings() )
	{
		saveFile << threshold << " ";
	}
	saveFile << "\n";
	for ( auto saOpt : getSoftwareAccumulationOptions ( ) )
	{
		saveFile << saOpt.accumAll << " " << saOpt.accumNum << " ";
	}
	saveFile << "\n";

	saveFile << "END_PICTURE_SETTINGS\n";
}

andorPicSettingsGroup PictureSettingsControl::getPictureSettingsFromConfig ( std::ifstream& configFile, Version ver )
{
	UINT picsPerRep;
	andorPicSettingsGroup fileSettings;
	if ( ver <= Version ( "4.7" ) )
	{
		int oldPicsPerRepTrash = 0;
		configFile >> oldPicsPerRepTrash;
	}
	for ( auto& color : fileSettings.colors )
	{
		configFile >> color;
	}
	if ( ver <= Version ( "4.7" ) )
	{
		std::vector<float> oldExposureTimeTrash(4);
		for ( auto& exposure : oldExposureTimeTrash )
		{
			configFile >> exposure;
		}
	}
	for ( auto& threshold : fileSettings.thresholdStrs )
	{
		configFile >> threshold;
	}
	if ( ver > Version ( "4.3" ) )
	{
		for ( auto& opt : fileSettings.saOpts )
		{
			configFile >> opt.accumAll >> opt.accumNum;
		}
	}
	return fileSettings;
}

void PictureSettingsControl::handleOpenConfig(std::ifstream& openFile, Version ver, AndorCamera* andor)
{
	ProfileSystem::checkDelimiterLine(openFile, "PICTURE_SETTINGS");
	auto settings = getPictureSettingsFromConfig ( openFile, ver );
	updateAllSettings ( settings );
	ProfileSystem::checkDelimiterLine(openFile, "END_PICTURE_SETTINGS");
}

void PictureSettingsControl::setSoftwareAccumulationOptions ( std::array<softwareAccumulationOption, 4> opts )
{
	for ( auto picInc : range ( 4 ) )
	{
		softwareAccumulateAll[ picInc ].SetCheck ( opts[ picInc ].accumAll );
		softwareAccumulateNum[ picInc ].SetWindowTextA ( cstr ( opts[ picInc ].accumNum ) );
	}
}

std::array<softwareAccumulationOption, 4> PictureSettingsControl::getSoftwareAccumulationOptions ( )
{
	std::array<softwareAccumulationOption, 4> opts;
	for ( auto picInc : range(4))
	{
		opts[ picInc ].accumAll = softwareAccumulateAll[ picInc ].GetCheck ( );
		CString numTxt;
		softwareAccumulateNum[ picInc ].GetWindowTextA ( numTxt );
		try
		{
			opts[ picInc ].accumNum  = boost::lexical_cast<UINT>( numTxt );
		}
		catch ( boost::bad_lexical_cast& )
		{
			thrower ( "Failed to convert software accumulation number to an unsigned integer!" );
		}
	}
	return opts;
}

void PictureSettingsControl::setPictureControlEnabled (int pic, bool enabled)
{
	if (pic > 3)
	{
		return;
	}
	exposureEdits[pic].EnableWindow (enabled);
	thresholdEdits[pic].EnableWindow (enabled);
	colormapCombos[pic].EnableWindow (enabled);
	displayTypeCombos[pic].EnableWindow (enabled);
	softwareAccumulateAll[pic].EnableWindow (enabled);
	softwareAccumulateNum[pic].EnableWindow (enabled);
}


CBrush* PictureSettingsControl::colorControls(int id, CDC* colorer )
{
	/// Exposures
	if (id >= exposureEdits.front().GetDlgCtrlID() && id <= exposureEdits.back().GetDlgCtrlID())
	{
		int picNum = id - exposureEdits.front().GetDlgCtrlID();
		if (!exposureEdits[picNum].IsWindowEnabled())
		{
			return NULL;
		}
		colorer->SetTextColor( _myRGBs["AndorWin-Text"]);
		colorer->SetBkColor (_myRGBs["Interactable-Bkgd"]);
		return _myBrushes["Interactable-Bkgd"];
	}
	/// Thresholds
	else if (id >= thresholdEdits.front().GetDlgCtrlID() && id <= thresholdEdits.back().GetDlgCtrlID())
	{
		int picNum = id - thresholdEdits.front ().GetDlgCtrlID ();
		if (!thresholdEdits[picNum].IsWindowEnabled ())
		{
			return NULL;
		}
		colorer->SetTextColor (_myRGBs["AndorWin-Text"]);
		colorer->SetBkColor (_myRGBs["Interactable-Bkgd"]);
		return _myBrushes["Interactable-Bkgd"];
	}
	else
	{
		return NULL;
	}
}


UINT PictureSettingsControl::getPicsPerRepetition()
{
	UINT which = 0, count=0;
	for ( auto& ctrl : totalNumberChoice )
	{
		count++;		
		which = ctrl.GetCheck ( ) ? count : which;
	}
	if ( which == 0 )
	{
		thrower ( "ERROR: failed to get pics per repetition?!?" );
	}
	return which;
}


void PictureSettingsControl::setUnofficialPicsPerRep( UINT picNum )
{
	if ( picNum < 1 || picNum > 4 )
	{
		thrower ( "Tried to set bad number of pics per rep: " + str ( picNum ) );
	}
	int count = 0;
	for (auto& totalNumRadio : totalNumberChoice)
	{
		count++;
		totalNumRadio.SetCheck (count == picNum);
		setPictureControlEnabled (count-1, count <= picNum);
	}
}


void PictureSettingsControl::handleOptionChange( int id )
{
	if (id >= totalNumberChoice.front().GetDlgCtrlID() && id <= totalNumberChoice.back().GetDlgCtrlID())
	{
		int picNum = id - totalNumberChoice.front().GetDlgCtrlID();
		// this message can weirdly get set after a configuration opens as well, it only means to set the number if the 
		// relevant button is now checked.
		if ( totalNumberChoice[picNum].GetCheck( ) )
		{
			setUnofficialPicsPerRep( picNum + 1 );
		}
	}
	else if (id >= colormapCombos[0].GetDlgCtrlID() && id <= colormapCombos[3].GetDlgCtrlID())
	{
		id -= colormapCombos[0].GetDlgCtrlID();
		int color = colormapCombos[id].GetCurSel( );
		settings.colors[id] = color;
	}
}


std::array<float, 4> PictureSettingsControl::getExposureTimes ( )
{
	std::array<float, 4> times;
	for ( auto ctrlNum : range(exposureEdits.size()) )
	{
		auto& ctrl = exposureEdits[ ctrlNum ];
		CString txt;
		ctrl.GetWindowTextA ( txt );
		try
		{
			times[ ctrlNum ] = boost::lexical_cast<double>( txt ) * 1e-3;
		}
		catch ( boost::bad_lexical_cast )
		{
			thrower ( "Failed to convert exposure time to a float!" );
		}
	}
	return times;
}


std::vector<float> PictureSettingsControl::getUsedExposureTimes()
{
	updateSettings( );
	auto allTimes = getExposureTimes();
	std::vector<float> usedTimes(std::begin(allTimes), std::end(allTimes));
	usedTimes.resize ( getPicsPerRepetition ( ));
	return usedTimes;
}


void PictureSettingsControl::setThresholds( std::array<std::string, 4> newThresholds)
{
	for (UINT thresholdInc = 0; thresholdInc < newThresholds.size(); thresholdInc++)
	{
		thresholdEdits[thresholdInc].SetWindowTextA(newThresholds[thresholdInc].c_str());
	}
}


std::array<int, 4> PictureSettingsControl::getPictureColors()
{
	updateSettings ( );
	return settings.colors;
}


void PictureSettingsControl::updateColormaps ( std::array<int, 4> colorIndexes )
{
	settings.colors = colorIndexes;
	for ( auto picInc : range(4) )
	{
		colormapCombos[ picInc ].SetCurSel ( settings.colors[ picInc ] );
	}
}


void PictureSettingsControl::setUnofficialExposures ( std::vector<float> times )
{
	UINT count = 0;
	for ( auto ti : times )
	{
		exposureEdits[ count++ ].SetWindowTextA ( cstr ( ti*1e3 ) );
	}
}


void PictureSettingsControl::updateAllSettings ( andorPicSettingsGroup inputSettings )
{
	updateColormaps ( inputSettings.colors );
	setThresholds ( inputSettings.thresholdStrs );
	setSoftwareAccumulationOptions (inputSettings.saOpts);
}


/**/
std::array<std::vector<int>, 4> PictureSettingsControl::getThresholds ( )
{
	updateSettings ( );
	return settings.thresholds;
}

void PictureSettingsControl::updateSettings( )
{
	// grab the thresholds
	for (auto thresholdInc : range(4) )
	{
		auto& picThresholds = settings.thresholds[ thresholdInc ];
		picThresholds.resize ( 1 );
		CString textEdit;
		thresholdEdits[thresholdInc].GetWindowTextA( textEdit );
		int threshold;
		try
		{
			threshold = boost::lexical_cast<int>( str( textEdit ) );
			picThresholds[ 0 ] = threshold;
		}
		catch ( boost::bad_lexical_cast& )
		{
			picThresholds.clear ( );
			// assume it's a file location.
			std::ifstream thresholdFile;
			thresholdFile.open ( str(textEdit).c_str() );
			if ( !thresholdFile.is_open ( ) )
			{
				thrower  ( "ERROR: failed to convert threshold number " + str ( thresholdInc + 1 ) + " to an integer, "
						 "and it wasn't the address of a threshold-file." );  
			}
			while ( true )
			{
				double indv_file_threshold;
				thresholdFile >> indv_file_threshold;
				if ( thresholdFile.eof ( ) )
				{
					break;
				}
				picThresholds.push_back ( indv_file_threshold );
			}
		}
		thresholdEdits[thresholdInc].RedrawWindow( );
	}
}


void PictureSettingsControl::rearrange( AndorRunModes::mode cameraMode, AndorTriggerMode::mode triggerMode, int width, 
										int height, fontMap fonts )
{
	totalPicNumberLabel.rearrange(cameraMode, triggerMode, width, height, fonts);
	pictureLabel.rearrange(cameraMode, triggerMode, width, height, fonts);
	exposureLabel.rearrange(cameraMode, triggerMode, width, height, fonts);
	thresholdLabel.rearrange(cameraMode, triggerMode, width, height, fonts);
	colormapLabel.rearrange(cameraMode, triggerMode, width, height, fonts);
	displayTypeLabel.rearrange( cameraMode, triggerMode, width, height, fonts );
	softwareAccumulationLabel.rearrange ( cameraMode, triggerMode, width, height, fonts );
	for (auto& control : pictureNumbers)
	{
		control.rearrange(cameraMode, triggerMode, width, height, fonts);
	}
	for (auto& control : totalNumberChoice)
	{
		control.rearrange(cameraMode, triggerMode, width, height, fonts);
	}
	for (auto& control : exposureEdits)
	{
		control.rearrange(cameraMode, triggerMode, width, height, fonts);
	}
	for (auto& control : thresholdEdits)
	{
		control.rearrange(cameraMode, triggerMode, width, height, fonts);
	}
	for ( auto& control : colormapCombos )
	{
		control.rearrange( cameraMode, triggerMode, width, height, fonts );
	}
	for ( auto& control : displayTypeCombos )
	{
		control.rearrange( cameraMode, triggerMode, width, height, fonts );
	}
	for ( auto& control : softwareAccumulateAll )
	{
		control.rearrange ( cameraMode, triggerMode, width, height, fonts );
	}
	for ( auto& control : softwareAccumulateNum )
	{
		control.rearrange ( cameraMode, triggerMode, width, height, fonts );
	}
}
