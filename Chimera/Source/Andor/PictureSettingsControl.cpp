// created by Mark O. Brown

#include "stdafx.h"
#include "PictureSettingsControl.h"

#include "Andor/AndorCameraCore.h"
#include "Andor/CameraSettingsControl.h"
#include "PrimaryWindows/AndorWindow.h"
#include "ConfigurationSystems/ProfileSystem.h"

#include "Commctrl.h"
#include <boost/lexical_cast.hpp>

void PictureSettingsControl::initialize( POINT& pos, CWnd* parent, int& id )
{
	// introducing things row by row
	/// Set Picture Options
	UINT count = 0;
	/// Picture Numbers
	pictureLabel.sPos = { pos.x, pos.y, pos.x + 100, pos.y + 20 };
	pictureLabel.Create( "Picture #:", NORM_STATIC_OPTIONS, pictureLabel.sPos, parent,
						 PICTURE_SETTINGS_ID_START + count++ );
	pictureLabel.fontType = fontTypes::SmallFont;

	for ( auto picInc : range(4) )
	{
		pictureNumbers[picInc].sPos = { pos.x + 100 + 95 * picInc, pos.y, pos.x + 100 + 95 * (picInc + 1), pos.y + 20 };
		pictureNumbers[picInc].Create( cstr( picInc + 1 ), NORM_STATIC_OPTIONS, pictureNumbers[picInc].sPos,
									   parent, PICTURE_SETTINGS_ID_START + count++ );
	}
	pos.y += 20;
	/// Total picture number
	totalPicNumberLabel.sPos = { pos.x, pos.y, pos.x + 100, pos.y + 20 };
	totalPicNumberLabel.Create( "Total Picture #", NORM_STATIC_OPTIONS, totalPicNumberLabel.sPos, parent,
								PICTURE_SETTINGS_ID_START + count++ );
	totalPicNumberLabel.fontType = fontTypes::SmallFont;
	for ( auto picInc : range ( 4 ) )
	{
		totalNumberChoice[picInc].sPos = { pos.x + 100 + 95 * picInc, pos.y, pos.x + 100 + 95 * (picInc + 1), pos.y + 20 };
		totalNumberChoice[ picInc ].Create ( "", ( picInc == 0 ) ? ( NORM_RADIO_OPTIONS | WS_GROUP ) : NORM_RADIO_OPTIONS,
											 totalNumberChoice[ picInc ].sPos, parent, PICTURE_SETTINGS_ID_START + count++ );
		totalNumberChoice[ picInc ].SetCheck ( picInc == 0 );
	}
	pos.y += 25;
	/// Exposure Times
	exposureLabel.sPos = { pos.x, pos.y, pos.x + 100, pos.y + 20 };
	exposureLabel.Create( "Exposure (ms):", NORM_STATIC_OPTIONS, exposureLabel.sPos, parent,
						  PICTURE_SETTINGS_ID_START + count++ );
	exposureLabel.fontType = fontTypes::SmallFont;
	for ( auto picInc : range(4) )
	{
		exposureEdits[picInc].sPos = { pos.x + 100 + 95 * picInc , pos.y, pos.x + 100 + 95 * (picInc + 1), pos.y + 20 };
		exposureEdits[picInc].Create( NORM_EDIT_OPTIONS, exposureEdits[picInc].sPos, parent,
									  PICTURE_SETTINGS_ID_START + count++ );
	}
	pos.y += 20;
	setUnofficialExposures ( std::vector<float> ( 4, 10 / 1000.0f ) );

	/// Thresholds
	thresholdLabel.sPos = { pos.x, pos.y, pos.x + 100, pos.y + 20 };
	thresholdLabel.Create( "Threshold (cts)", NORM_STATIC_OPTIONS, thresholdLabel.sPos, parent,
						   PICTURE_SETTINGS_ID_START + count++ );
	thresholdLabel.fontType = fontTypes::SmallFont;
	for ( auto picInc : range(4) )
	{
		thresholdEdits[picInc].sPos = { pos.x + 100 + 95 * picInc, pos.y, pos.x + 100 + 95 * (picInc + 1), pos.y + 20 };
		thresholdEdits[picInc].Create( NORM_EDIT_OPTIONS | ES_AUTOHSCROLL, thresholdEdits[picInc].sPos, parent,
									   PICTURE_SETTINGS_ID_START + count++ );
		thresholdEdits[picInc].SetWindowTextA( "100" );
		settings.thresholds[ picInc ] = { 100 };
	}
	pos.y += 20;
	/// colormaps
	colormapLabel.sPos = { pos.x, pos.y, pos.x + 100, pos.y + 25 };
	colormapLabel.Create( "Colormap", NORM_STATIC_OPTIONS, colormapLabel.sPos, parent,
						  PICTURE_SETTINGS_ID_START + count++ );
	colormapLabel.fontType = fontTypes::SmallFont;
	for ( auto picInc : range(4) )
	{
		colormapCombos[picInc].sPos = { pos.x + 100 + 95 * picInc, pos.y, pos.x + 100 + 95 * (picInc + 1), pos.y + 65 };
		colormapCombos[picInc].Create( NORM_COMBO_OPTIONS, colormapCombos[picInc].sPos, parent,
									   PICTURE_SETTINGS_ID_START + count++ );
		colormapCombos[picInc].fontType = fontTypes::SmallFont;
		colormapCombos[picInc].AddString( "Dark Viridis" );
		colormapCombos[picInc].AddString( "Inferno" );
		colormapCombos[picInc].AddString( "Black & White" );
		colormapCombos[picInc].AddString( "Red-Black-Blue" );
		colormapCombos[picInc].SetCurSel( 0 );
		settings.colors[picInc] = 2;
	}
	pos.y += 25;
	/// display types
	displayTypeLabel.sPos = { pos.x, pos.y, pos.x + 100, pos.y + 25 };
	displayTypeLabel.Create( "Display-Type:", NORM_STATIC_OPTIONS, displayTypeLabel.sPos, parent,
							  PICTURE_SETTINGS_ID_START + count++ );
	displayTypeLabel.fontType = fontTypes::SmallFont;
	for ( auto picInc : range ( 4 ) )
	{
		displayTypeCombos[picInc].sPos = { pos.x + 100 + 95 * picInc, pos.y, pos.x + 100 + 95 * (picInc + 1), pos.y + 65 };
		displayTypeCombos[ picInc ].Create ( NORM_COMBO_OPTIONS, colormapCombos[ picInc ].sPos, parent,
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
	pos.y += 25;

	/// software accumulation mode	
	softwareAccumulationLabel.sPos = { pos.x, pos.y, pos.x + 100, pos.y + 20 };
	softwareAccumulationLabel.Create ( "Software Accum:", NORM_STATIC_OPTIONS, softwareAccumulationLabel.sPos,
									   parent, PICTURE_SETTINGS_ID_START + count++ );
	softwareAccumulationLabel.fontType = fontTypes::SmallFont;
	for ( auto picInc : range ( 4 ) )
	{
		softwareAccumulateAll[picInc].sPos = { pos.x + 100 + 95 * picInc, pos.y, pos.x + 100 + 95 * picInc+65, pos.y + 20 };
		softwareAccumulateAll[ picInc ].Create ( "All?", NORM_CHECK_OPTIONS, softwareAccumulateAll[ picInc ].sPos, parent,
												 PICTURE_SETTINGS_ID_START + count++ );
		softwareAccumulateAll[ picInc ].SetCheck ( 0 );
		softwareAccumulateAll[ picInc ].fontType = fontTypes::SmallFont;
		softwareAccumulateNum[picInc].sPos = { pos.x + 165 + 95 * picInc, pos.y, pos.x + 195 + 95 * picInc, pos.y + 20 };
		softwareAccumulateNum[ picInc ].Create ( NORM_EDIT_OPTIONS, softwareAccumulateNum[ picInc ].sPos, parent,
												 PICTURE_SETTINGS_ID_START + count++ );
		softwareAccumulateNum[ picInc ].SetWindowTextA ( "1" );
		softwareAccumulateNum[ picInc ].fontType = fontTypes::SmallFont;
	}
	pos.y += 20;
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

void PictureSettingsControl::handleSaveConfig(ConfigStream& saveFile)
{
	saveFile << "PICTURE_SETTINGS\n/*Color Options:*/ ";
	for (auto color : settings.colors)
	{
		saveFile << color << " ";
	}
	saveFile << "\n/*Threshold Settings:*/ ";
	for (auto threshold : getThresholdStrings() )
	{
		saveFile << threshold << " ";
	}
	saveFile << "\n/*Software Accumulation (accum all / Number)*/ ";
	for ( auto saOpt : getSoftwareAccumulationOptions ( ) )
	{
		saveFile << saOpt.accumAll << " " << saOpt.accumNum << " ";
	}
	saveFile << "\nEND_PICTURE_SETTINGS\n";
}

andorPicSettingsGroup PictureSettingsControl::getPictureSettingsFromConfig (ConfigStream& configFile )
{
	UINT picsPerRep;
	andorPicSettingsGroup fileSettings;
	if ( configFile.ver <= Version ( "4.7" ) )
	{
		int oldPicsPerRepTrash = 0;
		configFile >> oldPicsPerRepTrash;
	}
	for ( auto& color : fileSettings.colors )
	{
		configFile >> color;
	}
	if (configFile.ver <= Version ( "4.7" ) )
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
	if (configFile.ver > Version ( "4.3" ) )
	{
		for ( auto& opt : fileSettings.saOpts )
		{
			configFile >> opt.accumAll >> opt.accumNum;
		}
	}
	return fileSettings;
}

void PictureSettingsControl::handleOpenConfig(ConfigStream& openFile, AndorCameraCore* andor)
{
	ProfileSystem::checkDelimiterLine(openFile, "PICTURE_SETTINGS");
	auto settings = getPictureSettingsFromConfig ( openFile );
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
				if ( thresholdFile.eof ( ) ) { break; }
				picThresholds.push_back ( indv_file_threshold );
			}
		}
		thresholdEdits[thresholdInc].RedrawWindow( );
	}
}


void PictureSettingsControl::rearrange( int width, int height, fontMap fonts )
{
	totalPicNumberLabel.rearrange(width, height, fonts);
	pictureLabel.rearrange(width, height, fonts);
	exposureLabel.rearrange(width, height, fonts);
	thresholdLabel.rearrange(width, height, fonts);
	colormapLabel.rearrange(width, height, fonts);
	displayTypeLabel.rearrange(width, height, fonts );
	softwareAccumulationLabel.rearrange (width, height, fonts );
	for (auto& control : pictureNumbers)
	{
		control.rearrange(width, height, fonts);
	}
	for (auto& control : totalNumberChoice)
	{
		control.rearrange( width, height, fonts);
	}
	for (auto& control : exposureEdits)
	{
		control.rearrange(width, height, fonts);
	}
	for (auto& control : thresholdEdits)
	{
		control.rearrange(width, height, fonts);
	}
	for ( auto& control : colormapCombos )
	{
		control.rearrange(width, height, fonts );
	}
	for ( auto& control : displayTypeCombos )
	{
		control.rearrange(width, height, fonts );
	}
	for ( auto& control : softwareAccumulateAll )
	{
		control.rearrange ( width, height, fonts );
	}
	for ( auto& control : softwareAccumulateNum )
	{
		control.rearrange ( width, height, fonts );
	}
}
