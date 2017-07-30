// ScriptWritingInstructionsDialog.cpp : implementation file
//

// I have no idea what's going on in this file. It was created when I made one of the dialogs.

#include "stdafx.h"
#include "ScriptWritingInstructionsDialog.h"
#include "afxdialogex.h"

// ScriptWritingInstructionsDialog dialog

IMPLEMENT_DYNAMIC(ScriptWritingInstructionsDialog, CDialogEx)

ScriptWritingInstructionsDialog::ScriptWritingInstructionsDialog(CWnd* pParent /*=NULL*/)
	: CDialogEx(ScriptWritingInstructionsDialog::IDD, pParent)
{

}

ScriptWritingInstructionsDialog::~ScriptWritingInstructionsDialog()
{
}

void ScriptWritingInstructionsDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(ScriptWritingInstructionsDialog, CDialogEx)
	ON_EN_CHANGE(IDC_HELP_EDIT, &ScriptWritingInstructionsDialog::OnEnChangeEdit2)
END_MESSAGE_MAP()


// ScriptWritingInstructionsDialog message handlers


void ScriptWritingInstructionsDialog::OnEnChangeEdit2()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}
