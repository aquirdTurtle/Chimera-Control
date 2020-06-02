// created by Mark Brown
#include "stdafx.h"
#include "LongCSlider.h"
#include "boost/lexical_cast.hpp"


void LongCSlider::reposition (POINT loc, LONG totalHeight)
{
	if (!header || !edit || !slider){
		return;
	}
	header->setGeometry (loc.x, loc.y, 25, 20);
	header->raise ();
	edit->setGeometry (loc.x, loc.y + 20, 25, 20);
	edit->raise ();
	slider->setGeometry (loc.x, loc.y + 40, 25, totalHeight - 40);
	slider->raise ();
}

void LongCSlider::hide ( int hideornot ) {
	if (!header || !edit || !slider)
	{
		return;
	}
	header->setVisible (hideornot);
	edit->setVisible (hideornot);
	slider->setVisible (hideornot);
}

UINT LongCSlider::getEditId ( )
{
	return NULL;
}

void LongCSlider::initialize ( POINT& loc, IChimeraWindowWidget* parent, int width, int height, std::string headerText )
{
	header = new  QLabel (headerText.c_str (), parent);
	header->setGeometry (loc.x, loc.y, 25, 20);

	edit = new QLineEdit (parent);
	edit->setGeometry (loc.x, loc.y+20, 25, 20);
	parent->connect (edit, &QLineEdit::textChanged, 
		[this, parent]() {
			try {
				handleEdit ();
				parent->configUpdated ();
			}
			catch (Error& err) {
				parent->reportErr (err.trace ());
			} 
		});

	if ( height < 40 )
	{
		thrower ( "ERROR: Must initialize LongCSLider with a height greater than 40 to have room for edit and header controls!" );
	}
	slider = new QSlider (parent);
	slider->setGeometry (loc.x, loc.y + 40, 25, height - 40);
	slider->setRange ( 0, 1024 );
	slider->setSingleStep (1);
}

int LongCSlider::getSliderId ( )
{
	return NULL;
}

void LongCSlider::rearrange ( int width, int height, fontMap fonts ) {}

double LongCSlider::getValue ( )
{
	return currentValue;
}

void LongCSlider::setValue ( double value, bool updateEdit )
{
	if ( value < minVal )
	{
		thrower ( "Tried to set slider value below minimum value of " + str ( minVal ) + "!" );
	}
	else if ( value > maxVal )
	{
		thrower ( "Tried to set slider value above maximum value of " + str ( maxVal ) + "!" );
	}
	currentValue = value; 
	if ( updateEdit )
	{
		edit->setText ( cstr ( value ) );
	}
	double p = ( value - minVal ) / ( maxVal - minVal );
	int setP = int(p * 1024);
	slider->setSliderPosition ( setP );
	
}

void LongCSlider::handleEdit ()
{
	int val;
	try
	{
		val = int(boost::lexical_cast<double>( str ( edit->text() ) ));
	}
	catch ( boost::bad_lexical_cast& )
	{
		thrower( "Please enter a number." );
	}
	if ( val < minVal || val > maxVal )
	{
		thrower( "Please enter a number within the slider's range ("+str(minVal) + "," + str(maxVal) + ")" );
	}
	if ( val != currentValue )
	{
		setValue ( val, false );
	}
}

void LongCSlider::handleSlider ( UINT nPos )
{
	double p = nPos / 1024.0;
	int value = int(p * ( maxVal - minVal ) + minVal);
	setValue ( value );
}

