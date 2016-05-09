#include "stdafx.h"

#include "cameraWndProc.h"
#include "constants.h"
#include "myAndor.h"
#include "processTimer.h"
#include "stdio.h"
#include "appendText.h"
#include "externals.h"
#include "cameraThread.h"
#include "resource.h"
#include "fonts.h"
#include "createIdentityPalette.h"
#include "initializeCameraWindow.h"
#include "dialogProcedures.h"
#include <boost/algorithm/string.hpp>
#include "reorganizeWindow.h"
#include "acquisitionImmutables.h"
#include <string>
#include "Shellapi.h"
#include "fileManage.h"
#include "Commctrl.h"

LRESULT CALLBACK cameraWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) 
	{

		case WM_CREATE: 
		{
			initializeCameraWindow(hWnd);
			break;
		}
		case WM_CTLCOLOREDIT: 
		{
			DWORD CtrlID = GetDlgCtrlID((HWND)lParam);
			HDC hdcStatic = (HDC)wParam;
			SetTextColor(hdcStatic, RGB(255, 255, 255));
			SetBkColor(hdcStatic, RGB(100, 110, 100));
			return (INT_PTR)eGreyGreenBrush;
			break;
		}
		case WM_CTLCOLORSTATIC: 
		{
			DWORD CtrlID = GetDlgCtrlID((HWND)lParam);
			HDC hdcStatic = (HDC)wParam;
			SetTextColor(hdcStatic, RGB(255, 255, 255));
			SetBkColor(hdcStatic, RGB(0, 30, 0));
			return (INT_PTR)eDarkGreenBrush;
			break;
		}
		case WM_VSCROLL:
		{
			int controlID = GetDlgCtrlID((HWND)lParam);
			switch (controlID)
			{
				case IDC_MAX_SLIDER_1:
				{
					int sliderPosition;
					sliderPosition = SendMessage(eMaximumPictureSlider1.hwnd, TBM_GETPOS, 0, 0);
					eCurrentMaximumPictureCount[0] = (sliderPosition);
					SetWindowText(eMaxSliderNumberEdit1.hwnd, (std::to_string(eCurrentMaximumPictureCount[0])).c_str());
					break;
				}
				case IDC_MIN_SLIDER_1:
				{
					int sliderPosition;
					sliderPosition = SendMessage(eMinimumPictureSlider1.hwnd, TBM_GETPOS, 0, 0);
					eCurrentMinimumPictureCount[0] = sliderPosition;
					SetWindowText(eMinSliderNumberEdit1.hwnd, (std::to_string(eCurrentMinimumPictureCount[0])).c_str());
					break;
				}
				case IDC_MAX_SLIDER_2:
				{
					int sliderPosition;
					sliderPosition = SendMessage(eMaximumPictureSlider2.hwnd, TBM_GETPOS, 0, 0);
					eCurrentMaximumPictureCount[1] = (sliderPosition);
					SetWindowText(eMaxSliderNumberEdit2.hwnd, (std::to_string(eCurrentMaximumPictureCount[1])).c_str());
					break;
				}
				case IDC_MIN_SLIDER_2:
				{
					int sliderPosition;
					sliderPosition = SendMessage(eMinimumPictureSlider2.hwnd, TBM_GETPOS, 0, 0);
					eCurrentMinimumPictureCount[1] = sliderPosition;
					SetWindowText(eMinSliderNumberEdit2.hwnd, (std::to_string(eCurrentMinimumPictureCount[1])).c_str());
					break;
				}
				case IDC_MAX_SLIDER_3:
				{
					int sliderPosition;
					sliderPosition = SendMessage(eMaximumPictureSlider3.hwnd, TBM_GETPOS, 0, 0);
					eCurrentMaximumPictureCount[2] = (sliderPosition);
					SetWindowText(eMaxSliderNumberEdit3.hwnd, (std::to_string(eCurrentMaximumPictureCount[2])).c_str());
					break;
				}
				case IDC_MIN_SLIDER_3:
				{
					int sliderPosition;
					sliderPosition = SendMessage(eMinimumPictureSlider3.hwnd, TBM_GETPOS, 0, 0);
					eCurrentMinimumPictureCount[2] = sliderPosition;
					SetWindowText(eMinSliderNumberEdit3.hwnd, (std::to_string(eCurrentMinimumPictureCount[2])).c_str());
					break;
				}
				case IDC_MAX_SLIDER_4:
				{
					int sliderPosition;
					sliderPosition = SendMessage(eMaximumPictureSlider4.hwnd, TBM_GETPOS, 0, 0);
					eCurrentMaximumPictureCount[3] = sliderPosition;
					SetWindowText(eMaxSliderNumberEdit4.hwnd, (std::to_string(eCurrentMaximumPictureCount[3])).c_str());
					break;
				}
				case IDC_MIN_SLIDER_4:
				{
					int sliderPosition;
					sliderPosition = SendMessage(eMinimumPictureSlider4.hwnd, TBM_GETPOS, 0, 0);
					eCurrentMinimumPictureCount[3] = sliderPosition;
					SetWindowText(eMinSliderNumberEdit4.hwnd, (std::to_string(eCurrentMinimumPictureCount[3])).c_str());
					break;
				}
			}
			int experimentPictureNumber;
			if (eRealTimePictures)
			{
				experimentPictureNumber = 0;
			}
			else
			{
				experimentPictureNumber = (((eCurrentAccumulationNumber - 1) % ePicturesPerStack) % ePicturesPerExperiment);
			}
			if ((experimentPictureNumber == ePicturesPerExperiment - 1 || eRealTimePictures) && eDataExists)
			{
				myAndor::drawDataWindow();
			}
			break;
		}
		case WM_RBUTTONUP:
		{
			// get position of click.
			int xPos = LOWORD(lParam);
			int yPos = HIWORD(lParam);
			// check all boxes and all pictures.
			for (int pictureInc = 0; pictureInc < eImageBackgroundAreas.size(); pictureInc++)
			{
				for (int horizontalInc = 0; horizontalInc < ePixelRectangles[pictureInc].size(); horizontalInc++)
				{
					for (int verticalInc = 0; verticalInc < ePixelRectangles[pictureInc][horizontalInc].size(); verticalInc++)
					{
						RECT relevantRect = ePixelRectangles[pictureInc][horizontalInc][verticalInc];
						// check if inside box
						if (xPos < relevantRect.right && xPos > relevantRect.left && yPos < relevantRect.bottom && yPos > relevantRect.top)
						{
							RECT smallRect;
							smallRect.left = relevantRect.left + 7.0 * (relevantRect.right - relevantRect.left) / 16.0;
							smallRect.right = relevantRect.left + 9.0 * (relevantRect.right - relevantRect.left) / 16.0;
							smallRect.top = relevantRect.top + 7.0 * (relevantRect.bottom - relevantRect.top) / 16.0;
							smallRect.bottom = relevantRect.top + 9.0 * (relevantRect.bottom - relevantRect.top) / 16.0;
							// get appropriate brush and pen
							HDC hDC = GetDC(eCameraWindowHandle);
							SelectObject(hDC, GetStockObject(HOLLOW_BRUSH));
							SelectObject(hDC, GetStockObject(DC_PEN));
							if (eCurrentPicturePallete[pictureInc] == 0 || eCurrentPicturePallete[pictureInc] == 2)
							{
								SetDCPenColor(hDC, RGB(255, 0, 0));
								Ellipse(hDC, relevantRect.left, relevantRect.top, relevantRect.right, relevantRect.bottom);
								SelectObject(hDC, GetStockObject(DC_BRUSH));
								SetDCBrushColor(hDC, RGB(255, 0, 0));
							}
							else
							{
								SetDCPenColor(hDC, RGB(0, 255, 0));
								Ellipse(hDC, relevantRect.left, relevantRect.top, relevantRect.right, relevantRect.bottom);
								SelectObject(hDC, GetStockObject(DC_BRUSH));
								SetDCBrushColor(hDC, RGB(0, 255, 0));
							}
							Ellipse(hDC, smallRect.left, smallRect.top, smallRect.right, smallRect.bottom);
							ReleaseDC(eCameraWindowHandle, hDC);
							eCurrentlySelectedPixel.first = horizontalInc;
							eCurrentlySelectedPixel.second = ePixelRectangles[pictureInc][horizontalInc].size() - 1 - verticalInc;
						}
					}
				}
			}
			int experimentPictureNumber;
			if (eRealTimePictures)
			{
				experimentPictureNumber = 0;
			}
			else
			{
				experimentPictureNumber = (((eCurrentAccumulationNumber - 1) % ePicturesPerStack) % ePicturesPerExperiment);
			}
			if ((experimentPictureNumber == ePicturesPerExperiment - 1 || eRealTimePictures) && eDataExists)
			{
				myAndor::drawDataWindow();
			}
			break;
		}
		case WM_SIZE:
		{
			reorganizeWindow(eCurrentlySelectedCameraMode, hWnd);
			for (int imageLocation = 0; imageLocation < eImageBackgroundAreas.size(); imageLocation++)
			{
				int imageBoxWidth = eImageBackgroundAreas[imageLocation].right - eImageBackgroundAreas[imageLocation].left + 1;
				int imageBoxHeight = eImageBackgroundAreas[imageLocation].bottom - eImageBackgroundAreas[imageLocation].top + 1;
				double boxWidth = imageBoxWidth / eImageWidth;
				double boxHeight = imageBoxHeight / eImageHeight;
				if (boxWidth > boxHeight)
				{
					// scale the box width down.
					eImageDrawAreas[imageLocation].left = eImageBackgroundAreas[imageLocation].left;
					eImageDrawAreas[imageLocation].right = (int)eImageBackgroundAreas[imageLocation].left
						+ (eImageBackgroundAreas[imageLocation].right - eImageBackgroundAreas[imageLocation].left) * boxHeight / boxWidth;
					double pixelsAreaWidth = eImageDrawAreas[imageLocation].right - eImageDrawAreas[imageLocation].left + 1;
					// move to center
					eImageDrawAreas[imageLocation].left += (imageBoxWidth - pixelsAreaWidth) / 2;
					eImageDrawAreas[imageLocation].right += (imageBoxWidth - pixelsAreaWidth) / 2;
					eImageDrawAreas[imageLocation].top = eImageBackgroundAreas[imageLocation].top;
					eImageDrawAreas[imageLocation].bottom = eImageBackgroundAreas[imageLocation].bottom;
					double pixelsAreaHeight = imageBoxHeight;
				}
				else
				{
					// scale the box height down.
					eImageDrawAreas[imageLocation].left = eImageBackgroundAreas[imageLocation].left;
					eImageDrawAreas[imageLocation].right = eImageBackgroundAreas[imageLocation].right;
					double pixelsAreaWidth = imageBoxWidth;
					// move to center
					eImageDrawAreas[imageLocation].top = eImageBackgroundAreas[imageLocation].top;
					eImageDrawAreas[imageLocation].bottom = (int)eImageBackgroundAreas[imageLocation].top + (eImageBackgroundAreas[imageLocation].bottom - eImageBackgroundAreas[imageLocation].top) * boxWidth / boxHeight;
					double pixelsAreaHeight = eImageDrawAreas[imageLocation].bottom - eImageDrawAreas[imageLocation].top + 1;
					eImageDrawAreas[imageLocation].top += (imageBoxWidth - pixelsAreaHeight) / 2;
					eImageDrawAreas[imageLocation].bottom += (imageBoxWidth - pixelsAreaHeight) / 2;
				}
			}
			break;
		}
		case WM_COMMAND:
		{
			int controlID = LOWORD(wParam);
			switch (controlID)
			{
				case ID_PICTURES_REAL_TIME_PICTURES:
				{
					MENUITEMINFO itemInfo;
					itemInfo.cbSize = sizeof(MENUITEMINFO);
					itemInfo.fMask = MIIM_STATE;
					HMENU myMenu = GetMenu(hWnd);
					GetMenuItemInfo(myMenu, ID_PICTURES_REAL_TIME_PICTURES, FALSE, &itemInfo);
					if ((itemInfo.fState & MFS_CHECKED) == MFS_CHECKED)
					{
						CheckMenuItem(myMenu, ID_PICTURES_REAL_TIME_PICTURES, MF_UNCHECKED);
						eRealTimePictures = false;
					}
					else
					{
						CheckMenuItem(myMenu, ID_PICTURES_REAL_TIME_PICTURES, MF_CHECKED);
						eRealTimePictures = true;
					}
					break;
				}
				case IDC_MIN_SLIDER_1_NUMBER_EDIT:
				{
					if (HIWORD(wParam) == EN_KILLFOCUS)
					{
						TCHAR minNumText[256];
						int minNum;
						SendMessage(eMinSliderNumberEdit1.hwnd, WM_GETTEXT, 256, (LPARAM)minNumText);
						std::string minNumStr(minNumText);
						try
						{
							minNum = std::stoi(minNumStr);
						}
						catch (std::invalid_argument&)
						{
							MessageBox(0, "Number in Minimum edit didn't convert to an integer!", 0, 0);
							break;
						}
						eCurrentMinimumPictureCount[0] = minNum;
						SendMessage(eMinimumPictureSlider1.hwnd, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)eCurrentMinimumPictureCount[0]);
						int experimentPictureNumber;
						if (eRealTimePictures)
						{
							experimentPictureNumber = 0;
						}
						else
						{
							experimentPictureNumber = (((eCurrentAccumulationNumber - 1) % ePicturesPerStack) % ePicturesPerExperiment);
						}
						if ((experimentPictureNumber == ePicturesPerExperiment - 1 || eRealTimePictures) && eDataExists)
						{
							myAndor::drawDataWindow();
						}
					}
					break;
				}
				case IDC_MAX_SLIDER_1_NUMBER_EDIT:
				{
					if (HIWORD(wParam) == EN_KILLFOCUS)
					{
						TCHAR maxNumText[256];
						int maxNum;
						SendMessage(eMaxSliderNumberEdit1.hwnd, WM_GETTEXT, 256, (LPARAM)maxNumText);
						std::string maxNumStr(maxNumText);
						try
						{
							maxNum = std::stoi(maxNumStr);
						}
						catch (std::invalid_argument&)
						{
							MessageBox(0, "Number in Maximum edit didn't convert to an integer!", 0, 0);
							break;
						}
						eCurrentMaximumPictureCount[0] = maxNum;
						SendMessage(eMaximumPictureSlider1.hwnd, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)eCurrentMaximumPictureCount[0]);
						int experimentPictureNumber;
						if (eRealTimePictures)
						{
							experimentPictureNumber = 0;
						}
						else
						{
							experimentPictureNumber = (((eCurrentAccumulationNumber - 1) % ePicturesPerStack) % ePicturesPerExperiment);
						}
						if ((experimentPictureNumber == ePicturesPerExperiment - 1 || eRealTimePictures) && eDataExists)
						{
							myAndor::drawDataWindow();
						}
					}
					break;
				}
				case IDC_MIN_SLIDER_2_NUMBER_EDIT:
				{
					if (HIWORD(wParam) == EN_KILLFOCUS)
					{
						TCHAR minNumText[256];
						int minNum;
						SendMessage(eMinSliderNumberEdit2.hwnd, WM_GETTEXT, 256, (LPARAM)minNumText);
						std::string minNumStr(minNumText);
						try
						{
							minNum = std::stoi(minNumStr);
						}
						catch (std::invalid_argument&)
						{
							MessageBox(0, "Number in Minimum edit didn't convert to an integer!", 0, 0);
							break;
						}
						eCurrentMinimumPictureCount[1] = minNum;
						SendMessage(eMinimumPictureSlider2.hwnd, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)eCurrentMinimumPictureCount[1]);
						int experimentPictureNumber;
						if (eRealTimePictures)
						{
							experimentPictureNumber = 0;
						}
						else
						{
							experimentPictureNumber = (((eCurrentAccumulationNumber - 1) % ePicturesPerStack) % ePicturesPerExperiment);
						}
						if ((experimentPictureNumber == ePicturesPerExperiment - 1 || eRealTimePictures) && eDataExists)
						{
							myAndor::drawDataWindow();
						}
					}
					break;
				}
				case IDC_MAX_SLIDER_2_NUMBER_EDIT:
				{
					if (HIWORD(wParam) == EN_KILLFOCUS)
					{
						TCHAR maxNumText[256];
						int maxNum;
						SendMessage(eMaxSliderNumberEdit2.hwnd, WM_GETTEXT, 256, (LPARAM)maxNumText);
						std::string maxNumStr(maxNumText);
						try
						{
							maxNum = std::stoi(maxNumStr);
						}
						catch (std::invalid_argument&)
						{
							MessageBox(0, "Number in Maximum edit didn't convert to an integer!", 0, 0);
							break;
						}
						eCurrentMaximumPictureCount[1] = maxNum;
						SendMessage(eMaximumPictureSlider2.hwnd, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)eCurrentMaximumPictureCount[1]);
						int experimentPictureNumber;
						if (eRealTimePictures)
						{
							experimentPictureNumber = 0;
						}
						else
						{
							experimentPictureNumber = (((eCurrentAccumulationNumber - 1) % ePicturesPerStack) % ePicturesPerExperiment);
						}
						if ((experimentPictureNumber == ePicturesPerExperiment - 1 || eRealTimePictures) && eDataExists)
						{
							myAndor::drawDataWindow();
						}
					}
					break;
				}
				case IDC_MIN_SLIDER_3_NUMBER_EDIT:
				{
					if (HIWORD(wParam) == EN_KILLFOCUS)
					{
						TCHAR minNumText[256];
						int minNum;
						SendMessage(eMinSliderNumberEdit3.hwnd, WM_GETTEXT, 256, (LPARAM)minNumText);
						std::string minNumStr(minNumText);
						try
						{
							minNum = std::stoi(minNumStr);
						}
						catch (std::invalid_argument&)
						{
							MessageBox(0, "Number in Minimum edit didn't convert to an integer!", 0, 0);
							break;
						}
						eCurrentMinimumPictureCount[2] = minNum;
						SendMessage(eMinimumPictureSlider3.hwnd, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)eCurrentMinimumPictureCount[2]);
						int experimentPictureNumber;
						if (eRealTimePictures)
						{
							experimentPictureNumber = 0;
						}
						else
						{
							experimentPictureNumber = (((eCurrentAccumulationNumber - 1) % ePicturesPerStack) % ePicturesPerExperiment);
						}
						if ((experimentPictureNumber == ePicturesPerExperiment - 1 || eRealTimePictures) && eDataExists)
						{
							myAndor::drawDataWindow();
						}
					}
					break;
				}
				case IDC_MAX_SLIDER_3_NUMBER_EDIT:
				{
					if (HIWORD(wParam) == EN_KILLFOCUS)
					{
						TCHAR maxNumText[256];
						int maxNum;
						SendMessage(eMaxSliderNumberEdit3.hwnd, WM_GETTEXT, 256, (LPARAM)maxNumText);
						std::string maxNumStr(maxNumText);
						try
						{
							maxNum = std::stoi(maxNumStr);
						}
						catch (std::invalid_argument&)
						{
							MessageBox(0, "Number in Maximum edit didn't convert to an integer!", 0, 0);
							break;
						}
						eCurrentMaximumPictureCount[2] = maxNum;
						SendMessage(eMaximumPictureSlider3.hwnd, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)eCurrentMaximumPictureCount[2]);
						int experimentPictureNumber;
						if (eRealTimePictures)
						{
							experimentPictureNumber = 0;
						}
						else
						{
							experimentPictureNumber = (((eCurrentAccumulationNumber - 1) % ePicturesPerStack) % ePicturesPerExperiment);
						}
						if ((experimentPictureNumber == ePicturesPerExperiment - 1 || eRealTimePictures) && eDataExists)
						{
							myAndor::drawDataWindow();
						}
					}
					break;
				}
				case IDC_MIN_SLIDER_4_NUMBER_EDIT:
				{
					if (HIWORD(wParam) == EN_KILLFOCUS)
					{
						TCHAR minNumText[256];
						int minNum;
						SendMessage(eMinSliderNumberEdit4.hwnd, WM_GETTEXT, 256, (LPARAM)minNumText);
						std::string minNumStr(minNumText);
						try
						{
							minNum = std::stoi(minNumStr);
						}
						catch (std::invalid_argument&)
						{
							MessageBox(0, "Number in Minimum edit didn't convert to an integer!", 0, 0);
							break;
						}
						eCurrentMinimumPictureCount[3] = minNum;
						SendMessage(eMinimumPictureSlider4.hwnd, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)eCurrentMinimumPictureCount[3]);
						int experimentPictureNumber;
						if (eRealTimePictures)
						{
							experimentPictureNumber = 0;
						}
						else
						{
							experimentPictureNumber = (((eCurrentAccumulationNumber - 1) % ePicturesPerStack) % ePicturesPerExperiment);
						}
						if ((experimentPictureNumber == ePicturesPerExperiment - 1 || eRealTimePictures) && eDataExists)
						{
							myAndor::drawDataWindow();
						}
					}
					break;
				}
				case IDC_MAX_SLIDER_4_NUMBER_EDIT:
				{
					if (HIWORD(wParam) == EN_KILLFOCUS)
					{
						TCHAR maxNumText[256];
						int maxNum;
						SendMessage(eMaxSliderNumberEdit4.hwnd, WM_GETTEXT, 256, (LPARAM)maxNumText);
						std::string maxNumStr(maxNumText);
						try
						{
							maxNum = std::stoi(maxNumStr);
						}
						catch (std::invalid_argument&)
						{
							MessageBox(0, "Number in Maximum edit didn't convert to an integer!", 0, 0);
							break;
						}
						eCurrentMaximumPictureCount[3] = maxNum;
						SendMessage(eMaximumPictureSlider4.hwnd, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)eCurrentMaximumPictureCount[3]);
						int experimentPictureNumber;
						if (eRealTimePictures)
						{
							experimentPictureNumber = 0;
						}
						else
						{
							experimentPictureNumber = (((eCurrentAccumulationNumber - 1) % ePicturesPerStack) % ePicturesPerExperiment);
						}
						if ((experimentPictureNumber == ePicturesPerExperiment - 1 || eRealTimePictures) && eDataExists)
						{
							myAndor::drawDataWindow();
						}
					}
					break;
				}
				case ID_PLOTTING_ADD_PLOT:
				{
					// get text from all plots combo
					long long itemIndex = SendMessage(eAllPlotsCombo.hwnd, CB_GETCURSEL, 0, 0);
					//  check not -1
					if (itemIndex == -1)
					{
						break;
					}
					TCHAR currentPlotChars[256];
					SendMessage(eAllPlotsCombo.hwnd, (UINT)CB_GETLBTEXT, (WPARAM)itemIndex, (LPARAM)currentPlotChars);
					// add to plot
					SendMessage(eCurrentPlotsCombo.hwnd, (UINT)CB_ADDSTRING, 0, (LPARAM)currentPlotChars);
					break;
				}
				case ID_PLOTTING_REMOVE_PLOT:
				{
					// get text from all plots combo
					long long itemIndex = SendMessage(eCurrentPlotsCombo.hwnd, CB_GETCURSEL, 0, 0);
					//  check not -1
					if (itemIndex == -1)
					{
						break;
					}
					SendMessage(eCurrentPlotsCombo.hwnd, CB_DELETESTRING, itemIndex, 0);
					break;
				}
				case IDC_TRIGGER_COMBO:
				{
					if (HIWORD(wParam) == CBN_SELCHANGE)
					{
						TCHAR triggerModeChars[256];
						long long itemIndex = SendMessage(eTriggerComboHandle.hwnd, CB_GETCURSEL, 0, 0);
						if (itemIndex == -1)
						{
							break;
						}
						SendMessage(eTriggerComboHandle.hwnd, (UINT)CB_GETLBTEXT, (WPARAM)itemIndex, (LPARAM)triggerModeChars);
						eCurrentTriggerMode = std::string(triggerModeChars);
						reorganizeWindow(eCurrentlySelectedCameraMode, hWnd);
					}
				}
				case IDC_CAMERA_MODE_COMBO:
				{
					if (HIWORD(wParam) == CBN_SELCHANGE)
					{
						if (eSystemIsRunning)
						{
							appendText("Warning: System will not change camera mode until the acquisition is restarted.\r\n", IDC_ERROR_EDIT);
						}
						TCHAR  cameraModeChars[256];
						long long itemIndex = SendMessage(eCameraModeComboHandle.hwnd, CB_GETCURSEL, 0, 0);
						if (itemIndex == -1)
						{
							// user didn't select anything.
							break;
						}
						SendMessage(eCameraModeComboHandle.hwnd, (UINT)CB_GETLBTEXT, (WPARAM)itemIndex, (LPARAM)cameraModeChars);
						eCurrentlySelectedCameraMode = std::string(cameraModeChars);
						if (eCurrentlySelectedCameraMode == "Continuous Single Scans Mode")
						{
							eAcquisitionMode = 5;
							if (ePicturesPerStack != INT_MAX)
							{
								ePreviousPicturesPerSubSeries = ePicturesPerStack;
							}
							ePicturesPerStack = INT_MAX;
							SendMessage(eExperimentsPerStackDispHandle.hwnd, WM_SETTEXT, 0, (LPARAM)std::to_string(ePicturesPerStack).c_str());
						}
						else if (eCurrentlySelectedCameraMode == "Kinetic Series Mode")
						{
							eAcquisitionMode = 3;
							//ePicturesPerStack = ePreviousPicturesPerSubSeries;
							SendMessage(eExperimentsPerStackDispHandle.hwnd, WM_SETTEXT, 0, (LPARAM)std::to_string(ePicturesPerStack).c_str());
						}
						else if (eCurrentlySelectedCameraMode == "Accumulate Mode")	
						{
							eAcquisitionMode = 2;
							if (ePicturesPerStack != INT_MAX)
							{
								ePreviousPicturesPerSubSeries = ePicturesPerStack;
							}
							ePicturesPerStack = INT_MAX;
							SendMessage(eExperimentsPerStackDispHandle.hwnd, WM_SETTEXT, 0, (LPARAM)std::to_string(ePicturesPerStack).c_str());
						}
						reorganizeWindow(eCurrentlySelectedCameraMode, hWnd);
						break;
					}
					break;
				}
				case ID_PLOTTING_NEW_PLOT:
				{
					eCurrentPlottingInfo.clear();
					eCurrentPlottingInfo = PlottingInfo();
					DialogBox(eHInst, MAKEINTRESOURCE(IDD_PLOT_CREATOR), 0, (DLGPROC)dialogProcedures::plottingDialogProc);
					break;
				}
				case ID_PLOTTING_VIEW_PLOT_INFO:
				{
					// get name of file.
					TCHAR  selectedPlotText[256];
					long long itemIndex = SendMessage(eAllPlotsCombo.hwnd, CB_GETCURSEL, 0, 0);
					if (itemIndex == -1)
					{
						// user didn't select anything.
						break;
					}
					SendMessage(eAllPlotsCombo.hwnd, (UINT)CB_GETLBTEXT, (WPARAM)itemIndex, (LPARAM)selectedPlotText);
					std::string fileToView = PLOT_FILES_SAVE_LOCATION + std::string(selectedPlotText) + ".plot";
					eCurrentPlottingInfo.clear();
					eCurrentPlottingInfo.loadPlottingInfoFromFile(fileToView);
					std::string message = eCurrentPlottingInfo.returnAllInfo();
					int answer = DialogBoxParam(eHInst, MAKEINTRESOURCE(IDD_RICH_EDIT_OK_CANCEL_MESSAGE_BOX), 0, (DLGPROC)dialogProcedures::richEditOkCancelMessageBoxProc, (LPARAM)message.c_str());
					break;
				}
				case ID_PLOTTING_EDIT_PLOT:
				{
					// get name of file.
					TCHAR  selectedPlotText[256];
					long long itemIndex = SendMessage(eAllPlotsCombo.hwnd, CB_GETCURSEL, 0, 0);
					if (itemIndex == -1)
					{
						// user didn't select anything.
						break;
					}
					SendMessage(eAllPlotsCombo.hwnd, (UINT)CB_GETLBTEXT, (WPARAM)itemIndex, (LPARAM)selectedPlotText);
					std::string fileToEdit = PLOT_FILES_SAVE_LOCATION + std::string(selectedPlotText) + ".plot";
					eCurrentPlottingInfo.clear();
					eCurrentPlottingInfo.loadPlottingInfoFromFile(fileToEdit);
					DialogBox(eHInst, MAKEINTRESOURCE(IDD_PLOT_CREATOR), 0, (DLGPROC)dialogProcedures::plottingDialogProc);
					break;
				}
				case ID_PLOTTING_DELETE_SELECTED_PLOT_FILE:
				{
					// get name of file.
					TCHAR  selectedPlotText[256];
					long long itemIndex = SendMessage(eAllPlotsCombo.hwnd, CB_GETCURSEL, 0, 0);
					if (itemIndex == -1)
					{
						// user didn't select anything.
						break;
					}
					SendMessage(eAllPlotsCombo.hwnd, (UINT)CB_GETLBTEXT, (WPARAM)itemIndex, (LPARAM)selectedPlotText);
					std::string fileToDelete = PLOT_FILES_SAVE_LOCATION + std::string(selectedPlotText) + ".plot";
					// check to be sure.
					int answer = MessageBox(0, ("You are about to delete the following plot file: " + fileToDelete + ". Are you sure?").c_str(), 0, MB_YESNO);
					if (answer == IDNO)
					{
						break;
					}
					int result = DeleteFile(fileToDelete.c_str());
					if (result == 0)
					{
						MessageBox(0, "Delete Failed!", 0, 0);
					}
					fileManage::reloadCombo(eAllPlotsCombo.hwnd, PLOT_FILES_SAVE_LOCATION, "*.plot", "__NONE__");
					break;
				}
				case ID_PICTURES_CHANGE_PICTURE_COLORS:
				{
					int a = DialogBox(eHInst, MAKEINTRESOURCE(IDD_PICTURE_COLOR), 0, (DLGPROC)dialogProcedures::picturePalletesDialogProcedure);
					int b = GetLastError();
					break;
				}
				case IDA_F5:
				case ID_FILE_START:
				{
					if (eSystemIsRunning)
					{
						appendText("System is already running! Please Abort to restart.\r\n", IDC_ERROR_EDIT);
						break;
					}

					// Set the running version to whatever is selected at the beginning of this function.
					eCurrentlyRunningCameraMode = eCurrentlySelectedCameraMode;
					// check exposure times
					if (eExposureTimes.size() == 0)
					{
						MessageBox(0, "Please Set at least one exposure time.", 0, 0);
						break;
					}
					// Check Image parameters
					if (eLeftImageBorder > eRightImageBorder || eTopImageBorder > eBottomImageBorder)
					{
						MessageBox(0, "ERROR: Image start positions must not be greater than end positions\r\n", 0, 0);
						break;
					}
					if (eLeftImageBorder < 1 || eRightImageBorder > eXPixels)
					{
						MessageBox(0, "ERROR: Image horizontal borders must be greater than 0 and less than the detector width\r\n", 0, 0);
						break;
					}
					if (eTopImageBorder < 1 || eBottomImageBorder > eYPixels)
					{
						MessageBox(0, "ERROR: Image verttical borders must be greater than 0 and less than the detector height\r\n", 0, 0);
						break;
					}
					if ((eRightImageBorder - eLeftImageBorder + 1) % eHorizontalBinning != 0)
					{
						MessageBox(0, "ERROR: Image width must be a multiple of Horizontal Binning\r\n", 0, 0);
						break;
					}
					if ((eBottomImageBorder - eTopImageBorder + 1) % eVerticalBinning != 0)
					{
						MessageBox(0, "ERROR: Image height must be a multiple of Vertical Binning\r\n", 0, 0);
						break;
					}
					if (ePicturesPerExperiment <= 0)
					{
						MessageBox(0, "ERROR: Please set the number of pictures per experiment to a positive non-zero value.", 0, 0);
						break;
					}
					if (eCurrentlySelectedCameraMode == "Kinetic Series Mode") 
					{
						if (eKineticCycleTime == 0 && eCurrentTriggerMode == "Internal")
						{
							MessageBox(0, "ERROR: Since you are running in internal trigger mode, please Set a kinetic cycle time.", 0, 0);
							break;
						}
						if (eExperimentsPerStack <= 0)
						{
							MessageBox(0, "ERROR: Please set the \"Experiments per Stack\" variable to a positive non-zero value.", 0, 0);
							break;
						}
						if (eCurrentAccumulationStackNumber <= 0)
						{
							MessageBox(0, "ERROR: Please set the number of accumulation stacks to a positive non-zero value.", 0, 0);
							break;
						}

					}
					if (eCurrentlySelectedCameraMode == "Accumulate Mode")
					{
						if (eCurrentAccumulationModeTotalAccumulationNumber <= 0)
						{
							MessageBox(0, "ERROR: Please set the current Accumulation Number to a positive non-zero value.", 0, 0);
							break;
						}
						if (eAccumulationTime <= 0)
						{
							MessageBox(0, "ERROR: Please set the current Accumulation Time to a positive non-zero value.", 0, 0);
							break;
						}
					}
										
					/// check if actually ready to start.
					// get selected plots
					int plotNumber = SendMessage(eCurrentPlotsCombo.hwnd, CB_GETCOUNT, 0, 0);
					eCurrentPlotNames.clear();
					for (int plotInc = 0; plotInc < plotNumber; plotInc++)
					{
						TCHAR tempPlotName[256];
						SendMessage(eCurrentPlotsCombo.hwnd, CB_GETLBTEXT, (WPARAM)plotInc, (LPARAM)tempPlotName);
						eCurrentPlotNames.push_back(tempPlotName);
					}
					// check plotting Parameters.
					for (int plotInc = 0; plotInc < eCurrentPlotNames.size(); plotInc++)
					{
						PlottingInfo tempInfoCheck;
						tempInfoCheck.loadPlottingInfoFromFile(PLOT_FILES_SAVE_LOCATION + eCurrentPlotNames[plotInc] + ".plot");
						if (tempInfoCheck.getPictureNumber() != ePicturesPerExperiment)
						{
							MessageBox(0, ("ERROR: one of the plots selected, " + eCurrentPlotNames[plotInc] + ", is not built for the currently selected number"
								" of pictures per experiment. Please revise either the current setting or the plot file.").c_str(), 0, 0);
						}
					}
					std::string dialogMsg;
					dialogMsg = "Starting Parameters:\r\n%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\r\n";
					dialogMsg += "Current Camera Temperature Setting: " + std::to_string(eCameraTemperatureSetting) + "\r\n";
					dialogMsg += "Exposure Time: ";
					for (int exposureInc = 0; exposureInc < eExposureTimes.size(); exposureInc++)
					{
						dialogMsg += std::to_string(eExposureTimes[exposureInc]) + ", ";
					}
					dialogMsg += "\b\b\r\n";
					dialogMsg += "Image Settings: " + std::to_string(eLeftImageBorder) + " - " + std::to_string(eRightImageBorder) + ", "
						+ std::to_string(eTopImageBorder) + " - " + std::to_string(eBottomImageBorder) + "\r\n";
					dialogMsg += "\r\n";
					dialogMsg += "Kintetic Cycle Time: " + std::to_string(eKineticCycleTime) + "\r\n";
					dialogMsg += "Pictures per Experiment: " + std::to_string(ePicturesPerExperiment) + "\r\n";
					dialogMsg += "Pictures per Stack: " + std::to_string(ePicturesPerStack) + "\r\n";
					dialogMsg += "Stack Number: " + std::to_string(eCurrentAccumulationStackNumber) + "\r\n";
					dialogMsg += "Atom Threshold: " + std::to_string(eDetectionThreshold) + "\r\n";
					dialogMsg += "\r\n";
					dialogMsg += "Incrementing File Name: " + std::to_string(eIncSaveFileNameOption) + "\r\n";
					dialogMsg += "Current Plotting Options: \r\n";
					for (int plotInc = 0; plotInc < eCurrentPlotNames.size(); plotInc++)
					{
						dialogMsg += "\t" + eCurrentPlotNames[plotInc] + "\r\n";
					}
					int answer = DialogBoxParam(eHInst, MAKEINTRESOURCE(IDD_RICH_EDIT_OK_CANCEL_MESSAGE_BOX), 0, (DLGPROC)dialogProcedures::richEditOkCancelMessageBoxProc, (LPARAM)dialogMsg.c_str());
					if (answer == IDCANCEL)
					{
						break;
					}
					
					// tells system an acq has taken place
					if (myAndor::getStatus() != 0)
					{
						return -1;
					}

					eSystemIsRunning = true;
					time_t time_obj = time(0);   // get time now
					struct tm currentTime;
					localtime_s(&currentTime, &time_obj);
					std::string timeStr = "(" + std::to_string(currentTime.tm_year + 1900) + ":" + std::to_string(currentTime.tm_mon + 1) + ":"
						+ std::to_string(currentTime.tm_mday) + ")" + std::to_string(currentTime.tm_hour) + ":"
						+ std::to_string(currentTime.tm_min) + ":" + std::to_string(currentTime.tm_sec);
					appendText("\r\n**********" + timeStr + "**********\r\nSystem is Running.\r\n", IDC_STATUS_EDIT);
					appendText("\r\n******" + timeStr + "******\r\n", IDC_ERROR_EDIT);
					// Set hardware and start acquisition
					if (myAndor::setSystem() != 0)
					{
						eSystemIsRunning = false;
					}
					break;
				}
				case IDA_ESC:
				case ID_FILE_ABORTACQUISITION:
				{
					if (!eSystemIsRunning)
					{
						appendText("ERROR: System was not running.\r\n", IDC_ERROR_EDIT);
						break;
					}
					int fitsStatus = 0;
					eFitsOkay = false;
					// give the thread some time so that don't try to open a closed file... not very good solution.
					// TODO: Improve this
					Sleep(500);
					fits_close_file(eFitsFile, &fitsStatus);
					// print any error messages
					if (fitsStatus != 0)
					{
						std::vector<char> errMsg;
						errMsg.resize(80);
						//std::string errMsg;
						fits_get_errstatus(fitsStatus, errMsg.data());
						if (eFitsFile == NULL)
						{
							appendText("CFITS error: Could not create data file on andor. Is the andor connected???\r\n", IDC_STATUS_EDIT);
						}
						else
						{
							appendText("CFITS error: " + std::string(errMsg.data()) + "\r\n", IDC_STATUS_EDIT);
						}
					}
					// abort acquisition if in progress
					int status;
					std::string errMsg;
					if (!ANDOR_SAFEMODE)
					{
						errMsg = myAndor::andorErrorChecker(GetStatus(&status));
					}
					else
					{
						// so that this may continue...
						errMsg = "DRV_SUCCESS";
						status = DRV_ACQUIRING;
					}
					if (errMsg != "DRV_SUCCESS")
					{
						appendText("ERROR: GetStatus Error: " + errMsg + "\r\n", IDC_STATUS_EDIT);
						break;
					}
					if (status == DRV_ACQUIRING) 
					{
						eSystemIsRunning = false;
						if (!ANDOR_SAFEMODE)
						{
							errMsg = myAndor::andorErrorChecker(AbortAcquisition());
						}
						else
						{
							errMsg = "DRV_SUCCESS";
						}

						if (errMsg != "DRV_SUCCESS")
						{
							appendText("ERROR: Error aborting acquistion!", IDC_STATUS_EDIT);
						}
						else 
						{
							ePlotThreadExitIndicator = false;
							//eThreadExitIndicator = false;
							// Wait until plotting thread is complete.
							WaitForSingleObject(ePlottingThreadHandle, INFINITE);
							if (ANDOR_SAFEMODE)
							{
								eSystemIsRunning = false;
							}
							appendText("Aborting Acquisition", IDC_STATUS_EDIT);
						}
					}
					// or else let user know none is in progress
					else 
					{
						appendText("System was not Acquiring", IDC_STATUS_EDIT);
					}
					break;
				}
				case ID_FILE_EXIT:
				{
					int temperature;
					std::string errMsg;
					if (!ANDOR_SAFEMODE)
					{
//						int errorCode = 
						//MessageBox(0, std::to_string(errorCode).c_str(), 0, 0);
						errMsg = myAndor::andorErrorChecker(GetTemperature(&temperature));
						if (errMsg != "DRV_TEMPERATURE_OFF" && errMsg != "DRV_TEMPERATURE_STABILIZED" && errMsg != "DRV_TEMPERATURE_NOT_REACHED" && errMsg != "DRV_TEMPERATURE_DRIFT"
							&& errMsg == "DRV_TEMPERATURE_NOT_STABILIZED")
						{
							appendText("Error Getting Temperature before exiting: " + errMsg + "\r\n", IDC_STATUS_EDIT);
							break;
						}
					}
					else
					{
						temperature = 20;
					}

					if (temperature < 0) 
					{
						SendMessage(eStatusEditHandle.hwnd, WM_SETTEXT, 0, (LPARAM)("Please Only Quit when the Temperature of the Camera is "
							"above 0 C, or else rapid heating that occurs when the cooler is turned off at low "
							"temperatures might cause rapid mechanical expansions that could damage the camera!\r\n"));
					}
					else 
					{
						// The program is ending. Close the thread and exit the main UI processing loop.
						eThreadExitIndicator = false;
						std::string err;
						if (!ANDOR_SAFEMODE)
						{
							err = myAndor::andorErrorChecker(CancelWait());
						}
						else
						{
							err = "DRV_SUCCESS";
						}
						if (err != "DRV_SUCCESS") 
						{
							appendText("ERROR: cancelWait Failed: " + err, IDC_STATUS_EDIT);
							break;
						}
						appendText("Waiting for plotting to finish...\r\n", IDC_STATUS_EDIT);
						WaitForSingleObject(eCameraThreadHandle, INFINITE);
						CloseHandle(eCameraThreadHandle);
						PostQuitMessage(0);
					}
					// if here, camera is too cold.
					break;
				}
				case IDC_CLEAR_STATUS_BUTTON:
				{
					// Send blank message
					SendMessage(eStatusEditHandle.hwnd, WM_SETTEXT, NULL, (LPARAM)"");
					break;
				}
				case IDC_ERROR_CLEAR_BUTTON:
				{
					SendMessage(eErrorEditHandle.hwnd, WM_SETTEXT, NULL, (LPARAM)"");
					break;
				}
				case IDC_TEMP_OFF_BUTTON:
				{
					// it's simple to turn it off.
					myAndor::changeTemperatureSetting(true);
					break;
				}
				case IDC_SET_TEMP_BUTTON: 
				{
					myAndor::setTemperature();
					break;
				}
				case IDC_SET_EXPOSURE_BUTTON:
				{
					// Get the exposure time from the edit.
					SendMessage(eExposureDispHandle.hwnd, WM_SETTEXT, 0, (LPARAM)"");
					eExposureTimes.resize(4);
					TCHAR tempExposure[256];
					std::string tempExposureStr;
					int exposureCount = 0;
					bool breakBad = false;
					while (true)
					{
						if (exposureCount >= 4)
						{
							// recieved all four exposures.
							break;
						}
						if (exposureCount == 0)
						{
							SendMessage(eExposure1EditHandle.hwnd, WM_GETTEXT, 256, (LPARAM)tempExposure);
						}
						else if (exposureCount == 1)
						{
							SendMessage(eExposure2EditHandle.hwnd, WM_GETTEXT, 256, (LPARAM)tempExposure);
						}
						else if (exposureCount == 2)
						{
							SendMessage(eExposure3EditHandle.hwnd, WM_GETTEXT, 256, (LPARAM)tempExposure);
						}
						else if (exposureCount == 3)
						{
							SendMessage(eExposure4EditHandle.hwnd, WM_GETTEXT, 256, (LPARAM)tempExposure);
						}
						tempExposureStr = std::string(tempExposure);
						
						try
						{
							// try to make the text a float (usually works for most input)
							eExposureTimes[exposureCount] = std::stof(tempExposureStr) / 1000.0f;
							// check for negative value. Doesn't work first time.
							if (eExposureTimes[exposureCount] < 0)
							{
								if (exposureCount == 0)
								{
									MessageBox(0, ("ERROR: Invalid exposure time set for first time: " + std::to_string(eExposureTimes[0])
										+ ". You must set at least one positive exposure time.").c_str(), 0, 0);
									eExposureTimes.clear();
									break;
								}
								else
								{
									if (eExposureTimes[exposureCount] < 0)
									{
										eExposureTimes.resize(exposureCount);
										break;
									}
								}
							}
							else
							{
								// increment this for the next exposure.
								exposureCount++;
							}
						}
						catch (std::invalid_argument &exception)
						{
							// catch bad input
							MessageBox(0, "ERROR: Unable to convert exposure text to floating point number.", 0, 0);
							eExposureTimes.clear();
							SendMessage(eExposureDispHandle.hwnd, WM_SETTEXT, 0, (LPARAM)"");
							// because I'm in a while loop break doesn't work nicely...
							return DefWindowProc(hWnd, msg, wParam, lParam);
						}
					}
					// try to set this time.
					if (myAndor::setExposures() < 0)
					{
						appendText("ERROR: failed to set exposure times.", IDC_ERROR_EDIT);
						SendMessage(eExposureDispHandle.hwnd, WM_SETTEXT, 0, (LPARAM)"");
						break;
					}
					// now check actual times.
					if (myAndor::checkAcquisitionTimings() < 0)
					{
						// bad
						appendText("ERROR: Unable to check acquisition timings.\r\n", IDC_ERROR_EDIT);
						SendMessage(eExposureDispHandle.hwnd, WM_SETTEXT, 0, (LPARAM)"");
						break;
					}
					// now output things.
					if (eExposureTimes.size() <= 0)
					{
						// this shouldn't happend
						appendText("ERROR: reached bad location where eExposureTimes was of zero size, but this should have been detected earlier in the code.", IDC_ERROR_EDIT);
					}
					SendMessage(eExposureDispHandle.hwnd, WM_SETTEXT, 0, (LPARAM)"");
					appendText(std::to_string(eExposureTimes[0]), IDC_EXPOSURE_DISP);
					for (int exposureInc = 1; exposureInc < eExposureTimes.size(); exposureInc++)
					{
						appendText(" -> " + std::to_string(eExposureTimes[exposureInc]), IDC_EXPOSURE_DISP);
					}
					SendMessage(eKineticCycleTimeDispHandle.hwnd, WM_SETTEXT, 0, (LPARAM)std::to_string(eKineticCycleTime).c_str());
					SendMessage(eAccumulationTimeDisp.hwnd, WM_SETTEXT, 0, (LPARAM)std::to_string(eAccumulationTime).c_str());
					break;
				}
				case IDC_SET_IMAGE_PARAMS_BUTTON: 
				{
					// If new dimensions are set, we don't have data for those.
					
					eDataExists = false;
					// set all of the image parameters
					std::string tempStr;
					GetWindowText(eImgLeftSideEditHandle.hwnd, (LPSTR)tempStr.c_str(), 4);
					SendMessage(eImgLeftSideDispHandle.hwnd, WM_SETTEXT, 0, (LPARAM)tempStr.c_str());
					eLeftImageBorder = std::stoi(tempStr);
					GetWindowText(eImgRightSideEditHandle.hwnd, (LPSTR)tempStr.c_str(), 4);
					SendMessage(eImgRightSideDispHandle.hwnd, WM_SETTEXT, 0, (LPARAM)tempStr.c_str());
					eRightImageBorder = std::stoi(tempStr);
					GetWindowText(eImageBottomSideEditHandle.hwnd, (LPSTR)tempStr.c_str(), 4);
					SendMessage(eImageBottomSideDispHandle.hwnd, WM_SETTEXT, 0, (LPARAM)tempStr.c_str());
					eTopImageBorder = std::stoi(tempStr);
					GetWindowText(eImageTopSideEditHandle.hwnd, (LPSTR)tempStr.c_str(), 4);
					SendMessage(eImageTopSideDispHandle.hwnd, WM_SETTEXT, 0, (LPARAM)tempStr.c_str());
					eBottomImageBorder = std::stoi(tempStr);
					GetWindowText(eHorizontalBinningEditHandle.hwnd, (LPSTR)tempStr.c_str(), 4);
					SendMessage(eHorizontalBinningDispHandle.hwnd, WM_SETTEXT, 0, (LPARAM)tempStr.c_str());
					eHorizontalBinning = std::stoi(tempStr);
					GetWindowText(eVerticalBinningEditHandle.hwnd, (LPSTR)tempStr.c_str(), 4);
					SendMessage(eVerticalBinningDispHandle.hwnd, WM_SETTEXT, 0, (LPARAM)tempStr.c_str());
					eVerticalBinning = std::stoi(tempStr);
					eCurrentlySelectedPixel.first = 0;
					eCurrentlySelectedPixel.second = 0;
					// Calculate the number of actual pixels in each dimension.
					eImageWidth = (eRightImageBorder - eLeftImageBorder + 1) / eHorizontalBinning;
					eImageHeight = (eBottomImageBorder - eTopImageBorder + 1) / eVerticalBinning;

					// Check Image parameters
					if (eLeftImageBorder > eRightImageBorder || eTopImageBorder > eBottomImageBorder) 
					{
						appendText("ERROR: Image start positions must not be greater than end positions\r\n", IDC_ERROR_EDIT);
						break;
					}
					if (eLeftImageBorder < 1 || eRightImageBorder > eXPixels) 
					{
						appendText("ERROR: Image horizontal borders must be greater than 0 and less than the detector width\r\n", IDC_ERROR_EDIT);
						break;
					}
					if (eTopImageBorder < 1 || eBottomImageBorder > eYPixels) 
					{
						appendText("ERROR: Image verttical borders must be greater than 0 and less than the detector height\r\n", IDC_ERROR_EDIT);
						break;
					}
					if ((eRightImageBorder - eLeftImageBorder + 1) % eHorizontalBinning != 0) 
					{
						appendText("ERROR: Image width must be a multiple of Horizontal Binning\r\n", IDC_ERROR_EDIT);
						break;
					}
					if ((eBottomImageBorder - eTopImageBorder + 1) % eVerticalBinning != 0) 
					{
						appendText("ERROR: Image height must be a multiple of Vertical Binning\r\n", IDC_ERROR_EDIT);
						break;
					}
					// made it through successfully.
					
					for (int imageLocation = 0; imageLocation < eImageBackgroundAreas.size(); imageLocation++)
					{
						int imageBoxWidth = eImageBackgroundAreas[imageLocation].right - eImageBackgroundAreas[imageLocation].left + 1;
						int imageBoxHeight = eImageBackgroundAreas[imageLocation].bottom - eImageBackgroundAreas[imageLocation].top + 1;

						double boxWidth = imageBoxWidth / eImageWidth;
						double boxHeight = imageBoxHeight / eImageHeight;
						if (boxWidth > boxHeight)
						{
							// scale the box width down.
							eImageDrawAreas[imageLocation].left = eImageBackgroundAreas[imageLocation].left;
							eImageDrawAreas[imageLocation].right = (int)eImageBackgroundAreas[imageLocation].left
								+ (eImageBackgroundAreas[imageLocation].right - eImageBackgroundAreas[imageLocation].left) * boxHeight / boxWidth;
							double pixelsAreaWidth = eImageDrawAreas[imageLocation].right - eImageDrawAreas[imageLocation].left + 1;
							// move to center
							eImageDrawAreas[imageLocation].left += (imageBoxWidth - pixelsAreaWidth) / 2;
							eImageDrawAreas[imageLocation].right += (imageBoxWidth - pixelsAreaWidth) / 2;
							eImageDrawAreas[imageLocation].top = eImageBackgroundAreas[imageLocation].top;
							eImageDrawAreas[imageLocation].bottom = eImageBackgroundAreas[imageLocation].bottom;
							double pixelsAreaHeight = imageBoxHeight;
						}
						else
						{
							// cale the box height down.
							eImageDrawAreas[imageLocation].left = eImageBackgroundAreas[imageLocation].left;
							eImageDrawAreas[imageLocation].right = eImageBackgroundAreas[imageLocation].right;
							double pixelsAreaWidth = imageBoxWidth;
							// move to center
							eImageDrawAreas[imageLocation].top = eImageBackgroundAreas[imageLocation].top;
							eImageDrawAreas[imageLocation].bottom = (int)eImageBackgroundAreas[imageLocation].top + (eImageBackgroundAreas[imageLocation].bottom - eImageBackgroundAreas[imageLocation].top) * boxWidth / boxHeight;
							double pixelsAreaHeight = eImageDrawAreas[imageLocation].bottom - eImageDrawAreas[imageLocation].top + 1;
							eImageDrawAreas[imageLocation].top += (imageBoxWidth - pixelsAreaHeight) / 2;
							eImageDrawAreas[imageLocation].bottom += (imageBoxWidth - pixelsAreaHeight) / 2;
						}
					}
					// create rectangles for selection circle
					for (int pictureInc = 0; pictureInc < eImageDrawAreas.size(); pictureInc++)
					{
						ePixelRectangles[pictureInc].resize(eImageWidth);
						for (int widthInc = 0; widthInc < eImageWidth; widthInc++)
						{
							ePixelRectangles[pictureInc][widthInc].resize(eImageHeight);
							for (int heightInc = 0; heightInc < eImageHeight; heightInc++)
							{
								// for all 4 pictures...
								ePixelRectangles[pictureInc][widthInc][heightInc].left = (int)(eImageDrawAreas[pictureInc].left
												+ (double)widthInc * (eImageDrawAreas[pictureInc].right - eImageDrawAreas[pictureInc].left) / (double)eImageWidth + 2);
								ePixelRectangles[pictureInc][widthInc][heightInc].right = (int)(eImageDrawAreas[pictureInc].left
												+ (double)(widthInc + 1) * (eImageDrawAreas[pictureInc].right - eImageDrawAreas[pictureInc].left) / (double)eImageWidth + 2);
								ePixelRectangles[pictureInc][widthInc][heightInc].top = (int)(eImageDrawAreas[pictureInc].top
												+ (double)(heightInc) * (eImageDrawAreas[pictureInc].bottom - eImageDrawAreas[pictureInc].top) / (double)eImageHeight);
								ePixelRectangles[pictureInc][widthInc][heightInc].bottom = (int)(eImageDrawAreas[pictureInc].top
									+ (double)(heightInc + 1)* (eImageDrawAreas[pictureInc].bottom - eImageDrawAreas[pictureInc].top) / (double)eImageHeight);
							}
						}
					}


					break;
				}
				case IDC_SET_KINETIC_CYCLE_TIME_BUTTON: 
				{
					std::string tempStr;
					GetWindowText(eKineticCycleTimeEditHandle.hwnd, (LPSTR)tempStr.c_str(), 20);
					tempStr = std::string(tempStr.c_str());
					bool err = false;
					for (unsigned int strInc = 0; strInc < tempStr.size(); strInc++) 
					{
						if ((!isdigit(tempStr[strInc]) && (tempStr[strInc] != '.'))) 
						{
							char tempChar = tempStr[strInc];
							err = true;
							break;
						}
					}
					if (tempStr == "" || err == true) 
					{
						break;
					}
					eKineticCycleTime = std::stod(tempStr) / 1000.0;
					// get actual times.
					if (myAndor::setKineticCycleTime() < 0) 
					{
						appendText("ERROR: Failed to set kinetic cycle time.", IDC_ERROR_EDIT);
						break;
					}
					if (myAndor::checkAcquisitionTimings() < 0)
					{
						appendText("ERROR: Failed to check acquisiton timings.", IDC_ERROR_EDIT);
						break;
					}
					// now output things.
					if (eExposureTimes.size() > 0)
					{
						SendMessage(eExposureDispHandle.hwnd, WM_SETTEXT, 0, (LPARAM)"");
						appendText(std::to_string(eExposureTimes[0]), IDC_EXPOSURE_DISP);
						for (int exposureInc = 1; exposureInc < eExposureTimes.size(); exposureInc++)
						{
							appendText(" -> " + std::to_string(eExposureTimes[exposureInc]), IDC_EXPOSURE_DISP);
						}
					}
					SendMessage(eKineticCycleTimeDispHandle.hwnd, WM_SETTEXT, 0, (LPARAM)std::to_string(eKineticCycleTime).c_str());
					SendMessage(eAccumulationTimeDisp.hwnd, WM_SETTEXT, 0, (LPARAM)std::to_string(eAccumulationTime).c_str());
					// convert to s from ms.
					break;
				}
				case IDC_SET_EXPERIMENTS_PER_STACK_BUTTON: 
				{
					std::string tempStr;
					GetWindowText(eExperimentsPerStackEditHandle.hwnd, (LPSTR)tempStr.c_str(), 20);
					SendMessage(eExperimentsPerStackDispHandle.hwnd, WM_SETTEXT, 0, (LPARAM)tempStr.c_str());
					tempStr = std::string(tempStr.c_str());
					bool err = false;
					try 
					{
						eExperimentsPerStack = std::stoi(tempStr);
						ePicturesPerStack = eExperimentsPerStack * ePicturesPerExperiment;
						eTotalNumberOfPicturesInSeries = eCurrentAccumulationStackNumber * ePicturesPerStack;
						if (tempStr == "" || err == true)
						{
							appendText("ERROR: Empty Number Entered for Accumulations Per SubSeries.\r\n", IDC_ERROR_EDIT);
							break;
						}

					}
					catch (std::invalid_argument &exception)
					{
						MessageBox(0, "ERROR: number entered was not valid.", 0, 0);
						ePicturesPerStack = 0;
						break;
					}
					//
					break;
				}
				case IDC_SET_DETECTION_THRESHOLD_BUTTON: 
				{	
					std::string tempStr;
					GetWindowText(eAtomThresholdEditHandle.hwnd, (LPSTR)tempStr.c_str(), 20);
					SendMessage(eAtomThresholdDispHandle.hwnd, WM_SETTEXT, 0, (LPARAM)tempStr.c_str());
					tempStr = std::string(tempStr.c_str());
					bool err = false;
					for (unsigned int strInc = 0; strInc < tempStr.size(); strInc++) 
					{
						if (!isdigit(tempStr[strInc])) 
						{
							err = true;
						}
					}
					if (tempStr == "" || err == true) 
					{
						break;
					}
					int tempInt = std::stoi(tempStr);
					if (tempInt < 0 || tempInt > 10000) 
					{
						appendText("ERROR: attempted to set the detection threshold out of range (0,10000)\r\n", IDC_ERROR_EDIT);
						break;
					}
					eDetectionThreshold = tempInt;
					break;
				}
				case IDC_SET_ACCUMULATION_STACK_NUMBER: 
				{
					std::string tempStr;
					GetWindowText(eAccumulationStackNumberEditHandle.hwnd, (LPSTR)tempStr.c_str(), 20);
					SendMessage(eAccumulationStackNumberDispHandle.hwnd, WM_SETTEXT, 0, (LPARAM)tempStr.c_str());
					tempStr = std::string(tempStr.c_str());
					bool err = false;
					for (unsigned int strInc = 0; strInc < tempStr.size(); strInc++) 
					{
						if (!isdigit(tempStr[strInc])) 
						{
							err = true;
						}
					}
					if (tempStr == "" || err == true) 
					{
						break;
					}
					int tempInt = std::stoi(tempStr);
					if (tempInt < 0 || tempInt > 1000) 
					{
						appendText("ERROR: Invalid accumulation stack number: not in range (0, 1000).\r\n", IDC_ERROR_EDIT);
						break;
					}
					eCurrentAccumulationStackNumber = tempInt;
					eTotalNumberOfPicturesInSeries = eCurrentAccumulationStackNumber * ePicturesPerStack;
					break;
				}
				case IDC_SET_PICTURES_PER_EXPERIMENT_BUTTON:
				{
					std::string tempStr;
					GetWindowText(ePicturesPerExperimentEditHandle.hwnd, (LPSTR)tempStr.c_str(), 20);
					SendMessage(ePicturesPerExperimentDispHandle.hwnd, WM_SETTEXT, 0, (LPARAM)tempStr.c_str());
					tempStr = std::string(tempStr.c_str());
					bool err = false;
					try
					{
						int tempInt = std::stoi(tempStr);
						if (tempInt < 1 || tempInt > 4)
						{
							if (tempInt > 4)
							{
								int answer = MessageBox(0, "The program can only support imaging 4 images per experiment on the screen at one time. The grouping"
									" will work in terms of plotting and data analysis, but the images displayed on the screen will just \"wrap\". (e.g. if 5"
									" pictures per experiment, they will be displayed picture locations {1,2,3,4,1}, {1,2,3,4,1}, etc, where {} denotes 1 experiment).", 0, MB_OKCANCEL);
								if (answer == IDCANCEL)
								{
									break;
								}
							}
							else
							{
								appendText("ERROR: Invalid pictures per experiment number.\r\n", IDC_ERROR_EDIT);
								break;
							}
						}
						ePicturesPerExperiment = tempInt;
						ePicturesPerStack = eExperimentsPerStack * ePicturesPerExperiment;
						eTotalNumberOfPicturesInSeries = eCurrentAccumulationStackNumber * ePicturesPerStack;
					}
					catch (std::invalid_argument &exception)
					{
						MessageBox(0, "ERROR: Number entered could not be converted to integer.", 0, 0);
					}
					if (tempStr == "" || err == true) 
					{
						break;
					}


					break;
				}
				case IDC_SET_ACCUMULATION_NUMBER_BUTTON:
				{
					char tempChars[256];
					std::string tempStr;
					GetWindowText(eSetAccumulationNumberEdit.hwnd, (LPSTR)tempChars, 20);
					SendMessage(eSetAccumulationNumberDisp.hwnd, WM_SETTEXT, 0, (LPARAM)tempChars);
					tempStr = std::string(tempChars);
					bool err = false;
					for (unsigned int strInc = 0; strInc < tempStr.size(); strInc++)
					{
						if (!isdigit(tempStr[strInc]))
						{
							err = true;
						}
					}
					if (tempStr == "" || err == true)
					{
						break;
					}
					int tempInt = std::stoi(tempStr);
					if (tempInt < 0)
					{
						appendText("ERROR: Accumulation Number Must be Positive.\r\n", IDC_ERROR_EDIT);
						break;
					}
					eCurrentAccumulationModeTotalAccumulationNumber = tempInt;
					break;
				}
				case IDC_SET_ACCUMULATION_TIME_BUTTON:
				{
					char tempChars[256];
					std::string tempStr;
					GetWindowText(eAccumulationTimeEdit.hwnd, (LPSTR)tempChars, 20);
					
					tempStr = std::string(tempChars);
					bool err = false;
					for (unsigned int strInc = 0; strInc < tempStr.size(); strInc++)
					{
						if (!isdigit(tempStr[strInc]))
						{
							err = true;
						}
					}
					if (tempStr == "" || err == true)
					{
						break;
					}
					int tempInt = std::stoi(tempStr);
					if (tempInt < 0)
					{
						appendText("ERROR: Accumulation Time Must be Positive.\r\n", IDC_ERROR_EDIT);
						break;
					}
					eAccumulationTime = tempInt;
					// try to set this
					if (myAndor::setAccumulationCycleTime() < 0)
					{
						appendText("ERROR: Failed to set Acuumulation cycle time.", IDC_ERROR_EDIT);
						SendMessage(eAccumulationTimeDisp.hwnd, WM_SETTEXT, 0, (LPARAM)"");
						break;
					}
					if (myAndor::checkAcquisitionTimings() < 0)
					{
						appendText("ERROR: Failed to check acquisiton timings.", IDC_ERROR_EDIT);
						break;
					}
					// now output things.
					if (eExposureTimes.size() > 0)
					{
						SendMessage(eExposureDispHandle.hwnd, WM_SETTEXT, 0, (LPARAM)"");
						appendText(std::to_string(eExposureTimes[0]), IDC_EXPOSURE_DISP);
						for (int exposureInc = 1; exposureInc < eExposureTimes.size(); exposureInc++)
						{
							appendText(" -> " + std::to_string(eExposureTimes[exposureInc]), IDC_EXPOSURE_DISP);
						}
					}
					SendMessage(eKineticCycleTimeDispHandle.hwnd, WM_SETTEXT, 0, (LPARAM)std::to_string(eKineticCycleTime).c_str());
					SendMessage(eAccumulationTimeDisp.hwnd, WM_SETTEXT, 0, (LPARAM)std::to_string(eAccumulationTime).c_str());
					break;
				}
				case IDC_PLOTTING_FREQUENCY_BUTTON:
				{
					char tempChars[256];
					std::string tempStr;
					GetWindowText(ePlottingFrequencyEdit.hwnd, (LPSTR)tempChars, 20);
					tempStr = std::string(tempChars);
					bool err = false;
					for (unsigned int strInc = 0; strInc < tempStr.size(); strInc++)
					{
						if (!isdigit(tempStr[strInc]))
						{
							err = true;
						}
					}
					if (tempStr == "" || err == true)
					{
						break;
					}
					int tempInt = std::stoi(tempStr);
					if (tempInt <= 0)
					{
						appendText("ERROR: Plotting Frequency must be Greater than Zero.\r\n", IDC_ERROR_EDIT);
						break;
					}
					SendMessage(ePlottingFrequencyDisp.hwnd, WM_SETTEXT, 0, (LPARAM)tempChars);
					ePlottingFrequency = tempInt;
					break;
				}
				case IDC_INCREMENT_FILE_OPTION_BUTTON:
				{
					BOOL checked = IsDlgButtonChecked(hWnd, IDC_INCREMENT_FILE_OPTION_BUTTON);
					if (checked)
					{
						CheckDlgButton(hWnd, IDC_INCREMENT_FILE_OPTION_BUTTON, BST_UNCHECKED);
						eIncSaveFileNameOption = false;
					}
					else
					{
						CheckDlgButton(hWnd, IDC_INCREMENT_FILE_OPTION_BUTTON, BST_CHECKED);
						eIncSaveFileNameOption = true;
					}
					break;
				}
				case IDC_SET_EM_GAIN_MODE:
				{
					TCHAR emGainText[256];
					SendMessage(eEMGainEdit.hwnd, WM_GETTEXT, 256, (LPARAM)emGainText);
					int emGain;
					try
					{
						emGain = std::stoi(emGainText);
					}
					catch (std::invalid_argument &exception)
					{
						MessageBox(0, "ERROR: Couldn't convert EM Gain text to integer.", 0, 0);
						break;
					}
					// < 0 corresponds to NOT USING EM GAIN (using conventional gain).
					if (emGain < 0)
					{
						eEMGainMode = false;
						eEMGainLevel = 0;
						SendMessage(eEMGainDisplay.hwnd, WM_SETTEXT, 0, (LPARAM)"OFF");
					}
					else
					{
						eEMGainMode = true;
						eEMGainLevel = emGain;
						SendMessage(eEMGainDisplay.hwnd, WM_SETTEXT, 0, (LPARAM)("X" + std::to_string(eEMGainLevel)).c_str());
					}
					// change this immediately.
					if (myAndor::setGainMode() != 0)
					{
						return -1;
					}
					
					break;
				}
			}
		}
		case WM_PAINT:
		{
			// need to handle this eventually...
			eRedrawFlag = true;
			int experimentPictureNumber;
			if (eRealTimePictures)
			{
				experimentPictureNumber = 0;
			}
			else
			{
				experimentPictureNumber = (((eCurrentAccumulationNumber - 1) % ePicturesPerStack) % ePicturesPerExperiment);
			}
			if ((experimentPictureNumber == ePicturesPerExperiment - 1 || eRealTimePictures) && eDataExists)
			{
				//myAndor::drawDataWindow();
			}
			break;
		}
		case WM_TIMER: 
		{
			processTimer(wParam);
			break;
		}
		case WM_CLOSE:
		case WM_DESTROY: 
		{
			int temperature;
			std::string errMsg;
			if (!ANDOR_SAFEMODE)
			{
				errMsg = myAndor::andorErrorChecker(GetTemperature(&temperature));
				if (errMsg != "DRV_TEMP_OFF" && errMsg != "DRV_TEMP_STABILIZED" && errMsg != "DRV_TEMP_NOT_REACHED" && errMsg != "DRV_TEMP_DRIFT"
					&& errMsg == "DRV_TEMP_NOT_STABILIZED")
				{
					appendText("Error Getting Temperature before exiting: " + errMsg + "\r\n", IDC_STATUS_EDIT);
				}
			}
			else 
			{
				temperature = 25;
			}
			//
			if (temperature < 0) 
			{
				SendMessage(eStatusEditHandle.hwnd, WM_SETTEXT, 0, (LPARAM)("Please only quit when the temperature of the camera is "
							"above 0 C, or else rapid heating that occurs when the cooler is turned off at low "
							"temperatures might cause rapid mechanical expansions that could damage the camera!\r\n"));
			}
			else 
			{
				// The program is ending. Close the thread and exit the main UI processing loop.
				eThreadExitIndicator = false;
				std::string errMsg;
				if (!ANDOR_SAFEMODE)
				{
					errMsg = myAndor::andorErrorChecker(CancelWait());
				}
				else
				{
					errMsg = "DRV_SUCCESS";
				}
				if (errMsg != "DRV_SUCCESS") 
				{
					appendText("ERROR: cancelWait Failed: " + errMsg + "\r\n", IDC_ERROR_EDIT);
					break;
				}
				appendText("Waiting for plotting to finish...\r\n", IDC_STATUS_EDIT);
				WaitForSingleObject(eCameraThreadHandle, INFINITE);
				PostQuitMessage(0);
			}
			break;
		}
		default:
		{
			if (msg == ePlottingIsSlowMessage)
			{

				appendText("Warning: Plotting is running slow. Reduce plotting frequency to see plots in real-time.\r\n", IDC_ERROR_EDIT);

				break;
			}
			else if (msg == ePlottingCaughtUpMessage)
			{
				appendText("Plotting Caught Up to the camera acqusition.\r\n", IDC_ERROR_EDIT);
				break;
			}
			else if (msg == eAccMessageID) 
			{
				eCount3++;
				eCurrentAccumulationNumber = (int)lParam;
				SendMessage(eCurrentAccumulationNumDispHandle.hwnd, WM_SETTEXT, 0, (LPARAM)("Accumulation " + std::to_string((int)eCurrentAccumulationNumber) + "/"
										  + std::to_string((int)eTotalNumberOfPicturesInSeries)).c_str());
				int err = myAndor::acquireImageData();
				if (err == FALSE) 
				{
					appendText("ERROR: Acquisition Error: " + std::to_string(err), IDC_ERROR_EDIT);
					return 0;
				}
				UpdateWindow(eStatusEditHandle.hwnd);
				if (eCurrentlyRunningCameraMode == "Kinetic Series Mode")
				{
					int posSubSeries = (eCurrentAccumulationNumber % ePicturesPerStack) * 100.0 / ePicturesPerStack;
					int posSeries = eCurrentAccumulationNumber / (double)eTotalNumberOfPicturesInSeries * 100;
					SendMessage(eSeriesProgressBar.hwnd, PBM_SETPOS, (WPARAM)posSeries, 0);
					SendMessage(eSubSeriesProgressBar.hwnd, PBM_SETPOS, (WPARAM)posSubSeries, 0);
				}
				if (eCurrentlyRunningCameraMode == "Kinetic Series Mode" && eCurrentAccumulationNumber == eTotalNumberOfPicturesInSeries)
				{
					ePlotThreadExitIndicator = false;
					eSystemIsRunning = false;
					// Wait until plotting thread is complete.
					WaitForSingleObject(ePlottingThreadHandle, INFINITE);
					int fitsStatus;
					eFitsOkay = false;
					// give the thread some time so that don't try to open a closed file... not very good solution.
					// TODO: Improve this
					Sleep(500);
					fits_close_file(eFitsFile, &fitsStatus);
					// print any error messages
					if (fitsStatus != 0)
					{
						std::vector<char> errMsg;
						errMsg.resize(80);
						//std::string errMsg;
						fits_get_errstatus(fitsStatus, errMsg.data());
						if (eFitsFile == NULL)
						{
							appendText("CFITS error: Could not create data file on andor. Is the andor connected???\r\n", IDC_STATUS_EDIT);
						}
						else
						{
							appendText("CFITS error: " + std::string(errMsg.data()) + "\r\n", IDC_STATUS_EDIT);
						}
					}

				}
			}
			else if (msg == eFinMessageID) 
			{
				// Acquisition finished message
				// set the current accumulation number to be that of the last picture.
				eCurrentAccumulationNumber = eTotalNumberOfPicturesInSeries;
				// update the accumulation edit with the last accumulation.
				SendMessage(eCurrentAccumulationNumDispHandle.hwnd, WM_SETTEXT, 0, (LPARAM)("Accumulation " + std::to_string((int)eTotalNumberOfPicturesInSeries) + "/"
							+ std::to_string((int)eTotalNumberOfPicturesInSeries)).c_str());
				int err = myAndor::acquireImageData();
				if (err == FALSE) 
				{
					appendText("ERROR: Acquisition Error: " + std::to_string(err) + "\r\n", IDC_ERROR_EDIT);
					return 0;
				}
				ePlotThreadExitIndicator = false;
				// Wait until plotting thread is complete.
				WaitForSingleObject(ePlottingThreadHandle, INFINITE);
				eSystemIsRunning = false;
				//appendText((std::to_string(eCount1) + ", " + std::to_string(eCount2) + ", " + std::to_string(eCount3) + "\r\n"), IDC_STATUS_EDIT);
				eFitsOkay = false;
				// give the thread some time so that don't try to open a closed file... not very good solution.
				// TODO: Improve this
				Sleep(500);
				int fitsStatus = 0;
				fits_close_file(eFitsFile, &fitsStatus);
				// print any error messages
				if (fitsStatus != 0)
				{
					std::vector<char> errMsg;
					errMsg.resize(80);
					//std::string errMsg;
					fits_get_errstatus(fitsStatus, errMsg.data());
					if (eFitsFile == NULL)
					{
						appendText("CFITS error: Could not create data file on andor. Is the andor connected???\r\n", IDC_STATUS_EDIT);
					}
					else
					{
						appendText("CFITS error: " + std::string(errMsg.data()) + "\r\n", IDC_STATUS_EDIT);
					}
				}
				if (eCurrentlyRunningCameraMode == "Kinetic Series Mode")
				{
					int posSubSeries = (eCurrentAccumulationNumber % ePicturesPerStack) * 100.0 / ePicturesPerStack;
					int posSeries = eCurrentAccumulationNumber / (double)eTotalNumberOfPicturesInSeries * 100;
					SendMessage(eSeriesProgressBar.hwnd, PBM_SETPOS, (WPARAM)posSeries, 0);
					SendMessage(eSubSeriesProgressBar.hwnd, PBM_SETPOS, (WPARAM)posSubSeries, 0);
				}
				appendText("Finished Entire Experiment Sequence.", IDC_STATUS_EDIT);
				break;
			}
			else if (msg == eErrMessageID) 
			{
				appendText("ERROR: Get Acq Progress Error\r\n", IDC_ERROR_EDIT);
				UpdateWindow(eStatusEditHandle.hwnd);
				int fitsStatus = 0;
				eFitsOkay = false;
				// give the thread some time so that don't try to open a closed file... not very good solution.
				// TODO: Improve this
				Sleep(500);
				fits_close_file(eFitsFile, &fitsStatus);
				// print any error messages
				if (fitsStatus != 0)
				{
					std::vector<char> errMsg;
					errMsg.resize(80);
					//std::string errMsg;
					fits_get_errstatus(fitsStatus, errMsg.data());
					if (eFitsFile == NULL)
					{
						appendText("CFITS error: Could not create data file on andor. Is the andor connected???\r\n", IDC_STATUS_EDIT);
					}
					else
					{
						appendText("CFITS error: " + std::string(errMsg.data()) + "\r\n", IDC_STATUS_EDIT);
					}
				}
				break;
			}
		}
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}
