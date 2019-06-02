#include "stdafx.h"
#include "CppUnitTest.h"
#include "../Chimera/Andor.h"
#include "TestMacros.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace TestAndor
{
	TEST_CLASS( TestAndor )
	{
		public:
		TEST_METHOD( Init_Andor_Obj )
		{
			AndorCamera andor(true);
		}
		CONNECTED_TEST( c_Connect_To_Andor )
		{
			AndorCamera andor(false);
		}
	};
}
