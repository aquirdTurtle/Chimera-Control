#pragma once

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include "afxwin.h"
#include "Control.h"
//#include "StackWalker.h"

/*
 * A nice custom class and #define that makes my custom throws have file & code line information. Very nice.
 * stolen From http://stackoverflow.com/questions/348833/how-to-know-the-exact-line-of-code-where-where-an-exception-has-been-caused
 * Slightly modified.
 */
class my_exception : public std::runtime_error 
{
	public:
		my_exception(const std::string &arg, const char *file, int line) : std::runtime_error(arg) 
		{
			std::ostringstream out;
			out << file << ":" << line << ": " << arg;
			msg = out.str();
			bareMsg = arg;
		}

		~my_exception() throw() {}

		const char *what() const throw() 
		{
			return msg.c_str();
		}
		std::string whatStr() const throw()
		{
			return msg;
		}
		std::string whatBare() const throw()
		{
			return bareMsg;
		}
	private:
		std::string msg;
		std::string bareMsg;
};

/*
Tried to use this class from code exchange but it's old and uses deprecated functionality.
class myStackWalker : public StackWalker
{
	public:
		myStackWalker() : StackWalker() {}
		std::string ShowCallstack()
		{
			StackWalker::ShowCallstack();
			return callStack;
		}
	protected:
		virtual void OnOutput(LPCSTR szText)
		{
			callStack = std::string(szText); 
			StackWalker::OnOutput(szText);
		}
	private:
		std::string callStack;
};
*/
// the following gives any throw call file and line information.
#define thrower(arg) throw my_exception(arg, __FILE__, __LINE__)
// shows error message if it exists. Could be function but for consistency with other ERR_X Macros...
#define ERR_POP(string) {if (string != ""){errBox(string);}}
// shows error message and exits given function with error.
#define ERR_POP_RETURN(string) {if (string != ""){errBox(string); return true;}}

/*
* This functions appends the text "newText" to the edit control corresponding to textIDC.
*/
//void appendText(std::string newText, int textIDC, HWND parentWindow);
void appendText(std::string newText, CEdit& edit);
void appendText(std::string newText, Control<CRichEditCtrl>& edit);

#define cstr(input) std::to_string(input).c_str()

