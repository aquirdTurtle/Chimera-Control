#pragma once

#include <qlineedit.h>
#include <qpushbutton.h>
#include <qtextedit.h>
#include <qcheckbox.h>
#include <QComboBox.h>
#include <qradiobutton.h>
#include <PrimaryWindows/IChimeraQtWindow.h>

class CQLineEdit : public QLineEdit {
	public:
		CQLineEdit (IChimeraQtWindow* parent);
		CQLineEdit (const QString&, IChimeraQtWindow* parent);
};


class CQTextEdit : public QTextEdit
{
	public:
		CQTextEdit (IChimeraQtWindow* parent);
		CQTextEdit (const QString& text, IChimeraQtWindow* parent);
};

class CQCheckBox : public QCheckBox
{
	public:
		CQCheckBox (IChimeraQtWindow* parent);
		CQCheckBox (const QString& text, IChimeraQtWindow* parent);
};

class CQPushButton : public QPushButton
{
	public:
		CQPushButton (IChimeraQtWindow* parent);
		CQPushButton (const QString& text, IChimeraQtWindow* parent);
};


class CQComboBox : public QComboBox
{
	public:
		CQComboBox (IChimeraQtWindow* parent);
};

class CQRadioButton : public QRadioButton
{
	public:
		CQRadioButton (IChimeraQtWindow* parent);
		CQRadioButton (const QString& text, IChimeraQtWindow* parent);
};