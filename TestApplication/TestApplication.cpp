#include "stdafx.h"
#include <iostream>
#include "Python.h"

int main()
{
	Py_Initialize();

	FILE* PythonScriptFile;
	fopen_s(&PythonScriptFile, "C:\\Users\\Mark\\Documents\\My Data Analysis\\Simple Atom Analysis.py", "r");
	if (PythonScriptFile)
	{
		PyRun_SimpleFile(PythonScriptFile, "Simple Atom Analysis.py");
		fclose(PythonScriptFile);
	}

	Py_Finalize();
	std::cin.get();
	return 0;
}
