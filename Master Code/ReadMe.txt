========================================================================
    Master Code Project Overview
========================================================================

==== Introduction
- Please feel free to contact me at any time with questions, bug reports, feature requests, or general support or 
	advice. I'm happy to help the project be as useful as possible to others who wish to use it. Contact me at 
	Mark.O.Brown@colorado.edu with any such questions

==== Main Systems Overview
- TTL System
	We use a Viewpoint DIO card for an extensive and important digital output system. We call it the TTL system. the 
	TTL system triggers virtually everything in the system to control the timing of everything in the experiment. 
- DAC System
	we use 3 (at the moment) national instruments Analog out cards.
- Agilent System
	We use a series of agilent arbitrary function generators during the experiment. they are programmed via Agilent's 
	VISA protocol on a usb cable.
- Tektronics System
	We use a couple tektronics generators for some higher frequency RF. They are programmed via GPIB.
- Rhode Schwartz System
	The Rhode-Schwartz generator (RSG) is a nice microwave generator we use in the experiment. It's programmed via GPIB.

==== Some Code Systems
- MFC and Win32
	win32 is the C core of the GUI program. It interacts directly with windows. MFC (Microsoft foundation classes) 
	is a system of classes that act as wrappers around win32, making writing guis much easier. An early implementation 
	of the code was written in raw win32. Unfortunately, since this is written in MFC at this point, the project is 
	incompatible with UNIX based OSs (linux & MacOS). In principle, if you grab 
- thrower("Error message") and the Error class
	This is a little macro that I use extensively for throwing errors. It throws a custom exception object "Error".
	The macro adds the file and line number to the throw function call so that it is easy to find from the gui where 
	the throw happened, for debugging purposes.
	- The core of this was stolen from a stack overflow page.
- EmbeddedPythonHandler
	this is an embedded python interpreter that you can send python commands to. Sending commands to it is like sending
	commands to a python IDLE command prompt. It can pop or throw errors when python throws errors. Some things are 
	just that much easier to do in python that implementing this was worth it. 
	- Note that I avoid extensive usage of the Python-C api by using the simplistic "run this line in the python 
		interpreter" approach. The Python-C api can be used to generate real python objects and functions etc. in C
		(rather than in a string), but I found it rather hard to use properly because of memory management issues.
- ScriptStream
	This is a custom stream object (similar to std::iostream or std::stringstream) that I use to manage my scripting 
	languages. Primarily, it does some standard preprocessing of the strings as they get outputted to the 
- Texting system
	This uses python to notify users of interesting events via text, for example it can notify the user when an 
	experiment ends. It uses python to send an email to a phone-provider (i.e. verizon, AT&T) specific email address
	which forwards a text to your phone. Since this requires an email be sent, it requires you to log in to an email 
	account. Note that google warns that the simplistic method I use to do this emailing is not very secure, and that I
	had to specifically allow such methods to be used with my email address. The password that you enter into the system
	is not held particularly securely. After one person enters the password into the program instance, it would not be 
	very hard for someone else to extract the password from the program.

==== Simple program modification instructions

- "I want to add a GUI element to an existing system (e.g. adding another button to the ttl system)"
	- Add the element as a private member of the system (e.g. "Control<CButton> myButton")
	- initialize the element in the system's initialize function.
	- add a rearrange() command inside the system's rearrange function (e.g. myButton.rearrange(...))
	- Add any special handling (e.g. handling button press, edit change) to the window to which the control belongs.

- "I want to create a new system (e.g. adding a control for a different type of function generator)."
	- Create a class for that system. You may consider copying and renaming the class of another similar system already
		in this code if it is similar.
	- Give it whatever gui elements and internal data structures you wish. 
	- Assuming the system has gui elements, create an "Initialize" and "Rearrange" functions, similar to those in other
		systems for initializing and moving around controls.
	- Of course create functions and handling for any tasks you want the system to be able to do.
	- Create an instance of the class object as a private member of the window to which it belongs (the "parent window")
	- call the "Initialize" function you created in the OnInitDialog() function of the parent window.
	- call the "Rearrange" function you created in the OnMove() function of the parent window.

- "I want to change what happens during the experiment procedure."
	- Most likely you want to change ExperimentManager::experimentThreadProcedure function. Read the function carefully.

- "I want to add / change scripting keywords / functionality."
	- You want to add or modify the big if / else if / else if control structure in 
		ExperimentManager::analyzeMasterScript and ExperimentManager::analyzeFunction functions. Make sure you change 
		both functions simultaneously!

- "I want to change the data output formatting / contents."
	- 

- "I want to plot something in real time from the program."
	- I tried for a while and failed at implementing a real C plotting utility. It was hard, most systems seemed built for 
		linux systems. If you wish to do this yourself, 
	- I have implemented the gnuplotiostream tool from ___ which you can make use of. I suggest that you take a close 
		look at the way I do this and copy from that.
	- It is also not hard to do plotting through the embedded python handler. This might be the easiest route for you.

======== Installation
- Visual Studio. The most recent version that I've used is 2017, although things shoulid generally be able to work with
	newer versions without too much trouble.
- Boost
- Hopefully everything else can be included in the project.
- ????
