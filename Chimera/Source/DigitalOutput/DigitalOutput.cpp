#include "stdafx.h"
#include "DigitalOutput.h"


void DigitalOutput::updateStatus ( ){
	if ( check == nullptr )
	{
		return;
	}
	if (check->isChecked () != status)
	{
		check->setChecked (status);
	}
}


void DigitalOutput::initLoc ( unsigned numIn, DoRows::which rowIn ){
	row = rowIn;
	num = numIn;
}


void DigitalOutput::set ( bool stat ){
	status = stat;
	updateStatus ( );
}


bool DigitalOutput::getStatus ( ){
	return status;
}


std::pair<DoRows::which, unsigned> DigitalOutput::getPosition ( ){
	return { row, num };
}


void DigitalOutput::setName ( std::string nameStr ){
	check->setToolTip ( nameStr.c_str());
}

void DigitalOutput::setHoldStatus ( bool stat ){
	holdStatus = stat;
}

void DigitalOutput::initialize ( QPoint& pos, IChimeraQtWindow* parent ){
	check = new CQCheckBox ("", parent);
	check->setGeometry ({ QPoint{ long (pos.x()), long (pos.y()) }, QPoint{ long (pos.x() + 28), long (pos.y() + 28) } });
}


void DigitalOutput::enable ( bool enabledStatus ){
	check->setEnabled( enabledStatus );
}


DigitalOutput& allDigitalOutputs::operator()( unsigned num, DoRows::which row ){
	return core[ unsigned long ( row ) * 16L + num ];
}

allDigitalOutputs::allDigitalOutputs ( ){
	for ( auto row : DoRows::allRows )
	{
		for ( auto num : range ( numColumns ) )
		{
			(*this)( num, row ).initLoc ( num, row );
		}
	}
}
