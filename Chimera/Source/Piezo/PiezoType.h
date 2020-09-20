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
	// the edits will update when the display updates, making the edits always reflect the most recent value. 
	const bool editReflective;
};

