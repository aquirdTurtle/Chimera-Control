#pragma once
#include "Windows.h"
#include <string>
#include "Richedit.h"
int appendColoredText(std::string text, HWND richEditHandle, int richEditID, CHARFORMAT2 format, HWND ownderHandle);