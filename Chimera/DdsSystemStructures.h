#pragma once 
#include <array>

struct ddsConnectionType
{
	enum class type
	{
		None, Serial, Async
	};
};

struct ddsRampInfo
{
	double freq1; // in MHZ
	double freq2;
	double amp1;  // between 0 and 100
	double amp2;
	double rampTime; // in milliseconds
};

struct ddsRampInfoForm
{
	USHORT index;
	UINT channel;
	Expression freq1Form; // in MHZ
	Expression freq2Form;
	Expression amp1Form;  // between 0 and 100
	Expression amp2Form;
	Expression rampTimeForm; // in milliseconds
};

template<typename type>
class ddsBox
{
	public:
		// a wrapper to make a matrix of "type" with the dimensions of the dds box - i.e. one value of "type" for each 
		// channel of each board.
		type& operator ()(UINT boardNumber, UINT channelNumber)
		{
			if ( boardNumber > 1 )
			{
				thrower ( "DDS board number out of range - should be 0 or 1" );
			}
			if ( channelNumber > 3 )
			{
				thrower ( "DDS channel number out of range - should be between 0 and 3 (inclusive)" );
			}
			return data[ boardNumber ][ channelNumber ];
		}

		type operator ()( UINT boardNumber, UINT channelNumber ) const
		{
			if ( boardNumber > 1 )
			{
				thrower ( "DDS board number out of range - should be 0 or 1" );
			}
			if ( channelNumber > 3 )
			{
				thrower ( "DDS channel number out of range - should be between 0 and 3 (inclusive)" );
			}
			return data[ boardNumber ][ channelNumber ];
		}

		std::array<type, 4>& getBoard ( UINT which )
		{
			if ( which > 1 )
			{
				thrower ( "DDS board number out of range - should be 0 or 1" );
			}
			return data[ boardNumber ];
		}

		UINT numBoards ( )
		{
			return 2;
		}
		UINT numChannels ( )
		{
			return 4;
		}

	private:
		std::array<std::array<type, 4>, 2> data;
};
