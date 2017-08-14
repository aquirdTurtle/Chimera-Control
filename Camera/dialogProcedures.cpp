#include "stdafx.h"
#include "dialogProcedures.h"
#include "fonts.h"
#include "resource.h"
#include "myPlot.h"
#include "fileManage.h"
// Includes all procedures for dialog boxes that I use in this program.
namespace dialogProcedures
{
	LRESULT CALLBACK richEditMessageBoxProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
	{
		switch (message)
		{
			case WM_INITDIALOG:
			{
				LoadLibrary(TEXT("Msftedit.dll"));
				eRichEditMessageBoxRichEditHandle = CreateWindowEx(0, _T("RICHEDIT50W"), "",
					WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL | ES_AUTOHSCROLL | WS_HSCROLL | ES_READONLY,
					0, 0, 745, 485, hwndDlg, (HMENU)IDC_RICH_EDIT_MESSAGE_BOX_RICH_EDIT_ID, GetModuleHandle(NULL), NULL);
				SendMessage(eRichEditMessageBoxRichEditHandle, WM_SETFONT, WPARAM(sCodeFontMax), TRUE);
				break;
			}
			case WM_CLOSE:
			case WM_DESTROY:
			{
				DestroyWindow(hwndDlg);
				break;
			}
		}
		return FALSE;
	}

	LRESULT CALLBACK richEditOkCancelMessageBoxProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
	{
		switch (message)
		{
			case WM_INITDIALOG:
			{
				LoadLibrary(TEXT("Msftedit.dll"));
				eRichEditOkCancelMessageBoxRichEditHandle = CreateWindowEx(0, _T("RICHEDIT50W"), "",
					WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL | ES_AUTOHSCROLL | WS_HSCROLL | ES_READONLY,
					0, 0, 745, 485, hwndDlg, (HMENU)IDC_RICH_EDIT_OK_CANCEL_MESSAGE_BOX_RICH_EDIT_ID, GetModuleHandle(NULL), NULL);
				SendMessage(eRichEditOkCancelMessageBoxRichEditHandle, WM_SETFONT, WPARAM(sCodeFontMax), TRUE);
				std::string msg((const char *)lParam);
				SendMessage(eRichEditOkCancelMessageBoxRichEditHandle, WM_SETTEXT, 0, (LPARAM)msg.c_str());
				return TRUE;
			}
			case WM_CLOSE:
			case WM_DESTROY:
			{
				return FALSE;
			}
			case WM_COMMAND:
			{
				switch (LOWORD(wParam))
				{
					case IDC_RICH_EDIT_OK_CANCEL_MESSAGE_BOX_OK_BUTTON_ID:
					{
						EndDialog(hwndDlg, IDOK);
						return TRUE;
					}
					case IDC_RICH_EDIT_OK_CANCEL_MESSAGE_BOX_CANCEL_ID:
					{
						EndDialog(hwndDlg, IDCANCEL);
						return TRUE;
					}
				}
			}
		}
		return FALSE;
	}

	LRESULT CALLBACK plottingDialogProc(HWND thisDialogHandle, UINT message, WPARAM wParam, LPARAM lParam)
	{
		switch (message)
		{
			case WM_INITDIALOG:
			{
				/// Load predetermined Combos.
				// Pixel #
				HWND pixelNumHandle = GetDlgItem(thisDialogHandle, IDC_PLOT_CREATOR_PIXEL_NUMBER_COMBO);
				for (int pixelInc = 0; pixelInc < eCurrentPlottingInfo.getPixelNumber(); pixelInc++)
				{
					SendMessage(pixelNumHandle, CB_ADDSTRING, 0, (LPARAM)("Pixel #" + std::to_string(pixelInc + 1) + " Location:").c_str());
				}
				SendMessage(pixelNumHandle, CB_ADDSTRING, 0, (LPARAM)"New Pixel");
				SendMessage(pixelNumHandle, CB_ADDSTRING, 0, (LPARAM)"Remove Pixel");
				EnableWindow(GetDlgItem(thisDialogHandle, IDC_PLOT_CREATOR_PIXEL_NUMBER_COMBO), false);
				EnableWindow(GetDlgItem(thisDialogHandle, IDC_PLOT_CREATOR_PIXEL_RO_EDIT), false);
				EnableWindow(GetDlgItem(thisDialogHandle, IDC_PLOT_CREATOR_PIXEL_COLLUMN_EDIT), false);
				// Data Set #
				HWND dataSetNumHandle = GetDlgItem(thisDialogHandle, IDC_PLOT_CREATOR_DATA_SET_COMBO);
				// add data sets
				for (int dataSetInc = 0; dataSetInc < eCurrentPlottingInfo.getDataSetNumber(); dataSetInc++)
				{
					SendMessage(dataSetNumHandle, CB_ADDSTRING, 0, (LPARAM)("Data Set #" + std::to_string(dataSetInc + 1)).c_str());
				}
				SendMessage(dataSetNumHandle, CB_ADDSTRING, 0, (LPARAM)"Add New Data Set");
				SendMessage(dataSetNumHandle, CB_ADDSTRING, 0, (LPARAM)"Remove Data Set");
				EnableWindow(GetDlgItem(thisDialogHandle, IDC_PLOT_CREATOR_PLOT_DATA_SET_CHECK), false);
				// General Plot Type
				HWND generalPlotTypeHandle = GetDlgItem(thisDialogHandle, IDC_PLOT_CREATOR_GENERAL_PLOT_TYPE_COMBO);
				SendMessage(generalPlotTypeHandle, CB_ADDSTRING, 0, (LPARAM)"Pixel Count Histograms");
				SendMessage(generalPlotTypeHandle, CB_ADDSTRING, 0, (LPARAM)"Pixel Counts");
				SendMessage(generalPlotTypeHandle, CB_ADDSTRING, 0, (LPARAM)"Atoms");
				// Picture #s
				HWND positiveResultPictureNumberHandle = GetDlgItem(thisDialogHandle, IDC_PLOT_CREATOR_POSITIVE_PICTURE_NUMBER_COMBO);
				HWND postSelectionPictureNumberHandle = GetDlgItem(thisDialogHandle, IDC_PLOT_CREATOR_POST_PICTURE_NUMBER_COMBO);
				for (int pictureInc = 0; pictureInc < ePicturesPerRepetition; pictureInc++)
				{
					std::string comboMsg = "Picture #" + std::to_string(pictureInc + 1);
					SendMessage(positiveResultPictureNumberHandle, CB_ADDSTRING, 0, (LPARAM)comboMsg.c_str());
					SendMessage(postSelectionPictureNumberHandle, CB_ADDSTRING, 0, (LPARAM)comboMsg.c_str());
				}
				SendMessage(positiveResultPictureNumberHandle, CB_ADDSTRING, 0, (LPARAM)"Add Picture to Experiment");
				SendMessage(postSelectionPictureNumberHandle, CB_ADDSTRING, 0, (LPARAM)"Add Picture to Experiment");
				SendMessage(positiveResultPictureNumberHandle, CB_ADDSTRING, 0, (LPARAM)"Remove Picture from Experiment");
				SendMessage(postSelectionPictureNumberHandle, CB_ADDSTRING, 0, (LPARAM)"Remove Picture from Experiment");
				EnableWindow(GetDlgItem(thisDialogHandle, IDC_PLOT_CREATOR_POSITIVE_PICTURE_NUMBER_COMBO), false);
				EnableWindow(GetDlgItem(thisDialogHandle, IDC_PLOT_CREATOR_POST_PICTURE_NUMBER_COMBO), false);
				EnableWindow(GetDlgItem(thisDialogHandle, IDC_PLOT_CREATOR_POST_SELECTION_ATOM_PRESENT_CHECK), false);
				EnableWindow(GetDlgItem(thisDialogHandle, IDC_PLOT_CREATOR_POSITIVE_ATOM_PRESENT_CHECK), false);
				EnableWindow(GetDlgItem(thisDialogHandle, IDC_PLOT_CREATOR_POST_SELECTION_NO_ATOM_CHECK), false);
				EnableWindow(GetDlgItem(thisDialogHandle, IDC_PLOT_CREATOR_POSITIVE_NO_ATOM_CHECK), false);
				// Pixel #s
				HWND positiveResultPixelNumberHandle = GetDlgItem(thisDialogHandle, IDC_PLOT_CREATOR_POSITIVE_PIXEL_NUMBER_COMBO);
				HWND postSelectionPixelNumberHandle = GetDlgItem(thisDialogHandle, IDC_PLOT_CREATOR_POST_PIXEL_NUMBER_COMBO);
				for (int pixelInc = 0; pixelInc < eCurrentPlottingInfo.getPixelNumber(); pixelInc++)
				{
					SendMessage(positiveResultPixelNumberHandle, CB_ADDSTRING, 0, (LPARAM)("Pixel #" + std::to_string(pixelInc + 1)).c_str());
					SendMessage(postSelectionPixelNumberHandle, CB_ADDSTRING, 0, (LPARAM)("Pixel #" + std::to_string(pixelInc + 1)).c_str());
				}
				SendMessage(positiveResultPixelNumberHandle, CB_SETCURSEL, 0, 0);
				SendMessage(postSelectionPixelNumberHandle, CB_SETCURSEL, 0, 0);
				EnableWindow(GetDlgItem(thisDialogHandle, IDC_PLOT_CREATOR_POSITIVE_PIXEL_NUMBER_COMBO), false);
				EnableWindow(GetDlgItem(thisDialogHandle, IDC_PLOT_CREATOR_POST_PIXEL_NUMBER_COMBO), false);
				// Conditions
				HWND postSelectionConditionNumber = GetDlgItem(thisDialogHandle, IDC_PLOT_CREATOR_POST_SELECTION_CONDITION_NUMBER_COMBO);
				for (int conditionInc = 0; conditionInc < eCurrentPlottingInfo.getConditionNumber(); conditionInc++)
				{
					SendMessage(postSelectionConditionNumber, CB_ADDSTRING, 0, (LPARAM)("Condition #" + std::to_string(conditionInc)).c_str());
				}
				SendMessage(postSelectionConditionNumber, CB_ADDSTRING, 0, (LPARAM)"Add New Condition");
				SendMessage(postSelectionConditionNumber, CB_ADDSTRING, 0, (LPARAM)"Remove Condition");
				EnableWindow(GetDlgItem(thisDialogHandle, IDC_PLOT_CREATOR_POST_SELECTION_CONDITION_NUMBER_COMBO), false);
				EnableWindow(GetDlgItem(thisDialogHandle, IDC_PLOT_CREATOR_POST_SHOW_ALL), false);
				EnableWindow(GetDlgItem(thisDialogHandle, IDC_PLOT_CREATOR_POSITIVE_SHOW_ALL), false);
				EnableWindow(GetDlgItem(thisDialogHandle, IDC_PLOT_CREATOR_LEGEND_TEXT_EDIT), false);
				EnableWindow(GetDlgItem(thisDialogHandle, IDC_GAUSSIAN_RADIO), false);
				EnableWindow(GetDlgItem(thisDialogHandle, IDC_LORENTZIAN_RADIO), false);
				EnableWindow(GetDlgItem(thisDialogHandle, IDC_SINE_RADIO), false);
				EnableWindow(GetDlgItem(thisDialogHandle, IDC_NO_FIT_RADIO), false);
				EnableWindow(GetDlgItem(thisDialogHandle, IDC_REAL_TIME_FIT_RADIO), false);
				EnableWindow(GetDlgItem(thisDialogHandle, IDC_FINISH_FIT_RADIO), false);
				// initialize the data set a bit
				eCurrentAnalysisSetSelectionNumber = -1;
				eCurrentPixelSelectionNumber = -1;
				eCurrentDataSetSelectionNumber = -1;
				eCurrentPositivePictureNumber = -1;
				eCurrentPositivePixelNumber = 0;
				eCurrentPostSelectionPictureNumber = -1;
				eCurrentPostSelectionPixelNumber = 0;
				eCurrentPostSelectionConditionNumber = -1;
				// load title
				SendMessage(GetDlgItem(thisDialogHandle, IDC_PLOT_CREATOR_PLOT_TITLE_EDIT), WM_SETTEXT, 0, (LPARAM)(eCurrentPlottingInfo.getTitle()).c_str());
				// load y axis
				SendMessage(GetDlgItem(thisDialogHandle, IDC_PLOT_CREATOR_Y_LABEL_EDIT), WM_SETTEXT, 0, (LPARAM)(eCurrentPlottingInfo.getYLabel()).c_str());
				// load x axis
				std::string tempXAxis = eCurrentPlottingInfo.getXAxis();
				if (tempXAxis == "Experiment Average")
				{
					CheckDlgButton(thisDialogHandle, IDC_PLOT_CREATOR_AVERAGE_EXPERIMENT_RADIO, BST_CHECKED);
				}
				else if (tempXAxis == "Running Average")
				{
					CheckDlgButton(thisDialogHandle, IDC_PLOT_CREATOR_RUNNING_AVERAGE, BST_CHECKED);
				}
				else if (tempXAxis != "")
				{
					MessageBox(0, "ERROR: X axis was not recognized. Continuing...", 0, 0);
				}
				// load filename
				SendMessage(GetDlgItem(thisDialogHandle, IDC_PLOT_CREATOR_FILENAME_EDIT), WM_SETTEXT, 0, (LPARAM)(eCurrentPlottingInfo.getFileName()).c_str());
				// load plot type
				std::string tempPlotType = eCurrentPlottingInfo.getPlotType();
				if (tempPlotType == "Atoms")
				{
					SendMessage(GetDlgItem(thisDialogHandle, IDC_PLOT_CREATOR_GENERAL_PLOT_TYPE_COMBO), CB_SETCURSEL, 0, 0);
				}
				else if (tempPlotType == "Pixel Counts")
				{
					SendMessage(GetDlgItem(thisDialogHandle, IDC_PLOT_CREATOR_GENERAL_PLOT_TYPE_COMBO), CB_SETCURSEL, 1, 0);
				}
				else if (tempPlotType == "Pixel Counts Histograms")
				{
					SendMessage(GetDlgItem(thisDialogHandle, IDC_PLOT_CREATOR_GENERAL_PLOT_TYPE_COMBO), CB_SETCURSEL, 2, 0);
				}
				else if (tempPlotType != "")
				{
					MessageBox(0, "ERROR: Plot type not recognized. Continue...", 0, 0);
				}
				break;
			}
			case WM_COMMAND:
			{
				switch (LOWORD(wParam))
				{
					case IDC_PLOT_CREATOR_OK_BUTTON:
					{
						/// Save Everything
						
						// General Parameters: ////////
						// Get the title
						TCHAR titleText[256];
						SendMessage(GetDlgItem(thisDialogHandle, IDC_PLOT_CREATOR_PLOT_TITLE_EDIT), WM_GETTEXT, 256, (LPARAM)titleText);
						eCurrentPlottingInfo.changeTitle(titleText);
						// get the y label
						TCHAR yLabelText[256];
						SendMessage(GetDlgItem(thisDialogHandle, IDC_PLOT_CREATOR_Y_LABEL_EDIT), WM_GETTEXT, 256, (LPARAM)yLabelText);
						eCurrentPlottingInfo.changeYLabel(yLabelText);
						// filename
						TCHAR fileNameText[256];
						SendMessage(GetDlgItem(thisDialogHandle, IDC_PLOT_CREATOR_FILENAME_EDIT), WM_GETTEXT, 256, (LPARAM)fileNameText);
						eCurrentPlottingInfo.changeFileName(fileNameText);
						
						// x axis
						int runningAverageCheck = IsDlgButtonChecked(thisDialogHandle, IDC_PLOT_CREATOR_RUNNING_AVERAGE);
						int experimentAverageCheck = IsDlgButtonChecked(thisDialogHandle, IDC_PLOT_CREATOR_AVERAGE_EXPERIMENT_RADIO);
						if (experimentAverageCheck == BST_CHECKED)
						{
							if (runningAverageCheck == BST_CHECKED)
							{
								MessageBox(0, "Please select only one x-axis option.", 0, 0);
								break;
							}
							eCurrentPlottingInfo.changeXAxis("Experiment Average");
						}
						else if (runningAverageCheck == BST_CHECKED)
						{
							if (experimentAverageCheck == BST_CHECKED)
							{
								MessageBox(0, "Please select only one x-axis option.", 0, 0);
								break;
							}
							eCurrentPlottingInfo.changeXAxis("Running Average");
						}
						else
						{
							MessageBox(0, "Please select an x-axis option.", 0, 0);
							break;
						}

						/// get the (current) analysis pixel locations
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

						// get the text from the rows and collumns.
						// avoid the cases where these have been set to 0 as this indicates that there is no actual selection active.
						myPlot::saveAnalysisPixelLocations(thisDialogHandle, false);
						myPlot::saveDataSet(thisDialogHandle, false);
						int answer = DialogBoxParam(eHInst, MAKEINTRESOURCE(IDD_RICH_EDIT_OK_CANCEL_MESSAGE_BOX), 0, 
													(DLGPROC)dialogProcedures::richEditOkCancelMessageBoxProc, (LPARAM)(eCurrentPlottingInfo.returnAllInfo()).c_str());						
						if (answer == IDOK)
						{
							// save.
							eCurrentPlottingInfo.savePlotInfo();
							fileManage::reloadCombo(eAllPlotsCombo.hwnd, PLOT_FILES_SAVE_LOCATION, "*.plot", "__NONE__");
							answer = MessageBox(0, "Close plot creator?", 0, MB_YESNO);
							if (answer == IDYES)
							{
								EndDialog(thisDialogHandle, 0);
							}
						}
						
						break;
					}
					case IDC_PLOT_CREATOR_CANCEL_BUTTON:
					{
						EndDialog(thisDialogHandle, 1);
						break;
					}
					case IDC_PLOT_CREATOR_PIXELS_PER_GROUP:
					{
						if (HIWORD(wParam) == EN_KILLFOCUS)
						{
							TCHAR text[256];
							GetWindowText(GetDlgItem(thisDialogHandle, IDC_PLOT_CREATOR_PIXELS_PER_GROUP), text, 256);
							std::string textStr(text);
							int pixelNum;
							try
							{
								pixelNum = std::stoi(textStr);
							}
							catch (std::invalid_argument&)
							{
								errBox("ERROR: Pixels per Group did not convert to an integer!");
								SendMessage(GetDlgItem(thisDialogHandle, IDC_PLOT_CREATOR_PIXELS_PER_GROUP), WM_SETTEXT, 0, (LPARAM)"");
								pixelNum = -1;
							}
							if (pixelNum == -1)
							{
								break;
							}
							eCurrentPlottingInfo.resetPixelNumber(pixelNum);
							HWND pixelTrueHandle = GetDlgItem(thisDialogHandle, IDC_PLOT_CREATOR_POSITIVE_PIXEL_NUMBER_COMBO);
							HWND pixelPostHandle = GetDlgItem(thisDialogHandle, IDC_PLOT_CREATOR_POST_PIXEL_NUMBER_COMBO);
							SendMessage(pixelTrueHandle, CB_RESETCONTENT, 0, 0);
							SendMessage(pixelPostHandle, CB_RESETCONTENT, 0, 0);
							for (int pixelInc = 0; pixelInc < pixelNum; pixelInc++)
							{
								std::string pixelNumberNewItem = "Pixel #" + std::to_string(pixelInc + 1) + " Location:";
								std::string pixelTrueConditionStr = "Pixel #" + std::to_string(pixelInc + 1);
								SendMessage(pixelTrueHandle, CB_ADDSTRING, 0, (LPARAM)pixelTrueConditionStr.c_str());
								SendMessage(pixelPostHandle, CB_ADDSTRING, 0, (LPARAM)pixelTrueConditionStr.c_str());
							}
						}
						myPlot::enableAndDisable(thisDialogHandle);
						break;
					}
					case IDC_PLOT_CREATOR_PIXEL_NUMBER_COMBO:
					{
						if (HIWORD(wParam) == CBN_SELCHANGE)
						{
							// Save the old stuff.
							HWND pixelNumberHandle = GetDlgItem(thisDialogHandle, IDC_PLOT_CREATOR_PIXEL_NUMBER_COMBO);
							TCHAR pixelLocationComboText[256];
							long long itemIndex = SendMessage(pixelNumberHandle, CB_GETCURSEL, 0, 0);
							if (itemIndex == -1)
							{
								// user didn't select anything, nvm.
								break;
							}
							myPlot::saveAnalysisPixelLocations(thisDialogHandle, true);

							SendMessage(pixelNumberHandle, (UINT)CB_GETLBTEXT, (WPARAM)itemIndex, (LPARAM)pixelLocationComboText);
							std::string pixelNumberComboString(pixelLocationComboText);
							HWND pixelTrueHandle = GetDlgItem(thisDialogHandle, IDC_PLOT_CREATOR_POSITIVE_PIXEL_NUMBER_COMBO);
							HWND pixelPostHandle = GetDlgItem(thisDialogHandle, IDC_PLOT_CREATOR_POST_PIXEL_NUMBER_COMBO);
							if (pixelNumberComboString == "New Pixel")
							{
								eCurrentPixelSelectionNumber = -1;
								int numberOfItems = SendMessage(pixelNumberHandle, CB_GETCOUNT, 0, 0);

								std::string pixelNumberNewItem = "Pixel #" + std::to_string(numberOfItems - 1) + " Location:";
								SendMessage(pixelNumberHandle, CB_ADDSTRING, 0, (LPARAM)pixelNumberNewItem.c_str());
								std::string pixelTrueConditionStr = "Pixel #" + std::to_string(numberOfItems - 1);
								SendMessage(pixelTrueHandle, CB_ADDSTRING, 0, (LPARAM)pixelTrueConditionStr.c_str());
								SendMessage(pixelPostHandle, CB_ADDSTRING, 0, (LPARAM)pixelTrueConditionStr.c_str());
								eCurrentPlottingInfo.addPixel();
							}
							else if (pixelNumberComboString == "Remove Pixel")
							{
								eCurrentPixelSelectionNumber = -1;
								int numberOfItems = SendMessage(pixelNumberHandle, CB_GETCOUNT, 0, 0);
								if (numberOfItems < 4)
								{
									MessageBox(0, "Can't delete last pixel.", 0, 0);
									break;
								}
								SendMessage(pixelNumberHandle, CB_DELETESTRING, numberOfItems - 2, 0);
								int numberPixelItems = SendMessage(pixelTrueHandle, CB_GETCOUNT, 0, 0);
								SendMessage(pixelTrueHandle, CB_DELETESTRING, numberPixelItems - 1, 0);
								SendMessage(pixelPostHandle, CB_DELETESTRING, numberPixelItems - 1, 0);
								eCurrentPlottingInfo.removePixel();
							}
							else
							{
								// Selected a specific pixel to set.								
								eCurrentPixelSelectionNumber = itemIndex - 1;
								int row, collumn;
								if (eCurrentAnalysisSetSelectionNumber >= 0 && eCurrentPixelSelectionNumber >= 0)
								{
									eCurrentPlottingInfo.getPixelLocation(eCurrentPixelSelectionNumber, eCurrentAnalysisSetSelectionNumber, row, collumn);
									SendMessage(GetDlgItem(thisDialogHandle, IDC_PLOT_CREATOR_PIXEL_RO_EDIT), WM_SETTEXT, 0,
												(LPARAM)std::to_string(row).c_str());
									SendMessage(GetDlgItem(thisDialogHandle, IDC_PLOT_CREATOR_PIXEL_COLLUMN_EDIT), WM_SETTEXT, 0,
												(LPARAM)std::to_string(collumn).c_str());
								}
							}
							myPlot::enableAndDisable(thisDialogHandle);
						}
						break;
					}
					case IDC_PLOT_CREATOR_DATA_SET_COMBO:
					{
						if (HIWORD(wParam) == CBN_SELCHANGE)
						{
							HWND dataSetCombo = GetDlgItem(thisDialogHandle, IDC_PLOT_CREATOR_DATA_SET_COMBO);
							TCHAR dataSetComboText[256];
							long long itemIndex = SendMessage(dataSetCombo, CB_GETCURSEL, 0, 0);
							if (itemIndex == -1)
							{
								// user didn't select anything.
								break;
							}
							// save data set.
							myPlot::saveDataSet(thisDialogHandle, true);
							SendMessage(dataSetCombo, (UINT)CB_GETLBTEXT, (WPARAM)itemIndex, (LPARAM)dataSetComboText);
							std::string dataSetString(dataSetComboText);
							if (dataSetString == "Add New Data Set")
							{
								EnableWindow(GetDlgItem(thisDialogHandle, IDC_PLOT_CREATOR_PLOT_DATA_SET_CHECK), false);
								eCurrentDataSetSelectionNumber = -1;
								int numberOfItems = SendMessage(dataSetCombo, CB_GETCOUNT, 0, 0);
								std::string atomLocationNewItem = "Data Set #" + std::to_string(numberOfItems - 1);
								eCurrentPlottingInfo.addDataSet();
								SendMessage(dataSetCombo, CB_ADDSTRING, 0, (LPARAM)atomLocationNewItem.c_str());
							}
							else if (dataSetString == "Remove Data Set")
							{
								EnableWindow(GetDlgItem(thisDialogHandle, IDC_PLOT_CREATOR_PLOT_DATA_SET_CHECK), false);
								eCurrentDataSetSelectionNumber = -1;
								int numberOfItems = SendMessage(dataSetCombo, CB_GETCOUNT, 0, 0);
								if (numberOfItems < 4)
								{
									MessageBox(0, "Can't delete last data set.", 0, 0);
									break;
								}
								// make data set struct smaller.
								eCurrentPlottingInfo.removeDataSet();
								SendMessage(dataSetCombo, CB_DELETESTRING, numberOfItems - 2, 0);
							}
							else
							{
								int numberOfItems = SendMessage(dataSetCombo, CB_GETCOUNT, 0, 0);
								// there's an add option at the beginning.
								eCurrentDataSetSelectionNumber = itemIndex - 1;
								// reload things...
								// get the plot truth.
								// if plotting...
								if (eCurrentPlottingInfo.getPlotThisDataValue(eCurrentDataSetSelectionNumber))
								{
									// check it.
									CheckDlgButton(thisDialogHandle, IDC_PLOT_CREATOR_PLOT_DATA_SET_CHECK, BST_CHECKED);
								}
								else
								{
									// uncheck it.
									CheckDlgButton(thisDialogHandle, IDC_PLOT_CREATOR_PLOT_DATA_SET_CHECK, BST_UNCHECKED);
								}
								int fitCase = eCurrentPlottingInfo.getFitOption(eCurrentDataSetSelectionNumber);
								switch (fitCase)
								{
									case GAUSSIAN_FIT:
									{
										CheckDlgButton(thisDialogHandle, IDC_GAUSSIAN_RADIO, BST_CHECKED);
										CheckDlgButton(thisDialogHandle, IDC_LORENTZIAN_RADIO, BST_UNCHECKED);
										CheckDlgButton(thisDialogHandle, IDC_SINE_RADIO, BST_UNCHECKED);
										break;
									}
									case LORENTZIAN_FIT:
									{
										CheckDlgButton(thisDialogHandle, IDC_GAUSSIAN_RADIO, BST_UNCHECKED);
										CheckDlgButton(thisDialogHandle, IDC_LORENTZIAN_RADIO, BST_CHECKED);
										CheckDlgButton(thisDialogHandle, IDC_SINE_RADIO, BST_UNCHECKED);
										break;
									}
									case SINE_FIT:
									{
										CheckDlgButton(thisDialogHandle, IDC_GAUSSIAN_RADIO, BST_UNCHECKED);
										CheckDlgButton(thisDialogHandle, IDC_LORENTZIAN_RADIO, BST_UNCHECKED);
										CheckDlgButton(thisDialogHandle, IDC_SINE_RADIO, BST_CHECKED);
										break;
									}
									default:
									{
										CheckDlgButton(thisDialogHandle, IDC_GAUSSIAN_RADIO, BST_UNCHECKED);
										CheckDlgButton(thisDialogHandle, IDC_LORENTZIAN_RADIO, BST_UNCHECKED);
										CheckDlgButton(thisDialogHandle, IDC_SINE_RADIO, BST_UNCHECKED);
										break;
									}
								}
								int whenCase = eCurrentPlottingInfo.getWhenToFit(eCurrentDataSetSelectionNumber);
								switch (whenCase)
								{
									case NO_FIT:
									{
										CheckDlgButton(thisDialogHandle, IDC_NO_FIT_RADIO, BST_CHECKED);
										CheckDlgButton(thisDialogHandle, IDC_REAL_TIME_FIT_RADIO, BST_UNCHECKED);
										CheckDlgButton(thisDialogHandle, IDC_FINISH_FIT_RADIO, BST_UNCHECKED);
										break;
									}
									case REAL_TIME_FIT:
									{
										CheckDlgButton(thisDialogHandle, IDC_NO_FIT_RADIO, BST_UNCHECKED);
										CheckDlgButton(thisDialogHandle, IDC_REAL_TIME_FIT_RADIO, BST_UNCHECKED);
										CheckDlgButton(thisDialogHandle, IDC_FINISH_FIT_RADIO, BST_CHECKED);
										break;
									}
									case FIT_AT_END:
									{
										CheckDlgButton(thisDialogHandle, IDC_NO_FIT_RADIO, BST_UNCHECKED);
										CheckDlgButton(thisDialogHandle, IDC_REAL_TIME_FIT_RADIO, BST_CHECKED);
										CheckDlgButton(thisDialogHandle, IDC_FINISH_FIT_RADIO, BST_UNCHECKED);
										break;
									}
									default:
									{
										CheckDlgButton(thisDialogHandle, IDC_NO_FIT_RADIO, BST_UNCHECKED);
										CheckDlgButton(thisDialogHandle, IDC_REAL_TIME_FIT_RADIO, BST_UNCHECKED);
										CheckDlgButton(thisDialogHandle, IDC_FINISH_FIT_RADIO, BST_UNCHECKED);
										break;
									}
								}

								if (eCurrentPlottingInfo.getPlotType() == "Atoms")
								{
									if (eCurrentPositivePictureNumber >= 0)
									{
										// load current things.
										int currentValue = eCurrentPlottingInfo.getTruthCondition(eCurrentDataSetSelectionNumber, eCurrentPositivePixelNumber,
											eCurrentPositivePictureNumber);
										if (currentValue == 1)
										{
											CheckDlgButton(thisDialogHandle, IDC_PLOT_CREATOR_POSITIVE_ATOM_PRESENT_CHECK, BST_CHECKED);
											CheckDlgButton(thisDialogHandle, IDC_PLOT_CREATOR_POSITIVE_NO_ATOM_CHECK, BST_UNCHECKED);
										}
										else if (currentValue == -1)
										{
											CheckDlgButton(thisDialogHandle, IDC_PLOT_CREATOR_POSITIVE_ATOM_PRESENT_CHECK, BST_UNCHECKED);
											CheckDlgButton(thisDialogHandle, IDC_PLOT_CREATOR_POSITIVE_NO_ATOM_CHECK, BST_CHECKED);
										}
										else
										{
											CheckDlgButton(thisDialogHandle, IDC_PLOT_CREATOR_POSITIVE_ATOM_PRESENT_CHECK, BST_UNCHECKED);
											CheckDlgButton(thisDialogHandle, IDC_PLOT_CREATOR_POSITIVE_NO_ATOM_CHECK, BST_UNCHECKED);
										}
									}
									if (eCurrentPostSelectionConditionNumber >= 0 && eCurrentPostSelectionPictureNumber >= 0)
									{
										// load current things.
										int currentValue = eCurrentPlottingInfo.getPostSelectionCondition(eCurrentDataSetSelectionNumber,
											eCurrentPostSelectionConditionNumber, eCurrentPostSelectionPixelNumber, eCurrentPostSelectionPictureNumber);
										if (currentValue == 1)
										{
											CheckDlgButton(thisDialogHandle, IDC_PLOT_CREATOR_POST_SELECTION_ATOM_PRESENT_CHECK, BST_CHECKED);
											CheckDlgButton(thisDialogHandle, IDC_PLOT_CREATOR_POST_SELECTION_NO_ATOM_CHECK, BST_UNCHECKED);
										}
										else if (currentValue == -1)
										{
											CheckDlgButton(thisDialogHandle, IDC_PLOT_CREATOR_POST_SELECTION_ATOM_PRESENT_CHECK, BST_UNCHECKED);
											CheckDlgButton(thisDialogHandle, IDC_PLOT_CREATOR_POST_SELECTION_NO_ATOM_CHECK, BST_CHECKED);
										}
										else
										{
											CheckDlgButton(thisDialogHandle, IDC_PLOT_CREATOR_POST_SELECTION_ATOM_PRESENT_CHECK, BST_UNCHECKED);
											CheckDlgButton(thisDialogHandle, IDC_PLOT_CREATOR_POST_SELECTION_NO_ATOM_CHECK, BST_UNCHECKED);
										}
									}
								}
								else
								{
									int pixel, picture;
									if (eCurrentPlottingInfo.getDataCountsLocation(eCurrentDataSetSelectionNumber, pixel, picture) == 0)
									{
										eCurrentPositivePictureNumber = picture;
										eCurrentPositivePixelNumber = pixel;
										SendMessage(GetDlgItem(thisDialogHandle, IDC_PLOT_CREATOR_POSITIVE_PICTURE_NUMBER_COMBO), CB_SETCURSEL, picture + 1,
													0);
										SendMessage(GetDlgItem(thisDialogHandle, IDC_PLOT_CREATOR_POSITIVE_PIXEL_NUMBER_COMBO), CB_SETCURSEL, pixel, 0);
									}
									else
									{
										eCurrentPositivePictureNumber = -1;
										eCurrentPositivePixelNumber = -1;
										SendMessage(GetDlgItem(thisDialogHandle, IDC_PLOT_CREATOR_POSITIVE_PICTURE_NUMBER_COMBO), CB_SETCURSEL, - 1, 0);
										SendMessage(GetDlgItem(thisDialogHandle, IDC_PLOT_CREATOR_POSITIVE_PIXEL_NUMBER_COMBO), CB_SETCURSEL, - 1, 0);
									}
								}
								if (eCurrentDataSetSelectionNumber != -1)
								{
									// always load legend...
									SendMessage(GetDlgItem(thisDialogHandle, IDC_PLOT_CREATOR_LEGEND_TEXT_EDIT), WM_SETTEXT, 0,
										(LPARAM)(eCurrentPlottingInfo.getLegendText(eCurrentDataSetSelectionNumber)).c_str());
								}
							}
							myPlot::enableAndDisable(thisDialogHandle);
						}
						break;
					}
					case IDC_PLOT_CREATOR_POSITIVE_PICTURE_NUMBER_COMBO:
					{
						if (HIWORD(wParam) == CBN_SELCHANGE)
						{

							HWND positivePictureNumberCombo = GetDlgItem(thisDialogHandle, IDC_PLOT_CREATOR_POSITIVE_PICTURE_NUMBER_COMBO);
							TCHAR positivePictureNumberText[256];
							long long itemIndex = SendMessage(positivePictureNumberCombo, CB_GETCURSEL, 0, 0);
							if (itemIndex == -1)
							{
								// user didn't select anything.
								break;
							}
							// save options.
							myPlot::savePositiveConditions(thisDialogHandle, true);

							SendMessage(positivePictureNumberCombo, (UINT)CB_GETLBTEXT, (WPARAM)itemIndex, (LPARAM)positivePictureNumberText);
							std::string positivePictureNumberString(positivePictureNumberText);

							if (positivePictureNumberString == "Add Picture to Experiment")
							{
								eCurrentPositivePictureNumber = -1;
								// TODO: update arrays with new size.
								ePicturesPerRepetition++;
								ePictureOptionsControl.setPicturesPerExperiment(ePicturesPerRepetition);
								std::string picturesPerExperimentMsg = "Picture #" + std::to_string(ePicturesPerRepetition);
								SendMessage(positivePictureNumberCombo, CB_ADDSTRING, 0, (LPARAM)picturesPerExperimentMsg.c_str());
								SendMessage(GetDlgItem(thisDialogHandle, IDC_PLOT_CREATOR_POST_PICTURE_NUMBER_COMBO), CB_ADDSTRING, 0,
											(LPARAM)picturesPerExperimentMsg.c_str());
								eCurrentPlottingInfo.addPicture();
							}
							else if (positivePictureNumberString == "Remove Picture from Experiment")
							{
								eCurrentPositivePictureNumber = -1;
								if (ePicturesPerRepetition == 1)
								{
									MessageBox(0, "Can't delete last picture in experiment.", 0, 0);
									break;
								}
								ePicturesPerRepetition--;
								ePictureOptionsControl.setPicturesPerExperiment(ePicturesPerRepetition);
								int numberOfItems = SendMessage(positivePictureNumberCombo, CB_GETCOUNT, 0, 0);
								SendMessage(positivePictureNumberCombo, CB_DELETESTRING, numberOfItems - 2, 0);
								SendMessage(GetDlgItem(thisDialogHandle, IDC_PLOT_CREATOR_POST_PICTURE_NUMBER_COMBO), CB_DELETESTRING, numberOfItems - 2, 0);
								eCurrentPlottingInfo.removePicture();
							}
							else
							{
								int numberOfItems = SendMessage(positivePictureNumberCombo, CB_GETCOUNT, 0, 0);
								// update this guy. There's an "add" option at the beginning.
								eCurrentPositivePictureNumber = itemIndex - 1;
								if (eCurrentDataSetSelectionNumber != -1)
								{
									// load current things.
									if (eCurrentPlottingInfo.getPlotType() == "Atoms")
									{
										int currentValue = eCurrentPlottingInfo.getTruthCondition(eCurrentDataSetSelectionNumber, eCurrentPositivePixelNumber,
											eCurrentPositivePictureNumber);
										if (currentValue == 1)
										{
											CheckDlgButton(thisDialogHandle, IDC_PLOT_CREATOR_POSITIVE_ATOM_PRESENT_CHECK, BST_CHECKED);
											CheckDlgButton(thisDialogHandle, IDC_PLOT_CREATOR_POSITIVE_NO_ATOM_CHECK, BST_UNCHECKED);
										}
										else if (currentValue == -1)
										{
											CheckDlgButton(thisDialogHandle, IDC_PLOT_CREATOR_POSITIVE_ATOM_PRESENT_CHECK, BST_UNCHECKED);
											CheckDlgButton(thisDialogHandle, IDC_PLOT_CREATOR_POSITIVE_NO_ATOM_CHECK, BST_CHECKED);
										}
										else
										{
											CheckDlgButton(thisDialogHandle, IDC_PLOT_CREATOR_POSITIVE_ATOM_PRESENT_CHECK, BST_UNCHECKED);
											CheckDlgButton(thisDialogHandle, IDC_PLOT_CREATOR_POSITIVE_NO_ATOM_CHECK, BST_UNCHECKED);
										}
									}
									else
									{
										CheckDlgButton(thisDialogHandle, IDC_PLOT_CREATOR_POSITIVE_ATOM_PRESENT_CHECK, BST_UNCHECKED);
										CheckDlgButton(thisDialogHandle, IDC_PLOT_CREATOR_POSITIVE_NO_ATOM_CHECK, BST_UNCHECKED);
									}
								}
							}
							myPlot::enableAndDisable(thisDialogHandle);
						}
						break;
					}
					case IDC_PLOT_CREATOR_POSITIVE_PIXEL_NUMBER_COMBO:
					{
						if (HIWORD(wParam) == CBN_SELCHANGE)
						{

							HWND positivePixelNumberCombo = GetDlgItem(thisDialogHandle, IDC_PLOT_CREATOR_POSITIVE_PIXEL_NUMBER_COMBO);
							TCHAR positivePixelNumberText[256];
							long long itemIndex = SendMessage(positivePixelNumberCombo, CB_GETCURSEL, 0, 0);
							if (itemIndex == -1)
							{
								// user didn't select anything.
								break;
							}
							if (eCurrentPlottingInfo.getPlotType() == "Atoms")
							{
								// save options.
								myPlot::savePositiveConditions(thisDialogHandle, true);

								SendMessage(positivePixelNumberCombo, (UINT)CB_GETLBTEXT, (WPARAM)itemIndex, (LPARAM)positivePixelNumberText);
								std::string positivePixelNumberString(positivePixelNumberText);
								int numberOfItems = SendMessage(positivePixelNumberCombo, CB_GETCOUNT, 0, 0);
								// update this guy.
								// no "add option".
								eCurrentPositivePixelNumber = itemIndex;
								if (eCurrentDataSetSelectionNumber != -1 && eCurrentPositivePictureNumber != -1)
								{
									if (eCurrentPlottingInfo.getPlotType() == "Atoms")
									{
										// load current things.
										int currentValue = eCurrentPlottingInfo.getTruthCondition(eCurrentDataSetSelectionNumber, eCurrentPositivePixelNumber,
											eCurrentPositivePictureNumber);
										if (currentValue == 1)
										{
											CheckDlgButton(thisDialogHandle, IDC_PLOT_CREATOR_POSITIVE_ATOM_PRESENT_CHECK, BST_CHECKED);
											CheckDlgButton(thisDialogHandle, IDC_PLOT_CREATOR_POSITIVE_NO_ATOM_CHECK, BST_UNCHECKED);
										}
										else if (currentValue == -1)
										{
											CheckDlgButton(thisDialogHandle, IDC_PLOT_CREATOR_POSITIVE_ATOM_PRESENT_CHECK, BST_UNCHECKED);
											CheckDlgButton(thisDialogHandle, IDC_PLOT_CREATOR_POSITIVE_NO_ATOM_CHECK, BST_CHECKED);
										}
										else
										{
											CheckDlgButton(thisDialogHandle, IDC_PLOT_CREATOR_POSITIVE_ATOM_PRESENT_CHECK, BST_UNCHECKED);
											CheckDlgButton(thisDialogHandle, IDC_PLOT_CREATOR_POSITIVE_NO_ATOM_CHECK, BST_UNCHECKED);
										}
									}
									else
									{
										CheckDlgButton(thisDialogHandle, IDC_PLOT_CREATOR_POSITIVE_ATOM_PRESENT_CHECK, BST_UNCHECKED);
										CheckDlgButton(thisDialogHandle, IDC_PLOT_CREATOR_POSITIVE_NO_ATOM_CHECK, BST_UNCHECKED);
									}
								}
							}
							else
							{
								SendMessage(positivePixelNumberCombo, (UINT)CB_GETLBTEXT, (WPARAM)itemIndex, (LPARAM)positivePixelNumberText);
								std::string positivePixelNumberString(positivePixelNumberText);
								eCurrentPositivePixelNumber = itemIndex;
								eCurrentPlottingInfo.setDataCountsLocation(eCurrentDataSetSelectionNumber, eCurrentPositivePixelNumber, eCurrentPositivePictureNumber);
							}
						}
						break;
					}
					case IDC_PLOT_CREATOR_POSITIVE_ATOM_PRESENT_CHECK:
					{
						if (eCurrentDataSetSelectionNumber == -1)
						{
							MessageBox(0, "Please first select a data set.", 0, 0);
							break;
						}
						if (eCurrentPositivePictureNumber == -1)
						{
							MessageBox(0, "Please first select a Picture.", 0, 0);
							break;
						}
						BOOL checked = IsDlgButtonChecked(thisDialogHandle, IDC_PLOT_CREATOR_POSITIVE_ATOM_PRESENT_CHECK);
						if (checked)
						{
							CheckDlgButton(thisDialogHandle, IDC_PLOT_CREATOR_POSITIVE_ATOM_PRESENT_CHECK, BST_UNCHECKED);
						}
						else
						{
							CheckDlgButton(thisDialogHandle, IDC_PLOT_CREATOR_POSITIVE_ATOM_PRESENT_CHECK, BST_CHECKED);
							// only one can be checked at a time.
							CheckDlgButton(thisDialogHandle, IDC_PLOT_CREATOR_POSITIVE_NO_ATOM_CHECK, BST_UNCHECKED);
						}
						break;
					}
					case IDC_PLOT_CREATOR_POSITIVE_NO_ATOM_CHECK:
					{
						if (eCurrentDataSetSelectionNumber == -1)
						{
							MessageBox(0, "Please first select a data set.", 0, 0);
							break;
						}
						if (eCurrentPositivePictureNumber == -1)
						{
							MessageBox(0, "Please first select a Picture.", 0, 0);
							break;
						}
						BOOL checked = IsDlgButtonChecked(thisDialogHandle, IDC_PLOT_CREATOR_POSITIVE_NO_ATOM_CHECK);
						if (checked)
						{
							CheckDlgButton(thisDialogHandle, IDC_PLOT_CREATOR_POSITIVE_NO_ATOM_CHECK, BST_UNCHECKED);
						}
						else
						{
							CheckDlgButton(thisDialogHandle, IDC_PLOT_CREATOR_POSITIVE_NO_ATOM_CHECK, BST_CHECKED);
							// only one can be checked at a time.
							CheckDlgButton(thisDialogHandle, IDC_PLOT_CREATOR_POSITIVE_ATOM_PRESENT_CHECK, BST_UNCHECKED);
						}
						break;
					}
					case IDC_PLOT_CREATOR_POSITIVE_SHOW_ALL:
					{	
						myPlot::savePositiveConditions(thisDialogHandle, false);
						HWND dataSetCombo = GetDlgItem(thisDialogHandle, IDC_PLOT_CREATOR_DATA_SET_COMBO);
						HWND pictureCombo = GetDlgItem(thisDialogHandle, IDC_PLOT_CREATOR_POSITIVE_PICTURE_NUMBER_COMBO);
						HWND pixelCombo = GetDlgItem(thisDialogHandle, IDC_PLOT_CREATOR_POSITIVE_PIXEL_NUMBER_COMBO);

						std::string allConditionsString = "All Current Truth Conditions:\r\n=========================\r\n\r\n";
						for (int dataSetInc = 0; dataSetInc < SendMessage(dataSetCombo, CB_GETCOUNT, 0, 0) - 2; dataSetInc++)
						{
							allConditionsString += "Data Set #" + std::to_string(dataSetInc + 1) + ":\r\n=====\r\n";
							for (int pictureInc = 0; pictureInc < SendMessage(pictureCombo, CB_GETCOUNT, 0, 0) - 2; pictureInc++)
							{
								allConditionsString += "Picture #" + std::to_string(pictureInc + 1) + ":\r\n";
								for (int pixelInc = 0; pixelInc < SendMessage(pixelCombo, CB_GETCOUNT, 0, 0); pixelInc++)
								{
									allConditionsString += "Pixel #" + std::to_string(pixelInc + 1) + ":";
									int currentValue = eCurrentPlottingInfo.getTruthCondition(dataSetInc, pixelInc, pictureInc);
									if (currentValue == 1)
									{
										allConditionsString += " Atom Present";
									}
									else if (currentValue == -1)
									{
										allConditionsString += " Atom Not Present";
									}
									else
									{
										allConditionsString += " No Condition";
									}
									allConditionsString += "\r\n";
								}
							}
							allConditionsString += "=====\r\n";
						}
						MessageBox(0, allConditionsString.c_str(), 0, 0);
						break;
					}
					case IDC_PLOT_CREATOR_POST_PICTURE_NUMBER_COMBO:
					{
						if (HIWORD(wParam) == CBN_SELCHANGE)
						{
							HWND postSelectionPictureNumberCombo = GetDlgItem(thisDialogHandle, IDC_PLOT_CREATOR_POST_PICTURE_NUMBER_COMBO);
							TCHAR postSelectionPictureNumberText[256];
							long long itemIndex = SendMessage(postSelectionPictureNumberCombo, CB_GETCURSEL, 0, 0);
							if (itemIndex == -1)
							{
								// user didn't select anything.
								break;
							}
							myPlot::savePostSelectionConditions(thisDialogHandle, true);
							SendMessage(postSelectionPictureNumberCombo, (UINT)CB_GETLBTEXT, (WPARAM)itemIndex, (LPARAM)postSelectionPictureNumberText);
							std::string postSelectionPictureNumberString(postSelectionPictureNumberText);
							if (postSelectionPictureNumberString == "Add Picture to Experiment")
							{
								eCurrentPostSelectionPictureNumber = -1;
								ePicturesPerRepetition++;
								ePictureOptionsControl.setPicturesPerExperiment(ePicturesPerRepetition);
								std::string picturesPerExperimentMsg = "Picture #" + std::to_string(ePicturesPerRepetition);
								SendMessage(postSelectionPictureNumberCombo, CB_ADDSTRING, 0, (LPARAM)picturesPerExperimentMsg.c_str());
								SendMessage(GetDlgItem(thisDialogHandle, IDC_PLOT_CREATOR_POSITIVE_PICTURE_NUMBER_COMBO), CB_ADDSTRING, 0,
											 (LPARAM)picturesPerExperimentMsg.c_str());
								eCurrentPlottingInfo.addPicture();
							}
							else if (postSelectionPictureNumberString == "Remove Picture from Experiment")
							{
								eCurrentPostSelectionPictureNumber = -1;
								if (ePicturesPerRepetition == 1)
								{
									MessageBox(0, "Can't delete last Picture in Experiment.", 0, 0);
									break;
								}

								ePicturesPerRepetition--;
								int numberOfItems = SendMessage(postSelectionPictureNumberCombo, CB_GETCOUNT, 0, 0);
								ePictureOptionsControl.setPicturesPerExperiment(ePicturesPerRepetition);
								SendMessage(postSelectionPictureNumberCombo, CB_DELETESTRING, numberOfItems - 2, 0);
								SendMessage(GetDlgItem(thisDialogHandle, IDC_PLOT_CREATOR_POSITIVE_PICTURE_NUMBER_COMBO), CB_DELETESTRING, numberOfItems - 2, 0);
								eCurrentPlottingInfo.removePicture();
							}
							else
							{
								int numberOfItems = SendMessage(postSelectionPictureNumberCombo, CB_GETCOUNT, 0, 0);
								eCurrentPostSelectionPictureNumber = itemIndex - 1;
								
								if (eCurrentDataSetSelectionNumber != -1 && eCurrentPostSelectionConditionNumber != -1)
								{
									// load current things.
									int currentValue = eCurrentPlottingInfo.getPostSelectionCondition(eCurrentDataSetSelectionNumber, eCurrentPostSelectionConditionNumber, 
																									  eCurrentPostSelectionPixelNumber, eCurrentPostSelectionPictureNumber);
									if (currentValue == 1)
									{
										CheckDlgButton(thisDialogHandle, IDC_PLOT_CREATOR_POST_SELECTION_ATOM_PRESENT_CHECK, BST_CHECKED);
										CheckDlgButton(thisDialogHandle, IDC_PLOT_CREATOR_POST_SELECTION_NO_ATOM_CHECK, BST_UNCHECKED);
									}
									else if (currentValue == -1)
									{
										CheckDlgButton(thisDialogHandle, IDC_PLOT_CREATOR_POST_SELECTION_ATOM_PRESENT_CHECK, BST_UNCHECKED);
										CheckDlgButton(thisDialogHandle, IDC_PLOT_CREATOR_POST_SELECTION_NO_ATOM_CHECK, BST_CHECKED);
									}
									else
									{
										CheckDlgButton(thisDialogHandle, IDC_PLOT_CREATOR_POST_SELECTION_ATOM_PRESENT_CHECK, BST_UNCHECKED);
										CheckDlgButton(thisDialogHandle, IDC_PLOT_CREATOR_POST_SELECTION_NO_ATOM_CHECK, BST_UNCHECKED);
									}
								}
							}
							myPlot::enableAndDisable(thisDialogHandle);
						}
						break;
					}
					case IDC_PLOT_CREATOR_POST_PIXEL_NUMBER_COMBO:
					{
						if (HIWORD(wParam) == CBN_SELCHANGE)
						{
							HWND postSelectionPixelNumberCombo = GetDlgItem(thisDialogHandle, IDC_PLOT_CREATOR_POST_PIXEL_NUMBER_COMBO);
							TCHAR postSelectionPixelNumberText[256];
							long long itemIndex = SendMessage(postSelectionPixelNumberCombo, CB_GETCURSEL, 0, 0);
							if (itemIndex == -1)
							{
								// user didn't select anything.
								break;
							}
							// save options.
							myPlot::savePostSelectionConditions(thisDialogHandle, true);

							SendMessage(postSelectionPixelNumberCombo, (UINT)CB_GETLBTEXT, (WPARAM)itemIndex, (LPARAM)postSelectionPixelNumberText);
							std::string postSelectionPixelNumberString(postSelectionPixelNumberText);
							int numberOfItems = SendMessage(postSelectionPixelNumberCombo, CB_GETCOUNT, 0, 0);
							// update this guy.
							// no "add option". No need to -1.
							eCurrentPostSelectionPixelNumber = itemIndex;
							if (eCurrentDataSetSelectionNumber != -1 && eCurrentPostSelectionConditionNumber != -1 && eCurrentPostSelectionPictureNumber != -1)
							{
								// load current things.
								int currentValue = eCurrentPlottingInfo.getPostSelectionCondition(eCurrentDataSetSelectionNumber, 
															eCurrentPostSelectionConditionNumber, eCurrentPostSelectionPixelNumber, eCurrentPostSelectionPictureNumber);
								if (currentValue == 1)
								{
									CheckDlgButton(thisDialogHandle, IDC_PLOT_CREATOR_POST_SELECTION_ATOM_PRESENT_CHECK, BST_CHECKED);
									CheckDlgButton(thisDialogHandle, IDC_PLOT_CREATOR_POST_SELECTION_NO_ATOM_CHECK, BST_UNCHECKED);
								}
								else if (currentValue == -1)
								{
									CheckDlgButton(thisDialogHandle, IDC_PLOT_CREATOR_POST_SELECTION_ATOM_PRESENT_CHECK, BST_UNCHECKED);
									CheckDlgButton(thisDialogHandle, IDC_PLOT_CREATOR_POST_SELECTION_NO_ATOM_CHECK, BST_CHECKED);
								}
								else
								{
									CheckDlgButton(thisDialogHandle, IDC_PLOT_CREATOR_POST_SELECTION_ATOM_PRESENT_CHECK, BST_UNCHECKED);
									CheckDlgButton(thisDialogHandle, IDC_PLOT_CREATOR_POST_SELECTION_NO_ATOM_CHECK, BST_UNCHECKED);
								}
							}
						}
						break;
					}
					case IDC_PLOT_CREATOR_POST_SELECTION_CONDITION_NUMBER_COMBO:
					{
						if (HIWORD(wParam) == CBN_SELCHANGE)
						{
							HWND postSelectionConditionNumberCombo = GetDlgItem(thisDialogHandle, IDC_PLOT_CREATOR_POST_SELECTION_CONDITION_NUMBER_COMBO);
							TCHAR postSelectionConditionNumberText[256];
							long long itemIndex = SendMessage(postSelectionConditionNumberCombo, CB_GETCURSEL, 0, 0);
							if (itemIndex == -1)
							{
								// user didn't select anything.
								break;
							}
							// save things
							myPlot::savePostSelectionConditions(thisDialogHandle, true);
							SendMessage(postSelectionConditionNumberCombo, (UINT)CB_GETLBTEXT, (WPARAM)itemIndex, (LPARAM)postSelectionConditionNumberText);
							std::string postSelectionConditionNumberString(postSelectionConditionNumberText);
							if (postSelectionConditionNumberString == "Add New Condition")
							{
								eCurrentPostSelectionConditionNumber = -1;
								int numberOfItems = SendMessage(postSelectionConditionNumberCombo, CB_GETCOUNT, 0, 0);
								std::string conditionStr = "Condition #" + std::to_string(numberOfItems - 1);
								SendMessage(postSelectionConditionNumberCombo, CB_ADDSTRING, 0, (LPARAM)conditionStr.c_str());
								eCurrentPlottingInfo.addPostSelectionCondition();
							}
							else if (postSelectionConditionNumberString == "Remove Condition")
							{
								eCurrentPostSelectionConditionNumber = -1;
								int numberOfItems = SendMessage(postSelectionConditionNumberCombo, CB_GETCOUNT, 0, 0);
								if (numberOfItems < 3)
								{
									MessageBox(0, "All conditions are gone.", 0, 0);
									break;
								}
								eCurrentPlottingInfo.removePostSelectionCondition();
								SendMessage(postSelectionConditionNumberCombo, CB_DELETESTRING, numberOfItems - 2, 0);
							}
							else
							{
								int numberOfItems = SendMessage(postSelectionConditionNumberCombo, CB_GETCOUNT, 0, 0);
								eCurrentPostSelectionConditionNumber = itemIndex - 1;
								// load current things...

								if (eCurrentDataSetSelectionNumber != -1 && eCurrentPostSelectionPictureNumber != -1)
								{
									// load current things.
									int currentValue = eCurrentPlottingInfo.getPostSelectionCondition(eCurrentDataSetSelectionNumber,
										eCurrentPostSelectionConditionNumber, eCurrentPostSelectionPixelNumber, eCurrentPostSelectionPictureNumber);
									if (currentValue == 1)
									{
										CheckDlgButton(thisDialogHandle, IDC_PLOT_CREATOR_POST_SELECTION_ATOM_PRESENT_CHECK, BST_CHECKED);
										CheckDlgButton(thisDialogHandle, IDC_PLOT_CREATOR_POST_SELECTION_NO_ATOM_CHECK, BST_UNCHECKED);
									}
									else if (currentValue == -1)
									{
										CheckDlgButton(thisDialogHandle, IDC_PLOT_CREATOR_POST_SELECTION_ATOM_PRESENT_CHECK, BST_UNCHECKED);
										CheckDlgButton(thisDialogHandle, IDC_PLOT_CREATOR_POST_SELECTION_NO_ATOM_CHECK, BST_CHECKED);
									}
									else
									{
										CheckDlgButton(thisDialogHandle, IDC_PLOT_CREATOR_POST_SELECTION_ATOM_PRESENT_CHECK, BST_UNCHECKED);
										CheckDlgButton(thisDialogHandle, IDC_PLOT_CREATOR_POST_SELECTION_NO_ATOM_CHECK, BST_UNCHECKED);
									}
								}
							}
							myPlot::enableAndDisable(thisDialogHandle);
						}
						break;
					}
					case IDC_PLOT_CREATOR_POST_SELECTION_ATOM_PRESENT_CHECK:
					{
						if (eCurrentDataSetSelectionNumber == -1)
						{
							MessageBox(0, "Please first select a data set.", 0, 0);
							break;
						}
						if (eCurrentPostSelectionConditionNumber == -1)
						{
							MessageBox(0, "Please first select a condition #.", 0, 0);
							break;
						}
						if (eCurrentPostSelectionPictureNumber == -1)
						{
							MessageBox(0, "Please first select a Picture.", 0, 0);
							break;
						}

						BOOL checked = IsDlgButtonChecked(thisDialogHandle, IDC_PLOT_CREATOR_POST_SELECTION_ATOM_PRESENT_CHECK);
						if (checked)
						{
							CheckDlgButton(thisDialogHandle, IDC_PLOT_CREATOR_POST_SELECTION_ATOM_PRESENT_CHECK, BST_UNCHECKED);
						}
						else
						{
							CheckDlgButton(thisDialogHandle, IDC_PLOT_CREATOR_POST_SELECTION_ATOM_PRESENT_CHECK, BST_CHECKED);
							// only one can be checked at a time.
							CheckDlgButton(thisDialogHandle, IDC_PLOT_CREATOR_POST_SELECTION_NO_ATOM_CHECK, BST_UNCHECKED);
						}
						break;
					}
					case IDC_PLOT_CREATOR_POST_SELECTION_NO_ATOM_CHECK:
					{
						if (eCurrentDataSetSelectionNumber == -1)
						{
							MessageBox(0, "Please first select a data set.", 0, 0);
							break;
						}
						if (eCurrentPostSelectionConditionNumber == -1)
						{
							MessageBox(0, "Please first select a condition #.", 0, 0);
							break;
						}
						if (eCurrentPostSelectionPictureNumber == -1)
						{
							MessageBox(0, "Please first select a Picture.", 0, 0);
							break;
						}
						BOOL checked = IsDlgButtonChecked(thisDialogHandle, IDC_PLOT_CREATOR_POST_SELECTION_NO_ATOM_CHECK);
						if (checked)
						{
							CheckDlgButton(thisDialogHandle, IDC_PLOT_CREATOR_POST_SELECTION_NO_ATOM_CHECK, BST_UNCHECKED);
						}
						else
						{
							CheckDlgButton(thisDialogHandle, IDC_PLOT_CREATOR_POST_SELECTION_NO_ATOM_CHECK, BST_CHECKED);
							// only one can be checked at a time.
							CheckDlgButton(thisDialogHandle, IDC_PLOT_CREATOR_POST_SELECTION_ATOM_PRESENT_CHECK, BST_UNCHECKED);
						}
						break;
					}
					case IDC_PLOT_CREATOR_POST_SHOW_ALL:
					{
						myPlot::savePostSelectionConditions(thisDialogHandle, false);
						HWND dataSetCombo = GetDlgItem(thisDialogHandle, IDC_PLOT_CREATOR_DATA_SET_COMBO);
						HWND pictureCombo = GetDlgItem(thisDialogHandle, IDC_PLOT_CREATOR_POST_PICTURE_NUMBER_COMBO);
						HWND pixelCombo = GetDlgItem(thisDialogHandle, IDC_PLOT_CREATOR_POST_PIXEL_NUMBER_COMBO);
						HWND conditionCombo = GetDlgItem(thisDialogHandle, IDC_PLOT_CREATOR_POST_SELECTION_CONDITION_NUMBER_COMBO);
						std::string allConditionsString = "All Current Post-Selection Conditions:\r\n=========================\r\n\r\n";
						for (int conditionInc = 0; conditionInc < SendMessage(conditionCombo, CB_GETCOUNT, 0, 0) - 2; conditionInc++)
						{
							allConditionsString += "*****Condition #" + std::to_string(conditionInc + 1) + "*****\r\n";
							for (int dataSetInc = 0; dataSetInc < SendMessage(dataSetCombo, CB_GETCOUNT, 0, 0) - 2; dataSetInc++)
							{
								allConditionsString += "Data Set #" + std::to_string(dataSetInc + 1) + ":\r\n=====\r\n";
								for (int pictureInc = 0; pictureInc < SendMessage(pictureCombo, CB_GETCOUNT, 0, 0) - 2; pictureInc++)
								{
									allConditionsString += "Picture #" + std::to_string(pictureInc + 1) + ":\r\n";
									for (int pixelInc = 0; pixelInc < SendMessage(pixelCombo, CB_GETCOUNT, 0, 0); pixelInc++)
									{
										allConditionsString += "Pixel #" + std::to_string(pixelInc + 1) + ":";
										int currentValue = eCurrentPlottingInfo.getPostSelectionCondition(dataSetInc, conditionInc, pixelInc, pictureInc);
										if (currentValue == 1)
										{
											allConditionsString += " Atom Present";
										}
										else if (currentValue == -1)
										{
											allConditionsString += " Atom Not Present";
										}
										else
										{
											allConditionsString += " No Condition";
										}
										allConditionsString += "\r\n";
									}
								}
								allConditionsString += "=====\r\n";
							}
						}
						MessageBox(0, allConditionsString.c_str(), 0, 0);
						break;
					}
					case IDC_PLOT_CREATOR_GENERAL_PLOT_TYPE_COMBO:
					{
						if (HIWORD(wParam) == CBN_SELCHANGE)
						{
							// get the text.		
							HWND generalPlotTypeCombo = GetDlgItem(thisDialogHandle, IDC_PLOT_CREATOR_GENERAL_PLOT_TYPE_COMBO);
							TCHAR plotTypeText[256];
							long long itemIndex = SendMessage(generalPlotTypeCombo, CB_GETCURSEL, 0, 0);
							if (itemIndex == -1)
							{
								// user didn't select anything.
								break;
							}
							if (eCurrentPlottingInfo.getPlotType() == "Atoms")
							{
								// save stuff.
								if (eCurrentDataSetSelectionNumber != -1)
								{
									if (eCurrentPositivePictureNumber >= 0)
									{
										// load current things.
										int currentValue = eCurrentPlottingInfo.getTruthCondition(eCurrentDataSetSelectionNumber, eCurrentPositivePixelNumber,
											eCurrentPositivePictureNumber);
										if (currentValue == 1)
										{
											CheckDlgButton(thisDialogHandle, IDC_PLOT_CREATOR_POSITIVE_ATOM_PRESENT_CHECK, BST_CHECKED);
											CheckDlgButton(thisDialogHandle, IDC_PLOT_CREATOR_POSITIVE_NO_ATOM_CHECK, BST_UNCHECKED);
										}
										else if (currentValue == -1)
										{
											CheckDlgButton(thisDialogHandle, IDC_PLOT_CREATOR_POSITIVE_ATOM_PRESENT_CHECK, BST_UNCHECKED);
											CheckDlgButton(thisDialogHandle, IDC_PLOT_CREATOR_POSITIVE_NO_ATOM_CHECK, BST_CHECKED);
										}
										else
										{
											CheckDlgButton(thisDialogHandle, IDC_PLOT_CREATOR_POSITIVE_ATOM_PRESENT_CHECK, BST_UNCHECKED);
											CheckDlgButton(thisDialogHandle, IDC_PLOT_CREATOR_POSITIVE_NO_ATOM_CHECK, BST_UNCHECKED);
										}
									}
									if (eCurrentPostSelectionConditionNumber >= 0 && eCurrentPostSelectionPictureNumber >= 0)
									{
										// load current things.
										int currentValue = eCurrentPlottingInfo.getPostSelectionCondition(eCurrentDataSetSelectionNumber,
											eCurrentPostSelectionConditionNumber, eCurrentPostSelectionPixelNumber, eCurrentPostSelectionPictureNumber);
										if (currentValue == 1)
										{
											CheckDlgButton(thisDialogHandle, IDC_PLOT_CREATOR_POST_SELECTION_ATOM_PRESENT_CHECK, BST_CHECKED);
											CheckDlgButton(thisDialogHandle, IDC_PLOT_CREATOR_POST_SELECTION_NO_ATOM_CHECK, BST_UNCHECKED);
										}
										else if (currentValue == -1)
										{
											CheckDlgButton(thisDialogHandle, IDC_PLOT_CREATOR_POST_SELECTION_ATOM_PRESENT_CHECK, BST_UNCHECKED);
											CheckDlgButton(thisDialogHandle, IDC_PLOT_CREATOR_POST_SELECTION_NO_ATOM_CHECK, BST_CHECKED);
										}
										else
										{
											CheckDlgButton(thisDialogHandle, IDC_PLOT_CREATOR_POST_SELECTION_ATOM_PRESENT_CHECK, BST_UNCHECKED);
											CheckDlgButton(thisDialogHandle, IDC_PLOT_CREATOR_POST_SELECTION_NO_ATOM_CHECK, BST_UNCHECKED);
										}
									}
								}
							}
							SendMessage(generalPlotTypeCombo, (UINT)CB_GETLBTEXT, (WPARAM)itemIndex, (LPARAM)plotTypeText);
							std::string plotTypeString(plotTypeText);
							eCurrentPlottingInfo.changeGeneralPlotType(plotTypeString);
							myPlot::enableAndDisable(thisDialogHandle);
						}
						break;
					}
					case IDC_PLOT_CREATOR_PLOT_DATA_SET_CHECK:
					{
						BOOL checked = IsDlgButtonChecked(thisDialogHandle, IDC_PLOT_CREATOR_PLOT_DATA_SET_CHECK);
						if (checked)
						{
							// uncheck it.
							CheckDlgButton(thisDialogHandle, IDC_PLOT_CREATOR_PLOT_DATA_SET_CHECK, BST_UNCHECKED);
							eCurrentPlottingInfo.setPlotData(eCurrentDataSetSelectionNumber, false);
						}
						else
						{
							// check it.
							CheckDlgButton(thisDialogHandle, IDC_PLOT_CREATOR_PLOT_DATA_SET_CHECK, BST_CHECKED);
							eCurrentPlottingInfo.setPlotData(eCurrentDataSetSelectionNumber, true);
						}
						break;
					}
				}
			}
		}
		return FALSE;
	}

	INT_PTR CALLBACK textPromptDialogProcedure(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
	{
		switch (message)
		{
			case WM_INITDIALOG:
			{
				// lParam contains the text to be displayed on this guy. 
				CreateWindowEx(0, "EDIT", (const char*)lParam, WS_CHILD | WS_VISIBLE | ES_READONLY | ES_MULTILINE | ES_LEFT, 0, 0, 400, 60, hDlg, (HMENU)-1, eHInst, NULL);
				return (INT_PTR)TRUE;
			}
			case WM_COMMAND:
			{
				switch (LOWORD(wParam))
				{
				case IDCANCEL:
				{
					EndDialog(hDlg, WPARAM(""));
					return (INT_PTR)TRUE;
				}
				case IDOK:
				{
					HWND inputEdit = GetDlgItem(hDlg, IDC_INPUT_NAME_EDIT);
					int editLength = GetWindowTextLength(inputEdit);
					TCHAR* buffer;
					buffer = new TCHAR[1024];
					GetWindowText(inputEdit, buffer, 1024);
					EndDialog(hDlg, WPARAM(buffer));
					return (INT_PTR)TRUE;
				}
				}
				break;
			}
		}
		return (INT_PTR)FALSE;
	}

}