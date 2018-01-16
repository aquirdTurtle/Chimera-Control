#pragma once
#include "Control.h"
#include "windows.h"
#include "afxwin.h"
#include "nidaqmx2.h"
#include <array>
#include "DaqMxFlume.h"
#include "constants.h"


class AiSystem
{
	public:
		void initDaqmx( );
		void initialize( POINT& loc, CWnd* parent, int& id );
		void refreshDisplays( );
		void rearrange( int width, int height, fontMap fonts );
		void refreshCurrentValues( );
		std::array<float64, NUMBER_AI_CHANNELS> getSingleSnapArray( );
		std::vector<float64> getSingleSnap( );
		void armAquisition( UINT numSnapshots );
		void getAquisitionData( );
		bool wantsQueryBetweenVariations( );
	private:
		Control<CStatic> title;
		std::array<Control<CStatic>, NUMBER_AI_CHANNELS> dacLabels;
		std::array<Control<CStatic>, NUMBER_AI_CHANNELS> voltDisplays;
		Control<CButton> getValuesButton;
		Control<CButton> continuousQueryCheck;
		Control<CButton> queryBetweenVariations;
		// float64 should just be a double type.
		std::array<float64, NUMBER_AI_CHANNELS> currentValues;
		std::vector<float64> aquisitionData;
		TaskHandle analogInTask0 = NULL;
		DaqMxFlume daqmx;
};



