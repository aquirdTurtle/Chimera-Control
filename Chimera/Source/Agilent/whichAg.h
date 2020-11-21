#pragma once

struct AgilentEnum {
	enum class name {
		TopBottom, Axial, Flashing, Microwave
	};
	static const std::array<name, 4> allAgs;
	static std::string toStr (name m_) {
		switch (m_) {
		case name::TopBottom:
			return "TopBottom";
		case name::Axial:
			return "Axial";
		case name::Flashing:
			return "Flashing";
		case name::Microwave:
			return "Microwave";
		}
		return "";
	}
	static name fromStr (std::string txt) {
		for (auto opt : allAgs) {
			if (toStr (opt) == txt) {
				return opt;
			}
		}
		thrower ("Failed to convert string to Which Agilent option!");
		return name::TopBottom;
	}
};
