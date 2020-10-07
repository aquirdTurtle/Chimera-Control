// created by Mark O. Brown
#pragma once

#include "Control.h"
#include "PrimaryWindows/IChimeraQtWindow.h"
#include <qlabel.h>

class ProfileIndicator {
	public:
		void initialize(QPoint position, QWidget* parent );
		void update(std::string text);
	private:
		QLabel* header;
		QLabel* indicator;
};

