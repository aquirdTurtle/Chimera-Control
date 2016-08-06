
// MFCApplication3Dlg.h : header file
//

#pragma once


// CMFCApplication3Dlg dialog
class CMFCApplication3Dlg : public CDialogEx
{
	// Construction
	public:
		CMFCApplication3Dlg(CWnd* pParent = NULL);	// standard constructor
		CButton test;
		CToolTipCtrl tool;
		CMFCButton fancy;
		afx_msg void OnBnClickedOk();
		afx_msg void OnBnClickedButton1();
		// Dialog Data
		#ifdef AFX_DESIGN_TIME
				enum { IDD = IDD_MFCAPPLICATION3_DIALOG };
		#endif
	protected:
		virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


	// Implementation
	protected:
		HICON m_hIcon;

		// Generated message map functions
		virtual BOOL OnInitDialog();
		afx_msg void OnPaint();
		afx_msg HCURSOR OnQueryDragIcon();
		DECLARE_MESSAGE_MAP()
};
