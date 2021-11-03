#include "stdafx.h"
#include "DoCore.h"
#include "DoStructures.h"

DoCore::DoCore (bool ftSafemode, bool serialSafemode) : ftFlume (ftSafemode),	winSerial (serialSafemode, ""), 
														names(4, 16){
	try	{
		connectType = ftdiConnectionOption::Async;
		ftdi_connectasync ("FT2E722BB"); // This is the address for the do system, hard-coded for the moment it looks 
		// like. Not sure why this isn't a "normal" usb address, but I do remember this from before, and am almost sure 
		// this is associated with the board itself and not the connection to the computer, like com-ports typically are.
	}
	catch (ChimeraError &){
		throwNested ("Failed to initialize DO Core!?!");
	}
}

DoCore::~DoCore () { ftdi_disconnect (); }

void DoCore::setNames (Matrix<std::string> namesIn){
	names = namesIn;
}

void DoCore::ftdi_connectasync (const char devSerial[]){
	if (ftFlume.getNumDevices () <= 0){
		thrower ("No devices found.");
	}
	ftFlume.open (devSerial);
	ftFlume.setUsbParams ();
	connectType = ftdiConnectionOption::Async;
}

void DoCore::ftdi_disconnect (){
	if (connectType == ftdiConnectionOption::Serial){
		winSerial.close ();
	}
	else if (connectType == ftdiConnectionOption::Async){
		ftFlume.close ();
	}
	else{
		thrower ("No connection to close...");
	}
	connectType = ftdiConnectionOption::None;
}


unsigned long DoCore::ftdi_trigger (){
	std::vector<unsigned char> dataBuffer = { 161, 0, 0, 0, 0, 0, 1 };
	if (connectType == ftdiConnectionOption::Serial){
		unsigned long totalBytesSent = 0;
		while (totalBytesSent < 7){
			winSerial.write (std::string (dataBuffer.begin (), dataBuffer.end ()));
		}
		return totalBytesSent;
	}
	else if (connectType == ftdiConnectionOption::Async){
		return ftFlume.trigger ();
	}
	return 0;
}


/*
* Takes data from "mem" structure and writes to the dio board.
*/
unsigned long DoCore::ftdi_write (unsigned variation, bool loadSkip){
	if (connectType == ftdiConnectionOption::Serial || connectType == ftdiConnectionOption::Async){
		auto& buf = loadSkip ? finFtdiBuffers_loadSkip (variation) : finFtdiBuffers (variation);
		// please note that Serial mode has not been thoroughly tested (by me, MOB at least)!
		bool proceed = true;
		int count = 0;
		int idx = 0;
		unsigned int totalBytes = 0;
		unsigned int number = 0;
		unsigned long dwNumberOfBytesSent = 0;
		if (connectType == ftdiConnectionOption::Serial){
			while (dwNumberOfBytesSent < buf.bytesToWrite){
				/*auto bytesWritten = winSerial.writeFile( dwNumberOfBytesSent, buf.pts );
				if ( bytesWritten > 0 )
				{
					++totalBytes;
				}
				else
				{
					thrower ( "bad value for dwNumberOfBytesWritten: " + str( bytesWritten ) );
				}*/
			}
			totalBytes += dwNumberOfBytesSent;
		}
		else{
			totalBytes += ftFlume.write (buf.pts, buf.bytesToWrite);
		}
		return totalBytes;
	}
	else{
		thrower ("No ftdi connection exists! Can't write without a connection.");
	}
	return 0;
}

void DoCore::fillFtdiDataBuffer (std::vector<unsigned char>& dataBuffer, unsigned offset, unsigned count, ftdiPt pt){
	if (offset + 20 >= dataBuffer.size ()){
		thrower ("tried to write data buffer out of bounds!");
	}

	dataBuffer[offset] = WBWRITE;
	dataBuffer[offset + 1] = (((TIMEOFFS + count) >> 8) & 0xFF);
	dataBuffer[offset + 2] = ((TIMEOFFS + count) & 0xFF);
	dataBuffer[offset + 3] = (((pt.time) >> 24) & 0xFF);
	dataBuffer[offset + 4] = (((pt.time) >> 16) & 0xFF);
	dataBuffer[offset + 5] = (((pt.time) >> 8) & 0xFF);
	dataBuffer[offset + 6] = ((pt.time) & 0xFF);

	dataBuffer[offset + 7] = WBWRITE;
	dataBuffer[offset + 8] = (((BANKAOFFS + count) >> 8) & 0xFF);
	dataBuffer[offset + 9] = ((BANKAOFFS + count) & 0xFF);
	dataBuffer[offset + 10] = pt.pts[0];
	dataBuffer[offset + 11] = pt.pts[1];
	dataBuffer[offset + 12] = pt.pts[2];
	dataBuffer[offset + 13] = pt.pts[3];

	dataBuffer[offset + 14] = WBWRITE;
	dataBuffer[offset + 15] = (((BANKBOFFS + count) >> 8) & 0xFF);
	dataBuffer[offset + 16] = ((BANKBOFFS + count) & 0xFF);
	dataBuffer[offset + 17] = pt.pts[4];
	dataBuffer[offset + 18] = pt.pts[5];
	dataBuffer[offset + 19] = pt.pts[6];
	dataBuffer[offset + 20] = pt.pts[7];
}

void DoCore::convertToFinalFtdiFormat (unsigned variation){
	for (auto loadSkip : { false, true }){
		// first convert from diosnapshot to ftdi snapshot
		auto& snaps = loadSkip ? ftdiSnaps_loadSkip (variation) : ftdiSnaps (variation);
		auto& buf = loadSkip ? finFtdiBuffers_loadSkip (variation) : finFtdiBuffers (variation);
		// please note that Serial mode has not been thoroughly tested (by me, MOB at least)!
		auto bufSize = (connectType == ftdiConnectionOption::Serial ? DIO_BUFFERSIZESER : DIO_BUFFERSIZEASYNC);
		buf.pts = std::vector<unsigned char> (bufSize * DIO_MSGLENGTH * DIO_WRITESPERDATAPT, 0);
		bool proceed = true;
		int count = 0;
		unsigned int totalBytes = 0;
		buf.bytesToWrite = 0;
		unsigned int number = 0;
		while ((number < bufSize) && proceed){
			unsigned offset = DIO_WRITESPERDATAPT * number * DIO_MSGLENGTH;
			fillFtdiDataBuffer (buf.pts, offset, count, snaps[count]);
			if (snaps[count] == ftdiPt ({ 0,0,0,0,0,0,0,0,0 }) && number != 0){
				proceed = false;//this is never false since we reach 43008 size?
			}
			if (count == NUMPOINTS){
				thrower ("Non-Terminated table, data was filled all the way to end of data array... "
					"Unit will not work right..., last element of data should be all zeros.");
			}
			number++;
			count++;
			buf.bytesToWrite += DIO_WRITESPERDATAPT * DIO_MSGLENGTH;
		}
	}
}


std::array< std::array<bool, 16>, 4 > DoCore::getFinalSnapshot (){
	auto numVar = ttlSnapshots.getNumVariations ();
	if (numVar > 0){
		if (ttlSnapshots (numVar - 1).size () > 0){
			return ttlSnapshots ( numVar - 1 ).back ().ttlStatus;
		}
	}
	thrower ("Attempted to get final snapshot from dio system but no snapshots!");
}


std::string DoCore::getDoSystemInfo (){
	unsigned numDev;
	std::string msg = "";
	try{
		numDev = ftFlume.getNumDevices ();
		msg += "Number ft devices: " + str (numDev) + "\n";
	}
	catch (ChimeraError & err){
		msg += "Failed to Get number ft Devices! Error was: " + err.trace ();
	}
	msg += ftFlume.getDeviceInfoList ();
	return msg;
}

/* mostly if not entirely used for setting dacs */
void DoCore::standardNonExperimentStartDoSequence (DoSnapshot initSnap){
	organizeTtlCommands (0, initSnap);
	findLoadSkipSnapshots (0, std::vector<parameterType> (), 0);
	convertToFtdiSnaps (0);
	convertToFinalFtdiFormat (0);
	ftdi_write (0, false);
	ftdi_trigger ();
	FtdiWaitTillFinished (0);
}

void DoCore::initializeDataObjects (unsigned variationNum){
	ttlCommandFormList = std::vector<DoCommandForm>();
	ttlCommandList.uniformSizeReset (variationNum);
	ttlSnapshots.uniformSizeReset (variationNum);
	loadSkipTtlSnapshots.uniformSizeReset (variationNum);
	ftdiSnaps.uniformSizeReset (variationNum);
	ftdiSnaps_loadSkip.uniformSizeReset (variationNum);
	finFtdiBuffers.uniformSizeReset (variationNum);
	finFtdiBuffers_loadSkip.uniformSizeReset (variationNum);
}


void DoCore::ttlOn (unsigned row, unsigned column, timeType time){
	ttlCommandFormList.push_back ({ {row, column}, time, {}, true });
}


void DoCore::ttlOff (unsigned row, unsigned column, timeType time){
	ttlCommandFormList.push_back ({ {row, column}, time, {}, false });
}


void DoCore::ttlOnDirect (unsigned row, unsigned column, double timev, unsigned variation){
	DoCommand command;
	command.line = { row, column };
	command.time = timev;
	command.value = true;
	ttlCommandList (variation).push_back (command);
}


void DoCore::ttlOffDirect (unsigned row, unsigned column, double timev, unsigned variation){
	DoCommand command;
	command.line = { row, column };
	command.time = timev;
	command.value = false;
	ttlCommandList (variation).push_back (command);
}


void DoCore::restructureCommands (){
	/* this is to be done after key interpretation. */
	if (ttlCommandFormList.size () == 0){
		thrower ("No TTL Commands???");
	}
	ttlCommandList.resizeVariations (ttlCommandFormList[0].timeVals.size ());
	for (auto varInc : range (ttlCommandList.getNumVariations ())){
		for (auto& cmd : ttlCommandFormList){
			DoCommand nCmd;
			nCmd.line = cmd.line;
			nCmd.time = cmd.timeVals[varInc];
			nCmd.value = cmd.value;
			ttlCommandList (varInc).push_back (nCmd);
		}
	}
}



void DoCore::sizeDataStructures (unsigned variations){
	/// imporantly, this sizes the relevant structures.
	ttlSnapshots.uniformSizeReset (variations);
	ttlCommandList.uniformSizeReset (variations);
	loadSkipTtlSnapshots.uniformSizeReset (variations);
	ftdiSnaps.uniformSizeReset (variations);
	finFtdiBuffers.uniformSizeReset (variations);
	ftdiSnaps_loadSkip.uniformSizeReset (variations);
	finFtdiBuffers_loadSkip.uniformSizeReset (variations);
}


/*
 * Read key values from variables and convert command form to the final commands.
 */
void DoCore::calculateVariations (std::vector<parameterType>& params, ExpThreadWorker* threadworker){
	unsigned variations = params.size () == 0 ? 1 : params.front ().keyValues.size ();
	if (variations == 0){
		variations = 1;
	}
	sizeDataStructures (variations);
	// and interpret the command list for each variation.
	for (auto& dioCommandForm : ttlCommandFormList){
		dioCommandForm.timeVals.resize (variations);
	}
	for (auto variationNum : range (variations)){
		for (auto& dioCommandForm : ttlCommandFormList){
			double variableTime = 0;
			// add together current values for all variable times.
			if (dioCommandForm.time.first.size () != 0){
				for (auto varTime : dioCommandForm.time.first){
					variableTime += varTime.evaluate (params, variationNum);
				}
			}
			dioCommandForm.timeVals[variationNum] = variableTime + dioCommandForm.time.second;
		}
	}
	restructureCommands ();
}

std::vector<double> DoCore::getFinalTimes ()
{
	std::vector<double> finTimes;
	finTimes.resize (ttlSnapshots.getNumVariations ());
	for (auto varNum : range (ttlSnapshots.getNumVariations ())){
		finTimes[varNum] = ttlSnapshots (varNum).back ().time;
	}
	return finTimes;
}


std::vector<std::vector<plotDataVec>> DoCore::getPlotData (unsigned variation){
	std::vector<std::vector<plotDataVec>> ttlData(4, std::vector<plotDataVec>(16));
	std::string message;
	if (ttlSnapshots.getNumVariations () <= variation) {
		thrower ("Attempted to retrieve ttl data from variation " + str (variation) + ", which does not "
			"exist in the dio code object!");
	}
	// each element of ttlData should be one ttl line.
	unsigned linesPerPlot = 64 / ttlData.size ();
	for (auto line : range (64)) {
		auto& data = ttlData[line / linesPerPlot][line % linesPerPlot];
		data.clear ();
		for (auto snapn : range(ttlSnapshots (variation).size())){
			if (snapn != 0){
				data.push_back ({ ttlSnapshots (variation)[snapn].time,
								  double (ttlSnapshots (variation)[snapn-1].ttlStatus[line / 16][line % 16]), 0 });
			}
			data.push_back ({ ttlSnapshots (variation)[snapn].time, 
							  double (ttlSnapshots (variation)[snapn].ttlStatus[line / 16][line % 16]), 0 });
		}
	}
	return ttlData;
}

/// DIO64 Wrapper functions that I actually use
std::string DoCore::getTtlSequenceMessage (unsigned variation)
{
	std::string message;

	if (ttlSnapshots.getNumVariations () <= variation)
	{
		thrower ("Attempted to retrieve ttl sequence message from snapshot " + str (variation) + ", which does not "
			"exist!");
	}
	for (auto snap : ttlSnapshots (variation))
	{
		message += str (snap.time) + ":\n";
		int rowInc = 0;
		for (auto row : snap.ttlStatus)
		{
			switch (rowInc)
			{
			case 0:
				message += "A: ";
				break;
			case 1:
				message += "B: ";
				break;
			case 2:
				message += "C: ";
				break;
			case 3:
				message += "D: ";
				break;
			}
			rowInc++;
			for (auto num : row)
			{
				message += str (num) + ", ";
			}
			message += "\r\n";
		}
		message += "\r\n---\r\n";
	}
	return message;
}

// counts the number of triggers on a given line.
// which.first = row, which.second = number.
unsigned DoCore::countTriggers (std::pair<DoRows::which, unsigned> which, unsigned variation)
{
	auto& snaps = ttlSnapshots (variation);
	unsigned count = 0;
	if (snaps.size () == 0)
	{
		return 0;
		//thrower ( "No ttl events to examine in countTriggers?" );
	}
	for (auto snapshotInc : range (ttlSnapshots (variation).size () - 1))
	{
		// count each rising edge. Also count if the first snapshot is high. 
		if ((snaps[snapshotInc].ttlStatus[int (which.first)][which.second] == false
			&& snaps[snapshotInc + 1].ttlStatus[int (which.first)][which.second] == true)
			|| (snaps[snapshotInc].ttlStatus[int (which.first)][which.second] == true
				&& snapshotInc == 0))
		{
			count++;
		}
	}
	return count;
}


unsigned long DoCore::ftdi_ForceOutput (DoRows::which row, int number, int state, std::array<std::array<bool, 16>, 4> status)
{
	//outputs (number, row).set (state);
	resetTtlEvents ();
	initializeDataObjects (0);
	sizeDataStructures (1);
	ttlSnapshots (0).push_back ({ 0.00001, status });
	convertToFtdiSnaps (0);
	convertToFinalFtdiFormat (0);
	auto bytesWritten = ftdi_write (0, false);
	ftdi_trigger ();
	return bytesWritten;
}


void DoCore::findLoadSkipSnapshots (double time, std::vector<parameterType>& variables, unsigned variation)
{
	// find the splitting time and set the loadSkip snapshots to have everything after that time.
	auto& snaps = ttlSnapshots (variation);
	auto& loadSkipSnaps = loadSkipTtlSnapshots (variation);
	for (auto snapshotInc : range (ttlSnapshots (variation).size () - 1))
	{
		if (snaps[snapshotInc].time < time && snaps[snapshotInc + 1].time >= time)
		{
			loadSkipSnaps = std::vector<DoSnapshot> (snaps.begin () + snapshotInc + 1, snaps.end ());
			break;
		}
	}
	// need to zero the times.
	for (auto& snapshot : loadSkipSnaps)
	{
		snapshot.time -= time;
	}
}


void DoCore::convertToFtdiSnaps (unsigned variation)
{
	// formatting of these snaps is similar to the word formatting of the viewpoint dio64 card; the ttl on/off 
	int snapIndex = 0;
	int val1, val2, fpgaBankCtr;
	unsigned long timeConv = 100000;
	for (auto loadSkip : { false, true })
	{
		auto ttlSnaps = loadSkip ? loadSkipTtlSnapshots (variation) : ttlSnapshots (variation);
		auto& ftSnaps = loadSkip ? ftdiSnaps_loadSkip (variation) : ftdiSnaps (variation);
		for (auto snapshot : ttlSnaps)
		{
			ftdiPt pt;
			fpgaBankCtr = 0;
			for (auto rowOfTtls : snapshot.ttlStatus)
			{
				// currently this is split an awkward because the viewpoint organization was organized in sets of 16, not 8.
				// convert first 8 of snap shot to int
				val1 = 0;
				for (auto i : range (8))
				{
					val1 = val1 + pow (2, i) * rowOfTtls[i];
				}
				// convert next 8 of snap shot to int
				val2 = 0;
				for (auto j : range (8))
				{
					val2 = val2 + pow (2, j) * rowOfTtls[j + 8];
				}
				pt.pts[fpgaBankCtr++] = val1;
				pt.pts[fpgaBankCtr++] = val2;
			}
			pt.time = snapshot.time * timeConv;
			if (snapIndex > ftSnaps.size ())
			{
				thrower ("More than 2048 Variations attempted to write to ftdi snapshots object!");
			}
			ftSnaps[snapIndex] = pt;
			snapIndex++;
		}
		ftSnaps[snapIndex] = { 0,0,0,0,0,0,0,0,0 };
	}
}

ExpWrap<std::vector<DoSnapshot>> DoCore::getTtlSnapshots ()
{
	/* used in the unit testing suite */
	return ttlSnapshots;
}

ExpWrap<finBufInfo> DoCore::getFinalFtdiData ()
{
	return finFtdiBuffers;
}

ExpWrap<std::array<ftdiPt, 2048>> DoCore::getFtdiSnaps ()
{
	return ftdiSnaps;
}


void DoCore::organizeTtlCommands (unsigned variation, DoSnapshot initSnap)
{
	// each element of this is a different time (the double), and associated with each time is a vector which locates 
	// which commands were on at this time, for ease of retrieving all of the values in a moment.
	std::vector<std::pair<double, std::vector<unsigned short>>> timeOrganizer;
	std::vector<DoCommand> orderedCommandList (ttlCommandList (variation));
	// sort using a lambda. std::sort is effectively a quicksort algorithm.
	std::sort (orderedCommandList.begin (), orderedCommandList.end (),
		[variation](DoCommand a, DoCommand b) {return a.time < b.time; });
	/// organize all of the commands.
	for (auto commandInc : range (ttlCommandList (variation).size ()))
	{
		// because the events are sorted by time, the time organizer will already be sorted by time, and therefore I 
		// just need to check the back value's time. DIO64 uses a 10MHz clock, can do 100ns spacing, check diff 
		// threshold to extra room. If dt<1ns, probably just some floating point issue. 
		// If 1ns<dt<100ns I want to actually complain to the user since it seems likely that  this was intentional and 
		// not a floating error.
		if (commandInc == 0 || fabs (orderedCommandList[commandInc].time - timeOrganizer.back ().first) > 1e-6)
		{
			// new time
			std::vector<USHORT> testVec = { USHORT (commandInc) };
			timeOrganizer.push_back ({ orderedCommandList[commandInc].time, testVec });
		}
		else
		{
			// old time
			timeOrganizer.back ().second.push_back (commandInc);
		}
	}
	if (timeOrganizer.size () == 0)
	{
		thrower ("No ttl commands! The Ttl system is the master behind everything in a repetition, and so it "
			"must contain something.\r\n");
	}
	/// now figure out the state of the system at each time.
	auto& snaps = ttlSnapshots (variation);
	snaps.clear ();
	// start with the initial status.
	snaps.push_back (initSnap);
	///
	if (timeOrganizer[0].first != 0)
	{
		// then there were no commands at time 0, so just set the initial state to be exactly the original state before
		// the experiment started. I don't need to modify the first snapshot in this case, it's already set. Add a snapshot
		// here so that the thing modified is the second snapshot not the first. 
		snaps.push_back (initSnap);
	}

	// handle the zero case specially. This may or may not be the literal first snapshot.
	snaps.back ().time = timeOrganizer[0].first;
	for (auto zeroInc : range (timeOrganizer[0].second.size ()))
	{
		// make sure to address the correct ttl. the ttl location is located in individuaTTL_CommandList but you need 
		// to make sure you access the correct command.
		unsigned cmdNum = timeOrganizer[0].second[zeroInc];
		unsigned row = orderedCommandList[cmdNum].line.first;
		unsigned column = orderedCommandList[cmdNum].line.second;
		snaps.back ().ttlStatus[row][column] = orderedCommandList[cmdNum].value;
	}
	///
	// already handled the first case.
	for (unsigned commandInc = 1; commandInc < timeOrganizer.size (); commandInc++)
	{
		// first copy the last set so that things that weren't changed remain unchanged.
		snaps.push_back (snaps.back ());
		snaps.back ().time = timeOrganizer[commandInc].first;
		for (auto cmdIndex : timeOrganizer[commandInc].second)
		{
			// see description of this command above... update everything that changed at this time.
			unsigned row = orderedCommandList[cmdIndex].line.first;
			unsigned column = orderedCommandList[cmdIndex].line.second;
			snaps.back ().ttlStatus[row][column] = orderedCommandList[cmdIndex].value;
		}
	}
	// phew. Check for good input by user:
	for (auto snapshot : snaps)
	{
		if (snapshot.time < 0)
		{
			thrower ("The code tried to set a ttl event at a negative time value! This is clearly not allowed."
				" Aborting.");
		}
	}
	/* Test to see if adding 1ms to the starting time would solve the problem of the initial state of ttl*/
	for (int i = 0; i < snaps.size (); i++) {
		snaps[i].time = snaps[i].time + 0.001;
	}
}


double DoCore::getFtdiTotalTime (unsigned variation)
{
	double time = -1;
	bool proceed = true;
	int counter = 0;
	for (auto snap : ftdiSnaps (variation))
	{
		if (snap == ftdiPt ({ 0, 0, 0, 0, 0, 0, 0, 0, 0 }) && time != -1 && counter >= 0 && proceed)
		{
			return (double (ftdiSnaps (variation)[--counter].time) / 100000);
			proceed = false;
		}
		time = snap.time;
		counter++;
	}
	return 0;
}


double DoCore::getTotalTime (unsigned variation)
{
	// ??? there used to be a +1 at the end of this...
	if (ftdiSnaps (variation).empty ()) { thrower ("nothing in ftdi snaps vector"); }
	else {
		return (ftdiSnaps (variation).back ().time
			+ 65535 * ftdiSnaps (variation).back ().time) / 10000.0 + 1;
	}
}

unsigned long DoCore::getNumberEvents (unsigned variation)
{
	return ttlSnapshots (variation).size ();
}

bool DoCore::getFtFlumeSafemode () { return ftFlume.getSafemodeSetting (); }
Matrix<std::string> DoCore::getAllNames () { return names; }
void DoCore::resetTtlEvents () { initializeDataObjects (0); }
void DoCore::wait2 (double time) { Sleep (time + 10); }
void DoCore::prepareForce () { initializeDataObjects (1); }

void DoCore::FtdiWaitTillFinished (unsigned variation){
	auto time = getFtdiTotalTime (variation);
	wait2 (time);
}

bool DoCore::isValidTTLName (std::string name){
	DoRows::which row;
	unsigned number;
	return getNameIdentifier (name, row, number) != -1;
}

/*
Returns a single number which corresponds to the dio control with the name
*/
int DoCore::getNameIdentifier (std::string name, DoRows::which& row, unsigned& number){
	for (auto rowInc : range(names.getRows())){
		for (auto numberInc : range (names.getCols())){
			std::string DioName = str (names (rowInc, numberInc), 13, false, true);
			// second of the || is standard name which is always acceptable.
			if (DioName == name || name == DoRows::toStr (DoRows::which(rowInc)) + str (numberInc)){
				row = DoRows::which(rowInc);
				number = numberInc;
				return int (rowInc) * names.getCols () + numberInc;
			}
		}
	}
	// else not a name.
	return -1;
}

void DoCore::handleTtlScriptCommand (std::string command, timeType time, std::string name, Expression pulseLength,
									 std::vector<parameterType>& vars, std::string scope){
	if (!isValidTTLName (name)){
		thrower ("the name " + name + " is not the name of a ttl!");
	}
	timeType pulseEndTime = time;
	unsigned collumn;
	DoRows::which row;
	getNameIdentifier (name, row, collumn);
	if (command == "on:"){
		ttlOn (int (row), collumn, time);
	}
	else if (command == "off:"){
		ttlOff (int (row), collumn, time);
	}
	else if (command == "pulseon:" || command == "pulseoff:"){
		try	{
			pulseEndTime.second += pulseLength.evaluate ();
		}
		catch (ChimeraError&){
			pulseLength.assertValid (vars, scope);
			pulseEndTime.first.push_back (pulseLength);
		}
		if (command == "pulseon:"){
			ttlOn (int (row), collumn, time);
			ttlOff (int (row), collumn, pulseEndTime);
		}
		if (command == "pulseoff:"){
			ttlOff (int (row), collumn, time);
			ttlOn (int (row), collumn, pulseEndTime);
		}
	}
}

void DoCore::handleTtlScriptCommand (std::string command, timeType time, std::string name, 
	std::vector<parameterType>& vars, std::string scope){
	// use an empty expression.
	handleTtlScriptCommand (command, time, name, Expression (), vars, scope);
}

void DoCore::standardExperimentPrep (unsigned variationInc, double currLoadSkipTime, std::vector<parameterType>& expParams){
	organizeTtlCommands (variationInc);
	findLoadSkipSnapshots (currLoadSkipTime, expParams, variationInc);
	convertToFtdiSnaps (variationInc);
	convertToFinalFtdiFormat (variationInc);
}