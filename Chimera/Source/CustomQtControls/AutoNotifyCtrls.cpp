#include "stdafx.h"
#include "AutoNotifyCtrls.h"
///
CQLineEdit::CQLineEdit (IChimeraWindowWidget* parent) : QLineEdit (parent){
	if (parent != Q_NULLPTR){
		parent->connect (this, &QLineEdit::textChanged, parent, &IChimeraWindowWidget::configUpdated);
	}
}

CQLineEdit::CQLineEdit (const QString& txt, IChimeraWindowWidget* parent) : QLineEdit(txt, parent){
	if (parent != Q_NULLPTR){
		parent->connect (this, &QLineEdit::textChanged, parent, &IChimeraWindowWidget::configUpdated);
	}
}

///
CQTextEdit::CQTextEdit (IChimeraWindowWidget* parent) : QTextEdit (parent){
	if (parent != Q_NULLPTR){
		parent->connect (this, &QTextEdit::textChanged, parent, &IChimeraWindowWidget::configUpdated);
	}
}

CQTextEdit::CQTextEdit (const QString& txt, IChimeraWindowWidget* parent) : QTextEdit (txt, parent){
	if (parent != Q_NULLPTR){
		parent->connect (this, &QTextEdit::textChanged, parent, &IChimeraWindowWidget::configUpdated);
	}
}
///
CQCheckBox::CQCheckBox (IChimeraWindowWidget* parent) : QCheckBox (parent){
	if (parent != Q_NULLPTR){
		parent->connect (this, &QCheckBox::stateChanged, parent, &IChimeraWindowWidget::configUpdated);
	}
}

CQCheckBox::CQCheckBox (const QString& txt, IChimeraWindowWidget* parent) : QCheckBox (txt, parent){
	if (parent != Q_NULLPTR){
		parent->connect (this, &QCheckBox::stateChanged, parent, &IChimeraWindowWidget::configUpdated);
	}
}

///
CQPushButton::CQPushButton (IChimeraWindowWidget* parent) : QPushButton (parent){
	if (parent != Q_NULLPTR)
	{
		parent->connect (this, &QPushButton::released, parent, &IChimeraWindowWidget::configUpdated);
	}
}

CQPushButton::CQPushButton (const QString& txt, IChimeraWindowWidget* parent) : QPushButton (txt, parent){
	if (parent != Q_NULLPTR){
		parent->connect (this, &QPushButton::released, parent, &IChimeraWindowWidget::configUpdated);
	}
}

///
CQComboBox::CQComboBox (IChimeraWindowWidget* parent) : QComboBox(parent){
	if (parent != Q_NULLPTR){
		parent->connect (this, qOverload<int> (&QComboBox::currentIndexChanged), parent, &IChimeraWindowWidget::configUpdated);
	}
}


CQRadioButton::CQRadioButton (IChimeraWindowWidget* parent) : QRadioButton (parent) {
	if (parent != Q_NULLPTR) {
		parent->connect (this, &QRadioButton::released, parent, &IChimeraWindowWidget::configUpdated);
	}
}

CQRadioButton::CQRadioButton (const QString& txt, IChimeraWindowWidget* parent) : QRadioButton (txt, parent) {
	if (parent != Q_NULLPTR) {
		parent->connect (this, &QRadioButton::released, parent, &IChimeraWindowWidget::configUpdated);
	}
}
