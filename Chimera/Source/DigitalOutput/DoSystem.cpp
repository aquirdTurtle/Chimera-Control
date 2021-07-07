// created by Mark O. Brown
#include "stdafx.h"

#include "DoSystem.h"

#include "ConfigurationSystems/Version.h"
#include "LowLevel/constants.h"
#include "PrimaryWindows/QtAuxiliaryWindow.h"
#include "GeneralUtilityFunctions/range.h"

#include <sstream>
#include <unordered_map>
#include <bitset>
#include "nidaqmx2.h"
#include <boost/lexical_cast.hpp>

// I don't use this because I manually import dll functions.
// #include "Dio64.h"
DoSystem::DoSystem( IChimeraQtWindow* parent, bool ftSafemode, bool serialSafemode ) 
	: core(ftSafemode, serialSafemode), IChimeraSystem(parent){
	//for (auto& out : outputs) { out.set(0); }
}

DoSystem::~DoSystem() { }

void DoSystem::handleSaveConfig(ConfigStream& saveFile){
	/// ttl settings
	saveFile << "TTLS\n";
	// nothing at the moment.
	saveFile << "END_TTLS\n";
}

void DoSystem::handleOpenConfig(ConfigStream& openFile){
	if ( openFile.ver < Version ( "3.7" ) ){
		for ( auto i : range ( 64 ) ){
			// used to store an initial ttl config in the config file.
			std::string trash;
			openFile >> trash;
		}
	}
}


void DoSystem::setTtlStatusNoForceOut(std::array< std::array<bool, 16>, 4 > status){
	for ( auto rowInc : range(status.size()) ){
		for ( auto numInc : range(status[rowInc].size()) ){
			outputs ( numInc, DoRows::which ( rowInc ) ).set ( status[ rowInc ][ numInc ] );
		}
	}
}

Matrix<std::string> DoSystem::getAllNames(){
	return core.getAllNames ();
}

void DoSystem::updatePush( DoRows::which row, unsigned number ){
	outputs ( number, row ).updateStatus ( );
}

void DoSystem::handleInvert(){
	for ( auto& out : outputs ){
		// seems like I need a ! here...
		out.set (out.getStatus ()); 
		core.ftdi_ForceOutput (out.getPosition ().first, out.getPosition ().second, out.getStatus (), getCurrentStatus ());
	}
}

void DoSystem::updateDefaultTtl( DoRows::which row, unsigned column, bool state){
	outputs ( column, row ).defaultStatus = state;
}

bool DoSystem::getDefaultTtl( DoRows::which row, unsigned column){
	return outputs ( column, row ).defaultStatus;
}

std::pair<unsigned, unsigned> DoSystem::getTtlBoardSize(){
	return { outputs.numRows, outputs.numColumns };
}



void DoSystem::initialize( QPoint& loc, IChimeraQtWindow* parent ){
	auto& px = loc.rx (), & py = loc.ry ();
	// title
	ttlTitle = new QLabel ("DIGITAL OUTPUT SYSTEM", parent);
	ttlTitle->setGeometry (px, py, 480, 25);
	py += 25;
	// all number numberLabels
	ttlHold = new CQPushButton ("Hold Current Values", parent);
	ttlHold->setGeometry (px, py, 240, 20);
	ttlHold->setToolTip ("Press this button to change multiple TTLs simultaneously. Press the button, then change the "
		"ttls, then press the button again to release it. Upon releasing the button, the TTLs will change.");
	parent->connect (ttlHold, &QPushButton::clicked, [parent, this]() {
		try{
			parent->configUpdated ();
			handleHoldPress ();
			emit notification({ "Handling Hold Press.\n", 2, "DO_SYSTEM" });
		}
		catch (ChimeraError& exception){
			emit error ("TTL Hold Handler Failed: " + exception.qtrace () + "\n");
		}
	});
	ttlHold->setCheckable (true);

	zeroTtls = new CQPushButton ("Zero DOs", parent);
	zeroTtls->setGeometry (px + 240, py, 240, 20);
	zeroTtls->setToolTip( "Press this button to set all ttls to their zero (false) state." );
	parent->connect (zeroTtls, &QPushButton::released, [parent, this]() {
		try	{
			zeroBoard ();
			parent->configUpdated();
			emit notification({ "Zero'd DOs.\n",2, "DO_SYSTEM" });
		}
		catch (ChimeraError& exception) {
			emit notification({ "Failed to Zero DOs!!!\n",1, "DO_SYSTEM" });
			emit error(exception.qtrace ());
		}
	});
	py += 20;

	for (long ttlNumberInc : range (ttlNumberLabels.size ())) {
		ttlNumberLabels[ttlNumberInc] = new QLabel (cstr (ttlNumberInc), parent);
		ttlNumberLabels[ttlNumberInc]->setGeometry ({ QPoint (px + 32 + ttlNumberInc * 28, py),
													  QPoint (px + 32 + (ttlNumberInc + 1) * 28, py + 20) });
	}
	py += 20;
	// all row numberLabels
	for ( auto row : DoRows::allRows ) {
		ttlRowLabels[int (row)] = new QLabel ((DoRows::toStr (row)).c_str(), parent);
		ttlRowLabels[int (row)]->setGeometry (px, py + int (row) * 28, 32, 28);
	}
	// all push buttons
	unsigned runningCount = 0;
	auto startX = px + 32;
	for (auto row : DoRows::allRows ){
		px = startX;
		for (unsigned number = 0; number < outputs.numColumns; number++){
			auto& out = outputs (number, row);
			out.initialize ( loc, parent );
			parent->connect ( out.check, &QCheckBox::stateChanged, [this, &out, parent]() {
				try {
					handleTTLPress (out);
					emit notification({ "Handled DO " + qstr(DoRows::toStr(out.getPosition().first)) + ","
									  + qstr(out.getPosition().second) + " State Change.\n", 2, "DO_SYSTEM" });
					parent->configUpdated ();
				}
				catch (ChimeraError& exception)	{
					emit error ("DO Press Handler Failed.\n" + exception.qtrace () + "\n");
				}
			});
			px += 28;
		}
		py += 28;
	}
	px = startX - 32;
}

int DoSystem::getNumberOfTTLRows(){
	return outputs.numRows;
}

int DoSystem::getNumberOfTTLsPerRow(){
	return outputs.numColumns;
}

void DoSystem::handleTTLPress(DigitalOutput& out){
	
	if ( ! ttlHold->isDown() ){
		//out.set (!out.getStatus ());
		out.set (out.check->isChecked ());
		core.ftdi_ForceOutput(out.getPosition().first, out.getPosition().second, !out.getStatus(), getCurrentStatus ());
	}
	else{
		out.setHoldStatus ( !out.holdStatus );
	}
}

// this function handles when the hold button is pressed.
void DoSystem::handleHoldPress(){
	if (ttlHold->isChecked()){
		// set all the holds.
		//holdStatus = false;
		// make changes
		for ( auto& out : outputs )	{
			out.set ( out.holdStatus );
			core.ftdi_ForceOutput (out.getPosition ().first, out.getPosition ().second, out.getStatus (), getCurrentStatus());
		}
	}
	else{
		//holdStatus = true;
		for ( auto& out : outputs )	{
			out.setHoldStatus ( out.getStatus ( ) );
		}
	}
}

std::array< std::array<bool, 16>, 4 > DoSystem::getCurrentStatus( ){
	std::array< std::array<bool, 16>, 4 > currentStatus;
	for ( auto& out : outputs ){
		currentStatus[ int ( out.getPosition ( ).first ) ][ out.getPosition ( ).second ] = out.getStatus();
	}
	return currentStatus;
}

void DoSystem::setName( DoRows::which row, unsigned number, std::string name){
	if (name == ""){
		// no empty names allowed.
		return;
	}
	outputs ( number, row ).setName ( name );
	auto names = core.getAllNames ();
	names (unsigned(row), number) = name;
	core.setNames (names);
}

std::string DoSystem::getName( DoRows::which row, unsigned number){
	return core.getAllNames ()(unsigned (row), number);
}

bool DoSystem::getTtlStatus(DoRows::which row, int number){
	return outputs ( number, row ).getStatus ( );
}

allDigitalOutputs& DoSystem::getDigitalOutputs ( ){
	return outputs;
}

std::pair<USHORT, USHORT> DoSystem::calcDoubleShortTime( double time ){
	USHORT lowordTime, hiwordTime;
	// convert to system clock ticks. Assume that the crate is running on a 10 MHz signal, so multiply by
	// 10,000,000, but then my time is in milliseconds, so divide that by 1,000, ending with multiply by 10,000
	lowordTime = unsigned __int64( time * 10000 ) % 65535;
	hiwordTime = unsigned __int64( time * 10000 ) / 65535;
	if ( unsigned __int64( time * 10000 ) / 65535 > 65535 ){
		thrower ( "DIO system was asked to calculate a time that was too long! this is limited by the card." );
	}
	return { lowordTime, hiwordTime };
}

std::string DoSystem::getDoSystemInfo () {	return core.getDoSystemInfo (); }
bool DoSystem::getFtFlumeSafemode () { return core.getFtFlumeSafemode (); }

void DoSystem::zeroBoard( ){
	for ( auto& out : outputs ){
		out.set (0); 
		core.ftdi_ForceOutput (out.getPosition ().first, out.getPosition ().second, 0, getCurrentStatus ());	
	}
}

DoCore& DoSystem::getCore (){
	return core;
}

