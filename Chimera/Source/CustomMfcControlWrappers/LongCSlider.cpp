// created by Mark Brown
#include "stdafx.h"
#include "LongCSlider.h"
#include "boost/lexical_cast.hpp"

void LongCSlider::reposition (POINT loc, LONG totalHeight){
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
	if (!header || !edit || !slider){
		return;
	}
	header->setVisible (hideornot);
	edit->setVisible (hideornot);
	slider->setVisible (hideornot);
}

unsigned LongCSlider::getEditId ( ){
	return NULL;
}

void LongCSlider::initialize ( POINT& loc, IChimeraQtWindow* parent, int width, int height, std::string headerText ){
	header = new  QLabel (headerText.c_str (), parent);
	header->setGeometry (loc.x, loc.y, 25, 20);

	edit = new CQLineEdit (parent);
	edit->setGeometry (loc.x, loc.y+20, 25, 20);
	parent->connect (edit, &QLineEdit::textChanged, 
		[this, parent]() {
			try {
				handleEdit ();
				parent->configUpdated ();
			}
			catch (ChimeraError& err) {
				parent->reportErr (err.qtrace ());
			} 
		});
	edit->setStyleSheet ("Font : 6pt");
	if ( height < 40 ){
		thrower ( "ERROR: Must initialize LongCSLider with a height greater than 40 to have room for edit and header controls!" );
	}
	slider = new QSlider (parent);
	slider->setGeometry (loc.x, loc.y + 40, 25, height - 40);
	slider->setRange ( minVal, maxVal );
	slider->setSingleStep (1);
	parent->connect (slider, &QSlider::valueChanged, [this, parent](int value) {
		handleSlider (value); 
		parent->configUpdated ();
		});
}

int LongCSlider::getSliderId ( ){
	return NULL;
}

double LongCSlider::getValue ( ){
	return currentValue;
}

void LongCSlider::setValue ( int value, bool updateEdit ){
	if ( value < minVal ){
		thrower ( "Tried to set slider value below minimum value of " + str ( minVal ) + "!" );
	}
	else if ( value > maxVal ){
		thrower ( "Tried to set slider value above maximum value of " + str ( maxVal ) + "!" );
	}
	currentValue = value; 
	if ( updateEdit ){
		edit->setText ( cstr ( value ) );
	}
	slider->setSliderPosition ( value );
}

void LongCSlider::handleEdit (){
	int val;
	try	{
		val = int(boost::lexical_cast<int>( str ( edit->text() ) ));
	}
	catch ( boost::bad_lexical_cast& ){
		thrower( "Please enter a number." );
	}
	if ( val < minVal || val > maxVal ){
		thrower( "Please enter a number within the slider's range ("+str(minVal) + "," + str(maxVal) + ")" );
	}
	if ( val != currentValue ){
		setValue ( val, false );
	}
}

void LongCSlider::handleSlider ( int nPos ){
	setValue (nPos);
}

