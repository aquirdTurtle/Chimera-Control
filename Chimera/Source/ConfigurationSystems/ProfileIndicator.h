// created by Mark O. Brown
#pragma once

#include "Control.h"
#include "PrimaryWindows/IChimeraWindowWidget.h"
#include <qlabel.h>

class ProfileIndicator {
	public:
		void initialize(POINT position, QWidget* parent );
		void update(std::string text);
		void rearrange(int width, int height, fontMap fonts);
	private:
		QLabel* header;
		QLabel* indicator;
};

