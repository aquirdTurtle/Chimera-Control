#pragma once
#include <Windows.h>
#include "MyTTLClass.h"
#include "MyDACClass.h"
#include "MyVariableClass.h"
class MasterWindow
{
	public:
		static MasterWindow* InitializeWindowInfo(HINSTANCE hInstance);
		MasterWindow() : masterWindowHandle(0) {}
		~MasterWindow();

	private:
		LRESULT MasterWindowMessageHandler(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		static LRESULT CALLBACK MasterWindowProcedure(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		std::string title;
		int InitializeWindowControls();
		HWND masterWindowHandle;
		MyTTLClass ttlBoard;
		MyDACClass dacBoards;
		MyVariableClass variables;
		//static HINSTANCE cHInstance;
};