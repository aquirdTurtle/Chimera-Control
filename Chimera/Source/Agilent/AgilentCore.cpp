#include "stdafx.h"
#include "AgilentCore.h"
#include "DigitalOutput/DoCore.h"
#include "Scripts/ScriptStream.h"
#include <ExperimentThread/ExpThreadWorker.h>
#include <ConfigurationSystems/ConfigSystem.h>
#include <AnalogInput/CalibrationManager.h>

AgilentCore::AgilentCore (const agilentSettings& settings) : 
	visaFlume (settings.safemode, settings.address),
	sampleRate (settings.sampleRate),
	initSettings (settings),
	triggerRow (settings.triggerRow),
	triggerNumber (settings.triggerNumber),
	memoryLoc (settings.memoryLocation),
	configDelim (settings.configurationFileDelimiter),
	agilentName (settings.deviceName),
	setupCommands (settings.setupCommands){
	calibrations[0].includesSqrt = false;
	calibrations[0].calibrationCoefficients = settings.calibrationCoeff;
	// could probably set these properly but in general wasn't doing this when was doing manual calibrations.
	calibrations[0].maxval = 1e6;
	calibrations[0].minval = -1e6;
	calibrations[1].includesSqrt = false;
	calibrations[1].calibrationCoefficients = settings.calibrationCoeff;

	try{
		visaFlume.open ();
	}
	catch (ChimeraError& err){
		errBox("Error seen while opening connection to " + agilentName + " Agilent:" + err.trace());
	}
}

AgilentCore::~AgilentCore (){
	visaFlume.close ();
}

void AgilentCore::initialize (){
	try{
		deviceInfo = visaFlume.identityQuery ();
		isConnected = true;
	}
	catch (ChimeraError&){
		deviceInfo = "Disconnected";
		isConnected = false;
	}

}

std::pair<DoRows::which, unsigned> AgilentCore::getTriggerLine (){
	return { triggerRow, triggerNumber };
}

std::string AgilentCore::getDeviceInfo (){
	return deviceInfo;
}

void AgilentCore::analyzeAgilentScript ( scriptedArbInfo& infoObj, std::vector<parameterType>& variables, 
										 std::string& warnings ){
	ScriptStream stream;
	ExpThreadWorker::loadAgilentScript (infoObj.fileAddress.expressionStr, stream);
	int currentSegmentNumber = 0;
	infoObj.wave.resetNumberOfTriggers ();
	// Procedurally readbtn lines into segment objects.
	while (!stream.eof ()){
		int leaveTest;
		try	{
			leaveTest = infoObj.wave.analyzeAgilentScriptCommand (currentSegmentNumber, stream, variables, warnings);
		}
		catch (ChimeraError&){
			throwNested ("Error seen while analyzing agilent script command for agilent " + this->configDelim);
		}
		if (leaveTest < 0){
			thrower ("IntensityWaveform.analyzeAgilentScriptCommand threw an error! Error occurred in segment #"
				+ str (currentSegmentNumber) + ".");
		}
		if (leaveTest == 1){
			// readbtn function is telling this function to stop reading the file because it's at its end.
			break;
		}
		currentSegmentNumber++;
	}
}

std::string AgilentCore::getDeviceIdentity (){
	std::string msg;
	try{
		msg = visaFlume.identityQuery ();
	}
	catch (ChimeraError & err){
		msg == err.trace ();
	}
	if (msg == ""){
		msg = "Disconnected...\n";
	}
	return msg;
}


void AgilentCore::setAgilent (unsigned var, std::vector<parameterType>& params, deviceOutputInfo runSettings, 
							  ExpThreadWorker* expWorker){
	if (!connected ()){
		return;
	}
	auto notify = expWorker != nullptr // if in expworker, emit notification, else no way to notify currently. 
		? std::function{ [expWorker] (QString note, unsigned debugLevel) {emit expWorker->notification (note, debugLevel); } }
		: std::function{ [expWorker] (QString note, unsigned debugLevel) { } };
	try{
		notify ("Writing Agilent output sync option: " + qstr (runSettings.synced), 2);
		visaFlume.write ("OUTPut:SYNC " + str (runSettings.synced));
	}
	catch (ChimeraError&){
		//errBox ("Failed to set agilent output synced?!");
	}
	for (auto chan : range (unsigned (2)))	{
		auto& channel = runSettings.channel[chan];
		auto stdNote = qstr ("Programming Agilent " + agilentName + " Channel " + str (chan) + " ");
		try{
			switch (channel.option){
				case AgilentChannelMode::which::No_Control:
					break;
				case AgilentChannelMode::which::Output_Off:
					notify (stdNote + "Output off.\n",1);
					outputOff (chan + 1);
					break;
				case AgilentChannelMode::which::DC:
					notify (stdNote + " DC Voltage.\n", 1);
					setDC (chan + 1, channel.dc, var);
					break;
				case AgilentChannelMode::which::Sine:
					notify (stdNote + " Sine Wave.\n", 1);
					setSine (chan + 1, channel.sine, var);
					break;
				case AgilentChannelMode::which::Square:
					notify (stdNote + " Square Wave.\n", 1);
					setSquare (chan + 1, channel.square, var);
					break;
				case AgilentChannelMode::which::Preloaded:
					notify (stdNote + " Preloaded Wave.\n", 1);
					setExistingWaveform (chan + 1, channel.preloadedArb);
					break;
				case AgilentChannelMode::which::Script:
					notify (stdNote + " Script \"" + qstr(channel.scriptedArb.fileAddress) + "\"\n", 1);
					handleScriptVariation (var, channel.scriptedArb, chan + 1, params);
					setScriptOutput (var, channel.scriptedArb, chan + 1);
					break;
				default:
					thrower ("Unrecognized channel " + str (chan) + " setting?!?!?!: "
							 + AgilentChannelMode::toStr (channel.option));
			}
		}
		catch (ChimeraError & err){
			throwNested ("Error seen while programming agilent output for " + configDelim + " agilent channel "
				+ str (chan + 1) + ": " + err.whatBare ());
		}
	}
}


// stuff that only has to be done once.
void AgilentCore::prepAgilentSettings (unsigned channel){
	if (channel != 1 && channel != 2){
		thrower ("Bad value for channel in prepAgilentSettings! Channel shoulde be 1 or 2.");
	}
	// Set timout, sample rate, filter parameters, trigger settings.
	visaFlume.setAttribute (VI_ATTR_TMO_VALUE, 40000);
	visaFlume.write ("SOURCE1:FUNC:ARB:SRATE " + str (sampleRate));
	visaFlume.write ("SOURCE2:FUNC:ARB:SRATE " + str (sampleRate));
}

/*
 * This function tells the agilent to use sequence # (varNum) and sets settings correspondingly.
 */
void AgilentCore::setScriptOutput (unsigned varNum, scriptedArbInfo scriptInfo, unsigned chan){
	if (scriptInfo.wave.isVaried () || varNum == 0){
		prepAgilentSettings (chan);
		// check if effectively dc
		if (scriptInfo.wave.minsAndMaxes.size () == 0){
			thrower ("script wave min max size is zero???");
		}
		auto& minMaxs = scriptInfo.wave.minsAndMaxes[varNum];
		if (fabs (minMaxs.first - minMaxs.second) < 1e-6){
			dcInfo tempDc;
			tempDc.dcLevel = str(minMaxs.first);
			tempDc.dcLevel.internalEvaluate (std::vector<parameterType>(), 1);
			tempDc.useCal = scriptInfo.useCal;
			setDC (chan, tempDc, 0);
		}
		else{
			auto schan = "SOURCE" + str (chan);
			// Load sequence that was previously loaded.
			visaFlume.write ("MMEM:LOAD:DATA" + str (chan) + " \"" + memoryLoc + ":\\sequence" + str (varNum) + ".seq\"");
			visaFlume.write (schan + ":FUNC ARB");
			visaFlume.write (schan + ":FUNC:ARB \"" + memoryLoc + ":\\sequence" + str (varNum) + ".seq\"");
			// set the offset and then the low & high. this prevents accidentally setting low higher than high or high 
			// higher than low, which causes agilent to throw annoying errors.
			visaFlume.write (schan + ":VOLT:OFFSET " + str ((minMaxs.first + minMaxs.second) / 2) + " V");
			visaFlume.write (schan + ":VOLT:LOW " + str (minMaxs.first) + " V");
			visaFlume.write (schan + ":VOLT:HIGH " + str (minMaxs.second) + " V");
			visaFlume.write ("OUTPUT" + str (chan) + " ON");
		}
	}
}


void AgilentCore::outputOff (int channel){
	if (channel != 1 && channel != 2){
		thrower ("bad value for channel inside outputOff! Channel shoulde be 1 or 2.");
	}
	channel++;
	visaFlume.write ("OUTPUT" + str (channel) + " OFF");
}


bool AgilentCore::connected (){
	return isConnected;
}


void AgilentCore::setDC (int channel, dcInfo info, unsigned var){
	if (channel != 1 && channel != 2){
		thrower ("Bad value for channel inside setDC! Channel shoulde be 1 or 2.");
	}
	try {
		visaFlume.write ("SOURce" + str (channel) + ":APPLy:DC DEF, DEF, "
			+ str (convertPowerToSetPoint (info.dcLevel.getValue (var), info.useCal, calibrations[channel - 1])) + " V");
	}
	catch (ChimeraError&) {
		throwNested ("Seen while programming DC for channel " + str (channel) + " (1-indexed).");
	}
}


void AgilentCore::setExistingWaveform (int channel, preloadedArbInfo info){
	if (channel != 1 && channel != 2){
		thrower ("Bad value for channel in setExistingWaveform! Channel shoulde be 1 or 2.");
	}
	auto sStr = "SOURCE" + str (channel);
	visaFlume.write (sStr + ":DATA:VOL:CLEAR");
	// Load sequence that was previously loaded.
	visaFlume.write ("MMEM:LOAD:DATA \"" + info.address.expressionStr + "\"");
	// tell it that it's outputting something arbitrary (not sure if necessary)
	visaFlume.write (sStr + ":FUNC ARB");
	// tell it what arb it's outputting.
	visaFlume.write (sStr + ":FUNC:ARB \"" + memoryLoc + ":\\" + info.address.expressionStr + "\"");
	programBurstMode (channel, info.burstMode);
	visaFlume.write ("OUTPUT" + str (channel) + " ON");
}


void AgilentCore::programBurstMode (int channel, bool burstOption) {
	auto sStr = "SOURCE" + str (channel);
	if (burstOption) {
		// not really bursting... but this allows us to reapeat on triggers. Might be another way to do this.
		visaFlume.write (sStr + ":BURST::MODE TRIGGERED");
		visaFlume.write (sStr + ":BURST::NCYCLES 1");
		visaFlume.write (sStr + ":BURST::PHASE 0");
		visaFlume.write (sStr + ":BURST::STATE ON");
	}
	else {
		visaFlume.write (sStr + ":BURST::STATE OFF");
	}
}


// set the agilent to output a square wave.
void AgilentCore::setSquare (int channel, squareInfo info, unsigned var){
	if (channel != 1 && channel != 2){
		thrower ("Bad Value for Channel in setSquare! Channel shoulde be 1 or 2.");
	}
	try {
		visaFlume.write ("SOURCE" + str (channel) + ":APPLY:SQUARE " + str (info.frequency.getValue (var)) + " KHZ, "
			+ str (convertPowerToSetPoint (info.amplitude.getValue (var), info.useCal, calibrations[channel - 1])) + " VPP, "
			+ str (convertPowerToSetPoint (info.offset.getValue (var), info.useCal, calibrations[channel - 1])) + " V");
	}
	catch (ChimeraError&) {
		throwNested ("Seen while programming Square Wave for channel " + str (channel) + " (1-indexed).");
	}
}


void AgilentCore::setSine (int channel, sineInfo info, unsigned var){
	if (channel != 1 && channel != 2){
		thrower ("Bad value for channel in setSine! Channel shoulde be 1 or 2.");
	}
	try {
	visaFlume.write ("SOURCE" + str (channel) + ":APPLY:SINUSOID " + str (info.frequency.getValue(var)) + " KHZ, "
		+ str (convertPowerToSetPoint (info.amplitude.getValue(var), info.useCal, calibrations[channel - 1])) + " VPP");
	}
	catch (ChimeraError&) {
		throwNested ("Seen while programming Sine Wave for channel " + str(channel) + " (1-indexed).");
	}

}


void AgilentCore::convertInputToFinalSettings (unsigned chan, deviceOutputInfo& info, std::vector<parameterType>& params){
	unsigned totalVariations = (params.size () == 0) ? 1 : params.front ().keyValues.size ();
	channelInfo& channel = info.channel[chan];
	try	{
		switch (channel.option)
		{
		case AgilentChannelMode::which::No_Control:
		case AgilentChannelMode::which::Output_Off:
			break;
		case AgilentChannelMode::which::DC:
			channel.dc.dcLevel.internalEvaluate (params, totalVariations);
			break;
		case AgilentChannelMode::which::Sine:
			channel.sine.frequency.internalEvaluate (params, totalVariations);
			channel.sine.amplitude.internalEvaluate (params, totalVariations);
			break;
		case AgilentChannelMode::which::Square:
			channel.square.frequency.internalEvaluate (params, totalVariations);
			channel.square.amplitude.internalEvaluate (params, totalVariations);
			channel.square.offset.internalEvaluate (params, totalVariations);
			break;
		case AgilentChannelMode::which::Preloaded:
		case AgilentChannelMode::which::Script:
			break;
		default:
			thrower ("Unrecognized Agilent Setting: " + AgilentChannelMode::toStr (channel.option));
		}
	}
	catch ( std::out_of_range& ){
		throwNested ("unrecognized variable!");
	}
}

/**
 * This function tells the agilent to put out the DC default waveform.
 */
void AgilentCore::setDefault (int channel){
	try {
		// turn it to the default voltage...
		std::string setPointString = str (convertPowerToSetPoint (AGILENT_DEFAULT_POWER, true, calibrations[channel - 1]));
		visaFlume.write ("SOURce" + str (channel) + ":APPLy:DC DEF, DEF, " + setPointString + " V");
	}
	catch (ChimeraError &) {
		throwNested ("Seen while programming default voltage.");
	}
}
/**
 * expects the inputted power to be in -MILI-WATTS!
 * returns set point in VOLTS
 */
double AgilentCore::convertPowerToSetPoint ( double requestedSetting, bool conversionOption, 
											 calResult calibration){
	// requested setting is the voltage or power settting coming from the calibration, depending on how the agilent
	// was calibrated (power typically for tweezer powers, voltage on PD otherwise). 
	if (conversionOption){
		// build the polynomial calibration.
		double setPointInVolts = CalibrationManager::calibrationFunction (requestedSetting, calibration);
		return setPointInVolts;
	}
	else{
		// no conversion
		return requestedSetting;
	}
}


std::vector<std::string> AgilentCore::getStartupCommands (){
	return setupCommands;
}

void AgilentCore::programSetupCommands (){
	try{
		for (auto cmd : setupCommands){
			visaFlume.write (cmd);
		}
	}
	catch (ChimeraError&){
		throwNested ("Failed to program setup commands for " + agilentName + " Agilent!");
	}
}


void AgilentCore::handleScriptVariation (unsigned variation, scriptedArbInfo& scriptInfo, unsigned channel,
										 std::vector<parameterType>& params){
	prepAgilentSettings (channel);
	programSetupCommands ();
	if (scriptInfo.wave.isVaried () || variation == 0){
		unsigned totalSegmentNumber = scriptInfo.wave.getSegmentNumber ();
		scriptInfo.wave.replaceVarValues (variation, params);
		// Loop through all segments
		for (auto segNumInc : range(totalSegmentNumber)){
			// Use that information to writebtn the data.
			try{
				scriptInfo.wave.writeData (segNumInc, sampleRate);
			}
			catch (ChimeraError&){
				throwNested ("IntensityWaveform.writeData threw an error! Error occurred in segment #"
					+ str (totalSegmentNumber));
			}
		}
		// order matters.
		// loop through again and calc/normalize/writebtn values.
		scriptInfo.wave.convertPowersToVoltages (scriptInfo.useCal, calibrations[channel-1]);
		scriptInfo.wave.calcMinMax ();
		scriptInfo.wave.minsAndMaxes.resize (variation + 1);
		scriptInfo.wave.minsAndMaxes[variation].second = scriptInfo.wave.getMaxVolt ();
		scriptInfo.wave.minsAndMaxes[variation].first = scriptInfo.wave.getMinVolt ();
		scriptInfo.wave.normalizeVoltages ();
		visaFlume.write ("SOURCE" + str (channel) + ":DATA:VOL:CLEAR");
		prepAgilentSettings (channel);
		for (unsigned segNumInc : range (totalSegmentNumber)){
			visaFlume.write (scriptInfo.wave.compileAndReturnDataSendString (segNumInc, variation,
				totalSegmentNumber, channel));
			// Save the segment
			visaFlume.write ("MMEM:STORE:DATA" + str (channel) + " \"" + memoryLoc + ":\\segment"
				+ str (segNumInc + totalSegmentNumber * variation) + ".arb\"");
		}
		scriptInfo.wave.compileSequenceString (totalSegmentNumber, variation, channel);
		// submit the sequence
		visaFlume.write (scriptInfo.wave.returnSequenceString ());
		// Save the sequence
		visaFlume.write ("SOURCE" + str (channel) + ":FUNC:ARB sequence" + str (variation));
		visaFlume.write ("MMEM:STORE:DATA" + str (channel) + " \"" + memoryLoc + ":\\sequence"
			+ str (variation) + ".seq\"");
		// clear temporary memory.
		visaFlume.write ("SOURCE" + str (channel) + ":DATA:VOL:CLEAR");
	}
}

deviceOutputInfo AgilentCore::getSettingsFromConfig (ConfigStream& file){
	auto readFunc = ConfigSystem::getGetlineFunc (file.ver);
	deviceOutputInfo tempSettings;
	file >> tempSettings.synced;
	std::array<std::string, 2> channelNames = { "CHANNEL_1", "CHANNEL_2" };
	unsigned chanInc = 0;
	for (auto& channel : tempSettings.channel){
		ConfigSystem::checkDelimiterLine (file, channelNames[chanInc]);
		// the extra step in all of the following is to remove the , at the end of each input.
		std::string input;
		file >> input;
		try{
			channel.option = file.ver < Version ("4.2") ?
				AgilentChannelMode::which (boost::lexical_cast<int>(input) + 2) : AgilentChannelMode::fromStr (input);
		}
		catch (boost::bad_lexical_cast&){
			throwNested ("Bad channel " + str (chanInc + 1) + " option!");
		}
		std::string calibratedOption;
		file.get ();
		readFunc (file, channel.dc.dcLevel.expressionStr);
		if (file.ver > Version ("2.3")){
			file >> channel.dc.useCal;
			file.get ();
		}
		readFunc (file, channel.sine.amplitude.expressionStr);
		readFunc (file, channel.sine.frequency.expressionStr);
		if (file.ver > Version ("2.3")){
			file >> channel.sine.useCal;
			file.get ();
		}
		readFunc (file, channel.square.amplitude.expressionStr);
		readFunc (file, channel.square.frequency.expressionStr);
		readFunc (file, channel.square.offset.expressionStr);
		if (file.ver > Version ("2.3")){
			file >> channel.square.useCal;
			file.get ();
		}
		readFunc (file, channel.preloadedArb.address.expressionStr);
		if (file.ver > Version ("2.3")){
			file >> channel.preloadedArb.useCal;
			file.get ();
		}
		if (file.ver >= Version ("5.9")) {
			file >> channel.preloadedArb.burstMode;
			file.get ();
		}
		readFunc (file, channel.scriptedArb.fileAddress.expressionStr);
		if (file.ver > Version ("2.3")){
			file >> channel.scriptedArb.useCal;
		}
		chanInc++;
	}
	return tempSettings;
}


void AgilentCore::logSettings (DataLogger& log, ExpThreadWorker* threadworker){
	try	{
		H5::Group agilentsGroup;
		try{
			agilentsGroup = log.file.createGroup ("/Agilents");
		}
		catch (H5::Exception&){
			agilentsGroup = log.file.openGroup ("/Agilents");
		}
		
		H5::Group singleAgilent (agilentsGroup.createGroup (getDelim()));
		unsigned channelCount = 1;
		log.writeDataSet (getStartupCommands (), "Startup-Commands", singleAgilent);
		for (auto& channel : expRunSettings.channel){
			H5::Group channelGroup (singleAgilent.createGroup ("Channel-" + str (channelCount)));
			std::string outputModeName = AgilentChannelMode::toStr (channel.option);
			log.writeDataSet (outputModeName, "Output-Mode", channelGroup);
			H5::Group dcGroup (channelGroup.createGroup ("DC-Settings"));
			log.writeDataSet (channel.dc.dcLevel.expressionStr, "DC-Level", dcGroup);
			H5::Group sineGroup (channelGroup.createGroup ("Sine-Settings"));
			log.writeDataSet (channel.sine.frequency.expressionStr, "Frequency", sineGroup);
			log.writeDataSet (channel.sine.amplitude.expressionStr, "Amplitude", sineGroup);
			H5::Group squareGroup (channelGroup.createGroup ("Square-Settings"));
			log.writeDataSet (channel.square.amplitude.expressionStr, "Amplitude", squareGroup);
			log.writeDataSet (channel.square.frequency.expressionStr, "Frequency", squareGroup);
			log.writeDataSet (channel.square.offset.expressionStr, "Offset", squareGroup);
			H5::Group preloadedArbGroup (channelGroup.createGroup ("Preloaded-Arb-Settings"));
			log.writeDataSet (channel.preloadedArb.address.expressionStr, "Address", preloadedArbGroup); 
			H5::Group scriptedArbSettings (channelGroup.createGroup ("Scripted-Arb-Settings"));
			log.writeDataSet (channel.scriptedArb.fileAddress.expressionStr, "Script-File-Address", scriptedArbSettings);
			// TODO: load script file itself
			ScriptStream stream;
			try{
				ExpThreadWorker::loadAgilentScript (channel.scriptedArb.fileAddress.expressionStr, stream);
				log.writeDataSet (stream.str (), "Agilent-Script-Script", scriptedArbSettings);
			}
			catch (ChimeraError&){
				// failed to open, that's probably fine, 
				log.writeDataSet ("Script Failed to load.", "Agilent-Script-Script", scriptedArbSettings);
			}
			channelCount++;
		}
	}
	catch (H5::Exception err){
		log.logError (err);
		throwNested ("ERROR: Failed to log Agilent parameters in HDF5 file: " + err.getDetailMsg ());
	}
}

void AgilentCore::loadExpSettings (ConfigStream& script){
	ConfigSystem::stdGetFromConfig (script, *this, expRunSettings);
	experimentActive = (expRunSettings.channel[0].option != AgilentChannelMode::which::No_Control
						|| expRunSettings.channel[1].option != AgilentChannelMode::which::No_Control);
}

void AgilentCore::calculateVariations (std::vector<parameterType>& params, ExpThreadWorker* threadWorker){
	std::string commwarnings;
	for (auto channelInc : range (2)){
		if (expRunSettings.channel[channelInc].scriptedArb.fileAddress.expressionStr != ""){
			analyzeAgilentScript (expRunSettings.channel[channelInc].scriptedArb, params, commwarnings);
		}
	}
	convertInputToFinalSettings (0, expRunSettings, params);
	convertInputToFinalSettings (1, expRunSettings, params);
}

void AgilentCore::setRunSettings (deviceOutputInfo newSettings) {
	// used by "program now".
	expRunSettings = newSettings;
}

void AgilentCore::programVariation (unsigned variation, std::vector<parameterType>& params, ExpThreadWorker* threadWorker){
	setAgilent (variation, params, expRunSettings, threadWorker);
}

void AgilentCore::checkTriggers (unsigned variationInc, DoCore& ttls, ExpThreadWorker* threadWorker){
	std::array<bool, 2> agMismatchVec = { false, false };
	for (auto chan : range (2)){
		auto& agChan = expRunSettings.channel[chan];
		if (agChan.option != AgilentChannelMode::which::Script || agMismatchVec[chan]){
			continue;
		}
		unsigned actualTrigs = experimentActive ? ttls.countTriggers (getTriggerLine (), variationInc) : 0;
		unsigned agilentExpectedTrigs = agChan.scriptedArb.wave.getNumTrigs ();
		std::string infoString = "Actual/Expected " + getDelim() + " Triggers: "
			+ str (actualTrigs) + "/" + str (agilentExpectedTrigs) + ".";
		if (actualTrigs != agilentExpectedTrigs){
			emit threadWorker->warn (cstr (
				"WARNING: Agilent " + getDelim () + " is not getting triggered by the ttl system the same "
				"number of times a trigger command appears in the agilent channel " + str (chan + 1) + " script. "
				+ infoString + " First seen in variation #" + str (variationInc) + ".\r\n"));
			agMismatchVec[chan] = true;
		}
	}
}

void AgilentCore::setCalibration (calResult newCal, unsigned chan) {
	if (chan != 1 && chan != 2) {
		thrower ("ERROR: Agilent channel must be 1 or 2!");
	}
	calibrations[chan - 1] = newCal;
}