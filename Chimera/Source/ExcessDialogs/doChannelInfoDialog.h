#pragma once

#include <QDialog.h>
#include <qlabel.h>
#include <QLineEdit.h>
#include <QPushButton.h>
#include <array>

class DoSystem;

struct ttlInputStruct{
	DoSystem* ttls;
};

class doChannelInfoDialog : public QDialog{
	Q_OBJECT;
	public:
		doChannelInfoDialog (ttlInputStruct* inputPtr);
	public Q_SLOTS:
		void handleOk ();
		void handleCancel ();
	private:
		ttlInputStruct* input;
		std::array<QLabel*, 16> numberlabels;
		std::array<QLabel*, 4> rowLabels;
		std::array<std::array<QLineEdit*, 16>, 4> edits;
		QPushButton * okBtn, * cancelBtn;
};
