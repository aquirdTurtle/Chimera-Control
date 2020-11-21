#pragma once

#include <qdialog.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <Plotting/PlottingInfo.h>

class QtPlotDesignerDlg : public QDialog {
	Q_OBJECT;
	public:
		QtPlotDesignerDlg ();

		QtPlotDesignerDlg (unsigned pictureNumber) : picNumber (pictureNumber), currentPlotInfo (picNumber){
		}
		QtPlotDesignerDlg (std::string fileName) : picNumber (PlottingInfo::getPicNumberFromFile (fileName)), 
			currentPlotInfo (fileName) {
		}
		void init();
		void handlePixelEditChange ();
		void handleSave ();
		void handleCancel ();
		void handleDataSetComboChange ();
		void handlePrcPictureNumberChange ();
		void handlePrcPixelNumberChange ();
		void handlePscConditionNumberChange ();
		void handlePscPictureNumberChange ();
		void handlePscPixelNumberChange ();
		void handlePrcShowAll ();
		void handlePscShowAll ();
		void handleGeneralPlotTypeChange ();
		void savePositiveConditions (bool clear);
		void savePostSelectionConditions (bool clear);
		void saveDataSet (bool clear);
		void enableAndDisable ();
		void setFitRadios ();
		void loadPositiveResultSettings ();
		void loadPostSelectionConditions ();

	private:
		const unsigned picNumber;
		int currentDataSet = -1, currentPrcPicture = -1, currentPrcPixel = -1, currentPscCondition = -1, 
			currentPscPicture = -1, currentPscPixel = -1;
		PlottingInfo currentPlotInfo;
		/// Lots of controls on this guy
		QLabel* plotPropertiesText;

		QLabel* plotTitleText;
		QLineEdit* plotTitleEdit;

		QLabel* yLabelText;
		QLineEdit* yLabelEdit;

		QLabel* plotFilenameText;
		QLineEdit* plotFilenameEdit;

		QLabel* generalPlotTypeText;
		QComboBox* generalPlotTypeCombo;

		QLabel* dataSetNumberText;
		QComboBox* dataSetNumCombo;

		QPushButton* plotThisDataBox;

		QLabel* xAxisText;
		QPushButton* averageEachVariation;
		QPushButton* runningAverage;

		QLabel* analysisLocationsText;
		QLabel* pixelsPerAnalysisGroupText;
		QLineEdit* pixelsPerAnalysisGroupEdit;
		// PRC
		QLabel* positiveResultConditionText;
		QLabel* prcPictureNumberText;
		QComboBox* prcPicNumCombo;
		QLabel* prcPixelNumberText;
		QComboBox* prcPixelNumCombo;
		QPushButton* prcAtomBox;
		QPushButton* prcNoAtomBox;
		QPushButton* prcShowAllButton;

		// PSC
		QLabel* postSelectionConditionText;
		QLabel* pscConditionNumberText;
		QComboBox* pscConditionNumCombo;
		QLabel* pscPictureNumberText;
		QComboBox* pscPicNumCombo;
		QLabel* pscPixelNumberText;
		QComboBox* pscPixelNumCombo;
		QPushButton* pscAtomBox;
		QPushButton* pscNoAtomBox;
		QPushButton* pscShowAllButton;

		//
		QLabel* aestheticsText;
		QLabel* binWidthText;
		QLineEdit* binWidthEdit;
		QLabel* legendText;
		QLineEdit* legendEdit;

		QLabel* fitsText;
		QPushButton* gaussianFit;
		QPushButton* lorentzianFit;
		QPushButton* decayingSineFit;
		QPushButton* noFit;
		QPushButton* realTimeFit;
		QPushButton* atFinishFit;

		QPushButton* saveButton;
		QPushButton* cancelButton;
};
