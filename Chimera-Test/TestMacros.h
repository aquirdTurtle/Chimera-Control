#pragma once
#include "CppUnitTest.h"
#include "afxwin.h"
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

// This my own macro. connected tests must be run with devices connected in order to succeed.
#define CONNECTED_TEST( testMethod ) \
		BEGIN_TEST_METHOD_ATTRIBUTE( testMethod )\
		TEST_METHOD_ATTRIBUTE( L"Connected", "T" )\
		END_TEST_METHOD_ATTRIBUTE( )\
		TEST_METHOD( testMethod )
