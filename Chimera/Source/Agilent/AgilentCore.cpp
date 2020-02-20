#include "stdafx.h"
#include "AgilentCore.h"
#include "Scripts/ScriptStream.h"
#include "ExperimentThread/ExperimentThreadManager.h"

AgilentCore::AgilentCore (const agilentSettings& settings) : 
	visaFlume (settings.safemode, settings.address),
	sampleRate (settings.sampleRate),
	initSettings (settings),
	triggerRow (settings.triggerRow),
	triggerNumber (settings.triggerNumber),
	memoryLoc (settings.memoryLocation),
	configDelim (settings.configurationFileDelimiter),
	calibrationCoefficients (settings.calibrationCoeff),
	agilentName (settings.deviceName),
	setupCommands (settings.setupCommands)
{
	try
	{
		visaFlume.open ();
	}
	catch (Error&)
	{
		throwNested ("Error seen while initializing " + agilentName + " Agilent");
	}
}

AgilentCore::~AgilentCore ()
{
	visaFlume.close ();
}

void AgilentCore::initialize ()
{
	try
	{
		deviceInfo = visaFlume.identityQuery ();
		isConnected = true;
	}
	catch (Error&)
	{
		deviceInfo = "Disconnected";
		isConnected = false;
	}

}

std::pair<DoRows::which, UINT> AgilentCore::getTriggerLine ()
{
	return { triggerRow, triggerNumber };
}
/*
void AgilentCore::analyzeAgilentScript (UINT chan, std::vector<parameterType>& vars)
{
	if (settings.channel[chan].option == AgilentChannelMode::which::Script)
	{
		analyzeAgilentScript (settings.channel[chan].scriptedArb, vars);
	}
}*/

std::string AgilentCore::getDeviceInfo ()
{
	return deviceInfo;
}

void AgilentCore::analyzeAgilentScript (scriptedArbInfo& infoObj, std::vector<parameterType>& variables)
{
	ScriptStream stream;
	ExperimentThreadManager::loadAgilentScript (infoObj.fileAddress, stream);
	int currentSegmentNumber = 0;
	infoObj.wave.resetNumberOfTriggers ();
	// Procedurally read lines into segment objects.
	while (!stream.eof ())
	{
		int leaveTest;
		try
		{
			leaveTest = infoObj.wave.analyzeAgilentScriptCommand (currentSegmentNumber, stream, variables);
		}
		catch (Error&)
		{
			throwNested ("Error seen while analyzing agilent script command for agilent " + this->configDelim);
		}
		if (leaveTest < 0)
		{
			thrower ("IntensityWaveform.analyzeAgilentScriptCommand threw an error! Error occurred in segment #"
				+ str (currentSegmentNumber) + ".");
		}
		if (leaveTest == 1)
		{
			// read function is telling this function to stop reading the file because it's at its end.
			break;
		}
		currentSegmentNumber++;
	}
}


std::string AgilentCore::getDeviceIdentity ()
{
	std::string msg;
	try
	{
		msg = visaFlume.identityQuery ();
	}
	catch (Error & err)
	{
		msg == err.trace ();
	}
	if (msg == "")
	{
		msg = "Disconnected...\n";
	}
	return msg;
}


void AgilentCore::setAgilent (UINT var, std::vector<parameterType>& params, deviceOutputInfo runSettings)
{
	if (!connected ())
	{
		return;
	}
	try
	{
		visaFlume.write ("OUTPut:SYNC " + str (runSettings.synced));
	}
	catch (Error&)
	{
		errBox ("Failed to set agilent output synced?!");
	}
	for (auto chan : range (UINT (2)))
	{
		auto& channel = runSettings.channel[chan];
		try
		{
			switch (channel.option)
			{
				case AgilentChannelMode::which::No_Control:
					break;
				case AgilentChannelMode::which::Output_Off:
					outputOff (chan + 1);
					break;
				case AgilentChannelMode::which::DC:
					setDC (chan + 1, channel.dc, var);
					break;
				case AgilentChannelMode::which::Sine:
					setSine (chan + 1, channel.sine, var);
					break;
				case AgilentChannelMode::which::Square:
					setSquare (chan + 1, channel.square, var);
					break;
				case AgilentChannelMode::which::Preloaded:
					setExistingWaveform (chan + 1, channel.preloadedArb);
					break;
				case AgilentChannelMode::which::Script:
					handleScriptVariation (var, channel.scriptedArb, chan + 1, params);
					setScriptOutput (var, channel.scriptedArb, chan + 1);
					break;
				default:
					thrower ("unrecognized channel " + str (chan) + " setting: "
						+ AgilentChannelMode::toStr (channel.option));
			}
		}
		catch (Error & err)
		{
			throwNested ("Error seen while programming agilent output for " + configDelim + " agilent channel "
				+ str (chan + 1) + ": " + err.whatBare ());
		}
	}
}


// stuff that only has to be done once.
void AgilentCore::prepAgilentSettings (UINT channel)
{
	if (channel != 1 && channel != 2)
	{
		thrower ("Bad value for channel in prepAgilentSettings!");
	}
	// Set timout, sample rate, filter parameters, trigger settings.
	visaFlume.setAttribute (VI_ATTR_TMO_VALUE, 40000);
	visaFlume.write ("SOURCE1:FUNC:ARB:SRATE " + str (sampleRate));
	visaFlume.write ("SOURCE2:FUNC:ARB:SRATE " + str (sampleRate));
}

/*
 * This function tells the agilent to use sequence # (varNum) and sets settings correspondingly.
 */
void AgilentCore::setScriptOutput (UINT varNum, scriptedArbInfo scriptInfo, UINT chan)
{
	if (scriptInfo.wave.isVaried () || varNum == 0)
	{
		prepAgilentSettings (chan);
		// check if effectively dc
		if (scriptInfo.wave.minsAndMaxes.size () == 0)
		{
			thrower ("script wave min max size is zero???");
		}
		auto& minMaxs = scriptInfo.wave.minsAndMaxes[varNum];
		if (fabs (minMaxs.first - minMaxs.second) < 1e-6)
		{
			dcInfo tempDc;
			tempDc.dcLevel = str(minMaxs.first);
			tempDc.dcLevel.internalEvaluate (std::vector<parameterType>(), 1);
			tempDc.useCal = scriptInfo.useCal;
			setDC (chan, tempDc, 0);
		}
		else
		{
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


void AgilentCore::outputOff (int channel)
{
	if (channel != 1 && channel != 2)
	{
		thrower ("bad value for channel inside outputOff!");
	}
	channel++;
	visaFlume.write ("OUTPUT" + str (channel) + " OFF");
}


bool AgilentCore::connected ()
{
	return isConnected;
}


void AgilentCore::setDC (int channel, dcInfo info, UINT var)
{
	if (channel != 1 && channel != 2)
	{
		thrower ("Bad value for channel inside setDC!");
	}
	visaFlume.write ("SOURce" + str (channel) + ":APPLy:DC DEF, DEF, "
		+ str (convertPowerToSetPoint (info.dcLevel.getValue(var), info.useCal, calibrationCoefficients)) + " V");
}


void AgilentCore::setExistingWaveform (int channel, preloadedArbInfo info)
{
	if (channel != 1 && channel != 2)
	{
		thrower ("Bad value for channel in setExistingWaveform!");
	}
	auto sStr = "SOURCE" + str (channel);
	visaFlume.write (sStr + ":DATA:VOL:CLEAR");
	// Load sequence that was previously loaded.
	visaFlume.write ("MMEM:LOAD:DATA \"" + info.address + "\"");
	// tell it that it's outputting something arbitrary (not sure if necessary)
	visaFlume.write (sStr + ":FUNC ARB");
	// tell it what arb it's outputting.
	visaFlume.write (sStr + ":FUNC:ARB \"" + memoryLoc + ":\\" + info.address + "\"");
	// not really bursting... but this allows us to reapeat on triggers. Might be another way to do this.
	visaFlume.write (sStr + ":BURST::MODE TRIGGERED");
	visaFlume.write (sStr + ":BURST::NCYCLES 1");
	visaFlume.write (sStr + ":BURST::PHASE 0");
	visaFlume.write (sStr + ":BURST::STATE ON");
	visaFlume.write ("OUTPUT" + str (channel) + " ON");
}


// set the agilent to output a square wave.
void AgilentCore::setSquare (int channel, squareInfo info, UINT var)
{
	if (channel != 1 && channel != 2)
	{
		thrower ("Bad Value for Channel in setSquare!");
	}
	visaFlume.write ("SOURCE" + str (channel) + ":APPLY:SQUARE " + str (info.frequency.getValue(var)) + " KHZ, "
		+ str (convertPowerToSetPoint (info.amplitude.getValue(var), info.useCal, calibrationCoefficients)) + " VPP, "
		+ str (convertPowerToSetPoint (info.offset.getValue(var), info.useCal, calibrationCoefficients)) + " V");
}


void AgilentCore::setSine (int channel, sineInfo info, UINT var)
{
	if (channel != 1 && channel != 2)
	{
		thrower ("Bad value for channel in setSine");
	}
	visaFlume.write ("SOURCE" + str (channel) + ":APPLY:SINUSOID " + str (info.frequency.getValue(var)) + " KHZ, "
		+ str (convertPowerToSetPoint (info.amplitude.getValue(var), info.useCal, calibrationCoefficients)) + " VPP");
}


void AgilentCore::convertInputToFinalSettings (UINT totalVariations, UINT chan, deviceOutputInfo& info,
												std::vector<parameterType>& variables)
{
	// iterate between 0 and 1...
	channelInfo& channel = info.channel[chan];
	try
	{
		switch (channel.option)
		{
		case AgilentChannelMode::which::No_Control:
		case AgilentChannelMode::which::Output_Off:
			break;
		case AgilentChannelMode::which::DC:
			channel.dc.dcLevel.internalEvaluate (variables, totalVariations);
			break;
		case AgilentChannelMode::which::Sine:
			channel.sine.frequency.internalEvaluate (variables, totalVariations);
			channel.sine.amplitude.internalEvaluate (variables, totalVariations);
			break;
		case AgilentChannelMode::which::Square:
			channel.square.frequency.internalEvaluate (variables, totalVariations);
			channel.square.amplitude.internalEvaluate (variables, totalVariations);
			channel.square.offset.internalEvaluate (variables, totalVariations);
			break;
		case AgilentChannelMode::which::Preloaded:
			break;
		case AgilentChannelMode::which::Script:
			/*for (auto variation : range (totalVariations))
			{
				handleScriptVariation (variation, channel.scriptedArb, chan + 1, variables);
			}*/ // this used to be here. now it's just a special case of the program
			break;
		default:
			thrower ("Unrecognized Agilent Setting: " + AgilentChannelMode::toStr (channel.option));
		}
	}
	catch ( std::out_of_range& )
	{
		throwNested ("unrecognized variable!");
	}
}

/**
 * This function tells the agilent to put out the DC default waveform.
 */
void AgilentCore::setDefault (int channel)
{
	// turn it to the default voltage...
	std::string setPointString = str (convertPowerToSetPoint (AGILENT_DEFAULT_POWER, true, calibrationCoefficients));
	visaFlume.write ("SOURce" + str (channel) + ":APPLy:DC DEF, DEF, " + setPointString + " V");
}
/**
 * expects the inputted power to be in -MILI-WATTS!
 * returns set point in VOLTS
 */
double AgilentCore::convertPowerToSetPoint ( double powerInMilliWatts, bool conversionOption, 
											 std::vector<double> calibCoeff)
{
	if (conversionOption)
	{
		double setPointInVolts = 0;
		if (calibCoeff.size () == 0)
		{
			thrower ("Wanted agilent calibration but no calibration given to conversion function!");
		}
		// build the polynomial calibration.
		UINT polyPower = 0;
		for (auto coeff : calibCoeff)
		{
			setPointInVolts += coeff * std::pow (powerInMilliWatts, polyPower++);
		}
		//double setPointInVolts = slope * powerInMilliWatts + offset;
		return setPointInVolts;
	}
	else
	{
		// no conversion
		return powerInMilliWatts;
	}
}



std::vector<std::string> AgilentCore::getStartupCommands ()
{
	return setupCommands;
}

void AgilentCore::programSetupCommands ()
{
	try
	{
		for (auto cmd : setupCommands)
		{
			visaFlume.write (cmd);
		}
	}
	catch (Error&)
	{
		throwNested ("Failed to program setup commands for " + agilentName + " Agilent!");
	}
}


void AgilentCore::handleScriptVariation (UINT variation, scriptedArbInfo& scriptInfo, UINT channel,
										 std::vector<parameterType>& params)
{
	prepAgilentSettings (channel);
	programSetupCommands ();
	if (scriptInfo.wave.isVaried () || variation == 0)
	{
		UINT totalSegmentNumber = scriptInfo.wave.getSegmentNumber ();
		scriptInfo.wave.replaceVarValues (variation, params);
		// Loop through all segments
		for (auto segNumInc : range(totalSegmentNumber))
		{
			// Use that information to write the data.
			try
			{
				scriptInfo.wave.writeData (segNumInc, sampleRate);
			}
			catch (Error&)
			{
				throwNested ("IntensityWaveform.writeData threw an error! Error occurred in segment #"
					+ str (totalSegmentNumber));
			}
		}
		// order matters.
		// loop through again and calc/normalize/write values.
		scriptInfo.wave.convertPowersToVoltages (scriptInfo.useCal, calibrationCoefficients);
		scriptInfo.wave.calcMinMax ();
		scriptInfo.wave.minsAndMaxes.resize (variation + 1);
		scriptInfo.wave.minsAndMaxes[variation].second = scriptInfo.wave.getMaxVolt ();
		scriptInfo.wave.minsAndMaxes[variation].first = scriptInfo.wave.getMinVolt ();
		scriptInfo.wave.normalizeVoltages ();
		visaFlume.write ("SOURCE" + str (channel) + ":DATA:VOL:CLEAR");
		prepAgilentSettings (channel);
		for (UINT segNumInc : range (totalSegmentNumber))
		{
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
