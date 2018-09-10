#include "stdafx.h"
#include "NiawgStructures.h"

const std::array<niawgWavePower::mode, 4> niawgWavePower::allModes = { mode::constant, mode::capped, mode::unrestricted};
const niawgWavePower::mode niawgWavePower::defaultMode = CONST_POWER_OUTPUT ? niawgWavePower::mode::constant : niawgWavePower::mode::unrestricted;

std::string niawgWavePower::toStr ( niawgWavePower::mode m )
{
	switch ( m )
	{
		case mode::constant:
			return "constant";
		case mode::capped:
			return "capped";
		case mode::unrestricted:
			return "unrestricted";
	}
}

niawgWavePower::mode niawgWavePower::fromStr ( std::string txt )
{
	for ( auto m : allModes )
	{
		if (txt == toStr(m))
		{
			return m;
		}
	}
}


const std::array<niawgLibOption::mode, 4> niawgLibOption::allModes = { mode::allowed, mode::banned, mode::forced };
const niawgLibOption::mode niawgLibOption::defaultMode = mode::allowed;


std::string niawgLibOption::toStr ( niawgLibOption::mode m )
{
	switch ( m )
	{
		case mode::allowed:
			return "allowed";
		case mode::banned:
			return "banned";
		case mode::forced:
			return "forced";
	}
}

niawgLibOption::mode niawgLibOption::fromStr ( std::string txt )
{
	for ( auto m : allModes )
	{
		if ( txt == toStr ( m ) )
		{
			return m;
		}
	}
}
