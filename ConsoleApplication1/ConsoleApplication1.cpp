// ConsoleApplication1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include "H5Cpp.h"
#include <vector>
#include "Windows.h"

#define size 15

int main()
{
	std::vector<long> pic;
	pic.resize( size*size );
	unsigned long long count = 0;
	for (auto& elem : pic)
	{
		elem = count++;
	}
	try
	{
		H5::H5File file( "C:\\Users\\Mark\\Documents\\Quantum-Gas-Assembly-Control\\Data\\2017\\August\\August 15\\"
						 "Raw Data\\data_3.h5", H5F_ACC_TRUNC );
		H5::Group picGroup( file.createGroup( "/Andor" ) );
		hsize_t dims[3] = {10, size, size };
		H5::DataSpace dataspace( 3, dims );		
		H5::DataSet dataset = picGroup.createDataSet( "Pictures", H5::PredType::STD_I32BE, dataspace );
		hsize_t slabdim[3] = { 1, size, size };
		H5::DataSpace memspace( 3, slabdim );
		hsize_t offset[3] = { 0, 0, 0 };
		unsigned start = GetTickCount64();
		for (int count = 0; count < 1e7; count++)
		{
			dataspace.selectHyperslab( H5S_SELECT_SET, slabdim, offset );
			dataset.write( pic.data(), H5::PredType::STD_I32BE, memspace, dataspace );
		}
		unsigned stop = GetTickCount64();
		std::cout << "time: " << (stop - start) / 1.0e7<< "\n\n";
		for (int conut = 0; conut < 10; conut++)
		{
			offset[0] = conut;
			dataspace.selectHyperslab( H5S_SELECT_SET, slabdim, offset );
			dataset.write( pic.data(), H5::PredType::STD_I32BE, memspace, dataspace );
		}
		hsize_t attrDim[1] = { 2 };
		H5::DataSpace attrDataspace( 1, attrDim );
		H5::Attribute attr = dataset.createAttribute("hello", H5::PredType::C_S1, attrDataspace );
		//long data[2] = { size, size };
		std::string str = "12";
		attr.write( H5::PredType::C_S1, str.c_str() );
		/*
		H5::H5File file( "TestHDF5.h5", H5F_ACC_RDWR );
		H5::DataSet dataset = file.openDataSet( "Andor-Pictures" );
		dataset.read( (void*)pic, H5::PredType::NATIVE_INT );
		for (unsigned inc = 0; inc < size * size; inc++)
		{
			std::cout << pic[inc] << ", ";
		}
		*/
		//dataset.write( pic, H5::PredType::NATIVE_INT );

	}
	catch (H5::Exception error)
	{
		std::cout << error.getDetailMsg();
	}
	std::cout << "\nfin";
	std::cin.get();
	return 0;
}

