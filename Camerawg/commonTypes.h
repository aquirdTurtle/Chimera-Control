#pragma once

#include <unordered_map>
#include <afxwin.h>

// these get passed around a lot, so I typedef'd them for convenience.
typedef std::unordered_map<std::string, CBrush*> brushMap;
typedef std::unordered_map<std::string, COLORREF> rgbMap;
typedef std::unordered_map<HWND, std::string> toolTipTextMap;
typedef std::unordered_map<std::string, CFont*> fontMap;
enum fontTypes { Normal, Heading, Code, Small, Large };
