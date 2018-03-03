#include "stdafx.h"
#include "CppUnitTest.h"
#include "../Chimera/MultidimensionalKey.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace TestMatrix
{
	TEST_CLASS( TestKey )
	{
		public:
		TEST_METHOD( InitKey )
		{
			multiDimensionalKey<double> testKey;
		}
		TEST_METHOD( GenerateKey )
		{
			// todo
		}
	};
}