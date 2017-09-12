
#include "stdafx.h"
#include "PlotDesignerDialog.h"


IMPLEMENT_DYNAMIC( PlotDesignerDialog, CDialog )

BEGIN_MESSAGE_MAP( PlotDesignerDialog, CDialog )
	ON_WM_CTLCOLOR()
	ON_COMMAND( IDC_PLOT_CREATOR_SAVE, PlotDesignerDialog::handleSave )
	ON_COMMAND( IDC_PLOT_CREATOR_CANCEL, PlotDesignerDialog::handleCancel )
	ON_COMMAND( IDC_PRC_ATOM, PlotDesignerDialog::handlePrcAtom )
	ON_COMMAND( IDC_PRC_NOATOM, PlotDesignerDialog::handlePrcNoAtom )
	ON_COMMAND( IDC_PSC_ATOM, PlotDesignerDialog::handlePscAtom )
	ON_COMMAND( IDC_PSC_NOATOM, PlotDesignerDialog::handlePscNoAtom )

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

	// 
END_MESSAGE_MAP()


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
	prcPixelNumberCombo.Clear();
	prcPixelNumberCombo.ResetContent();
	pscPixelNumberCombo.Clear();
	pscPixelNumberCombo.ResetContent();
	for (auto num : range( pixelNum ))
	{
		std::string text( "Pixel #" + str( num + 1 ) );
		prcPixelNumberCombo.AddString( cstr( text ) );
		pscPixelNumberCombo.AddString( cstr( text ) );
	}
	currentPlotInfo.resetPixelNumber( pixelNum );
}

void PlotDesignerDialog::handlePrcAtom()
{
	if (prcAtomBox.GetCheck())
	{
		prcNoAtomBox.SetCheck( 0 );
	}
}


void PlotDesignerDialog::handlePrcNoAtom()
{
	if (prcNoAtomBox.GetCheck())
	{
		prcAtomBox.SetCheck( 0 );
	}
}


void PlotDesignerDialog::handlePscAtom()
{
	if (pscAtomBox.GetCheck())
	{
		pscNoAtomBox.SetCheck( 0 );
	}
}


void PlotDesignerDialog::handlePscNoAtom()
{
	if (pscNoAtomBox.GetCheck())
	{
		pscAtomBox.SetCheck( 0 );
	}
}


BOOL PlotDesignerDialog::OnInitDialog()
{
	POINT pos = { 0,0 };
	UINT id = 5000;
	plotPropertiesText.Create( "Plot Properties", WS_CHILD | WS_VISIBLE, { pos.x, pos.y, pos.x + 480, pos.y += 25 },
							   this, id++ );
	plotPropertiesText.SetFont( (*dlgFonts)["Heading Font Large"] );

	plotTitleText.Create( "Plot Title", WS_CHILD | WS_VISIBLE | WS_BORDER, { pos.x, pos.y, pos.x + 240, pos.y + 25 },
						  this, id++ );
	plotTitleEdit.Create( WS_CHILD | WS_VISIBLE | WS_TABSTOP, { pos.x + 240, pos.y, pos.x + 480, pos.y += 25 }, this, id++ );
	plotTitleEdit.SetWindowText( cstr( currentPlotInfo.getTitle() ) );

	yLabelText.Create( "Y-Axis Label", WS_CHILD | WS_VISIBLE | WS_BORDER, { pos.x, pos.y, pos.x + 240, pos.y + 25 },
					   this, id++ );
	yLabelEdit.Create( WS_CHILD | WS_VISIBLE | WS_TABSTOP, { pos.x + 240, pos.y, pos.x + 480, pos.y += 25 }, this, id++ );
	yLabelEdit.SetWindowText( cstr( currentPlotInfo.getYLabel() ) );

	plotFilenameText.Create( "Plot Filename", WS_CHILD | WS_VISIBLE, { pos.x, pos.y, pos.x + 240, pos.y + 25 }, this,
							 id++ );
	plotFilenameEdit.Create( WS_CHILD | WS_VISIBLE | WS_TABSTOP, { pos.x + 240, pos.y, pos.x + 480, pos.y += 25 }, this, id++ );
	plotFilenameEdit.SetWindowText( cstr( currentPlotInfo.getFileName() ) );

	generalPlotTypeText.Create( "Plot Type", WS_CHILD | WS_VISIBLE, { pos.x, pos.y, pos.x + 480, pos.y += 25 }, this,
								id++ );
	generalPlotTypeCombo.Create( CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE | WS_TABSTOP, { pos.x, \
								 pos.y, pos.x + 480, pos.y + 200 }, this, IDC_GENERAL_PLOT_TYPE );
	generalPlotTypeCombo.AddString( "Pixel Count Histograms" );
	generalPlotTypeCombo.AddString( "Pixel Counts" );
	generalPlotTypeCombo.AddString( "Atoms" );
	generalPlotTypeCombo.SelectString( 0, cstr( currentPlotInfo.getPlotType() ) );
	pos.y += 25;

	dataSetNumberText.Create( "Data Set #", WS_CHILD | WS_VISIBLE | WS_BORDER, { pos.x, pos.y, pos.x + 240, pos.y + 25 },
							  this, id++ );
	dataSetNumberCombo.Create( CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE | WS_TABSTOP, { pos.x + 240,
							   pos.y, pos.x + 480, pos.y + 200 }, this, IDC_PLOT_CREATOR_DATASET_COMBO );
	dataSetNumberCombo.AddString( "Data Set #1" );
	dataSetNumberCombo.AddString( "Add New Data Set" );
	dataSetNumberCombo.AddString( "Remove Data Set" );
	pos.y += 25;

	plotThisDataBox.Create( "Plot This Data", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, { pos.x + 240, pos.y, pos.x + 480,
							pos.y += 25 }, this, id++ );

	// PRC
	pos.y += 10;
	positiveResultConditionText.Create( "Positive Result Condition", WS_CHILD | WS_VISIBLE, { pos.x, pos.y, pos.x + 480,
										pos.y += 25 }, this, id++ );
	positiveResultConditionText.SetFont( (*dlgFonts)["Heading Font Large"] );

	prcPictureNumberText.Create( "Picture Number", WS_CHILD | WS_VISIBLE | WS_BORDER, { pos.x, pos.y, pos.x + 240,
								 pos.y + 25 }, this, id++ );
	prcPictureNumberCombo.Create( CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE | WS_TABSTOP,
									{ pos.x	+ 240, pos.y, pos.x + 480, pos.y + 200 }, this, IDC_PRC_PICTURE_NUMBER );
	for (auto num : range( picNumber ))
	{
		prcPictureNumberCombo.AddString( cstr( "Picture #" + str( num + 1 ) ) );
	}

	pos.y += 25;

	prcPixelNumberText.Create( "Pixel Number", WS_CHILD | WS_VISIBLE | WS_BORDER, { pos.x, pos.y, pos.x + 240,
							   pos.y + 25 }, this, id++ );
	prcPixelNumberCombo.Create( CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE | WS_TABSTOP, { pos.x + 240,
								pos.y, pos.x + 480, pos.y + 200 }, this, IDC_PRC_PIXEL_NUMBER );
	prcPixelNumberCombo.AddString( "Pixel #1" );

	pos.y += 25;
	prcAtomBox.Create( "Atom", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, { pos.x, pos.y, pos.x + 240, pos.y + 25 }, this,
					   IDC_PRC_ATOM );
	prcNoAtomBox.Create( "No Atom", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, { pos.x + 240, pos.y, pos.x + 480,
						 pos.y += 25 }, this, IDC_PRC_NOATOM );
	prcShowAllButton.Create( "Show All", WS_CHILD | WS_VISIBLE, { pos.x + 240, pos.y, pos.x + 480, pos.y += 25 },
							 this, IDC_PRC_SHOW_ALL );
	pos.y += 10;
	fitsText.Create( "Fits", WS_CHILD | WS_VISIBLE, { pos.x, pos.y, pos.x + 240, pos.y += 25 }, this, id++ );
	fitsText.SetFont( (*dlgFonts)["Heading Font Large"] );
	gaussianFit.Create( "Gaussian", WS_CHILD | WS_VISIBLE | WS_GROUP | BS_AUTORADIOBUTTON, { pos.x, pos.y, pos.x + 240,
						pos.y += 25 }, this, id++ );

	lorentzianFit.Create( "Lorentzian", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON, { pos.x, pos.y, pos.x + 240,
						  pos.y += 25 }, this, id++ );
	decayingSineFit.Create( "Decaying Sine", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON, { pos.x, pos.y, pos.x + 240,
							pos.y + 25 }, this, id++ );
	pos.y -= 50;
	noFit.Create( "None", WS_CHILD | WS_VISIBLE | WS_GROUP | BS_AUTORADIOBUTTON, { pos.x + 240, pos.y, pos.x + 480,
				  pos.y += 25 }, this, id++ );
	noFit.SetCheck( 1 );

	realTimeFit.Create( "Real Time Fit", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON, { pos.x + 240, pos.y, pos.x + 480,
						pos.y += 25 }, this, id++ );
	atFinishFit.Create( "At Finish", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON, { pos.x + 240, pos.y, pos.x + 480,
						pos.y += 25 }, this, id++ );
	setFitRadios();

	pos.x = 480;
	pos.y = 0;

	analysisLocationsText.Create( "Analysis Locations", WS_CHILD | WS_VISIBLE, { pos.x, pos.y, pos.x + 480,
								  pos.y += 25 }, this, id++ );
	analysisLocationsText.SetFont( (*dlgFonts)["Heading Font Large"] );
	pixelsPerAnalysisGroupText.Create( "Pixels Per Analysis Group", WS_CHILD | WS_VISIBLE | WS_BORDER, { pos.x, pos.y,
									   pos.x + 240, pos.y + 25 }, this, id++ );

	pixelsPerAnalysisGroupEdit.Create( WS_CHILD | WS_VISIBLE, { pos.x + 240, pos.y, pos.x + 480, pos.y += 25 }, this,
									   IDC_PIXELS_PER_ANALYSIS_GROUP );

	pixelsPerAnalysisGroupEdit.SetWindowText( "1" );

	xAxisText.Create( "X-Axis", WS_CHILD | WS_VISIBLE, { pos.x, pos.y, pos.x + 480, pos.y += 25 }, this, id++ );
	xAxisText.SetFont( (*dlgFonts)["Heading Font Large"] );

	averageEachVariation.Create( "Average Each Variation", WS_CHILD | WS_VISIBLE | WS_GROUP | BS_AUTORADIOBUTTON,
	{ pos.x, pos.y, pos.x + 480, pos.y += 25 }, this, id++ );
	averageEachVariation.SetCheck( 1 );

	runningAverage.Create( "Running Average (Continuous Mode Only)", WS_CHILD | WS_VISIBLE | WS_GROUP | BS_AUTORADIOBUTTON,
	{ pos.x, pos.y, pos.x + 480, pos.y += 25 }, this, id++ );

	// 
	aestheticsText.Create( "Aesthetics", WS_CHILD | WS_VISIBLE, { pos.x, pos.y, pos.x + 480, pos.y += 25 }, this, id++ );
	aestheticsText.SetFont( (*dlgFonts)["Heading Font Large"] );
	legendText.Create( "Legend Text", WS_CHILD | WS_VISIBLE, { pos.x, pos.y, pos.x + 240, pos.y + 25 }, this, id++ );
	legendEdit.Create( WS_CHILD | WS_VISIBLE, { pos.x + 240, pos.y, pos.x + 480, pos.y += 25 }, this, id++ );

	// PSC
	pos.y += 10;
	postSelectionConditionText.Create( "Post-Selection Conditions", WS_CHILD | WS_VISIBLE, { pos.x, pos.y, pos.x + 480,
									   pos.y += 25 }, this, id++ );
	postSelectionConditionText.SetFont( (*dlgFonts)["Heading Font Large"] );
	pscConditionNumberText.Create( "Condition Number", WS_CHILD | WS_VISIBLE, { pos.x, pos.y, pos.x + 240,
								   pos.y + 25 }, this, id++ );
	pscConditionNumberCombo.Create( CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE | WS_TABSTOP, { pos.x
									+ 240, pos.y, pos.x + 480, pos.y + 200 }, this, IDC_PSC_CONDITION_NUMBER );
	pscConditionNumberCombo.AddString( "Condition #1" );
	pscConditionNumberCombo.AddString( "Add New Condition" );
	pscConditionNumberCombo.AddString( "Remove Condition" );

	pos.y += 25;

	pscPictureNumberText.Create( "Picture Number", WS_CHILD | WS_VISIBLE, { pos.x, pos.y, pos.x + 240,
								 pos.y + 25 }, this, id++ );
	pscPictureNumberCombo.Create( CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE | WS_TABSTOP, { pos.x
								  + 240, pos.y, pos.x + 480,
								  pos.y + 200 }, this, IDC_PSC_PICTURE_NUMBER );
	for (auto num : range( picNumber ))
	{
		pscPictureNumberCombo.AddString( cstr( "Picture #" + str( num + 1 ) ) );
	}
	pos.y += 25;

	pscPixelNumberText.Create( "Pixel Number", WS_CHILD | WS_VISIBLE, { pos.x, pos.y, pos.x + 240, pos.y + 25 }, this,
							   id++ );
	pscPixelNumberCombo.Create( CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE | WS_TABSTOP, { pos.x + 240,
								pos.y, pos.x + 480, pos.y + 200 }, this, IDC_PSC_PIXEL_NUMBER );
	pscPixelNumberCombo.AddString( "Pixel #1" );
	pos.y += 25;
	pscAtomBox.Create( "Atom", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, { pos.x, pos.y, pos.x + 240, pos.y + 25 }, this,
					   IDC_PSC_ATOM );
	pscNoAtomBox.Create( "No Atom", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, { pos.x + 240, pos.y, pos.x + 480,
						 pos.y += 25 }, this, IDC_PSC_NOATOM );
	pscShowAllButton.Create( "Show All", WS_CHILD | WS_VISIBLE, { pos.x + 240, pos.y, pos.x + 480, pos.y += 25 },
							 this, IDC_PSC_SHOW_ALL );
	pos.y += 10;
	saveButton.Create( "SAVE", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, { pos.x, pos.y, pos.x + 240,
					   pos.y + 25 }, this, IDC_PLOT_CREATOR_SAVE );
	saveButton.SetFont( (*dlgFonts)["Heading Font Large"] );

	cancelButton.Create( "CANCEL", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, { pos.x + 240, pos.y, pos.x + 480,
						 pos.y + 25 }, this, IDC_PLOT_CREATOR_CANCEL );
	cancelButton.SetFont( (*dlgFonts)["Heading Font Large"] );

	enableAndDisable();

	return NULL;
}


void PlotDesignerDialog::handleSave()
{
	/// Save Everything
	// General Parameters: ////////
	// Get the title
	CString text;
	plotTitleEdit.GetWindowText( text );
	currentPlotInfo.changeTitle( str( text ) );
	// get the y label
	yLabelEdit.GetWindowText( text );
	currentPlotInfo.changeYLabel( str( text ) );
	// filename
	plotFilenameEdit.GetWindowText( text );
	currentPlotInfo.changeFileName( str( text ) );

	// x axis
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
	saveDataSet( false );

	int result = promptBox( currentPlotInfo.getAllSettingsString() , MB_OKCANCEL );
	if (result == IDOK)
	{
		// save.
		currentPlotInfo.savePlotInfo();
		//fileManage::reloadCombo(eAllPlotsCombo.hwnd, PLOT_FILES_SAVE_LOCATION, "*.plot", "__NONE__");
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
	int itemIndex = dataSetNumberCombo.GetCurSel();
	if (itemIndex == -1 || itemIndex == 0)
	{
		return;
	}
	int fitCase = currentPlotInfo.getFitOption( UINT(itemIndex - 1) );
	switch (fitCase)
	{
		case GAUSSIAN_FIT:
		{
			gaussianFit.SetCheck( 1 );
			lorentzianFit.SetCheck( 0 );
			decayingSineFit.SetCheck( 0 );
			break;
		}
		case LORENTZIAN_FIT:
		{
			gaussianFit.SetCheck( 0 );
			lorentzianFit.SetCheck( 1 );
			decayingSineFit.SetCheck( 0 );
			break;
		}
		case SINE_FIT:
		{
			gaussianFit.SetCheck( 0 );
			lorentzianFit.SetCheck( 0 );
			decayingSineFit.SetCheck( 1 );
			break;
		}
		default:
		{
			gaussianFit.SetCheck( 0 );
			lorentzianFit.SetCheck( 0 );
			decayingSineFit.SetCheck( 0 );
			break;
		}
	}
	int whenCase = currentPlotInfo.whenToFit( getCurrentDataSetNumber() );
	switch (whenCase)
	{
		case NO_FIT:
		{
			noFit.SetCheck( 1 );
			realTimeFit.SetCheck( 0 );
			atFinishFit.SetCheck( 0 );
			break;
		}
		case REAL_TIME_FIT:
		{
			noFit.SetCheck( 0 );
			realTimeFit.SetCheck( 1 );
			atFinishFit.SetCheck( 0 );
			break;
		}
		case FIT_AT_END:
		{
			noFit.SetCheck( 0 );
			realTimeFit.SetCheck( 0 );
			atFinishFit.SetCheck( 1 );
			break;
		}
		default:
		{
			noFit.SetCheck( 0 );
			realTimeFit.SetCheck( 0 );
			atFinishFit.SetCheck( 0 );
			break;
		}
	}
}


void PlotDesignerDialog::loadPositiveResultSettings()
{
	if (getPrcPictureNumber() >= 0 && getPrcPixelNumber() >= 0 && getCurrentDataSetNumber() >= 0)
	{
		// load current things.
		int currentValue = currentPlotInfo.getResultCondition( getCurrentDataSetNumber(),
															   getPrcPixelNumber(), getPrcPictureNumber() );
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
	if (getPscConditionNumber() >= 0 && getPscPictureNumber() >= 0 && getPscPixelNumber() >= 0)
	{
		int currentValue = currentPlotInfo.getPostSelectionCondition( getCurrentDataSetNumber(), getPscConditionNumber(),
																	  getPscPixelNumber(), getPscPictureNumber() );
		if (currentValue == 1)
		{
			pscAtomBox.SetCheck( 1 );
			pscNoAtomBox.SetCheck( 0 );
		}
		else if (currentValue == -1)
		{
			pscAtomBox.SetCheck( 0 );
			pscNoAtomBox.SetCheck( 1 );
		}
		else
		{
			pscAtomBox.SetCheck( 0 );
			pscNoAtomBox.SetCheck( 0 );
		}
	}
}


void PlotDesignerDialog::handleDataSetComboChange()
{
	CString txt;
	int itemIndex = dataSetNumberCombo.GetCurSel();
	if (itemIndex == -1)
	{
		// user didn't select anything.
		return;
	}
	// save data set before changing
	saveDataSet( true );

	dataSetNumberCombo.GetLBText( itemIndex, txt );
	std::string dataSetString( txt );

	if (dataSetString == "Add New Data Set")
	{
		dataSetNumberCombo.SetCurSel( -1 );
		int numberOfItems = dataSetNumberCombo.GetCount();
		currentPlotInfo.addDataSet();
		dataSetNumberCombo.AddString( cstr( "Data Set #" + str( numberOfItems - 1 ) ) );
	}
	else if (dataSetString == "Remove Data Set")
	{
		int numberOfItems = dataSetNumberCombo.GetCount();
		if (numberOfItems < 4)
		{
			errBox( "Can't delete last data set." );
			return;
		}
		// make data set struct smaller.
		currentPlotInfo.removeDataSet();
		dataSetNumberCombo.DeleteString( numberOfItems - 2 );
	}
	else
	{
		int numberOfItems = dataSetNumberCombo.GetCount();
		// reload things...
		// get the plot conditions.
		// if plotting...
		if (currentPlotInfo.getPlotThisDataValue( UINT(itemIndex) ))
		{
			// check it.
			plotThisDataBox.SetCheck( 1 );
		}
		else
		{
			plotThisDataBox.SetCheck( 0 );
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
				currentPlotInfo.getDataCountsLocation( getCurrentDataSetNumber(), pixel, picture );
				prcPictureNumberCombo.SetCurSel( picture + 1 );
				prcPixelNumberCombo.SetCurSel( pixel );
			}
			catch (Error&)
			{
				prcPictureNumberCombo.SetCurSel( -1 );
				prcPixelNumberCombo.SetCurSel( -1 );
			}
		}
		if (getCurrentDataSetNumber() != -1)
		{
			legendEdit.SetWindowText( cstr( currentPlotInfo.getLegendText( getCurrentDataSetNumber() ) ) );
		}
	}
	enableAndDisable();
}


void PlotDesignerDialog::handlePrcPictureNumberChange()
{
	if (prcPictureNumberCombo.GetCurSel() == -1)
	{
		// user didn't select anything.
		return;
	}
	// save options.
	try
	{
		// save before changing the number so that you save in the old config.
		savePositiveConditions( true );
		currentPrcPicture = getPrcPictureNumber();
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
	if (prcPixelNumberCombo.GetCurSel() == -1)
	{
		// user didn't select anything.
		return;
	}
	// save options.
	try
	{
		savePositiveConditions( true );
		currentPrcPixel = getPrcPixelNumber();
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
	if (pscConditionNumberCombo.GetCurSel() == -1)
	{
		// user didn't select anything.
		return;
	}
	// save options.
	try
	{
		savePostSelectionConditions( true );
		CString txt;
		pscConditionNumberCombo.GetLBText( getPscConditionNumber(), txt );

		if (txt == "Add New Condition")
		{
			pscConditionNumberCombo.Clear();
			currentPscCondition = -1;
			UINT currentConditionNumber = pscConditionNumberCombo.GetCount() - 2;
			pscConditionNumberCombo.ResetContent();
			// +1 for new condition
			for (auto num : range( currentConditionNumber + 1 ))
			{
				pscConditionNumberCombo.AddString( cstr( "Condition #" + str( num + 1 ) ) );
			}
			pscConditionNumberCombo.AddString( "Add New Condition" );
			pscConditionNumberCombo.AddString( "Add Remove Condition" );
			currentPlotInfo.addPostSelectionCondition();
		}
		else if (txt == "Remove Condition")
		{
			pscConditionNumberCombo.Clear();
			currentPscCondition = -1;
			UINT currentConditionNumber = pscConditionNumberCombo.GetCount() - 2;
			if (currentConditionNumber == 0)
			{
				errBox( "No Condition to remove!" );
			}
			pscConditionNumberCombo.ResetContent();
			// -1 for removed condition
			for (auto num : range( currentConditionNumber - 1 ))
			{
				pscConditionNumberCombo.AddString( cstr( "Condition #" + str( num + 1 ) ) );
			}
			pscConditionNumberCombo.AddString( "Add New Condition" );
			pscConditionNumberCombo.AddString( "Add Remove Condition" );
			currentPlotInfo.removePostSelectionCondition();
		}
		else
		{
			currentPscCondition = getPscConditionNumber();
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
	if (pscPictureNumberCombo.GetCurSel() == -1)
	{
		// user didn't select anything.
		return;
	}
	// save options.
	try
	{
		savePostSelectionConditions( true );
		currentPscPicture = getPscPictureNumber();
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
	if (pscPixelNumberCombo.GetCurSel() == -1)
	{
		// user didn't select anything.
		return;
	}
	// save options.
	try
	{
		savePostSelectionConditions( true );
		currentPscPixel = getPscPixelNumber();
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
	// get the text.		
	int itemIndex = generalPlotTypeCombo.GetCurSel();
	if (itemIndex == -1)
	{
		// user didn't select anything.
		return;
	}
	if (currentPlotInfo.getPlotType() == "Atoms")
	{
		// not sure why save...
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
	if (currentPrcPicture >= 0 && currentPrcPixel >= 0 && getCurrentDataSetNumber() >= 0)
	{
		if (prcAtomBox.GetCheck())
		{
			currentPlotInfo.setResultCondition( getCurrentDataSetNumber(), currentPrcPixel, currentPrcPicture, 1 );
		}
		else if (prcNoAtomBox.GetCheck())
		{
			currentPlotInfo.setResultCondition( getCurrentDataSetNumber(), currentPrcPixel, currentPrcPicture, -1 );
		}
		else
		{
			// no condition.
			currentPlotInfo.setResultCondition( getCurrentDataSetNumber(), currentPrcPixel, currentPrcPicture, 0 );
		}
	}
	if (clear)
	{
		prcAtomBox.SetCheck( 0 );
		prcNoAtomBox.SetCheck( 0 );
	}
}


int PlotDesignerDialog::getCurrentDataSetNumber()
{
	return dataSetNumberCombo.GetCurSel();
}

int PlotDesignerDialog::getPrcPictureNumber()
{
	return prcPictureNumberCombo.GetCurSel();
}


int PlotDesignerDialog::getPrcPixelNumber()
{
	return prcPixelNumberCombo.GetCurSel();
}


int PlotDesignerDialog::getPscConditionNumber()
{
	return pscConditionNumberCombo.GetCurSel();
}


int PlotDesignerDialog::getPscPictureNumber()
{
	return pscPictureNumberCombo.GetCurSel();
}


int PlotDesignerDialog::getPscPixelNumber()
{
	return pscPixelNumberCombo.GetCurSel();
}


void PlotDesignerDialog::savePostSelectionConditions( bool clear )
{
	// prob need to change this
	if (getCurrentDataSetNumber() >= 0 && currentPscCondition >= 0 && currentPscPixel >= 0 && currentPscPicture >= 0)
	{
		if (pscAtomBox.GetCheck())
		{
			// 1 is atom present condition
			currentPlotInfo.setPostSelectionCondition( getCurrentDataSetNumber( ), currentPscCondition, currentPscPixel,
													   currentPscPicture, 1 );
		}
		else if (pscNoAtomBox.GetCheck())
		{
			// -1 is no atom present condition
			currentPlotInfo.setPostSelectionCondition( getCurrentDataSetNumber( ), currentPscCondition, currentPscPixel,
													   currentPscPicture, -1 );
		}
		else
		{
			// no condition.
			currentPlotInfo.setPostSelectionCondition( getCurrentDataSetNumber( ), currentPscCondition, currentPscPixel,
													   currentPscPicture, 0 );
		}
	}
	if (clear)
	{
		pscAtomBox.SetCheck( 0 );
		pscNoAtomBox.SetCheck( 0 );
	}
}


void PlotDesignerDialog::saveDataSet( bool clear )
{
	// truth conditions
	savePositiveConditions( clear );
	// post selection
	savePostSelectionConditions( clear );

	if (getCurrentDataSetNumber() != -1)
	{
		// legend
		CString legendText;
		legendEdit.GetWindowText( legendText );
		currentPlotInfo.changeLegendText( getCurrentDataSetNumber(), str( legendText ) );
		// fit options
		if (gaussianFit.GetCheck())
		{
			currentPlotInfo.setFitOption( getCurrentDataSetNumber(), GAUSSIAN_FIT );
		}
		else if (lorentzianFit.GetCheck())
		{
			currentPlotInfo.setFitOption( getCurrentDataSetNumber(), LORENTZIAN_FIT );
		}
		else if (decayingSineFit.GetCheck())
		{
			currentPlotInfo.setFitOption( getCurrentDataSetNumber(), SINE_FIT );
		}
		// when to fit
		if (noFit.GetCheck())
		{
			currentPlotInfo.setWhenToFit( getCurrentDataSetNumber(), NO_FIT );
		}
		else if (atFinishFit.GetCheck())
		{
			currentPlotInfo.setWhenToFit( getCurrentDataSetNumber(), FIT_AT_END );
		}
		else if (realTimeFit.GetCheck())
		{
			currentPlotInfo.setWhenToFit( getCurrentDataSetNumber(), REAL_TIME_FIT );
		}
	}
}


void PlotDesignerDialog::enableAndDisable()
{
	if (currentPlotInfo.getPlotType() == "Atoms")
	{
		// Throughout this function, if a number such as the following returns as -1, it means that the number hasn't
		// been selected yet.
		if (getCurrentDataSetNumber() == -1)
		{
			// disable nearly everything. you need to set the data set number.
			pscConditionNumberCombo.EnableWindow( 0 );
			legendEdit.EnableWindow( 0 );
			prcPixelNumberCombo.EnableWindow( 0 );
			pscPixelNumberCombo.EnableWindow( 0 );
			prcPictureNumberCombo.EnableWindow( 0 );
			pscPictureNumberCombo.EnableWindow( 0 );
			pscAtomBox.EnableWindow( 0 );
			pscNoAtomBox.EnableWindow( 0 );
			prcAtomBox.EnableWindow( 0 );
			pscNoAtomBox.EnableWindow( 0 );
			plotThisDataBox.EnableWindow( 0 );
			//
			gaussianFit.EnableWindow( 0 );
			lorentzianFit.EnableWindow( 0 );
			decayingSineFit.EnableWindow( 0 );
			noFit.EnableWindow( 0 );
			realTimeFit.EnableWindow( 0 );
			atFinishFit.EnableWindow( 0 );
		}
		else
		{
			gaussianFit.EnableWindow();
			lorentzianFit.EnableWindow();
			decayingSineFit.EnableWindow();
			noFit.EnableWindow();
			realTimeFit.EnableWindow();
			atFinishFit.EnableWindow();
			//
			plotThisDataBox.EnableWindow();
			prcPictureNumberCombo.EnableWindow();
			pscConditionNumberCombo.EnableWindow();
			legendEdit.EnableWindow();

			if (getPrcPictureNumber() == -1)
			{
				prcPixelNumberCombo.EnableWindow( 0 );
				prcAtomBox.EnableWindow( 0 );
				prcNoAtomBox.EnableWindow( 0 );
			}
			else
			{
				prcPixelNumberCombo.EnableWindow();
				if (getPrcPixelNumber() == -1)
				{
					prcAtomBox.EnableWindow( 0 );
					prcNoAtomBox.EnableWindow( 0 );
				}
				else
				{
					prcAtomBox.EnableWindow();
					prcNoAtomBox.EnableWindow();
				}
			}

			if (getPscConditionNumber() == -1)
			{
				pscPixelNumberCombo.EnableWindow( 0 );
				pscPictureNumberCombo.EnableWindow( 0 );
				pscAtomBox.EnableWindow( 0 );
				pscNoAtomBox.EnableWindow( 0 );
			}
			else
			{
				pscPictureNumberCombo.EnableWindow();
				if (getPscPictureNumber() == -1)
				{
					pscPixelNumberCombo.EnableWindow( 0 );
					pscAtomBox.EnableWindow( 0 );
					pscNoAtomBox.EnableWindow( 0 );
				}
				else
				{
					pscPixelNumberCombo.EnableWindow();
					if (getPscPixelNumber() == -1)
					{
						pscAtomBox.EnableWindow( 0 );
						pscNoAtomBox.EnableWindow( 0 );
					}
					else
					{
						pscAtomBox.EnableWindow();
						pscNoAtomBox.EnableWindow();
					}
				}
			}
		}
		dataSetNumberCombo.EnableWindow();
		pscShowAllButton.EnableWindow();
		prcShowAllButton.EnableWindow();
		saveButton.EnableWindow();
		cancelButton.EnableWindow();
	}
	else if (currentPlotInfo.getPlotType() == "Pixel Counts" || currentPlotInfo.getPlotType() == "Pixel Count Histograms")
	{
		// not atom, so these are always disabled.
		prcAtomBox.EnableWindow( 0 );
		prcNoAtomBox.EnableWindow( 0 );

		if (getCurrentDataSetNumber() == -1)
		{
			prcPictureNumberCombo.EnableWindow( 0 );
			legendEdit.EnableWindow( 0 );
			prcPixelNumberCombo.EnableWindow( 0 );
			pscConditionNumberCombo.EnableWindow( 0 );
			pscPictureNumberCombo.EnableWindow( 0 );
			pscPixelNumberCombo.EnableWindow( 0 );
			pscAtomBox.EnableWindow( 0 );
			pscNoAtomBox.EnableWindow( 0 );

			gaussianFit.EnableWindow( 0 );
			lorentzianFit.EnableWindow( 0 );
			decayingSineFit.EnableWindow( 0 );
			noFit.EnableWindow( 0 );
			realTimeFit.EnableWindow( 0 );
			atFinishFit.EnableWindow( 0 );
		}
		else
		{
			gaussianFit.EnableWindow();
			lorentzianFit.EnableWindow();
			decayingSineFit.EnableWindow();
			noFit.EnableWindow();
			realTimeFit.EnableWindow();
			atFinishFit.EnableWindow();

			legendEdit.EnableWindow();
			prcPictureNumberCombo.EnableWindow();
			pscConditionNumberCombo.EnableWindow();
			plotThisDataBox.EnableWindow();

			if (getPrcPictureNumber() == -1)
			{
				prcPixelNumberCombo.EnableWindow( 0 );
			}
			else
			{
				prcPixelNumberCombo.EnableWindow();
			}

			if (getPscConditionNumber() == -1)
			{
				pscPictureNumberCombo.EnableWindow( 0 );
				pscPixelNumberCombo.EnableWindow( 0 );
				pscAtomBox.EnableWindow( 0 );
				pscNoAtomBox.EnableWindow( 0 );
			}
			else
			{
				pscPictureNumberCombo.EnableWindow();
				if (getPscPictureNumber() == -1)
				{
					pscPixelNumberCombo.EnableWindow( 0 );
					pscAtomBox.EnableWindow( 0 );
					pscNoAtomBox.EnableWindow( 0 );
				}
				else
				{
					pscPixelNumberCombo.EnableWindow();
					pscAtomBox.EnableWindow();
					pscNoAtomBox.EnableWindow();
				}
			}
		}

		dataSetNumberCombo.EnableWindow();
		pscShowAllButton.EnableWindow();
		prcShowAllButton.EnableWindow();
		saveButton.EnableWindow();
		cancelButton.EnableWindow();
	}
	else
	{
		// unexepcted general plot type?!?!?
	}
}

