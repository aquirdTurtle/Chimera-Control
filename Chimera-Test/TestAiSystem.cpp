#include "stdafx.h"
#include "CppUnitTest.h"
#include "../Chimera/AiSystem.h"
#include "../Chimera/Thrower.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace TestAiSystem
{
	TEST_CLASS( TestAiSys )
	{
		public:
		TEST_METHOD( InitAiSysConnected )
		{
			AiSystem aiSys;
			// make sure this can be done without errors getting thrown.
			aiSys.getSystemStatus();
		}
	};
}
