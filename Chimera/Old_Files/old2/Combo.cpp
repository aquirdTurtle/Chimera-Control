#include "stdafx.h"
#include "Combo.h"

Combo::Combo()
{
}

Combo::~Combo()
{
}

HBRUSH Combo::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	if (nCtlColor == CTLCOLOR_LISTBOX)
		pDC->SetBkColor(RGB(255, 0, 0));
		
	return CComboBox::OnCtlColor(pDC, pWnd, nCtlColor);
}
