#pragma once
#include "thrower.h"
#include "my_str.h"
#include <string>
#include "afxwin.h"


class Version
{
	public:
		Version( )
		{
			versionMajor = -1;
			versionMinor = -1;
		}
		Version( std::string versionStr )
		{
			double version;
			try
			{
				version = std::stod( versionStr );
				int periodPos = versionStr.find_last_of( '.' );
				std::string tempStr( versionStr.substr( 0, periodPos ) );
				versionMajor = std::stod( tempStr );
				tempStr = versionStr.substr( periodPos + 1, versionStr.size( ) );
				versionMinor = std::stod( tempStr );
			}
			catch ( std::invalid_argument& )
			{
				thrower( "ERROR: Version string failed to convert to double!" );
			}
		}
		std::string str( ) const 
		{
			return ::str(versionMajor) + "." + ::str(versionMinor);
		}
		bool operator>(Version& otherVersion )
		{
			if ( (otherVersion.versionMajor == versionMajor && versionMinor > otherVersion.versionMinor )
				 || versionMajor >  otherVersion.versionMajor )
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		bool operator>=( Version& otherVersion )
		{
			if ( (otherVersion.versionMajor == versionMajor && versionMinor >= otherVersion.versionMinor )
				 || versionMajor > otherVersion.versionMajor )
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		bool operator<( Version& otherVersion )
		{
			if ( (otherVersion.versionMajor == versionMajor && versionMinor < otherVersion.versionMinor )
				 || versionMajor < otherVersion.versionMajor )
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		bool operator<=( Version& otherVersion )
		{
			if ( (otherVersion.versionMajor == versionMajor && versionMinor <= otherVersion.versionMinor)
				 || versionMajor < otherVersion.versionMajor )
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		bool operator==( Version& otherVersion )
		{
			if (otherVersion.versionMajor == versionMajor && otherVersion.versionMinor == versionMinor)
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		ULONG versionMajor;
		ULONG versionMinor;
};