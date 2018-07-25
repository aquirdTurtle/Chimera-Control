#pragma once
#include "Windows.h"

struct imageParameters
{
	UINT left=1;
	UINT top = 1; 
	UINT right=100;
	UINT bottom=100;
	
	UINT horizontalBinning=1;
	UINT verticalBinning=1;

	UINT horRawPixelNum ( )
	{
		return this->right - this->left + 1;
	}
	UINT vertRawPixelNum ( )
	{
		return this->top - this->bottom + 1;
	}

	/* in units of # of binned pixels */
	UINT width ( )
	{ 
		if ( this->horizontalBinning == 0 )
		{
			thrower ( "ERROR: In imageParameters.width(), horizontalBinning was zero!" );
		}
		return horRawPixelNum() / this->horizontalBinning;
	}
	/* in units of # of binned pixels */
	UINT height( )
	{
		if ( this->verticalBinning == 0 )
		{
			thrower ( "ERROR: In imageParameters.width(), verticalBinning was zero!" );
		}
		return vertRawPixelNum() / this->verticalBinning;
	}
	/* in units of # of binned pixels */
	UINT size ( )
	{
		return this->width ( ) * this->height ( );
	}

	std::string currValues ( )
	{
		return ( str("Current image values are:") + 
				 " left = " + str ( left ) +
				 " right = " + str ( right ) +
				 " top = " + str ( top ) +
				 " bottom = " + str ( bottom ) +
				 " horizontalBinning = " + str ( horizontalBinning ) +
				 " verticalBinning = " + str ( verticalBinning ) );
	}

	/*
	If not consistent, the function throws.
	cam can be one of "andor", "ace", and "scout".
	*/
	bool checkConsistency ( std::string cam="andor" )
	{
		std::string errstr = "ERROR: inside " + cam + " camera settings, ";
		if ( left > right )
		{
			thrower ( errstr + "left > right." + currValues ( ) );
		}
		if (top < bottom )
		{
			thrower ( errstr  + "top < bottom" + currValues ( ) );
		}
		if ( horizontalBinning == 0 )
		{
			thrower ( errstr + "horizontalBinning == 0" + currValues ( ) );
		}
		if ( verticalBinning == 0 )
		{
			thrower ( errstr + "verticalBinning == 0" + currValues ( ) );
		}
		if ( left <= 0 )
		{
			thrower ( errstr + "left <= 0" + currValues ( ) );
		}
		if ( top <= 0 )
		{
			thrower ( errstr + "top <= 0" + currValues ( ) );
		}
		if ( cam == "andor" )
		{
			if ( right > 512 )
			{
				thrower ( errstr + "right > 512" + currValues ( ) );
			}
			if ( bottom > 512 )
			{
				thrower ( errstr + "bottom > 512" + currValues ( ) );
			}
		}
		if ( cam == "ace" )
		{
			if ( horRawPixelNum() % 16 != 0 || vertRawPixelNum() % 16 != 0 )
			{
				thrower ( "ERROR: In the ace camera, the number of pixels in each dimension must be a multiple "
						  "of 16!\r\n" + currValues());
			}
		}
	}
};

