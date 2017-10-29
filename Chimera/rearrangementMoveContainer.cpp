#include "rearrangementMoveContainer.h"
#include "miscellaneousCommonFunctions.h"

// the array gets sized only once in the constructor.
rearrangementMoveContainer::rearrangementMoveContainer( UINT rowsInGrid, UINT colsInGrid ) : 
	rows( rowsInGrid),
	cols(colsInGrid),
	moves(rows*cols*4)
{}


rearrangementMove rearrangementMoveContainer::operator()( UINT row, UINT col, directions direction ) const
{
	if ( row > rows )
	{
		thrower( "ERROR: row index out of range during rearrangementMoveContainer access!" );
	}
	if ( col > cols)
	{
		thrower( "ERROR: col index out of range during rearrangementMoveContainer access!" );
	}
	UINT rowOffset(row * cols * 4);
	UINT colOffset(col * 4);
	UINT index = rowOffset + colOffset + direction;
	return moves[index];
}


rearrangementMove & rearrangementMoveContainer::operator()( UINT row, UINT col, directions direction )
{
	if ( row > rows )
	{
		thrower( "ERROR: row index out of range during rearrangementMoveContainer access!" );
	}
	if ( col > cols )
	{
		thrower( "ERROR: col index out of range during rearrangementMoveContainer access!" );
	}
	UINT rowOffset( row * cols * 4 );
	UINT colOffset( col * 4 );
	UINT index = rowOffset + colOffset + direction;
	return moves[index];
}

