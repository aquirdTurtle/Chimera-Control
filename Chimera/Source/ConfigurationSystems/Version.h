// created by Mark O. Brown
#pragma once
#include "GeneralUtilityFunctions/thrower.h"
#include "GeneralUtilityFunctions/my_str.h"
#include <string>
#include <boost/lexical_cast.hpp>

/*	A small class that provides easy version checking.
*/
class Version{
	public:
		Version( )	{
			versionMajor = -1;
			versionMinor = -1;
		}
		Version( std::string versionStr ){
			double version;
			try	{
				version = boost::lexical_cast<double>( versionStr );
				auto periodPos = versionStr.find_last_of( '.' );
				std::string tempStr( versionStr.substr( 0, periodPos ) );
				versionMajor = boost::lexical_cast<unsigned long>( tempStr );
				tempStr = versionStr.substr( periodPos + size_t(1), versionStr.size( ) );
				versionMinor = boost::lexical_cast<unsigned long>( tempStr );
			}
			catch ( boost::bad_lexical_cast& ){
				throwNested ( "ERROR: Version string failed to convert to double!" );
			}
		}
		std::string str( ) const{
			return ::str(versionMajor) + "." + ::str(versionMinor);
		}
		bool operator>(Version& otherVersion ){
			if ( (otherVersion.versionMajor == versionMajor && versionMinor > otherVersion.versionMinor )
				 || versionMajor >  otherVersion.versionMajor )	{
				return true;
			}
			else{
				return false;
			}
		}
		bool operator>=( Version& otherVersion ){
			if ( (otherVersion.versionMajor == versionMajor && versionMinor >= otherVersion.versionMinor )
				 || versionMajor > otherVersion.versionMajor ){
				return true;
			}
			else{
				return false;
			}
		}
		bool operator<( Version& otherVersion ){
			if ( (otherVersion.versionMajor == versionMajor && versionMinor < otherVersion.versionMinor )
				 || versionMajor < otherVersion.versionMajor ){
				return true;
			}
			else{
				return false;
			}
		}
		bool operator<=( Version& otherVersion ){
			if ( (otherVersion.versionMajor == versionMajor && versionMinor <= otherVersion.versionMinor)
				 || versionMajor < otherVersion.versionMajor ){
				return true;
			}
			else{
				return false;
			}
		}
		bool operator==( Version& otherVersion ){
			if (otherVersion.versionMajor == versionMajor && otherVersion.versionMinor == versionMinor){
				return true;
			}
			else{
				return false;
			}
		}
		unsigned long versionMajor;
		unsigned long versionMinor;
};