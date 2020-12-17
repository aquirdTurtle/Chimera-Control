// created by Mark Brown
#include "stdafx.h"
#include "LongQSlider.h"
#include "boost/lexical_cast.hpp"

void LongQSlider::reposition (QPoint loc, long totalHeight){
	if (!header || !edit || !slider){
		return;
	}
	auto& px = loc.rx (), & py = loc.ry ();
	header->setGeometry (px, py, 25, 20);
	header->raise ();
	edit->setGeometry (px, py + 20, 25, 20);
	edit->raise ();
	slider->setGeometry (px, py + 40, 25, totalHeight - 40);
	slider->raise ();
}

void LongQSlider::hide ( int hideornot ) {
	if (!header || !edit || !slider){
		return;
	}
	header->setVisible (hideornot);
	edit->setVisible (hideornot);
	slider->setVisible (hideornot);
}

unsigned LongQSlider::getEditId ( ){
	return 0;
}

void LongQSlider::initialize ( QPoint& loc, IChimeraQtWindow* parent, int width, int height, std::string headerText ){
	auto& px = loc.rx (), & py = loc.ry ();
	header = new  QLabel (headerText.c_str (), parent);
	header->setGeometry (px, py, 25, 20);

	edit = new CQLineEdit (parent);
	edit->setGeometry (px, py+20, 25, 20);
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
		thrower ( "ERROR: Must initialize LongQSLider with a height greater than 40 to have room for edit and header controls!" );
	}
	slider = new QSlider (parent);
	slider->setGeometry (px, py + 40, 25, height - 40);
	slider->setRange ( minVal, maxVal );
	slider->setSingleStep (1);
	parent->connect (slider, &QSlider::valueChanged, [this, parent](int value) {
		handleSlider (value); 
		parent->configUpdated ();
		});
}

int LongQSlider::getSliderId ( ){
	return 0;
}

double LongQSlider::getValue ( ){
	return currentValue;
}

void LongQSlider::setValue ( int value, bool updateEdit ){
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

void LongQSlider::handleEdit (){
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

void LongQSlider::handleSlider ( int nPos ){
	setValue (nPos);
}

