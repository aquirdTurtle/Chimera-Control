#pragma once

//#include "Expression.h"

#include <string>
#include "Windows.h"
#include <unordered_map>
//#include <afxwin.h>
#include <chrono>

class Expression;
// these get passed around a lot, so I typedef'd them for convenience.
typedef std::unordered_map<std::string, CBrush*> brushMap;
typedef std::unordered_map<std::string, COLORREF> rgbMap;
typedef std::vector<CToolTipCtrl*> cToolTips;
typedef std::unordered_map<HWND, std::string> toolTipTextMap;
typedef std::unordered_map<std::string, CFont*> fontMap;

enum fontTypes { NormalFont, HeadingFont, CodeFont, SmallFont, LargeFont };

// first part of pair is the variable component of the time, second part is the "raw" or constant part of the time.
// this structure will be evaluated to determine the real time (given a variation # and variable values) that an 
// operation will take place at.
typedef std::pair<std::vector<Expression>, double> timeType;
typedef std::chrono::high_resolution_clock chronoClock;
typedef std::vector<std::chrono::time_point<chronoClock>> chronoTimes;

// the first value of the pair in the map is the array of values that the variable name takes on.
// the second value here stores whether it varies or not. This is mainly used to determine if the value is displayed on the status or not.
typedef std::unordered_map<std::string, std::pair<std::vector<double>, bool>> key;


