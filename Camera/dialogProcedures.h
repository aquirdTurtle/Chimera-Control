#pragma once

#include "Windows.h"

namespace dialogProcedures
{
	LRESULT CALLBACK richEditMessageBoxProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK richEditOkCancelMessageBoxProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK plottingDialogProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK picturePalletesDialogProcedure(HWND thisDialogHandle, UINT message, WPARAM wParam, LPARAM lParam);
}