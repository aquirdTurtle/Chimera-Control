
#include "stdafx.h"
#include "PlotDesignerDialog.h"


IMPLEMENT_DYNAMIC( PlotDesignerDialog, CDialog )

BEGIN_MESSAGE_MAP( PlotDesignerDialog, CDialog )
	ON_WM_CTLCOLOR()
	ON_COMMAND( IDC_PLOT_CREATOR_SAVE, PlotDesignerDialog::handleSave )
	ON_COMMAND( IDC_PLOT_CREATOR_CANCEL, PlotDesignerDialog::handleCancel )
	ON_COMMAND( IDC_PRC_SHOW_ALL, PlotDesignerDialog::handlePrcShowAll )
	ON_COMMAND( IDC_PSC_SHOW_ALL, PlotDesignerDialog::handlePscShowAll )
	ON_CBN_SELENDOK( IDC_PLOT_CREATOR_DATASET_COMBO, PlotDesignerDialog::handleDataSetComboChange )
	ON_CBN_SELENDOK( IDC_PSC_CONDITION_NUMBER, PlotDesignerDialog::handlePscConditionNumberChange )
	ON_CBN_SELENDOK( IDC_PSC_PICTURE_NUMBER, PlotDesignerDialog::handlePscPictureNumberChange )
	ON_CBN_SELENDOK( IDC_PSC_PIXEL_NUMBER, PlotDesignerDialog::handlePscPixelNumberChange )
	ON_CBN_SELENDOK( IDC_PRC_PICTURE_NUMBER, PlotDesignerDialog::handlePrcPictureNumberChange )
	ON_CBN_SELENDOK( IDC_PRC_PIXEL_NUMBER, PlotDesignerDialog::handlePrcPixelNumberChange )
	ON_CBN_SELENDOK( IDC_GENERAL_PLOT_TYPE, PlotDesignerDialog::handleGeneralPlotTypeChange )
	ON_EN_KILLFOCUS( IDC_PIXELS_PER_ANALYSIS_GROUP, PlotDesignerDialog::handlePixelEditChange )
END_MESSAGE_MAP()

BOOL PlotDesignerDialog::OnInitDialog( )
{
	POINT pos = { 0,0 };
	UINT id = 5000;
	plotPropertiesText.Create( "Plot Properties", NORM_HEADER_OPTIONS, { pos.x, pos.y, pos.x + 480, pos.y += 25 },
							   this, id++ );
	plotPropertiesText.SetFont( (*dlgFonts)["Heading Font Large"] );
	plotTitleText.Create( "Plot Title", NORM_STATIC_OPTIONS, { pos.x, pos.y, pos.x + 240, pos.y + 25 }, this, id++ );
	plotTitleEdit.Create( NORM_EDIT_OPTIONS, { pos.x + 240, pos.y, pos.x + 480, pos.y += 25 }, this, id++ );
	plotTitleEdit.SetWindowText( cstr( currentPlotInfo.getTitle( ) ) );
	yLabelText.Create( "Y-Axis Label", NORM_STATIC_OPTIONS, { pos.x, pos.y, pos.x + 240, pos.y + 25 }, this, id++ );
	yLabelEdit.Create( NORM_EDIT_OPTIONS, { pos.x + 240, pos.y, pos.x + 480, pos.y += 25 }, this, id++ );
	yLabelEdit.SetWindowText( cstr( currentPlotInfo.getYLabel( ) ) );

	plotFilenameText.Create( "Plot Filename", NORM_STATIC_OPTIONS, { pos.x, pos.y, pos.x + 240, pos.y + 25 }, this, 
							 id++ );
	plotFilenameEdit.Create( NORM_EDIT_OPTIONS, { pos.x + 240, pos.y, pos.x + 480, pos.y += 25 }, this, id++ );
	plotFilenameEdit.SetWindowText( cstr( currentPlotInfo.getFileName( ) ) );

	generalPlotTypeText.Create( "Plot Type", WS_CHILD | WS_VISIBLE, { pos.x, pos.y, pos.x + 480, pos.y += 25 }, this,
								id++ );
	generalPlotTypeCombo.Create( NORM_COMBO_OPTIONS, { pos.x, pos.y, pos.x + 480, pos.y + 200 }, this, 
								 IDC_GENERAL_PLOT_TYPE );
	generalPlotTypeCombo.AddString( "Pixel Count Histograms" );
	generalPlotTypeCombo.AddString( "Pixel Counts" );
	generalPlotTypeCombo.AddString( "Atoms" );
	generalPlotTypeCombo.SelectString( 0, cstr( currentPlotInfo.getPlotType( ) ) );
	pos.y += 25;
	dataSetNumberText.Create( "Data Set #", NORM_STATIC_OPTIONS, { pos.x, pos.y, pos.x + 240, pos.y + 25 }, this, id++ );
	dataSetNumCombo.Create( NORM_COMBO_OPTIONS, { pos.x + 240, pos.y, pos.x + 480, pos.y + 200 }, this, 
							   IDC_PLOT_CREATOR_DATASET_COMBO );
	dataSetNumCombo.AddString( "Data Set #1" );
	dataSetNumCombo.AddString( "Add New Data Set" );
	dataSetNumCombo.AddString( "Remove Data Set" );
	pos.y += 25;
	plotThisDataBox.Create( "Plot This Data", NORM_CHECK_OPTIONS, { pos.x + 240, pos.y, pos.x + 480, pos.y += 25 }, 
							this, id++ );
	// Positive Result Conditions
	pos.y += 10;
	positiveResultConditionText.Create( "Positive Result Condition", NORM_STATIC_OPTIONS, { pos.x, pos.y, pos.x + 480,
										pos.y += 25 }, this, id++ );
	positiveResultConditionText.SetFont( (*dlgFonts)["Heading Font Large"] );

	prcPictureNumberText.Create( "Picture Number", NORM_STATIC_OPTIONS, { pos.x, pos.y, pos.x + 240, pos.y + 25 }, this, id++ );
	prcPicNumCombo.Create( NORM_COMBO_OPTIONS,{ pos.x + 240, pos.y, pos.x + 480, pos.y + 200 }, this, 
								  IDC_PRC_PICTURE_NUMBER );
	for ( auto num : range( picNumber ) )
	{
		prcPicNumCombo.AddString( cstr( "Picture #" + str( num + 1 ) ) );
	}

	pos.y += 25;

	prcPixelNumberText.Create( "Pixel Number", NORM_STATIC_OPTIONS, { pos.x, pos.y, pos.x + 240, pos.y + 25 }, this, 
							   id++ );
	prcPixelNumCombo.Create( NORM_COMBO_OPTIONS, { pos.x + 240, pos.y, pos.x + 480, pos.y + 200 }, this, 
								IDC_PRC_PIXEL_NUMBER );
	prcPixelNumCombo.AddString( "Pixel #1" );

	pos.y += 25;
	prcAtomBox.Create( "Atom", NORM_CHECK_OPTIONS, { pos.x, pos.y, pos.x + 240, pos.y + 25 }, this, IDC_PRC_ATOM );
	prcNoAtomBox.Create( "No Atom", NORM_CHECK_OPTIONS, { pos.x + 240, pos.y, pos.x + 480, pos.y += 25 }, this, 
						 IDC_PRC_NOATOM );
	prcShowAllButton.Create( "Show All", NORM_PUSH_OPTIONS, { pos.x + 240, pos.y, pos.x + 480, pos.y += 25 }, this, 
							 IDC_PRC_SHOW_ALL );
	pos.y += 10;
	fitsText.Create( "Fits", NORM_STATIC_OPTIONS, { pos.x, pos.y, pos.x + 240, pos.y += 25 }, this, id++ );
	fitsText.SetFont( (*dlgFonts)["Heading Font Large"] );
	gaussianFit.Create( "Gaussian", NORM_RADIO_OPTIONS | WS_GROUP, { pos.x, pos.y, pos.x + 240, pos.y += 25 }, this, id++ );
	lorentzianFit.Create( "Lorentzian", NORM_RADIO_OPTIONS, { pos.x, pos.y, pos.x + 240, pos.y += 25 }, this, id++ );
	decayingSineFit.Create( "Decaying Sine", NORM_RADIO_OPTIONS, { pos.x, pos.y, pos.x + 240, pos.y + 25 }, this, id++ );
	pos.y -= 50;
	noFit.Create( "None", NORM_RADIO_OPTIONS | WS_GROUP, { pos.x + 240, pos.y, pos.x + 480, pos.y += 25 }, this, id++ );
	noFit.SetCheck( 1 );
	realTimeFit.Create( "Real Time Fit", NORM_RADIO_OPTIONS, { pos.x + 240, pos.y, pos.x + 480, pos.y += 25 }, this, id++ );
	atFinishFit.Create( "At Finish", NORM_RADIO_OPTIONS, { pos.x + 240, pos.y, pos.x + 480, pos.y += 25 }, this, id++ );
	setFitRadios( );
	pos.x = 480;
	pos.y = 0;
	analysisLocationsText.Create( "Analysis Locations", NORM_STATIC_OPTIONS, { pos.x, pos.y, pos.x + 480, pos.y += 25 },
								  this, id++ );
	analysisLocationsText.SetFont( (*dlgFonts)["Heading Font Large"] );
	pixelsPerAnalysisGroupText.Create( "Pixels Per Analysis Group", NORM_STATIC_OPTIONS, { pos.x, pos.y, pos.x + 240, 
									   pos.y + 25 }, this, id++ );
	pixelsPerAnalysisGroupEdit.Create( NORM_EDIT_OPTIONS, { pos.x + 240, pos.y, pos.x + 480, pos.y += 25 }, this,
									   IDC_PIXELS_PER_ANALYSIS_GROUP );
	pixelsPerAnalysisGroupEdit.SetWindowText( "1" );

	xAxisText.Create( "X-Axis", NORM_STATIC_OPTIONS, { pos.x, pos.y, pos.x + 480, pos.y += 25 }, this, id++ );
	xAxisText.SetFont( (*dlgFonts)["Heading Font Large"] );

	averageEachVariation.Create( "Average Each Variation", NORM_RADIO_OPTIONS | WS_GROUP, { pos.x, pos.y, 
								 pos.x + 480, pos.y += 25 }, this, id++ );
	averageEachVariation.SetCheck( 1 );
	runningAverage.Create( "Running Average (Continuous Mode Only)", NORM_RADIO_OPTIONS | WS_GROUP,	{ pos.x, pos.y, 
						   pos.x + 480, pos.y += 25 }, this, id++ );
	aestheticsText.Create( "Aesthetics", NORM_STATIC_OPTIONS, { pos.x, pos.y, pos.x + 480, pos.y += 25 }, this, id++ );
	aestheticsText.SetFont( (*dlgFonts)["Heading Font Large"] );
	legendText.Create( "Legend Text", NORM_STATIC_OPTIONS, { pos.x, pos.y, pos.x + 240, pos.y + 25 }, this, id++ );
	legendEdit.Create( NORM_EDIT_OPTIONS, { pos.x + 240, pos.y, pos.x + 480, pos.y += 25 }, this, id++ );
	binWidthText.Create( "Hist Bin Width:", NORM_STATIC_OPTIONS, { pos.x, pos.y, pos.x + 240, pos.y + 25 }, this, id++ );
	binWidthEdit.Create( NORM_EDIT_OPTIONS, { pos.x + 240, pos.y, pos.x + 480, pos.y += 25 }, this, id++ );
	binWidthEdit.SetWindowTextA( "10" );
	// Post selection conditions
	pos.y += 10;
	postSelectionConditionText.Create( "Post-Selection Conditions", NORM_STATIC_OPTIONS, { pos.x, pos.y, pos.x + 480,
									   pos.y += 25 }, this, id++ );
	postSelectionConditionText.SetFont( (*dlgFonts)["Heading Font Large"] );
	pscConditionNumberText.Create( "Condition Number", NORM_STATIC_OPTIONS, { pos.x, pos.y, pos.x + 240, pos.y + 25 }, 
								   this, id++ );
	pscConditionNumCombo.Create( NORM_COMBO_OPTIONS, { pos.x + 240, pos.y, pos.x + 480, pos.y + 200 }, this, 
									IDC_PSC_CONDITION_NUMBER );
	pscConditionNumCombo.AddString( "Condition #1" );
	pscConditionNumCombo.AddString( "Add New Condition" );
	pscConditionNumCombo.AddString( "Remove Condition" );
	pos.y += 25;
	pscPictureNumberText.Create( "Picture Number", NORM_STATIC_OPTIONS, { pos.x, pos.y, pos.x + 240, pos.y + 25 }, this, 
								 id++ );
	pscPicNumCombo.Create( NORM_COMBO_OPTIONS, { pos.x + 240, pos.y, pos.x + 480, pos.y + 200 }, this, 
								  IDC_PSC_PICTURE_NUMBER );
	for ( auto num : range( picNumber ) )
	{
		pscPicNumCombo.AddString( cstr( "Picture #" + str( num + 1 ) ) );
	}
	pos.y += 25;

	pscPixelNumberText.Create( "Pixel Number", NORM_STATIC_OPTIONS, { pos.x, pos.y, pos.x + 240, pos.y + 25 }, this,
							   id++ );
	pscPixelNumCombo.Create( NORM_COMBO_OPTIONS, { pos.x + 240, pos.y, pos.x + 480, pos.y + 200 }, this, 
								IDC_PSC_PIXEL_NUMBER );
	pscPixelNumCombo.AddString( "Pixel #1" );
	pos.y += 25;
	pscAtomBox.Create( "Atom", NORM_CHECK_OPTIONS, { pos.x, pos.y, pos.x + 240, pos.y + 25 }, this, IDC_PSC_ATOM );
	pscNoAtomBox.Create( "No Atom", NORM_CHECK_OPTIONS, { pos.x + 240, pos.y, pos.x + 480, pos.y += 25 }, this, 
						 IDC_PSC_NOATOM );
	pscShowAllButton.Create( "Show All", NORM_PUSH_OPTIONS, { pos.x + 240, pos.y, pos.x + 480, pos.y += 25 }, this, 
							 IDC_PSC_SHOW_ALL );
	pos.y += 10;
	saveButton.Create( "SAVE", NORM_PUSH_OPTIONS, { pos.x, pos.y, pos.x + 240, pos.y + 25 }, this, 
					   IDC_PLOT_CREATOR_SAVE );
	saveButton.SetFont( (*dlgFonts)["Heading Font Large"] );
	cancelButton.Create( "CANCEL", NORM_PUSH_OPTIONS, { pos.x + 240, pos.y, pos.x + 480, pos.y + 25 }, this, 
						 IDC_PLOT_CREATOR_CANCEL );
	cancelButton.SetFont( (*dlgFonts)["Heading Font Large"] );
	enableAndDisable( );
	return NULL;
}


void PlotDesignerDialog::handlePixelEditChange()
{
	int pixelNum;
	CString txt;
	pixelsPerAnalysisGroupEdit.GetWindowText( txt );
	try
	{
		pixelNum = std::stol( str( txt ) );
	}
	catch (std::invalid_argument&)
	{
		errBox( "ERROR: pixels per analysis group text failed to convert to an integer!" );
	}
	// change the pixel number in both of the edits.
	prcPixelNumCombo.Clear();
	prcPixelNumCombo.ResetContent();
	pscPixelNumCombo.Clear();
	pscPixelNumCombo.ResetContent();
	for (auto num : range( pixelNum ))
	{
		std::string text( "Pixel #" + str( num + 1 ) );
		prcPixelNumCombo.AddString( cstr( text ) );
		pscPixelNumCombo.AddString( cstr( text ) );
	}
	currentPlotInfo.resetPixelNumber( pixelNum );
}


void PlotDesignerDialog::handleSave()
{
	/// Save Everything
	// General Parameters: ////////
	CString text;
	plotTitleEdit.GetWindowText( text );
	currentPlotInfo.changeTitle( str( text ) );
	yLabelEdit.GetWindowText( text );
	currentPlotInfo.changeYLabel( str( text ) );
	plotFilenameEdit.GetWindowText( text );
	currentPlotInfo.changeFileName( str( text ) );
	int runningAverageCheck = runningAverage.GetCheck();
	int variationAverageCheck = averageEachVariation.GetCheck();
	if (variationAverageCheck == BST_CHECKED)
	{
		if (runningAverageCheck == BST_CHECKED)
		{
			errBox( "Please select only one x-axis option." );
			return;
		}
		currentPlotInfo.changeXAxis( "Variation Average" );
	}
	else if (runningAverageCheck == BST_CHECKED)
	{
		if (variationAverageCheck == BST_CHECKED)
		{
			errBox( "Please select only one x-axis option." );
			return;
		}
		currentPlotInfo.changeXAxis( "Running Average" );
	}
	else
	{
		errBox( "Please select an x-axis option." );
		return;
	}
	/// get the (current) analysis pixel locations
	try
	{
		saveDataSet( false );
	}
	catch ( Error& err )
	{
		errBox( err.what( ) );
		return;
	}
	int result = promptBox( currentPlotInfo.getAllSettingsString() , MB_OKCANCEL );
	if (result == IDOK)
	{
		try
		{
			currentPlotInfo.savePlotInfo( );
		}
		catch ( Error& err )
		{
			errBox( "ERROR while saving plot info: " + err.whatStr( ) );
		}
		result = promptBox( "Close plot creator?", MB_YESNO );
		if (result == IDYES)
		{
			EndDialog( 0 );
		}
	}
}


void PlotDesignerDialog::handleCancel()
{
	EndDialog( 0 );
}


void PlotDesignerDialog::setFitRadios()
{
	int itemIndex = dataSetNumCombo.GetCurSel();
	if (itemIndex == -1 || itemIndex == 0)
	{
		return;
	}
	int fitCase = currentPlotInfo.getFitOption( UINT(itemIndex - 1) );
	gaussianFit.SetCheck( fitCase == GAUSSIAN_FIT );
	lorentzianFit.SetCheck( fitCase == LORENTZIAN_FIT );
	decayingSineFit.SetCheck( fitCase == SINE_FIT );
	int whenCase = currentPlotInfo.whenToFit( dataSetNumCombo.GetCurSel( ) );
	noFit.SetCheck( whenCase == NO_FIT );
	realTimeFit.SetCheck( whenCase == REAL_TIME_FIT );
	atFinishFit.SetCheck( whenCase == FIT_AT_END );
}


void PlotDesignerDialog::loadPositiveResultSettings()
{
	if ( prcPicNumCombo.GetCurSel( ) >= 0 && prcPixelNumCombo.GetCurSel( ) >= 0 && dataSetNumCombo.GetCurSel( ) >= 0)
	{
		// load current things.
		int currentValue = currentPlotInfo.getResultCondition( dataSetNumCombo.GetCurSel( ), prcPixelNumCombo.GetCurSel(),
															   prcPicNumCombo.GetCurSel( ) );
		if (currentValue == 1)
		{
			prcAtomBox.SetCheck( 1 );
			prcNoAtomBox.SetCheck( 0 );
		}
		else if (currentValue == -1)
		{
			prcAtomBox.SetCheck( 0 );
			prcNoAtomBox.SetCheck( 1 );
		}
		else
		{
			prcAtomBox.SetCheck( 0 );
			prcNoAtomBox.SetCheck( 0 );
		}
	}
}


void PlotDesignerDialog::loadPostSelectionConditions()
{
	if ( pscConditionNumCombo.GetCurSel( ) >= 0 && pscPicNumCombo.GetCurSel( ) >= 0 
		 && pscPixelNumCombo.GetCurSel( ) >= 0)
	{
		int currentValue = currentPlotInfo.getPostSelectionCondition( dataSetNumCombo.GetCurSel( ), 
																	  pscConditionNumCombo.GetCurSel( ),
																	  pscPixelNumCombo.GetCurSel( ),
																	  pscPicNumCombo.GetCurSel( ) );
		pscAtomBox.SetCheck( currentValue == 1 );
		pscNoAtomBox.SetCheck( currentValue == -1 );
	}
}


void PlotDesignerDialog::handleDataSetComboChange()
{
	int itemIndex = dataSetNumCombo.GetCurSel();
	if (itemIndex == -1)
	{
		return;
	}
	try
	{
		saveDataSet( true );
	}
	catch ( Error& err )
	{
		errBox( err.what( ) );
		return;
	}
	CString txt;
	dataSetNumCombo.GetLBText( itemIndex, txt );
	std::string dataSetString( txt );
	if (dataSetString == "Add New Data Set")
	{
		dataSetNumCombo.SetCurSel( -1 );
		int numberOfItems = dataSetNumCombo.GetCount();
		currentPlotInfo.addDataSet();
		dataSetNumCombo.ResetContent( );
		for ( auto dsetInc : range( numberOfItems-1 ) )
		{
			dataSetNumCombo.AddString( cstr( "Data Set #" + str( dsetInc+1 ) ) );
		}
		dataSetNumCombo.AddString( "Add New Data Set" );
		dataSetNumCombo.AddString( "Remove Data Set" );		
	}
	else if (dataSetString == "Remove Data Set")
	{
		int numberOfItems = dataSetNumCombo.GetCount();
		if (numberOfItems < 4)
		{
			errBox( "Can't delete last data set." );
			return;
		}
		// make data set struct smaller.
		currentPlotInfo.removeDataSet();
		dataSetNumCombo.DeleteString( numberOfItems - 3 );
		currentDataSet = -1;
	}
	else
	{
		int numberOfItems = dataSetNumCombo.GetCount();
		try
		{
			plotThisDataBox.SetCheck( currentPlotInfo.getPlotThisDataValue( UINT( itemIndex ) ) );
		}
		catch ( Error& err )
		{
			errBox( err.what( ) );
			return;
		}
		if (currentPlotInfo.getPlotType() == "Atoms")
		{
			loadPositiveResultSettings();
			loadPostSelectionConditions();
		}
		else
		{
			UINT pixel, picture;
			try
			{
				currentPlotInfo.getDataCountsLocation( dataSetNumCombo.GetCurSel( ), pixel, picture );
				prcPicNumCombo.SetCurSel( picture + 1 );
				prcPixelNumCombo.SetCurSel( pixel );
			}
			catch (Error&)
			{
				prcPicNumCombo.SetCurSel( -1 );
				prcPixelNumCombo.SetCurSel( -1 );
			}
			UINT width = currentPlotInfo.getDataSetHistBinWidth( dataSetNumCombo.GetCurSel( ) );
			binWidthEdit.SetWindowTextA( cstr( width ) );
		}
		currentDataSet = dataSetNumCombo.GetCurSel( );
		if ( dataSetNumCombo.GetCurSel( ) != -1)
		{
			legendEdit.SetWindowText( cstr( currentPlotInfo.getLegendText( dataSetNumCombo.GetCurSel( ) ) ) );
		}
	}
	enableAndDisable();
}


void PlotDesignerDialog::handlePrcPictureNumberChange()
{
	if (prcPicNumCombo.GetCurSel() == -1)
	{
		return;
	}
	try
	{
		// save before changing the number so that you save in the old config.
		savePositiveConditions( true );
		currentPrcPicture = prcPicNumCombo.GetCurSel( );
		loadPositiveResultSettings();
	}
	catch (Error& err)
	{
		errBox( err.what() );
	}
	enableAndDisable();
}


void PlotDesignerDialog::handlePrcPixelNumberChange()
{
	if (prcPixelNumCombo.GetCurSel() == -1)
	{
		return;
	}
	try
	{
		savePositiveConditions( true );
		currentPrcPixel = prcPixelNumCombo.GetCurSel( );
		loadPositiveResultSettings();
	}
	catch (Error& err)
	{
		errBox( err.what() );
	}
	enableAndDisable();
}


void PlotDesignerDialog::handlePscConditionNumberChange()
{
	if (pscConditionNumCombo.GetCurSel() == -1)
	{
		return;
	}
	try
	{
		savePostSelectionConditions( true );
		CString txt;
		pscConditionNumCombo.GetLBText( pscConditionNumCombo.GetCurSel( ), txt );
		if (txt == "Add New Condition")
		{
			pscConditionNumCombo.Clear();
			currentPscCondition = -1;
			UINT currentConditionNumber = pscConditionNumCombo.GetCount() - 2;
			pscConditionNumCombo.ResetContent();
			// +1 for new condition
			for (auto num : range( currentConditionNumber + 1 ))
			{
				pscConditionNumCombo.AddString( cstr( "Condition #" + str( num + 1 ) ) );
			}
			pscConditionNumCombo.AddString( "Add New Condition" );
			pscConditionNumCombo.AddString( "Add Remove Condition" );
			currentPlotInfo.addPostSelectionCondition();
		}
		else if (txt == "Remove Condition")
		{
			pscConditionNumCombo.Clear();
			currentPscCondition = -1;
			UINT currentConditionNumber = pscConditionNumCombo.GetCount() - 2;
			if (currentConditionNumber == 0)
			{
				errBox( "No Condition to remove!" );
			}
			pscConditionNumCombo.ResetContent();
			// -1 for removed condition
			for (auto num : range( currentConditionNumber - 1 ))
			{
				pscConditionNumCombo.AddString( cstr( "Condition #" + str( num + 1 ) ) );
			}
			pscConditionNumCombo.AddString( "Add New Condition" );
			pscConditionNumCombo.AddString( "Add Remove Condition" );
			currentPlotInfo.removePostSelectionCondition();
		}
		else
		{
			currentPscCondition = pscConditionNumCombo.GetCurSel( );
			loadPostSelectionConditions();
		}
	}
	catch (Error& err)
	{
		errBox( err.what() );
	}
	enableAndDisable();
}


void PlotDesignerDialog::handlePscPictureNumberChange()
{
	if (pscPicNumCombo.GetCurSel() == -1)
	{
		return;
	}
	try
	{
		savePostSelectionConditions( true );
		currentPscPicture = pscPicNumCombo.GetCurSel( );
		loadPostSelectionConditions();
	}
	catch (Error& err)
	{
		errBox( err.what() );
	}
	enableAndDisable();
}


void PlotDesignerDialog::handlePscPixelNumberChange()
{
	if (pscPixelNumCombo.GetCurSel() == -1)
	{
		return;
	}
	try
	{
		savePostSelectionConditions( true );
		currentPscPixel = pscPixelNumCombo.GetCurSel( );
		loadPostSelectionConditions();
	}
	catch (Error& err)
	{
		errBox( err.what() );
	}
	enableAndDisable();
}


void PlotDesignerDialog::handlePrcShowAll()
{
	savePositiveConditions( false );
	infoBox( currentPlotInfo.getPrcSettingsString() );
}


void PlotDesignerDialog::handlePscShowAll()
{
	savePostSelectionConditions( false );
	infoBox( currentPlotInfo.getPscSettingsString() );
}


void PlotDesignerDialog::handleGeneralPlotTypeChange()
{
	int itemIndex = generalPlotTypeCombo.GetCurSel();
	if (itemIndex == -1)
	{
		return;
	}
	if (currentPlotInfo.getPlotType() == "Atoms")
	{
		saveDataSet( false );
	}
	CString txt;
	generalPlotTypeCombo.GetLBText( itemIndex, txt );
	std::string plotTypeString( txt );
	currentPlotInfo.changeGeneralPlotType( plotTypeString );
	enableAndDisable();
}


void PlotDesignerDialog::savePositiveConditions( bool clear )
{
	// make sure that pictures and pixels are selected.
	if (currentPrcPicture >= 0 && currentPrcPixel >= 0 && dataSetNumCombo.GetCurSel( ) >= 0)
	{
		currentPlotInfo.setResultCondition( dataSetNumCombo.GetCurSel( ), currentPrcPixel, currentPrcPicture, 
											prcAtomBox.GetCheck( ) - prcNoAtomBox.GetCheck( ) );
	}
	if (clear)
	{
		prcAtomBox.SetCheck( 0 );
		prcNoAtomBox.SetCheck( 0 );
	}
}


void PlotDesignerDialog::savePostSelectionConditions( bool clear )
{
	if ( dataSetNumCombo.GetCurSel( ) >= 0 && currentPscCondition >= 0 && currentPscPixel >= 0 && currentPscPicture >= 0 )
	{
		currentPlotInfo.setPostSelCondition( dataSetNumCombo.GetCurSel( ), currentPscCondition, currentPscPixel, 
											 currentPscPicture, pscAtomBox.GetCheck( ) - pscNoAtomBox.GetCheck( ) );
	}
	if (clear)
	{
		pscAtomBox.SetCheck( 0 );
		pscNoAtomBox.SetCheck( 0 );
	}
}


void PlotDesignerDialog::saveDataSet( bool clear )
{
	savePositiveConditions( clear );
	savePostSelectionConditions( clear );
	if (currentDataSet != -1)
	{
		// legend
		CString txt;
		legendEdit.GetWindowText( txt );
		currentPlotInfo.changeLegendText( currentDataSet, str( txt ) );
		binWidthEdit.GetWindowTextA( txt );
		UINT width;
		try
		{
			 width = std::stod( str( txt ) );
		}
		catch ( std::invalid_argument& )
		{
			thrower( "ERROR: Failed to convert histogram bin width to an unsigned integer! width text was: " + str( txt ) );
		}
		currentPlotInfo.setDataSetHistBinWidth( currentDataSet, width );
		// fit options
		if (gaussianFit.GetCheck())
		{
			currentPlotInfo.setFitOption( currentDataSet, GAUSSIAN_FIT );
		}
		else if (lorentzianFit.GetCheck())
		{
			currentPlotInfo.setFitOption( currentDataSet, LORENTZIAN_FIT );
		}
		else if (decayingSineFit.GetCheck())
		{
			currentPlotInfo.setFitOption( currentDataSet, SINE_FIT );
		}
		if (noFit.GetCheck())
		{
			currentPlotInfo.setWhenToFit( currentDataSet, NO_FIT );
		}
		else if (atFinishFit.GetCheck())
		{
			currentPlotInfo.setWhenToFit( currentDataSet, FIT_AT_END );
		}
		else if (realTimeFit.GetCheck())
		{
			currentPlotInfo.setWhenToFit( currentDataSet, REAL_TIME_FIT );
		}
		
	}
}


void PlotDesignerDialog::enableAndDisable()
{
	dataSetNumCombo.EnableWindow( );
	pscShowAllButton.EnableWindow( );
	prcShowAllButton.EnableWindow( );
	saveButton.EnableWindow( );
	cancelButton.EnableWindow( );
	// each of the bools here determines whether a combo has a selection or not. 
	// There are simply a set of combos that must have a valid selection for a control it to be active.		
	bool dataSetSel = (dataSetNumCombo.GetCurSel( ) != -1);
	gaussianFit.EnableWindow( dataSetSel );
	lorentzianFit.EnableWindow( dataSetSel );
	decayingSineFit.EnableWindow( dataSetSel );
	noFit.EnableWindow( dataSetSel );
	realTimeFit.EnableWindow( dataSetSel );
	atFinishFit.EnableWindow( dataSetSel );
	legendEdit.EnableWindow( dataSetSel );
	prcPicNumCombo.EnableWindow( dataSetSel );
	pscConditionNumCombo.EnableWindow( dataSetSel );
	plotThisDataBox.EnableWindow( dataSetSel );
	bool prcPictureNumSel = (prcPicNumCombo.GetCurSel( ) != -1);
	prcPixelNumCombo.EnableWindow( dataSetSel && prcPictureNumSel );
	bool pscConditionNumSel = (pscConditionNumCombo.GetCurSel( ) != -1);
	pscPicNumCombo.EnableWindow( dataSetSel && pscConditionNumSel );
	bool pscPictureNumSel = (pscPicNumCombo.GetCurSel( ) != -1);
	pscPixelNumCombo.EnableWindow( dataSetSel && pscConditionNumSel && pscPictureNumSel );
	bool pscPixelNumSel = (pscPixelNumCombo.GetCurSel( ) != -1);
	pscAtomBox.EnableWindow( dataSetSel && pscConditionNumSel && pscPictureNumSel && pscPixelNumSel );
	pscNoAtomBox.EnableWindow( dataSetSel && pscConditionNumSel && pscPictureNumSel && pscPixelNumSel );
	bool prcPixelNumSel = (prcPixelNumCombo.GetCurSel( ) != -1);
	prcAtomBox.EnableWindow( dataSetSel && prcPictureNumSel && prcPixelNumSel && currentPlotInfo.getPlotType( ) == "Atoms" );
	prcNoAtomBox.EnableWindow( dataSetSel && prcPictureNumSel && prcPixelNumSel && currentPlotInfo.getPlotType( ) == "Atoms" );
	binWidthEdit.EnableWindow( currentPlotInfo.getPlotType( ) == "Pixel Count Histograms" );
}
