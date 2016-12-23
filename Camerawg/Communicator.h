#pragma once

class MainWindow;
class ScriptingWindow;

/*
	* This class was desiged to hold a pointer to the main window that can only be used for communicating messages to said 
	* window. It prevents the need to pass the raw pointer to the main window which a small function shouldn't need.
*/
class Communicator
{
	public:
		void initialize(MainWindow* parent, ScriptingWindow* scriptingWin, CameraWindow* cameraWin);
		void sendError(std::string statusMsg, std::string shortMsg, std::string color);
		void sendFatalError(std::string statusMsg, std::string shortMsg, std::string color);
		void sendStatus(std::string statusMsg, std::string shortMsg, std::string color);
		void sendDebug(std::string statusMsg, std::string shortMsg, std::string color);
	private:
		MainWindow* parent;
		ScriptingWindow* scriptWin;
		CameraWindow* camWin;
};
