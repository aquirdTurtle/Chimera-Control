#include "stdafx.h"
#include "myAgilent.h"

/*
* This Namespace includes all of my function handling for interacting withe agilent waveform generator. It includes:
* The Segment Class
* The IntensityWaveform Class
* The agilentDefault function
* The agilentErrorCheck function
* The selectIntensityProfile function
*/
namespace myAgilent
{

	/*
	 * This function tells the agilent to put out the DC default waveform.
	 */
	void agilentDefault()
	{
		ULONG viDefaultRM, Instrument;
		ULONG actual;
		std::string SCPIcmd;
		if (!NIAWG_SAFEMODE)
		{
			viOpenDefaultRM(&viDefaultRM);
			int val = viOpen(viDefaultRM, (char *)INTENSITY_AGILENT_ADDRESS, VI_NULL, VI_NULL, &Instrument);
			// turn it to the default voltage...
			SCPIcmd = str("APPLy:DC DEF, DEF, ") + AGILENT_DEFAULT_DC;
			agilentErrorCheck(viWrite(Instrument, (unsigned char*)cstr(SCPIcmd), (ViUInt32)SCPIcmd.size(), &actual), Instrument);
			// and leave...
			viClose(Instrument);
			viClose(viDefaultRM);
		}

		// update current values
		eCurrentAgilentLow = std::stod(AGILENT_DEFAULT_DC);
		eCurrentAgilentHigh = std::stod(AGILENT_DEFAULT_DC);
	}


	bool analyzeIntensityScript( ScriptStream& intensityFile, IntensityWaveform* intensityWaveformData, 
								 int& currentSegmentNumber)
	{
		while (!intensityFile.eof())
		{
			// Procedurally read lines into segment informations.
			int leaveTest = intensityWaveformData->readIntoSegment(currentSegmentNumber, intensityFile);
			if (leaveTest == 1)
			{
				// read function is telling this function to stop reading the file because it's at its end.
				break;
			}
			currentSegmentNumber++;
		}
		return false;
	}

	/*
	 * programIntensity opens the intensity file, reads the contents, loads them into an appropriate data structure, then from this data structure writes
	 * segment and sequence information to the function generator.
	 */
	void programIntensity(UINT varNum, std::vector<variable> variables, std::vector<std::vector<double> > varValues, bool& intensityVaried, 
						  std::vector<std::pair<double, double>>& minsAndMaxes, std::vector<std::fstream>& intensityFiles,
						  std::vector<variable> singletons, profileSettings profile)
	{
		// Initialize stuff
		IntensityWaveform intensityWaveformSequence;

		int currentSegmentNumber = 0;

		// connect to the agilent. I refuse to use the stupid typecasts. The way you often see these variables defined is using stupid things like ViRsc, ViUInt32, etc.
		ULONG viDefaultRM = 0, Instrument = 0;
		ULONG actual;
		std::string SCPIcmd;
		if (!NIAWG_SAFEMODE)
		{
			viOpenDefaultRM(&viDefaultRM);
			viOpen(viDefaultRM, (char *)INTENSITY_AGILENT_ADDRESS, VI_NULL, VI_NULL, &Instrument);
			// ???
			agilentErrorCheck(viSetAttribute(Instrument, VI_ATTR_TMO_VALUE, 40000), Instrument);
			// Set sample rate
			SCPIcmd = "SOURCE1:FUNC:ARB:SRATE " + str(AGILENT_SAMPLE_RATE);
			agilentErrorCheck(viWrite(Instrument, (unsigned char*)cstr(SCPIcmd), (ViUInt32)SCPIcmd.size(), &actual), Instrument);
			// Set filtering state
			SCPIcmd = str("SOURCE1:FUNC:ARB:FILTER ") + AGILENT_FILTER_STATE;
			agilentErrorCheck(viWrite(Instrument, (unsigned char*)cstr(SCPIcmd), (ViUInt32)SCPIcmd.size(), &actual), Instrument);
			// Set Trigger Parameters
			SCPIcmd = str("TRIGGER1:SOURCE EXTERNAL");
			agilentErrorCheck(viWrite(Instrument, (unsigned char*)cstr(SCPIcmd), (ViUInt32)SCPIcmd.size(), &actual), Instrument);
			//
			SCPIcmd = str("TRIGGER1:SLOPE POSITIVE");
			agilentErrorCheck(viWrite(Instrument, (unsigned char*)cstr(SCPIcmd), (ViUInt32)SCPIcmd.size(), &actual), Instrument);
		}
		for (UINT sequenceInc = 0; sequenceInc < intensityFiles.size(); sequenceInc++)
		{
			ScriptStream intensityScript;
			intensityScript << intensityFiles[sequenceInc].rdbuf();
			if (analyzeIntensityScript(intensityScript, &intensityWaveformSequence, currentSegmentNumber))
			{
				thrower( "analyzeIntensityScript threw an error!" );
			}
		}
		UINT totalSegmentNumber = currentSegmentNumber;
		intensityVaried = intensityWaveformSequence.returnIsVaried();
		// if varied
		if (intensityWaveformSequence.returnIsVaried() )
		{
			// loop through # of variable values
			for (UINT varValueCount = 0; varValueCount < varValues[0].size(); varValueCount++)
			{
				// Loop through variable names
				for (UINT varNameCount = 0; varNameCount < variables.size(); varNameCount++)
				{
					// replace variable values where found
					intensityWaveformSequence.replaceVarValues(variables[varNameCount].name, varValues[varNameCount][varValueCount]);
				}
				// Loop through all segments
				for (UINT segNumInc = 0; segNumInc < totalSegmentNumber; segNumInc++)
				{
					// Use that information to write the data.
					if (intensityWaveformSequence.writeData(segNumInc) < 0)
					{
						thrower("ERROR: IntensityWaveform.writeData threw an error! Error occurred in segment #" 
								 + str(totalSegmentNumber) + ".");
					}
				}
				// loop through again and calc/normalize/write values.
				intensityWaveformSequence.convertPowersToVoltages();
				intensityWaveformSequence.calcMinMax();
				minsAndMaxes.resize(varValueCount + 1);
				minsAndMaxes[varValueCount].second = intensityWaveformSequence.returnMaxVolt();
				minsAndMaxes[varValueCount].first = intensityWaveformSequence.returnMinVolt();
				intensityWaveformSequence.normalizeVoltages();

				for (UINT segNumInc = 0; segNumInc < totalSegmentNumber; segNumInc++)
				{
					if (!NIAWG_SAFEMODE)
					{
						SCPIcmd = intensityWaveformSequence.compileAndReturnDataSendString(segNumInc, varValueCount, totalSegmentNumber);
						// send to the agilent.
						agilentErrorCheck(viWrite(Instrument, (unsigned char*)cstr(SCPIcmd), (ViUInt32)SCPIcmd.size(), &actual), Instrument);
						// Select the segment
						SCPIcmd = "SOURCE1:FUNC:ARB seg" + str(segNumInc + totalSegmentNumber * varValueCount);
						agilentErrorCheck(viWrite(Instrument, (unsigned char*)cstr(SCPIcmd), (ViUInt32)SCPIcmd.size(), &actual), Instrument);
						// Save the segment
						SCPIcmd = "MMEM:STORE:DATA \"INT:\\seg" + str(segNumInc + totalSegmentNumber * varValueCount) + ".arb\"";
						agilentErrorCheck(viWrite(Instrument, (unsigned char*)cstr(SCPIcmd), (ViUInt32)SCPIcmd.size(), &actual), Instrument);
						// increment for the next.
						SCPIcmd = str("TRIGGER1:SLOPE POSITIVE");
						agilentErrorCheck(viWrite(Instrument, (unsigned char*)cstr(SCPIcmd), (ViUInt32)SCPIcmd.size(), &actual), Instrument);
					}
				}

				// Now handle seqeunce creation / writing.
				intensityWaveformSequence.compileSequenceString(totalSegmentNumber, varValueCount);
				if (!NIAWG_SAFEMODE)
				{
					// submit the sequence
					SCPIcmd = intensityWaveformSequence.returnSequenceString();
					agilentErrorCheck(viWrite(Instrument, (unsigned char*)cstr(SCPIcmd), (ViUInt32)SCPIcmd.size(), &actual), Instrument);
					// Save the sequence
					SCPIcmd = "SOURCE1:FUNC:ARB seq" + str(varValueCount);
					agilentErrorCheck(viWrite(Instrument, (unsigned char*)cstr(SCPIcmd), (ViUInt32)SCPIcmd.size(), &actual), Instrument);

					SCPIcmd = "MMEM:STORE:DATA \"INT:\\seq" + str(varValueCount) + ".seq\"";
					agilentErrorCheck(viWrite(Instrument, (unsigned char*)cstr(SCPIcmd), (ViUInt32)SCPIcmd.size(), &actual), Instrument);
					// clear temporary memory.
					SCPIcmd = "SOURCE1:DATA:VOL:CLEAR";
					agilentErrorCheck(viWrite(Instrument, (unsigned char*)cstr(SCPIcmd), (ViUInt32)SCPIcmd.size(), &actual), Instrument);
				}
			}
		}
		// else not varying
		else
		{
			// Loop through all segments
			for (int segNumInc = 0; segNumInc < totalSegmentNumber; segNumInc++)
			{
				// Use that information to write the data.
				if (intensityWaveformSequence.writeData(segNumInc) < 0)
				{
					thrower("ERROR: IntensityWaveform.writeData threw an error! Error occurred in segment #" 
							 + str(totalSegmentNumber) + ".");
				}
			}
			// no reassignment nessesary, no variables
			intensityWaveformSequence.convertPowersToVoltages();
			intensityWaveformSequence.calcMinMax();
			minsAndMaxes.resize(1);
			minsAndMaxes[0].second = intensityWaveformSequence.returnMaxVolt();
			minsAndMaxes[0].first = intensityWaveformSequence.returnMinVolt();
			intensityWaveformSequence.normalizeVoltages();

			for (int segNumInc = 0; segNumInc < totalSegmentNumber; segNumInc++)
			{
				if (!NIAWG_SAFEMODE)
				{
					// Set output impedance...
					SCPIcmd = str("OUTPUT1:LOAD ") + AGILENT_LOAD;
					// set range of voltages...
					agilentErrorCheck(viWrite(Instrument, (unsigned char*)cstr(SCPIcmd), (ViUInt32)SCPIcmd.size(), &actual), Instrument);
					SCPIcmd = str("SOURCE1:VOLT:LOW ") + str(minsAndMaxes[0].first) + " V";
					agilentErrorCheck(viWrite(Instrument, (unsigned char*)cstr(SCPIcmd), (ViUInt32)SCPIcmd.size(), &actual), Instrument);
					SCPIcmd = str("SOURCE1:VOLT:HIGH ") + str(minsAndMaxes[0].second) + " V";
					agilentErrorCheck(viWrite(Instrument, (unsigned char*)cstr(SCPIcmd), (ViUInt32)SCPIcmd.size(), &actual), Instrument);
					// get the send string
					SCPIcmd = intensityWaveformSequence.compileAndReturnDataSendString(segNumInc, 0, totalSegmentNumber);
					// send to the agilent.
					agilentErrorCheck(viWrite(Instrument, (unsigned char*)cstr(SCPIcmd), (ViUInt32)SCPIcmd.size(), &actual), Instrument);

					// Select the segment
					SCPIcmd = "SOURCE1:FUNC:ARB seg" + str(segNumInc);
					agilentErrorCheck(viWrite(Instrument, (unsigned char*)cstr(SCPIcmd), (ViUInt32)SCPIcmd.size(), &actual), Instrument);
					// Save the segment
					SCPIcmd = "MMEM:STORE:DATA \"INT:\\seg" + str(segNumInc) + ".arb\"";
					agilentErrorCheck(viWrite(Instrument, (unsigned char*)cstr(SCPIcmd), (ViUInt32)SCPIcmd.size(), &actual), Instrument);
					// increment for the next.
				}
			}


			// Now handle seqeunce creation / writing.
			intensityWaveformSequence.compileSequenceString(totalSegmentNumber, 0);

			if (!NIAWG_SAFEMODE)
			{
				// submit the sequence
				SCPIcmd = intensityWaveformSequence.returnSequenceString();
				agilentErrorCheck(viWrite(Instrument, (unsigned char*)cstr(SCPIcmd), (ViUInt32)SCPIcmd.size(), &actual), Instrument);
				// Save the sequence
				SCPIcmd = "SOURCE1:FUNC:ARB seq" + str(0);
				viWrite(Instrument, (unsigned char*)cstr(SCPIcmd), (ViUInt32)SCPIcmd.size(), &actual);
				//		agilentErrorCheck(, Instrument);
				SCPIcmd = "MMEM:STORE:DATA \"INT:\\seq" + str(0) + ".seq\"";
				viWrite(Instrument, (unsigned char*)cstr(SCPIcmd), (ViUInt32)SCPIcmd.size(), &actual);
				//		agilentErrorCheck(, Instrument);
				// clear temporary memory.
				SCPIcmd = "SOURCE1:DATA:VOL:CLEAR";
				agilentErrorCheck(viWrite(Instrument, (unsigned char*)cstr(SCPIcmd), (ViUInt32)SCPIcmd.size(), &actual), Instrument);
			}
		}
		viClose(Instrument);
		viClose(viDefaultRM);
	}

	/*
	 * This function checks if the agilent throws an error or if there is an error communicating with the agilent. it returns -1 if error, 0 otherwise.
	 */
	int agilentErrorCheck(long status, ULONG vi)
	{
		long errorCode = 0;
		char buf[256] = { 0 };
		// Check comm status
		if (status < 0)	
		{
			// Error detected.
			std::string commErrMsg;
			commErrMsg = "ERROR: Communication error with agilent. Error Code: " + str(status);
			errBox(commErrMsg);
			return -1;
		}
		if (!NIAWG_SAFEMODE)
		{
			// Query the agilent for errors.
			viQueryf(vi, "SYST:ERR?\n", "%ld,%t", &errorCode, buf);
		}
		if (errorCode != 0)
		{
			// Agilent error
			std::string agErrMsg;
			agErrMsg = "ERROR: agilent returned error message: " + str(errorCode) + ":" + buf;
			errBox(agErrMsg);
			return -1;
		}
		return 0;
	}

	/*
	 * This function tells the agilent to use sequence # (varNum) and sets settings correspondingly.
	 */
	void setIntensity(UINT varNum, bool intensityIsVaried, std::vector<std::pair<double, double>> intensityMinMax)
	{
		if (intensityIsVaried || varNum == 0)
		{
			ULONG viDefaultRM, Instrument;
			ULONG actual;
			if (!NIAWG_SAFEMODE)
			{
				viOpenDefaultRM(&viDefaultRM);
				viOpen(viDefaultRM, (char *)INTENSITY_AGILENT_ADDRESS, VI_NULL, VI_NULL, &Instrument);
			}
			std::string SCPIcmd;
			if (!NIAWG_SAFEMODE)
			{
				// Load sequence that was previously loaded.
				SCPIcmd = "MMEM:LOAD:DATA \"INT:\\seq" + str(varNum) + ".seq\"";
				agilentErrorCheck(viWrite(Instrument, (unsigned char*)cstr(SCPIcmd), (ViUInt32)SCPIcmd.size(), &actual), Instrument);

				SCPIcmd = "SOURCE1:FUNC ARB";
				agilentErrorCheck(viWrite(Instrument, (unsigned char*)cstr(SCPIcmd), (ViUInt32)SCPIcmd.size(), &actual), Instrument);

				SCPIcmd = "SOURCE1:FUNC:ARB \"INT:\\seq" + str(varNum) + ".seq\"";
				agilentErrorCheck(viWrite(Instrument, (unsigned char*)cstr(SCPIcmd), (ViUInt32)SCPIcmd.size(), &actual), Instrument);

				// Set output impedance...
				SCPIcmd = str("OUTPUT1:LOAD ") + AGILENT_LOAD;

				agilentErrorCheck(viWrite(Instrument, (unsigned char*)cstr(SCPIcmd), (ViUInt32)SCPIcmd.size(), &actual), Instrument);
				SCPIcmd = str("SOURCE1:VOLT:LOW ") + str(intensityMinMax[varNum].first) + " V";
				agilentErrorCheck(viWrite(Instrument, (unsigned char*)cstr(SCPIcmd), (ViUInt32)SCPIcmd.size(), &actual), Instrument);
				SCPIcmd = str("SOURCE1:VOLT:HIGH ") + str(intensityMinMax[varNum].second) + " V";
				agilentErrorCheck(viWrite(Instrument, (unsigned char*)cstr(SCPIcmd), (ViUInt32)SCPIcmd.size(), &actual), Instrument);

				SCPIcmd = "OUTPUT1 ON";
				agilentErrorCheck(viWrite(Instrument, (unsigned char*)cstr(SCPIcmd), (ViUInt32)SCPIcmd.size(), &actual), Instrument);
				// and leave...
				viClose(Instrument);
				viClose(viDefaultRM);
			}
		}
	}
}
