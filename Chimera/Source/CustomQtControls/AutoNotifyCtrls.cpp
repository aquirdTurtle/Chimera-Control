#include "stdafx.h"
#include "AutoNotifyCtrls.h"
#include <QButtonGroup.h>
///
CQLineEdit::CQLineEdit (IChimeraQtWindow* parent) : QLineEdit (parent){
	if (parent != Q_NULLPTR){
		parent->connect (this, &QLineEdit::textChanged, parent, &IChimeraQtWindow::configUpdated);
	}
}

CQLineEdit::CQLineEdit (const QString& txt, IChimeraQtWindow* parent) : QLineEdit(txt, parent){
	if (parent != Q_NULLPTR){
		parent->connect (this, &QLineEdit::textChanged, parent, &IChimeraQtWindow::configUpdated);
	}
}

///
CQTextEdit::CQTextEdit (IChimeraQtWindow* parent) : QTextEdit (parent){
	if (parent != Q_NULLPTR){
		parent->connect (this, &QTextEdit::textChanged, parent, &IChimeraQtWindow::configUpdated);
	}
}

CQTextEdit::CQTextEdit (const QString& txt, IChimeraQtWindow* parent) : QTextEdit (txt, parent){
	if (parent != Q_NULLPTR){
		parent->connect (this, &QTextEdit::textChanged, parent, &IChimeraQtWindow::configUpdated);
	}
}
///
CQCheckBox::CQCheckBox (IChimeraQtWindow* parent) : QCheckBox (parent){
	if (parent != Q_NULLPTR){
		parent->connect (this, &QCheckBox::stateChanged, parent, &IChimeraQtWindow::configUpdated);
	}
}

CQCheckBox::CQCheckBox (const QString& txt, IChimeraQtWindow* parent) : QCheckBox (txt, parent){
	if (parent != Q_NULLPTR){
		parent->connect (this, &QCheckBox::stateChanged, parent, &IChimeraQtWindow::configUpdated);
	}
}

///
CQPushButton::CQPushButton (IChimeraQtWindow* parent) : QPushButton (parent){
	if (parent != Q_NULLPTR)
	{
		parent->connect (this, &QPushButton::released, parent, &IChimeraQtWindow::configUpdated);
	}
}

CQPushButton::CQPushButton (const QString& txt, IChimeraQtWindow* parent) : QPushButton (txt, parent){
	if (parent != Q_NULLPTR){
		parent->connect (this, &QPushButton::released, parent, &IChimeraQtWindow::configUpdated);
	}
}

///
CQComboBox::CQComboBox (IChimeraQtWindow* parent) : QComboBox(parent){
	if (parent != Q_NULLPTR){
		parent->connect (this, qOverload<int> (&QComboBox::currentIndexChanged), parent, &IChimeraQtWindow::configUpdated);
	}
}


CQRadioButton::CQRadioButton (IChimeraQtWindow* parent) : QRadioButton (parent) {
	if (parent != Q_NULLPTR) {
		parent->connect (this, &QRadioButton::released, parent, &IChimeraQtWindow::configUpdated);
	}
}

CQRadioButton::CQRadioButton (const QString& txt, IChimeraQtWindow* parent) : QRadioButton (txt, parent) {
	if (parent != Q_NULLPTR) {
		parent->connect (this, &QRadioButton::released, parent, &IChimeraQtWindow::configUpdated);
	}
}
