#pragma once

#include <string>

class SoundControl
{
	public:
		bool initialize();
		bool errorSound();
		bool finishedSound();
		bool alertSound();
	private:
		std::string errorSound;
		std::string finishedSound;
		std::string alertSound;
};
