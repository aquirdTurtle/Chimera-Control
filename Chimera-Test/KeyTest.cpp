#include "stdafx.h"
#include "CppUnitTest.h"
#include "../Chimera/MultidimensionalKey.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace TestKey
{
	TEST_CLASS( TestKey )
	{
		public:
		TEST_METHOD( InitKey )
		{
			multiDimensionalKey<double> testKey;
		}
	};
}
