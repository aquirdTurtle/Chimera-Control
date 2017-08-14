#pragma once

#include "resource.h"
#include "Control.h"

#include "PlottingInfo.h"

class PlotDesignerDialog : public CDialog
{
	DECLARE_DYNAMIC( PlotDesignerDialog );

	public:
	PlotDesignerDialog( fontMap* fonts, UINT pictureNumber ) : CDialog( IDD_LARGE_TEMPLATE ), picNumber( pictureNumber ),
		currentPlotInfo( picNumber )
	{
		dlgFonts = fonts;
		currentDataSet = currentPrcPicture = currentPrcPixel = currentPscCondition = currentPscPicture =
			currentPscPixel = -1;
	}
	PlotDesignerDialog( fontMap* fonts, std::string fileName ) : CDialog( IDD_LARGE_TEMPLATE ),
		picNumber( PlottingInfo::getPicNumberFromFile( fileName ) ), currentPlotInfo( fileName )
	{
		dlgFonts = fonts;
		currentDataSet = currentPrcPicture = currentPrcPixel = currentPscCondition = currentPscPicture =
			currentPscPixel = -1;
	}
	BOOL OnInitDialog() override;
	void handlePixelEditChange();
	void handleSave();
	void handleCancel();
	void handleDataSetComboChange();
	void handlePrcPictureNumberChange();
	void handlePrcPixelNumberChange();
	void handlePscConditionNumberChange();
	void handlePscPictureNumberChange();
	void handlePscPixelNumberChange();
	void handlePrcShowAll();
	void handlePscShowAll();
	void handleGeneralPlotTypeChange();
	void savePositiveConditions( bool clear );
	void savePostSelectionConditions( bool clear );
	void saveDataSet( bool clear );
	void enableAndDisable();

	void handlePrcAtom();
	void handlePrcNoAtom();
	void handlePscAtom();
	void handlePscNoAtom();

	int getCurrentDataSetNumber();
	int getPrcPictureNumber();
	int getPrcPixelNumber();
	int getPscConditionNumber();
	int getPscPictureNumber();
	int getPscPixelNumber();

	void setFitRadios();
	void loadPositiveResultSettings();
	void loadPostSelectionConditions();

	private:
	DECLARE_MESSAGE_MAP();
	fontMap* dlgFonts;
	const UINT picNumber;
	int currentDataSet, currentPrcPicture, currentPrcPixel, currentPscCondition, currentPscPicture, currentPscPixel;

	PlottingInfo currentPlotInfo;
	// Lots of controls on this guy
	Control<CStatic> plotPropertiesText;

	Control<CStatic> plotTitleText;
	Control<CEdit> plotTitleEdit;

	Control<CStatic> yLabelText;
	Control<CEdit> yLabelEdit;

	Control<CStatic> plotFilenameText;
	Control<CEdit> plotFilenameEdit;

	Control<CStatic> generalPlotTypeText;
	Control<CComboBox> generalPlotTypeCombo;

	Control<CStatic> dataSetNumberText;
	Control<CComboBox> dataSetNumberCombo;

	Control<CButton> plotThisDataBox;

	Control<CStatic> xAxisText;
	Control<CButton> averageEachVariation;
	Control<CButton> runningAverage;

	Control<CStatic> analysisLocationsText;
	Control<CStatic> pixelsPerAnalysisGroupText;
	Control<CEdit> pixelsPerAnalysisGroupEdit;

	// PRC
	Control<CStatic> positiveResultConditionText;
	Control<CStatic> prcPictureNumberText;
	Control<CComboBox> prcPictureNumberCombo;
	Control<CStatic> prcPixelNumberText;
	Control<CComboBox> prcPixelNumberCombo;
	Control<CButton> prcAtomBox;
	Control<CButton> prcNoAtomBox;
	Control<CButton> prcShowAllButton;

	// PSC
	Control<CStatic> postSelectionConditionText;
	Control<CStatic> pscConditionNumberText;
	Control<CComboBox> pscConditionNumberCombo;
	Control<CStatic> pscPictureNumberText;
	Control<CComboBox> pscPictureNumberCombo;
	Control<CStatic> pscPixelNumberText;
	Control<CComboBox> pscPixelNumberCombo;
	Control<CButton> pscAtomBox;
	Control<CButton> pscNoAtomBox;
	Control<CButton> pscShowAllButton;

	// 
	Control<CStatic> aestheticsText;
	Control<CStatic> legendText;
	Control<CEdit> legendEdit;

	Control<CStatic> fitsText;
	Control<CButton> gaussianFit;
	Control<CButton> lorentzianFit;
	Control<CButton> decayingSineFit;
	Control<CButton> noFit;
	Control<CButton> realTimeFit;
	Control<CButton> atFinishFit;

	Control<CButton> saveButton;
	Control<CButton> cancelButton;

};
