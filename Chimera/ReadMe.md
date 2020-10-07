﻿========================================================================
#    Master Code Project Overview
========================================================================

# Introduction
## Please feel free to contact me at any time with questions, bug reports, feature requests, or general support or 
	advice. I'm happy to help the project be as useful as possible to others who wish to use it. Contact me at 
	Mark.O.Brown@colorado.edu with any such questions
	
# Main Systems Overview
## TTL System
	We use a Viewpoint DIO card for an extensive and important digital output system. We call it the TTL system. the 
	TTL system triggers virtually everything in the system to control the timing of everything in the experiment. 
## DAC System
	we use 3 (at the moment) national instruments Analog out cards.
## Agilent System
	We use a series of agilent arbitrary function generators during the experiment. they are programmed via Agilent's 
	VISA protocol on a usb cable.
## Tektronics System
	We use a couple tektronics generators for some higher frequency RF. They are programmed via GPIB.
## Rhode Schwartz System
	The Rhode-Schwartz generator (RSG) is a nice microwave generator we use in the experiment. It's programmed via GPIB.

# Some Code Systems
## thrower("Error message") and the Error class
	= This is a little macro that I use extensively for throwing errors. It throws a custom exception object "Error".
	The macro adds the file and line number to the throw function call so that it is easy to find from the gui where 
	the throw happened, for debugging purposes.
	- The core of this was stolen from a stack overflow page.
## ScriptStream
	This is a custom stream object (similar to std::iostream or std::stringstream) that I use to manage my scripting 
	languages. Primarily, it does some standard preprocessing of the strings as they get outputted to the 
## Texting system
	This uses python to notify users of interesting events via text, for example it can notify the user when an 
	experiment ends. It uses python to send an email to a phone-provider (i.e. verizon, AT&T) specific email address
	which forwards a text to your phone. Since this requires an email be sent, it requires you to log in to an email 
	account. Note that google warns that the simplistic method I use to do this emailing is not very secure, and that I
	had to specifically allow such methods to be used with my email address. The password that you enter into the system
	is not held particularly securely. After one person enters the password into the program instance, it would not be 
	very hard for someone else to extract the password from the program.

# Simple program modification instructions


# Installation
- Important note: I've created this program as a 32-bit program because I only had the 32-bit version of the DIO64 dlls.
in principle this can all be done in 64-bit mode (which might be faster for some things), but I haven't set this up yet.
I will probably set this up soon when we move away from the DIO64 system.

## First, some light suggestions
	- Install Git. If you like these types of things, install a git gui client, my favorite is smartGit
	- Install CMake. If the installation below goes well, you won't need it, but it's generally useful for these
	types of things.
	- if you need to do either, I suggest using powershell over cmd. It's built in to windows & great.
	- You might like to install a better text editor than notepad. I suggest either "Atom" https://atom.io/ or 
	"notepad++" https://notepad-plus-plus.org/
## Install Visual Studio Community Version. The most recent version that I've used is visual studio 2017. Note that the 
VS version makes a big difference, some packages below would need to be recompiled with other versions.
	- The extra options don't matter very much.
	- I suggest that you don't install Python compattibility and install that separately.
	- Once you try to open my project, visual studio should prompt you to install the MFC tools which you need.
## Python
	- There are two aspects to this. You want standard python libraries for just using python, and you want the
	python3X.dll, python3X_D.dll, and python3X.lib (X -> version number, _D.dll being the debug version) files.
	These latter files are what my C code uses to do python programming, so compilation / running of my code will fail 
	without these.
	- I suggest downloading the Anaconda Distribution (make sure to get the 32-bit version) for general python usage.
	- I've included the python36.lib/dll and python36_D.lib/dll files in both the 32-bit and 64-bit folders.
	- If you want or need more modern or different python versions, you can find the release files in the anaconda 
	distribution filesystem (find the ".anaconda" folder), but anaconda doesn't include the debug distributions.
	You can compile these from source if you like, but you can also get them by downloading python directly from 
	python.org, you just need to do a custom install and specify debug files and symbols during the install.
## Boost
	- Download and build the lastest version of boost from boost.org. The build can take an hour or so. My 
	quick instructions, which work well for me:
	- Download & unzip the boost.7z file from boost.org.
	- run the "b2" file.
	- run the "bat". Now it's installed.
## HDF5
	- I've included HDF5 packages that I compiled myself using visual studio 2017 (necessary for compatibility) as
	zipped files in the project under the packages\ folder. You should just need to un-zip them to the correct 
	location and it should work fine. I've inlcuded both release and debug versions of the files. By default,
	my project will look for the files in "C:\Program Files(x86)\HDF_Group\HDF5\HDF5-1.10.1\Debug" or "...\Release".
	- IF you want to recompile the packages (otherwise skip this) (for example you want a newer HDF5 version), then
	there are some instructions on the HDF5 website, but I've also included my shortened version which worked for me.
		- Download and unzip the CMake version of the HDF5 Source (can find on website, it should be 
		the CMake-hdf5-XXX.zip file). It's important to unzip this to a file address that has NO SPACES in its path.
		- in the unzipped file,	open the file "HDF5config.cmake". Find the part that sets the 
		CTEST_CMAKE_GENERATOR constant. Add the following text to the logic structure there:

		elseif (${BUILD_GENERATOR} STREQUAL "VS2017")
			set (CTEST_CMAKE_GENERATOR "Visual Studio 15 2017")
			
		At the time of this writing, the HDF5 cmake system is not yet aware of Visual studio 2017, and this change is
		necessary to fix that. 
		- run the following command:

		ctest -S HDF5config.cmake,BUILD_GENERATOR=VS2017 -C Release -V -O hdf5.log

		or replace Release with Debug to get the debug version. HDF5 will now build, it will take some time.
		- After the build finishes, go into the new "build" folder and find the HDF5-XXX.zip file. This contains the 
		include, lib, and dll files that you need. Unzip it to the desired location, make sure the visual studio project
		is pointed to it, and you're good to go.
## Download and install the latest version of DAQmx from national instruments. They have installers for this.
	- make sure that you see the following file:
		"c:\..."
	If it's not there, you may have to search around for this file.
	- Note that installing either daqmx or fgen should also install NI-Visa, which my code also uses.
## Download and install the latest version of NI-FGEN from national instruments. They have installers for this.
		- make sure that you see the following file:
		"c:\..."
	If it's not there, you may have to search around for this file.
	- Note that installing either daqmx or fgen should also install NI-Visa, which my code also uses.
- download and install gnuplot from http://www.gnuplot.info/. my code uses via the gnuplot-iostream package. 
- Optional: Download and install Agilent's IO Libraries Suite for using agilents. My code should compile fine without 
this though. http://www.keysight.com/en/pd-1985909/io-libraries-suite?cc=US&lc=eng

# Setting up Visual Studio
- If you follow the above directions carefully and are a bit lucky, my code my just compile after installing everything.
However, more likely some files will have shifted around to different locations during your install process, and you'll 
need to redirect my project to the proper locations for those files.
- In order to compile, the following need to be set correctly:
	- "Include Directories" 
		- Otherwise includes, like "#include <boost...>" or "#include 'visa.h'" will fail. This is pretty obvious.
	- "Library Directories"
		- Tells the compiler where to look for the necessary *.lib files
	- "Additional Dependencies"
		- Tells the compiler the names of the libraries it needs to open. You shouldn't need to change this.