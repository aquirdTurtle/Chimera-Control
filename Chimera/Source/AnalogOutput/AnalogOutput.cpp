// created by Mark O. Brown
#include "stdafx.h"
#include "AnalogOutput.h"
#include "boost\lexical_cast.hpp"
#include <QlineEdit>
#include <QKeyEvent>

AnalogOutput::AnalogOutput( ){}

void AnalogOutput::initialize ( QPoint& pos, IChimeraQtWindow* parent, int whichDac) {
	auto& px = pos.rx (), & py = pos.ry ();
	label = new QLabel (cstr (whichDac), parent);
	label->setGeometry (QRect{ QPoint{px, py}, QPoint{px + 20, py + 20} });
	label->setToolTip ( (info.name + "\n" + info.note).c_str() );

	edit = new CQLineEdit ("0", parent);
	edit->setGeometry ({ QPoint{px + 20, py}, QPoint{px + 120, py += 20} });
	edit->setToolTip ( (info.name + "\r\n" + info.note).c_str() );
	edit->installEventFilter (parent);
	parent->connect (edit, &QLineEdit::textChanged, 		
		[this, parent]() {
			handleEdit ();
		});
	edit->setStyleSheet ("QLineEdit { border: none }");
}

bool AnalogOutput::eventFilter (QObject* obj, QEvent* event) {
	if (obj == edit) {
		if (event->type () == QEvent::KeyPress)
		{
			QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
			bool up = true;
			if (keyEvent->key () == Qt::Key_Up){
				up = true;
			}
			else if (keyEvent->key () == Qt::Key_Down){
				up = false;
			}
			else {
				return false;
			}
			auto txt = edit->text ();
			// make sure value in edit matches current value, else unclear what this functionality should do.
			double val;
			try	{
				val = boost::lexical_cast<double>(str(txt));
			}
			catch (boost::bad_lexical_cast)	{
				return true;
			}
			if (fabs (val - info.currVal) > 1e-12){
				return true;
			}
			auto decimalPos = txt.indexOf (".");
			auto cursorPos = edit->cursorPosition ();
			if (decimalPos == -1){
				// value is an integer, decimal is effectively at end.
				decimalPos = txt.size ();
			}
			// the order of the first digit
			double size = pow (10, decimalPos - 1);
			// handle the extra decimal character with the ternary operator here. 
			int editPlace = (cursorPos > decimalPos ? cursorPos - 1 : cursorPos);
			double inc = size / pow (10, editPlace);
			info.currVal += up ? inc : -inc;
			updateEdit (false);
			auto newTxt = edit->text (); 
			if (txt.indexOf ("-") == -1 && newTxt.indexOf("-") != -1){
				// then need to shift cursor to account for the negative.
				edit->setCursorPosition (cursorPos+1);
			}
			else if (txt.indexOf("-") != std::string::npos && newTxt.indexOf ("-") == std::string::npos){
				edit->setCursorPosition (cursorPos-1);
			}
			else {
				edit->setCursorPosition (cursorPos);
			}
			return true;
		}
		return false;
	}
	return false;
}

double AnalogOutput::getVal ( bool useDefault ){
	double val;
	try	{
		val = boost::lexical_cast<double>( str (edit->text () ) );
	}
	catch ( boost::bad_lexical_cast& ){
		if ( useDefault ){
			val = 0;
		}
		else{
			throwNested ( "value entered in DAC #" + str ( dacNum ) + " (" + str(edit->text())
						  + ") failed to convert to a double!" );
		}
	}
	return val;
}


void AnalogOutput::updateEdit ( bool roundToDacPrecision ){
	std::string valStr = roundToDacPrecision ? str ( roundToDacResolution ( info.currVal ), 13, true, false, true )
		: str ( info.currVal, 5, false, false, true );
	int pos = edit->cursorPosition ();
	edit->setText (cstr (valStr));
	edit->setCursorPosition (pos);
}

void AnalogOutput::setName ( std::string name ){
	if ( name == "" ){
		// no empty names allowed.
		return;
	}
	std::transform ( name.begin ( ), name.end ( ), name.begin ( ), ::tolower );
	info.name = name;
	edit->setToolTip ( cstr(info.name + "\r\n" + info.note));
}


void AnalogOutput::handleEdit ( bool roundToDacPrecision ){
	bool matches = false;
	try{
		if ( roundToDacPrecision ){
			double roundNum = roundToDacResolution ( info.currVal );
			if ( fabs ( roundToDacResolution ( info.currVal ) - boost::lexical_cast<double>( str(edit->text()) ) ) < 1e-8 )	{
				matches = true;
			}
		}
		else{
			if ( fabs ( info.currVal - boost::lexical_cast<double>( str (edit->text ()) ) ) < 1e-8 ){
				matches = true;
			}
		}
	}
	catch ( boost::bad_lexical_cast& ) { /* failed to convert to double. Effectively, doesn't match. */ }
}

double AnalogOutput::roundToDacResolution ( double num ){
	double dacResolution = 10.0 / pow ( 2, 16 );
	return long ( ( num + dacResolution / 2 ) / dacResolution ) * dacResolution;
}


void AnalogOutput::setNote ( std::string note ){
	info.note = note;
	edit->setToolTip ( (info.name + "\r\n" + info.note).c_str());
}

void AnalogOutput::disable ( ){
	edit->setEnabled (false);
}
