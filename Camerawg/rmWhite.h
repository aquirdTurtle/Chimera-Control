#pragma once
#include "stdafx.h"

#include <fstream>

/**
 * The purpose of this function is to look for any whitespace (tabs, spaces, or newlines) just past where the file cursor currently is, and remove said white-
 * space. This is to make it easier to read in data without worrying extensively about how the user formatted their instructions.
 * @param file This is the file whose white-space is about to be removed.
 */
void rmWhite(std::fstream &file);

