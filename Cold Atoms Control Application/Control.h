#pragma once

#include "Windows.h"

// this is a ubiquitus structure that I used for all of my controls (well, i'd like to, but only implemented in this cde recently)
// It contains information about controls that I'd like to keep track of and group together. For this code, it's minimal right now.
// It's a more important addition in other pieces of code and will grow more important later in this code.
// TODO: incorporate color of a control into this structure.
struct Control
{
	HWND hwnd;
	RECT normalPos;
};