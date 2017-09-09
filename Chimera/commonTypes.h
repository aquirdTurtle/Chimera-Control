#pragma once

#include <unordered_map>
#include <afxwin.h>
#include <chrono>

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
typedef std::pair<std::vector<std::string>, double> timeType;
typedef std::vector<std::chrono::time_point<std::chrono::high_resolution_clock>> chronoTimes;


