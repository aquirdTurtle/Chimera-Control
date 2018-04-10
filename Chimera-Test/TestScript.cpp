#include "stdafx.h"
#include "CppUnitTest.h"
#include "../Chimera/Script.h"


using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace TestScript
{
	TEST_CLASS( TestScript )
	{
		public:
		TEST_METHOD( InitScript )
		{
			Script testScript;
		}
	};
}