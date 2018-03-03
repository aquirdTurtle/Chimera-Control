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
		TEST_METHOD( InitAiSys )
		{
			AiSystem aiSys;
			Assert::ExpectException<Error>( [this, &aiSys] {aiSys.getSystemStatus( ); } );
		}
	};
}
