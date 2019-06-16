#include "stdafx.h"
#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include "SystemWithStdout.h"

std::string SystemWithStdout ( std::string cmdstr )
{
	std::array<char, 128> buffer;
	std::string result;
	std::unique_ptr<FILE, decltype( &_pclose )> pipe ( _popen ( cmdstr.c_str(), "r" ), _pclose );
	if ( !pipe )
	{
		throw std::runtime_error ( "popen() failed!" );
	}
	while ( fgets ( buffer.data ( ), buffer.size ( ), pipe.get ( ) ) != nullptr )
	{
		result += buffer.data ( );
	}
	return result;
}