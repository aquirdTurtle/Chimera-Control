#pragma once

#include <qlineedit.h>
#include <qpushbutton.h>
#include <qtextedit.h>
#include <qcheckbox.h>
#include <QComboBox.h>
#include <qradiobutton.h>
#include <PrimaryWindows/IChimeraWindowWidget.h>

class CQLineEdit : public QLineEdit 
{
	public:
		CQLineEdit (IChimeraWindowWidget* parent);
		CQLineEdit (const QString&, IChimeraWindowWidget* parent);
};


class CQTextEdit : public QTextEdit
{
	public:
		CQTextEdit (IChimeraWindowWidget* parent);
		CQTextEdit (const QString& text, IChimeraWindowWidget* parent);
};

class CQCheckBox : public QCheckBox
{
	public:
		CQCheckBox (IChimeraWindowWidget* parent);
		CQCheckBox (const QString& text, IChimeraWindowWidget* parent);
};

class CQPushButton : public QPushButton
{
	public:
		CQPushButton (IChimeraWindowWidget* parent);
		CQPushButton (const QString& text, IChimeraWindowWidget* parent);
};


class CQComboBox : public QComboBox
{
	public:
		CQComboBox (IChimeraWindowWidget* parent);
};

class CQRadioButton : public QRadioButton
{
	public:
		CQRadioButton (IChimeraWindowWidget* parent);
		CQRadioButton (const QString& text, IChimeraWindowWidget* parent);
};