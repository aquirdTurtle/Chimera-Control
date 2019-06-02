#include "stdafx.h"
#include "CppUnitTest.h"
#include "../Chimera/VisaFlume.h"
#include "../Chimera/DaqMxFlume.h"
#include "../Chimera/Fgen.h"
#include "../Chimera/GpibFlume.h"
#include "TestMacros.h"
#include "../Chimera/constants.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace TestFlumes
{		
	TEST_CLASS(TestFlumes)
	{
	public:
		TEST_METHOD( InitializeVisaFlume )
		{
			std::string addr("dummy_usb_address");
			bool safemode = true;
			VisaFlume testFlume( safemode, addr);
			Assert::AreEqual( testFlume.deviceSafemode, safemode );
			Assert::AreEqual( testFlume.usbAddress, addr );
		}
		TEST_METHOD( InitializeFgenFlume )
		{
			bool safemode = true;
			FgenFlume testFlume(safemode);
			Assert::AreEqual( testFlume.safemode, safemode );
		}
		TEST_METHOD( InitializeGpibFlume )
		{
			short id = 0;
			bool safemode = true;
			GpibFlume testFlume(id, safemode);
			Assert::AreEqual( testFlume.deviceSafemode, safemode );
			Assert::AreEqual( testFlume.deviceID, id );
		}
		TEST_METHOD( InitializeDaqmxFlume )
		{
			bool safemode = true;
			DaqMxFlume testFlume( safemode );
			Assert::AreEqual( testFlume.safemode, safemode );
		}
		CONNECTED_TEST( c_Connect_To_Rhode_Schwarz )
		{
			GpibFlume flume( RSG_ADDRESS, false );
			Assert::AreNotEqual( std::string(""), flume.queryIdentity( ) );
		}
	};
}
