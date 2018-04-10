#include "stdafx.h"
#include "CppUnitTest.h"
#include "../Chimera/Matrix.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace TestMatrix
{
	TEST_CLASS( TestMatrix )
	{
		public:
		TEST_METHOD( InitMatrix )
		{
			UINT rows( 10 ), cols( 11 );
			Matrix<double> testMatrix(rows, cols);
			Assert::AreEqual( testMatrix.getRows( ), rows );
			Assert::AreEqual( testMatrix.getCols( ), cols );
			for ( auto val : testMatrix )
			{
				Assert::AreEqual( 0.0, val );
			}
		}
		TEST_METHOD( MatrixAccess )
		{
			UINT rows( 10 ), cols( 11 );
			Matrix<double> testMatrix( rows, cols );
			testMatrix( 0, 0 ) = 19.4;
			testMatrix( 9, 10 ) = -13.4;
			testMatrix( 4, 7 ) = 1e4;
			Assert::AreEqual( testMatrix( 0, 0 ), 19.4 );
			Assert::AreEqual( testMatrix( 9, 10 ), -13.4 );
			Assert::AreEqual( testMatrix( 4, 7 ), 1e4 );
		}
		TEST_METHOD( MatrixUntainted )
		{
			// make sure changing one value leaves everything else fine.
			UINT rows( 10 ), cols( 11 );
			Matrix<double> testMatrix( rows, cols );
			testMatrix( 0, 0 ) = 19.4;
			UINT count = 0;
			for ( auto val : testMatrix )
			{
				count++;
				if ( count == 1 )
				{
					continue;
				}
				Assert::AreEqual( val, 0.0 );
			}
		}
		TEST_METHOD( Submatrix )
		{
			UINT rows( 10 ), cols( 11 );
			Matrix<double> testMatrix( rows, cols ), testMatrix2( 4, 5 );
			testMatrix( 2, 3 ) = 2001;
			testMatrix2( 2, 3 ) = 2001;
			Assert::AreEqual( 2001.0, testMatrix.submatrix( 0, 6, 0, 7 )(2, 3) );
			Assert::AreEqual( 2001.0, testMatrix.submatrix( 1, 6, 1, 7 )(1, 2) );
			Assert::AreEqual( 0.0, testMatrix.submatrix( 4, 4, 4, 4 )(2, 3) );
		}
		TEST_METHOD( MatrixExceptions )
		{
			UINT rows( 10 ), cols( 11 );
			Matrix<double> testMatrix( rows, cols );
			
			Assert::ExpectException<Error>( [this, &testMatrix] {testMatrix(20, 1); } );
			Assert::ExpectException<Error>( [this, &testMatrix] {testMatrix(1,-1); } );
			Assert::ExpectException<Error>( [this, &testMatrix] {testMatrix.submatrix( 0, 0, 15, 15 ); } );
			Assert::ExpectException<Error>( [this, &testMatrix] {testMatrix.submatrix( -1, 0, 5, 5 ); } );
			Assert::ExpectException<Error>( [this, &testMatrix] {testMatrix.submatrix( 8, 8, 5, 5 ); } );
		}
	};
}
