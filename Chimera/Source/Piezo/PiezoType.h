#pragma once

enum class PiezoType
{
	A,
	B,
	NONE
};

struct piezoSetupInfo{
	PiezoType type;
	std::string addr;
	std::string name;
	bool expActive = true;
};

