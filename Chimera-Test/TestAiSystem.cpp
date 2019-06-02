#include "stdafx.h"
#include "TestMacros.h"
#include "CppUnitTest.h"
#include "../Chimera/AiSystem.h"
#include "../Chimera/Thrower.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace TestAiSystem
{
	TEST_CLASS( TestAiSys )
	{
		public:
		CONNECTED_TEST ( c_InitAiSysConnected )
		{
			AiSystem aiSys;
			// make sure this can be done without errors getting thrown.
			aiSys.getSystemStatus();
		}
	};
}
