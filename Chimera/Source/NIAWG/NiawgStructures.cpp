// created by Mark O. Brown
#include "stdafx.h"
#include "NiawgStructures.h"

const std::array<niawgWavePower::mode, 4> niawgWavePower::allModes = { mode::constant, mode::capped, mode::unrestricted};
const niawgWavePower::mode niawgWavePower::defaultMode = niawgWavePower::mode::constant;

std::string niawgWavePower::toStr ( niawgWavePower::mode m_ ){
	switch ( m_ ){
		case mode::constant:
			return "constant";
		case mode::capped:
			return "capped";
		case mode::unrestricted:
			return "unrestricted";
	}
	thrower ( "Failed to convert niawg wave power mode to a string!" );
}

niawgWavePower::mode niawgWavePower::fromStr ( std::string txt ){
	for ( auto m_ : allModes ){
		if (txt == toStr(m_)){
			return m_;
		}
	}
	thrower ( "Failed to convert text to niawg wave power mode!" );
}

const std::array<niawgLibOption::mode, 4> niawgLibOption::allModes = { mode::allowed, mode::banned, mode::forced };
const niawgLibOption::mode niawgLibOption::defaultMode = mode::banned;

std::string niawgLibOption::toStr ( niawgLibOption::mode m ){
	switch ( m ){
		case mode::allowed:
			return "allowed";
		case mode::banned:
			return "banned";
		case mode::forced:
			return "forced";
	}
	thrower ( "Failed to convert niawg library mode option to a string!" );
}

niawgLibOption::mode niawgLibOption::fromStr ( std::string txt ){
	for ( auto m_ : allModes ){
		if ( txt == toStr ( m_ ) ){
			return m_;
		}
	}
	thrower ( "Failed to convert string to niawg lib option!" );
}
