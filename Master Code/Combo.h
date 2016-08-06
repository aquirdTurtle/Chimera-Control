#pragma once
#include "afxwin.h"
class Combo : public CComboBox
{

	public:
		Combo();
		~Combo();

		HBRUSH OnCtlColor(CDC * pDC, CWnd * pWnd, UINT nCtlColor);

};

