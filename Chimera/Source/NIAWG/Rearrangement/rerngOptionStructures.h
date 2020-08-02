// created by Mark O. Brown
#pragma once

#include "ParameterSystem/Expression.h"
#include <array>


struct rerngMode
{
	enum class mode
	{
		StandardFlashing,
		Ultrafast,
		Antoine,
		Lazy
	};
	static const std::array<mode, 4> allModes;// = { mode::StandardFlashing, mode::Ultrafast, mode::Antoine, mode::Lazy };
	static std::string toStr ( mode m );
	static mode fromStr ( std::string txt );
};

enum class smartRerngOption
{
	// none places target in center.
	none, 
	// find the optimal convolution match and use that to do the rearrangement algorithm
	convolution,
	// do the full optimization algorithm for all possibilities and use the one with the least moves
	full
};


enum class nonFlashingOption
{
	// no non-flashing moves
	none,
	// non-flashing moves only if no atoms in new row.
	cautious,
	full
};


// the reason to do partial is that I may run into power issues where I don't have enough power in the moving tweezers
// to spread over more than a certain number of atoms.
const unsigned PARTIAL_PARALLEL_LIMIT = 1;
enum class parallelMoveOption
{
	// no parallel moves
	none, 
	// parallel moves, but only two allowed to be combined at once.
	partial, 
	// as many as possible.
	full
};

/*
struct rerngGuiOptions
{
	bool active = false;
	double moveSpeed = 60e-6;
	double flashingRate = 1e6;
	Expression moveBiasExpr;
	double moveBias=1;
	// 
	double deadTime=0;

	// the static / moving time ratio, i.e. if 2 this might imply out of a total 60us move, the static part takes up
	// 40us and the moving part takes up 20us.
	double staticMovingRatio=1;
	bool outputInfo = false;
	bool outputIndv = false;
	bool preprogram = false;
	bool useCalibration = false;

	double finalMoveTime = 1e-3;
	// in s
	double fastMoveTime = 2e-6;
	// hard set right now.
	nonFlashingOption noFlashOption = nonFlashingOption::cautious;
	parallelMoveOption parallel = parallelMoveOption::none;
	smartRerngOption smartOption = smartRerngOption::full;

	rerngMode::mode rMode;
};
*/

struct rerngGuiOptions
{
	bool active = false;
	Expression moveSpeed = "60e-6";
	Expression flashingRate = "1e6";
	Expression moveBias = "1";
	Expression deadTime = "0";
	// the static / moving time ratio, i.e. if 2 this might imply out of a total 60us move, the static part takes up
	// 40us and the moving part takes up 20us.
	Expression staticMovingRatio = "1";
	Expression finalMoveTime = "1e-3";
	Expression fastMoveTime = "2e-6";
	//
	bool outputInfo = false;
	bool outputIndv = false;
	bool preprogram = false;
	bool useCalibration = false;
	rerngMode::mode rMode;
	// hard set right now.
	nonFlashingOption noFlashOption = nonFlashingOption::cautious;
	parallelMoveOption parallel = parallelMoveOption::none;
	smartRerngOption smartOption = smartRerngOption::full;
};

