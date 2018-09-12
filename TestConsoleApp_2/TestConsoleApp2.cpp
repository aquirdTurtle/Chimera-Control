// TestConsoleApp2.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Thrower.h"
#include <iostream>
// runs the sample function above and prints the caught exception
int main ( )
{
	try	{
		// [Doing important stuff...]
		try	{
			std::string s = "Hello, world!";
			try {
				int i = std::stoi ( s );
			}
			catch ( ... ) {
				thrower ( "Failed to convert string \"" + s + "\" to an integer!" );
			}
		}
		catch ( Error& e ) {
			thrower ( "Failed to [Do important stuff]!" );
		}
	}
	catch ( Error& e ) {
		std::cout << Error::getErrorStack ( e );
	}
	std::cin.get ( );
}
