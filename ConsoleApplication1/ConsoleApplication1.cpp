// ConsoleApplication1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/video/video.hpp>

#include "pylon/PylonIncludes.h"
#include <pylon/PylonGUI.h>

static const uint32_t c_countOfImagesToGrab = 10;

int main()
{
	Pylon::PylonAutoInitTerm autoInitTerm;

	try
	{
		Pylon::CInstantCamera camera( Pylon::CTlFactory::GetInstance().CreateFirstDevice() );
		// 
		std::cout << "Using device " << camera.GetDeviceInfo().GetModelName() << std::endl;
		GenApi::INodeMap& nodemap = camera.GetNodeMap();
		camera.Open();
		GenApi::CIntegerPtr width = nodemap.GetNode( "Width" );
		GenApi::CIntegerPtr height = nodemap.GetNode( "Height" );
		camera.MaxNumBuffer = 5;
		Pylon::CImageFormatConverter formatConverter;
		formatConverter.OutputPixelFormat = Pylon::PixelType_BGR8packed;
		Pylon::CPylonImage pylonImage;
		int grabbedImages = 0;
		cv::VideoWriter cvVideoCreator;
		cv::Mat openCvImage;
		std::string videoFileName = "openCvVideo.avi";
		cv::Size frameSize = cv::Size( (int)width->GetValue(), (int)height->GetValue() );
		cvVideoCreator.open( videoFileName, CV_FOURCC( 'D', 'I', 'V', 'X' ), 20, frameSize, true );

		camera.StartGrabbing( c_countOfImagesToGrab, Pylon::GrabStrategy_LatestImageOnly );

		Pylon::CGrabResultPtr ptrGrabResult;

		while (camera.IsGrabbing())
		{
			camera.RetrieveResult( 5000, ptrGrabResult, Pylon::TimeoutHandling_ThrowException );
			if (ptrGrabResult->GrabSucceeded())
			{
				std::cout << "SizeX: " << ptrGrabResult->GetWidth() << std::endl;
				std::cout << "SizeY: " << ptrGrabResult->GetHeight() << std::endl;
				formatConverter.Convert( pylonImage, ptrGrabResult );
				std::vector<std::vector<uint8_t>> image;
				uint8_t* data = (uint8_t*)pylonImage.GetBuffer();
				image.resize( ptrGrabResult->GetHeight() );
				int offset = 0;
				for (auto& elem : image)
				{
					elem = std::vector<uint8_t>( offset + data, offset + data + ptrGrabResult->GetWidth() );
					offset += ptrGrabResult->GetWidth();
				}
				openCvImage = cv::Mat( ptrGrabResult->GetHeight(), ptrGrabResult->GetWidth(), CV_8UC3, (uint8_t*)pylonImage.GetBuffer() );
				std::ostringstream s;
				s << "image_" << grabbedImages << ".jpg";
				std::string imageName( s.str() );
				cv::imwrite( imageName, openCvImage );
				grabbedImages++;
			}
			cvVideoCreator.write( openCvImage );

			cv::namedWindow( "OpenCV Display Window", CV_WINDOW_NORMAL );

			cv::imshow( "OpenCv Display Window", openCvImage );
			cv::waitKey( 1 );

		}
	}
	catch (Pylon::GenericException & err)
	{
		std::cout << "Error: " << err.what() << std::endl;
	}
	std::string stf;
	std::cin >> stf;

	return 0;
}
