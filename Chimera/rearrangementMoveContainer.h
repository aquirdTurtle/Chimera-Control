#pragma once

#include "rearrangementMove.h"
#include "windows.h"
#include <vector>

// this is more  or less a wrapper for a const 3-Dimensional vector. E.g. I could also implement this as
// std::vector<std::vector<std::array<rearrangementMove>, 4>> where the first two dimensions select move / row, and
// the 4 elements in that select the direction.
class rearrangementMoveContainer
{
	public:
		rearrangementMoveContainer( UINT rowsInGrid, UINT colsInGrid );
		rearrangementMove operator()( UINT row, UINT col, directions direction ) const;
		rearrangementMove & operator()( UINT row, UINT col, directions direction );
	private:
		std::vector<rearrangementMove> moves;
		const UINT rows, cols;
};

