// created by Mark O. Brown
#pragma once
#include <array>
#include "GeneralUtilityFunctions/range.h"

enum class ftdiConnectionOption
{
	None,
	Serial,
	Async
};


struct ftdiPt
{
	unsigned int time;
	std::array<unsigned char, 8> pts;

	bool operator==( const ftdiPt& p )
	{
		bool pts_match = true;
		for ( auto pt_count : range( pts.size( ) ) )
		{
			if ( pts[pt_count] != p.pts[pt_count] )
			{
				pts_match = false;
			}
		}
		return (time == p.time && pts_match);
	}
};

struct finBufInfo
{
	unsigned int bytesToWrite;
	/*
	FORMATTING NOTES:
	this is the final buffer that is sent to the fpga. Each snapshot of the sequence of dio configurations is organized
	3 groups of 7 characters. Each group starts with a character value of 161. The second and third character of each
	group are the address to writebtn the next four characters to. Then the last four characters are the data to writebtn.
	The first group is the time of the snapshot.
	The second group is the first 4 bytes of the dio configuration.
	The third group is the second 4 bytes of the dio configuration.
	Therefore, 

	pts[0] = 161
	pts[1] = 8
	pts[2] = 0
	pts[3] = Timestamp_Byte_1 (highest byte)
	pts[4] = Timestamp_Byte_2
	pts[5] = Timestamp_Byte_3
	pts[6] = Timestamp_Byte_4 (lowest byte)

	pts[7] = 161
	pts[8] = 16
	pts[9] = 0
	pts[10] = DIO_config_byte_1
	pts[11] = DIO_config_byte_2
	pts[12] = DIO_config_byte_3
	pts[13] = DIO_config_byte_4
	
	pts[14] = 161
	pts[15] = 24
	pts[16] = 0
	pts[17] = DIO_config_byte_5
	pts[18] = DIO_config_byte_6
	pts[19] = DIO_config_byte_7
	pts[20] = DIO_config_byte_8

	Then the addresses first increment in the low word of the address, so
	pts[21] = 161
	pts[22] = 8
	pts[23] = 1
	
	et cetra. 

	*/
	std::vector<unsigned char> pts;
};