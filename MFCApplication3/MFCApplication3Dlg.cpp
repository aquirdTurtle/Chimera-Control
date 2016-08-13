
// MFCApplication3Dlg.cpp : implementation file
//

#include "stdafx.h"
#include "MFCApplication3.h"
#include "MFCApplication3Dlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMFCApplication3Dlg dialog



CMFCApplication3Dlg::CMFCApplication3Dlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_MFCAPPLICATION3_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMFCApplication3Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CMFCApplication3Dlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CMFCApplication3Dlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON1, &CMFCApplication3Dlg::OnBnClickedButton1)
END_MESSAGE_MAP()


// CMFCApplication3Dlg message handlers

BOOL CMFCApplication3Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	
	//test.Create()
	if (test.Create(L"test?", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, CRect(10, 10, 100, 30), this, 10) == 0)
	{
		MessageBox(0);
	}
	if (tool.Create(this) == 0)
	{
		MessageBox(0);
	} // 'this', usually a CDialog window
	fancy.Create(L"fancy?", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, CRect(10, 50, 100, 70), this, 10);
	fancy.SetTooltip(L"tip?");

	//tool.SetMaxTipWidth(300); // if you need multiline messages
	//tool.SetTipBkColor(0x000000);
	//tool.SetTipTextColor(0xe0e0d0);

	// Attach a CListBox control (we can attach many controls)
	//tool.AddTool(&test, "stuff")
	if (tool.AddTool(this, L"Hey, i am a tooltip message!") == 0)
	{
		MessageBox(0, 0, 0);
	}
	tool.Activate(TRUE);
	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CMFCApplication3Dlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CMFCApplication3Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CMFCApplication3Dlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	CDialogEx::OnOK();
}


void CMFCApplication3Dlg::OnBnClickedButton1()
{
	// TODO: Add your control notification handler code here
}
