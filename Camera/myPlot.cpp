#include "stdafx.h"
#include "myPlot.h"
#include <string>
#include <vector>
#include "externals.h"
#include "PlottingInfo.h"
#include "Resource.h"
#include "Windows.h"

// namespace for plot stuffs.
namespace myPlot
{
	int saveAnalysisPixelLocations(HWND dialogHandle, bool clearEdits)
	{
		if (eCurrentAnalysisSetSelectionNumber >= 0 && eCurrentPixelSelectionNumber >= 0)
		{
			// row
			TCHAR rowText[256];
			SendMessage(GetDlgItem(dialogHandle, IDC_PLOT_CREATOR_PIXEL_RO_EDIT), WM_GETTEXT, 256, (LPARAM)rowText);
			std::string rowString(rowText);
			TCHAR collumnText[256];
			SendMessage(GetDlgItem(dialogHandle, IDC_PLOT_CREATOR_PIXEL_COLLUMN_EDIT), WM_GETTEXT, 256, (LPARAM)collumnText);
			std::string collumnString(collumnText);
			if (rowString != "" && collumnString != "")
			{
				int rowInt;
				int collumnInt;
				try
				{
					rowInt = std::stoi(rowString);
					collumnInt = std::stoi(collumnString);
					eCurrentPlottingInfo.setGroupLocation(eCurrentPixelSelectionNumber, eCurrentAnalysisSetSelectionNumber, rowInt, collumnInt);
				}
				catch (std::invalid_argument&)
				{
					MessageBox(0, "Ivalid Row or collumn argument! Data Not Saved.", 0, 0);
				}
			}
			if (clearEdits)
			{
				// clear the edits for clarity.
				SendMessage(GetDlgItem(dialogHandle, IDC_PLOT_CREATOR_PIXEL_COLLUMN_EDIT), WM_SETTEXT, 0, (LPARAM)"");
				SendMessage(GetDlgItem(dialogHandle, IDC_PLOT_CREATOR_PIXEL_RO_EDIT), WM_SETTEXT, 0, (LPARAM)"");

			}
		}
		return 0;
	}

	int savePositiveConditions(HWND dialogHandle, bool clear)
	{
		// make sure that pictures and pixels are selected.
		if (eCurrentPositivePictureNumber >= 0 && eCurrentPositivePixelNumber >= 0 && eCurrentDataSetSelectionNumber >= 0)
		{
			eCurrentDataSetSelectionNumber;
			BOOL atomChecked = IsDlgButtonChecked(dialogHandle, IDC_PLOT_CREATOR_POSITIVE_ATOM_PRESENT_CHECK);
			BOOL noAtomChecked = IsDlgButtonChecked(dialogHandle, IDC_PLOT_CREATOR_POSITIVE_NO_ATOM_CHECK);
			if (atomChecked)
			{
				eCurrentPlottingInfo.setTruthCondition(eCurrentDataSetSelectionNumber, eCurrentPositivePixelNumber, eCurrentPositivePictureNumber, 1);
			}
			else if (noAtomChecked)
			{
				eCurrentPlottingInfo.setTruthCondition(eCurrentDataSetSelectionNumber, eCurrentPositivePixelNumber, eCurrentPositivePictureNumber, -1);
			}
			else
			{
				// no condition.
				eCurrentPlottingInfo.setTruthCondition(eCurrentDataSetSelectionNumber, eCurrentPositivePixelNumber, eCurrentPositivePictureNumber, 0);
			}
		}
		if (clear)
		{
			CheckDlgButton(dialogHandle, IDC_PLOT_CREATOR_POSITIVE_ATOM_PRESENT_CHECK, BST_UNCHECKED);
			CheckDlgButton(dialogHandle, IDC_PLOT_CREATOR_POSITIVE_NO_ATOM_CHECK, BST_UNCHECKED);
		}
		return 0;
	}

	int savePostSelectionConditions(HWND dialogHandle, bool clear)
	{
		// make sure that the condition number, pictures and pixels are selected.
		if (eCurrentPostSelectionPictureNumber >= 0 && eCurrentPostSelectionPixelNumber >= 0 && eCurrentPostSelectionConditionNumber >= 0
			&& eCurrentDataSetSelectionNumber >= 0)
		{
			BOOL atomChecked = IsDlgButtonChecked(dialogHandle, IDC_PLOT_CREATOR_POST_SELECTION_ATOM_PRESENT_CHECK);
			BOOL noAtomChecked = IsDlgButtonChecked(dialogHandle, IDC_PLOT_CREATOR_POST_SELECTION_NO_ATOM_CHECK);
			if (atomChecked)
			{
				// 1 is atom present condition
				eCurrentPlottingInfo.setPostSelectionCondition(eCurrentDataSetSelectionNumber, eCurrentPostSelectionConditionNumber,
					eCurrentPostSelectionPixelNumber, eCurrentPostSelectionPictureNumber, 1);
			}
			else if (noAtomChecked)
			{
				// -1 is no atom present condition
				eCurrentPlottingInfo.setPostSelectionCondition(eCurrentDataSetSelectionNumber, eCurrentPostSelectionConditionNumber,
					eCurrentPostSelectionPixelNumber, eCurrentPostSelectionPictureNumber, -1);
			}
			else
			{
				// no condition.
				eCurrentPlottingInfo.setPostSelectionCondition(eCurrentDataSetSelectionNumber, eCurrentPostSelectionConditionNumber,
					eCurrentPostSelectionPixelNumber, eCurrentPostSelectionPictureNumber, 0);
			}
		}
		if (clear)
		{
			CheckDlgButton(dialogHandle, IDC_PLOT_CREATOR_POST_SELECTION_ATOM_PRESENT_CHECK, BST_UNCHECKED);
			CheckDlgButton(dialogHandle, IDC_PLOT_CREATOR_POST_SELECTION_NO_ATOM_CHECK, BST_UNCHECKED);
		}
		return 0;
	}

	int saveDataSet(HWND dialogHandle, bool clear)
	{
		// truth conditions
		myPlot::savePositiveConditions(dialogHandle, clear);
		// post selection
		myPlot::savePostSelectionConditions(dialogHandle, clear);
		
		if (eCurrentDataSetSelectionNumber != -1)
		{
			// legend
			TCHAR legendText[256];
			SendMessage(GetDlgItem(dialogHandle, IDC_PLOT_CREATOR_LEGEND_TEXT_EDIT), WM_GETTEXT, 256, (LPARAM)legendText);
			eCurrentPlottingInfo.changeLegendText(eCurrentDataSetSelectionNumber, legendText);
			// fit options
			if (IsDlgButtonChecked(dialogHandle, IDC_GAUSSIAN_RADIO))
			{
				eCurrentPlottingInfo.setFitOption(eCurrentDataSetSelectionNumber, GAUSSIAN_FIT);
			}
			else if (IsDlgButtonChecked(dialogHandle, IDC_LORENTZIAN_RADIO))
			{
				eCurrentPlottingInfo.setFitOption(eCurrentDataSetSelectionNumber, LORENTZIAN_FIT);
			}
			else if (IsDlgButtonChecked(dialogHandle, IDC_SINE_RADIO))
			{
				eCurrentPlottingInfo.setFitOption(eCurrentDataSetSelectionNumber, SINE_FIT);
			}
			// when to fit
			if (IsDlgButtonChecked(dialogHandle, IDC_NO_FIT_RADIO))
			{
				eCurrentPlottingInfo.setWhenToFit(eCurrentDataSetSelectionNumber, NO_FIT);
			}
			else if (IsDlgButtonChecked(dialogHandle, IDC_FINISH_FIT_RADIO))
			{
				eCurrentPlottingInfo.setWhenToFit(eCurrentDataSetSelectionNumber, FIT_AT_END);
			}
			else if (IsDlgButtonChecked(dialogHandle, IDC_REAL_TIME_FIT_RADIO))
			{
				eCurrentPlottingInfo.setWhenToFit(eCurrentDataSetSelectionNumber, REAL_TIME_FIT);
			}
		}


		return 0;
	}

	int enableAndDisable(HWND dialogHandle)
	{

		if (eCurrentPlottingInfo.getPlotType() == "Atoms")
		{
			// enable ALL things. Load.
			if (eCurrentDataSetSelectionNumber == -1)
			{
				EnableWindow(GetDlgItem(dialogHandle, IDC_PLOT_CREATOR_POST_SELECTION_CONDITION_NUMBER_COMBO), false);
				EnableWindow(GetDlgItem(dialogHandle, IDC_PLOT_CREATOR_LEGEND_TEXT_EDIT), false);
				EnableWindow(GetDlgItem(dialogHandle, IDC_PLOT_CREATOR_POSITIVE_PIXEL_NUMBER_COMBO), false);
				EnableWindow(GetDlgItem(dialogHandle, IDC_PLOT_CREATOR_POST_PIXEL_NUMBER_COMBO), false);
				EnableWindow(GetDlgItem(dialogHandle, IDC_PLOT_CREATOR_POSITIVE_PICTURE_NUMBER_COMBO), false);
				EnableWindow(GetDlgItem(dialogHandle, IDC_PLOT_CREATOR_POST_PICTURE_NUMBER_COMBO), false);
				EnableWindow(GetDlgItem(dialogHandle, IDC_PLOT_CREATOR_POST_SELECTION_ATOM_PRESENT_CHECK), false);
				EnableWindow(GetDlgItem(dialogHandle, IDC_PLOT_CREATOR_POSITIVE_ATOM_PRESENT_CHECK), false);
				EnableWindow(GetDlgItem(dialogHandle, IDC_PLOT_CREATOR_POST_SELECTION_NO_ATOM_CHECK), false);
				EnableWindow(GetDlgItem(dialogHandle, IDC_PLOT_CREATOR_POSITIVE_NO_ATOM_CHECK), false);
				EnableWindow(GetDlgItem(dialogHandle, IDC_PLOT_CREATOR_PLOT_DATA_SET_CHECK), false);
				//
				EnableWindow(GetDlgItem(dialogHandle, IDC_GAUSSIAN_RADIO), false);
				EnableWindow(GetDlgItem(dialogHandle, IDC_LORENTZIAN_RADIO), false);
				EnableWindow(GetDlgItem(dialogHandle, IDC_SINE_RADIO), false);
				EnableWindow(GetDlgItem(dialogHandle, IDC_NO_FIT_RADIO), false);
				EnableWindow(GetDlgItem(dialogHandle, IDC_REAL_TIME_FIT_RADIO), false);
				EnableWindow(GetDlgItem(dialogHandle, IDC_FINISH_FIT_RADIO), false);
				
			}
			else
			{
				EnableWindow(GetDlgItem(dialogHandle, IDC_GAUSSIAN_RADIO), true);
				EnableWindow(GetDlgItem(dialogHandle, IDC_LORENTZIAN_RADIO), true);
				EnableWindow(GetDlgItem(dialogHandle, IDC_SINE_RADIO), true);
				EnableWindow(GetDlgItem(dialogHandle, IDC_NO_FIT_RADIO), true);
				EnableWindow(GetDlgItem(dialogHandle, IDC_REAL_TIME_FIT_RADIO), true);
				EnableWindow(GetDlgItem(dialogHandle, IDC_FINISH_FIT_RADIO), true);
				//
				EnableWindow(GetDlgItem(dialogHandle, IDC_PLOT_CREATOR_PLOT_DATA_SET_CHECK), true);
				EnableWindow(GetDlgItem(dialogHandle, IDC_PLOT_CREATOR_PLOT_DATA_SET_CHECK), true);
				EnableWindow(GetDlgItem(dialogHandle, IDC_PLOT_CREATOR_LEGEND_TEXT_EDIT), true);
				EnableWindow(GetDlgItem(dialogHandle, IDC_PLOT_CREATOR_POSITIVE_PICTURE_NUMBER_COMBO), true);
				EnableWindow(GetDlgItem(dialogHandle, IDC_PLOT_CREATOR_POST_SELECTION_CONDITION_NUMBER_COMBO), true);

				if (eCurrentPositivePictureNumber == -1)
				{
					EnableWindow(GetDlgItem(dialogHandle, IDC_PLOT_CREATOR_POSITIVE_PIXEL_NUMBER_COMBO), false);
					EnableWindow(GetDlgItem(dialogHandle, IDC_PLOT_CREATOR_POSITIVE_ATOM_PRESENT_CHECK), false);
					EnableWindow(GetDlgItem(dialogHandle, IDC_PLOT_CREATOR_POSITIVE_NO_ATOM_CHECK), false);
				}
				else
				{
					EnableWindow(GetDlgItem(dialogHandle, IDC_PLOT_CREATOR_POSITIVE_PIXEL_NUMBER_COMBO), true);
					EnableWindow(GetDlgItem(dialogHandle, IDC_PLOT_CREATOR_POSITIVE_ATOM_PRESENT_CHECK), true);
					EnableWindow(GetDlgItem(dialogHandle, IDC_PLOT_CREATOR_POSITIVE_NO_ATOM_CHECK), true);
				}

				if (eCurrentPostSelectionConditionNumber == -1)
				{
					EnableWindow(GetDlgItem(dialogHandle, IDC_PLOT_CREATOR_POST_PIXEL_NUMBER_COMBO), false);
					EnableWindow(GetDlgItem(dialogHandle, IDC_PLOT_CREATOR_POST_PICTURE_NUMBER_COMBO), false);
					EnableWindow(GetDlgItem(dialogHandle, IDC_PLOT_CREATOR_POST_SELECTION_ATOM_PRESENT_CHECK), false);
					EnableWindow(GetDlgItem(dialogHandle, IDC_PLOT_CREATOR_POST_SELECTION_NO_ATOM_CHECK), false);
				}
				else
				{
					EnableWindow(GetDlgItem(dialogHandle, IDC_PLOT_CREATOR_POST_PICTURE_NUMBER_COMBO), true);
					if (eCurrentPostSelectionPictureNumber == -1)
					{
						EnableWindow(GetDlgItem(dialogHandle, IDC_PLOT_CREATOR_POST_PIXEL_NUMBER_COMBO), false);
						EnableWindow(GetDlgItem(dialogHandle, IDC_PLOT_CREATOR_POST_SELECTION_ATOM_PRESENT_CHECK), false);
						EnableWindow(GetDlgItem(dialogHandle, IDC_PLOT_CREATOR_POST_SELECTION_NO_ATOM_CHECK), false);
					}
					else
					{
						EnableWindow(GetDlgItem(dialogHandle, IDC_PLOT_CREATOR_POST_PIXEL_NUMBER_COMBO), true);
						EnableWindow(GetDlgItem(dialogHandle, IDC_PLOT_CREATOR_POST_SELECTION_ATOM_PRESENT_CHECK), true);
						EnableWindow(GetDlgItem(dialogHandle, IDC_PLOT_CREATOR_POST_SELECTION_NO_ATOM_CHECK), true);
					}
				}
			}

			EnableWindow(GetDlgItem(dialogHandle, IDC_PLOT_CREATOR_DATA_SET_COMBO), true);
			EnableWindow(GetDlgItem(dialogHandle, IDC_PLOT_CREATOR_POST_SHOW_ALL), true);
			EnableWindow(GetDlgItem(dialogHandle, IDC_PLOT_CREATOR_POSITIVE_SHOW_ALL), true);
			EnableWindow(GetDlgItem(dialogHandle, IDC_PLOT_CREATOR_OK_BUTTON), true);
		}
		else if (eCurrentPlottingInfo.getPlotType() == "Pixel Counts" || eCurrentPlottingInfo.getPlotType() == "Pixel Count Histograms")
		{
			EnableWindow(GetDlgItem(dialogHandle, IDC_PLOT_CREATOR_POSITIVE_ATOM_PRESENT_CHECK), false);
			EnableWindow(GetDlgItem(dialogHandle, IDC_PLOT_CREATOR_POSITIVE_NO_ATOM_CHECK), false);

			if (eCurrentDataSetSelectionNumber == -1)
			{
				EnableWindow(GetDlgItem(dialogHandle, IDC_PLOT_CREATOR_POSITIVE_PICTURE_NUMBER_COMBO), false);
				EnableWindow(GetDlgItem(dialogHandle, IDC_PLOT_CREATOR_LEGEND_TEXT_EDIT), false);
				EnableWindow(GetDlgItem(dialogHandle, IDC_PLOT_CREATOR_POSITIVE_PIXEL_NUMBER_COMBO), false);
				EnableWindow(GetDlgItem(dialogHandle, IDC_PLOT_CREATOR_POST_PICTURE_NUMBER_COMBO), false);
				EnableWindow(GetDlgItem(dialogHandle, IDC_PLOT_CREATOR_POST_SELECTION_ATOM_PRESENT_CHECK), false);
				EnableWindow(GetDlgItem(dialogHandle, IDC_PLOT_CREATOR_POST_PIXEL_NUMBER_COMBO), false);
				EnableWindow(GetDlgItem(dialogHandle, IDC_PLOT_CREATOR_POST_SELECTION_NO_ATOM_CHECK), false);
				EnableWindow(GetDlgItem(dialogHandle, IDC_GAUSSIAN_RADIO), false);
				EnableWindow(GetDlgItem(dialogHandle, IDC_LORENTZIAN_RADIO), false);
				EnableWindow(GetDlgItem(dialogHandle, IDC_SINE_RADIO), false);
				EnableWindow(GetDlgItem(dialogHandle, IDC_NO_FIT_RADIO), false);
				EnableWindow(GetDlgItem(dialogHandle, IDC_REAL_TIME_FIT_RADIO), false);
				EnableWindow(GetDlgItem(dialogHandle, IDC_FINISH_FIT_RADIO), false);
			}
			else
			{
				EnableWindow(GetDlgItem(dialogHandle, IDC_GAUSSIAN_RADIO), true);
				EnableWindow(GetDlgItem(dialogHandle, IDC_LORENTZIAN_RADIO), true);
				EnableWindow(GetDlgItem(dialogHandle, IDC_SINE_RADIO), true);
				EnableWindow(GetDlgItem(dialogHandle, IDC_NO_FIT_RADIO), true);
				EnableWindow(GetDlgItem(dialogHandle, IDC_REAL_TIME_FIT_RADIO), true);
				EnableWindow(GetDlgItem(dialogHandle, IDC_FINISH_FIT_RADIO), true);
				EnableWindow(GetDlgItem(dialogHandle, IDC_PLOT_CREATOR_LEGEND_TEXT_EDIT), true);
				EnableWindow(GetDlgItem(dialogHandle, IDC_PLOT_CREATOR_POSITIVE_PICTURE_NUMBER_COMBO), true);
				EnableWindow(GetDlgItem(dialogHandle, IDC_PLOT_CREATOR_POST_SELECTION_CONDITION_NUMBER_COMBO), true);
				EnableWindow(GetDlgItem(dialogHandle, IDC_PLOT_CREATOR_PLOT_DATA_SET_CHECK), true);
				if (eCurrentPositivePictureNumber == -1)
				{
					EnableWindow(GetDlgItem(dialogHandle, IDC_PLOT_CREATOR_POSITIVE_PIXEL_NUMBER_COMBO), false);
				}
				else
				{
					EnableWindow(GetDlgItem(dialogHandle, IDC_PLOT_CREATOR_POSITIVE_PIXEL_NUMBER_COMBO), true);
				}

				if (eCurrentPostSelectionConditionNumber == -1)
				{
					EnableWindow(GetDlgItem(dialogHandle, IDC_PLOT_CREATOR_POST_PICTURE_NUMBER_COMBO), false);
					EnableWindow(GetDlgItem(dialogHandle, IDC_PLOT_CREATOR_POST_SELECTION_ATOM_PRESENT_CHECK), false);
					EnableWindow(GetDlgItem(dialogHandle, IDC_PLOT_CREATOR_POST_PIXEL_NUMBER_COMBO), false);
					EnableWindow(GetDlgItem(dialogHandle, IDC_PLOT_CREATOR_POST_SELECTION_NO_ATOM_CHECK), false);
				}
				else
				{
					EnableWindow(GetDlgItem(dialogHandle, IDC_PLOT_CREATOR_POST_PICTURE_NUMBER_COMBO), true);
					if (eCurrentPostSelectionPictureNumber == -1)
					{
						EnableWindow(GetDlgItem(dialogHandle, IDC_PLOT_CREATOR_POST_SELECTION_ATOM_PRESENT_CHECK), false);
						EnableWindow(GetDlgItem(dialogHandle, IDC_PLOT_CREATOR_POST_PIXEL_NUMBER_COMBO), false);
						EnableWindow(GetDlgItem(dialogHandle, IDC_PLOT_CREATOR_POST_SELECTION_NO_ATOM_CHECK), false);
					}
					else
					{
						EnableWindow(GetDlgItem(dialogHandle, IDC_PLOT_CREATOR_POST_SELECTION_ATOM_PRESENT_CHECK), true);
						EnableWindow(GetDlgItem(dialogHandle, IDC_PLOT_CREATOR_POST_PIXEL_NUMBER_COMBO), true);
						EnableWindow(GetDlgItem(dialogHandle, IDC_PLOT_CREATOR_POST_SELECTION_NO_ATOM_CHECK), true);
					}
				}
			}




			EnableWindow(GetDlgItem(dialogHandle, IDC_PLOT_CREATOR_DATA_SET_COMBO), true);
			EnableWindow(GetDlgItem(dialogHandle, IDC_PLOT_CREATOR_POST_SHOW_ALL), true);
			EnableWindow(GetDlgItem(dialogHandle, IDC_PLOT_CREATOR_POSITIVE_SHOW_ALL), true);
			EnableWindow(GetDlgItem(dialogHandle, IDC_PLOT_CREATOR_OK_BUTTON), true);
		}

		if (eCurrentAnalysisSetSelectionNumber == -1)
		{
			// all disabled.
			EnableWindow(GetDlgItem(dialogHandle, IDC_PLOT_CREATOR_PIXEL_NUMBER_COMBO), false);
			EnableWindow(GetDlgItem(dialogHandle, IDC_PLOT_CREATOR_PIXEL_COLLUMN_EDIT), false);
			EnableWindow(GetDlgItem(dialogHandle, IDC_PLOT_CREATOR_PIXEL_RO_EDIT), false);
		}
		else
		{
			EnableWindow(GetDlgItem(dialogHandle, IDC_PLOT_CREATOR_PIXEL_NUMBER_COMBO), true);
			if (eCurrentPixelSelectionNumber == -1)
			{
				EnableWindow(GetDlgItem(dialogHandle, IDC_PLOT_CREATOR_PIXEL_COLLUMN_EDIT), false);
				EnableWindow(GetDlgItem(dialogHandle, IDC_PLOT_CREATOR_PIXEL_RO_EDIT), false);

			}
			else
			{
				EnableWindow(GetDlgItem(dialogHandle, IDC_PLOT_CREATOR_PIXEL_COLLUMN_EDIT), true);
				EnableWindow(GetDlgItem(dialogHandle, IDC_PLOT_CREATOR_PIXEL_RO_EDIT), true);

			}
		}

		return 0;
	}
}