#pragma once

#include "GeneralUtilityFunctions/thrower.h"
#include <string>
#include <array>

struct AiUnits{
	enum class which	{
		pdVolts, pdPower, atomsPower
	};
	static const std::array<which, 3> allOpts;
	static std::string toStr (which m_) {
		switch (m_) {
			case which::pdVolts:
				return "pdVolts";
			case which::pdPower:
				return "pdPower";
			case which::atomsPower:
				return "atomsPower";
		}
	}
	static which fromStr (std::string txt){
		for (auto opt : allOpts){
			if (toStr (opt) == txt){
				return opt;
			}
		}
		thrower ("Failed to convert string to AiUnits option!");
		return which::pdVolts;
	}
};
