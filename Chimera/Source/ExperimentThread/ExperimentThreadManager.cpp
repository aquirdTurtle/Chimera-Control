// created by Mark O. Brown
#include "stdafx.h"

#include "ExperimentThread/ExperimentThreadManager.h"
#include "DigitalOutput/DoSystem.h"
#include "AnalogOutput/AoSystem.h"
#include "GeneralObjects/CodeTimer.h"
#include "PrimaryWindows/AuxiliaryWindow.h"
#include "NIAWG/NiawgWaiter.h"
#include "NIAWG/NiawgSystem.h"
#include "ParameterSystem/Expression.h"
#include "PrimaryWindows/MainWindow.h"
#include "Andor/CameraSettingsControl.h"
#include "Scripts/ScriptStream.h"

#include "nidaqmx2.h"

#include <boost/algorithm/string/replace.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include <fstream>
  
std::string ExperimentThreadManager::abortString = "\r\nABORTED!\r\n";

/* deviceList class:
get by type
*/

ExperimentThreadManager::ExperimentThreadManager() {}

/*
 * The workhorse of actually running experiments. This thread procedure analyzes all of the GUI settings and current 
 * configuration settings to determine how to program and run the experiment.
 * @param voidInput: This is the only input to the proxcedure. It MUST be a pointer to a ExperimentThreadInput structure.
 * @return unsigned int: unused.
 */
unsigned int __stdcall ExperimentThreadManager::experimentThreadProcedure( void* voidInput )
{
	auto startTime = chronoClock::now ();
	std::unique_ptr< ExperimentThreadInput > input ((ExperimentThreadInput*)voidInput);
	input->thisObj->experimentIsRunning = true;
	input->comm.expQuiet = input->quiet;
	input->comm.expUpdate ("Starting Experiment Thread Procedure...\n");
	indvSeqElem expSeq;
	UINT repetitions = 1;
	mainOptions mainOpts;
	input->devices.getSingleDevice<AndorCameraCore> ().experimentActive = input->runList.andor;
	input->devices.getSingleDevice<BaslerCameraCore> ().experimentActive = input->runList.basler;
	input->devices.getSingleDevice<NiawgCore> ().expRerngGui = input->rerngGuiForm;
	try
	{
		input->comm.expUpdate ("Loading Experiment Settings...\n");
		auto expParams = ParameterSystem::combineParams ( 
										ParameterSystem::getConfigParamsFromFile (input->profile.configFilePath ()), 
										input->globalParameters );
		ConfigStream cStream (input->profile.configFilePath (), true);
		ScanRangeInfo varRangeInfo = ParameterSystem::getRangeInfoFromFile (input->profile.configFilePath ());
		input->thisObj->loadMasterScript (ProfileSystem::getMasterAddressFromConfig (input->profile), expSeq.masterStream);
		mainOpts = ProfileSystem::stdConfigGetter (cStream, "MAIN_OPTIONS", MainOptionsControl::getSettingsFromConfig );
		repetitions = ProfileSystem::stdConfigGetter (cStream, "REPETITIONS", Repetitions::getSettingsFromConfig );
		ParameterSystem::generateKey ( expParams, mainOpts.randomizeVariations, varRangeInfo);
		for (auto& device : input->devices.list)
		{
			device.get ().loadExpSettings (cStream);
		}
		if ( input->updatePlotterXVals ) updatePlotX_vals ( input, expParams );
		/// The Variation Calculation Step.
		input->comm.expUpdate ("Calculating All Variation Data...\r\n");
		for (auto& device : input->devices.list) {
			if (device.get ().experimentActive) {
				device.get ().calculateVariations (expParams, input->comm);
				if (input->thisObj->isAborting) thrower (abortString);
			}
		}
		calculateAdoVariations ( input, expParams, repetitions, expSeq.masterStream, mainOpts.atomSkipThreshold );
		runConsistencyChecks (input, expParams);
		/// log experiment parameters
		if (input->expType != ExperimentType::LoadMot)
		{
			input->logger.logMasterRuntime (repetitions, expParams);
			for (auto& device : input->devices.list) {
				if (device.get ().experimentActive) {
					device.get ().logSettings (input->logger);
				}
			}
		}
		/// Begin experiment 
		for (const auto& variationInc : range(determineVariationNumber (expParams)))
		{
			initVariation ( input, variationInc, expParams);
			input->comm.expUpdate ("Programming Devices for Variation... ");
			for (auto& device : input->devices.list) {
				if (device.get ().experimentActive) {
					device.get ().programVariation (variationInc, expParams);
				}
			}
			bool skipOption = input->skipNext == NULL ? false : input->skipNext->load();
			if (input->runList.master){ input->ttls.ftdi_write (variationInc, skipOption); }
			input->comm.expUpdate ("Running Experiment.\r\n");
			for (const auto& repInc : range(repetitions))
			{
				handlePause (input->comm, input->thisObj->isPaused, input->thisObj->isAborting);
				adoStart (input, repInc, variationInc, skipOption);
			}
		}
		for (auto& device : input->devices.list){
			if (device.get ().experimentActive){
				device.get ().normalFinish ();
			}
		}
		normalFinish (input->comm, input->expType, input->runList.master, startTime, input->aoSys );
	}
	catch (Error& exception){
		for (auto& device : input->devices.list){
			if (device.get ().experimentActive) {
				device.get ().errorFinish ();
			}
		}
		errorFinish (input->comm, input->thisObj->isAborting, exception, startTime);
	}
	input->thisObj->experimentIsRunning = false;
	return false;
}


void ExperimentThreadManager::analyzeMasterScript ( DoCore& ttls, AoSystem& aoSys, std::vector<parameterType>& vars, 
													ScriptStream& currentMasterScript, bool expectsLoadSkip,
													std::string& warnings, timeType& operationTime, 
													timeType& loadSkipTime )
{
	std::string currentMasterScriptText = currentMasterScript.str ( );
	loadSkipTime.first.clear ( );
	loadSkipTime.second = 0;
	// starts at 0.1 if not initialized by the user.
	operationTime.second = 0.1;
	operationTime.first.clear ( );
	if ( currentMasterScript.str ( ) == "" )
	{
		thrower ( "Master script is empty! (A low level bug, this shouldn't happen)" );
	}
	std::string word;
	currentMasterScript >> word;
	std::vector<UINT> totalRepeatNum, currentRepeatNum;
	std::vector<std::streamoff> repeatPos;
	// the analysis loop.
	bool loadSkipFound = false;
	std::string scope = PARENT_PARAMETER_SCOPE;                                                                                              
	while ( !( currentMasterScript.peek ( ) == EOF ) || word != "__end__" )
	{
 		if ( handleTimeCommands ( word, currentMasterScript, vars, scope, operationTime ) )
		{
			// got handled, so break out of the if-else by entering this scope.
		}
		else if ( handleVariableDeclaration ( word, currentMasterScript, vars, scope, warnings ) )
		{}
		else if ( handleDoCommands ( word, currentMasterScript, vars, ttls, scope, operationTime) )
		{}
		else if ( handleAoCommands ( word, currentMasterScript, vars, aoSys, ttls, scope, operationTime) )
		{}
		else if ( word == "callcppcode" )
		{
			// and that's it... 
			callCppCodeFunction ( );
		}
		else if ( word == "loadskipentrypoint!" )
		{
			loadSkipFound = true;
			loadSkipTime = operationTime;
		}
		/// Deal with RSG calls
		else if ( word == "rsg:" )
		{
			thrower ("\"rsg:\" command is deprecated! Please use the microwave system listview instead.");
		}
		else if ( handleFunctionCall ( word, currentMasterScript, vars, ttls, aoSys, warnings, 
									   PARENT_PARAMETER_SCOPE, operationTime ) )
		{ }
		else if ( word == "repeat:" )
		{
			Expression repeatStr;
			currentMasterScript >> repeatStr;
			try
			{
				totalRepeatNum.push_back ( repeatStr.evaluate ( ) );
			}
			catch ( Error& )
			{
				throwNested ( "the repeat number failed to convert to an integer! Note that the repeat number can not"
						  " currently be a variable." );
			}
			repeatPos.push_back ( currentMasterScript.tellg ( ) );
			currentRepeatNum.push_back ( 1 );
		}
		else if ( word == "end" )
		{
			// handle end of repeat
			if ( currentRepeatNum.size ( ) == 0 )
			{
				thrower ( "ERROR! Tried to end repeat structure in master script, but you weren't repeating!" );
			}
			if ( currentRepeatNum.back ( ) < totalRepeatNum.back ( ) )
			{
				currentMasterScript.seekg ( repeatPos.back ( ) );
				currentRepeatNum.back ( )++;
			}
			else
			{
				currentRepeatNum.pop_back ( );
				repeatPos.pop_back ( );
				totalRepeatNum.pop_back ( );
			}
		}
		else
		{
			word = ( word == "" ) ? "[EMPTY-STRING]" : word;
			thrower ( "unrecognized master script command: \"" + word + "\"" );
		}
		word = "";
		currentMasterScript >> word;
	}
	if ( expectsLoadSkip && !loadSkipFound )
	{
		thrower ( "Expected load skip in script, but the load skip command was not found during script analysis!" );
	}
}


void ExperimentThreadManager::analyzeFunction ( std::string function, std::vector<std::string> args, DoCore& ttls,
												AoSystem& aoSys, std::vector<parameterType>& params, 
												std::string& warnings,  timeType& operationTime, std::string callingScope )
{	
	/// load the file
	std::fstream functionFile;
	// check if file address is good.
	FILE *file;
	fopen_s ( &file, cstr ( FUNCTIONS_FOLDER_LOCATION + function + "." + FUNCTION_EXTENSION ), "r" );
	if ( !file )
	{
		thrower ( "Function " + function + " does not exist! The master script tried to open this function, it"
				  " tried and failed to open the location " + FUNCTIONS_FOLDER_LOCATION + function + "."
				  + FUNCTION_EXTENSION + "." );
	}
	else
	{
		fclose ( file );
	}
	functionFile.open ( FUNCTIONS_FOLDER_LOCATION + function + "." + FUNCTION_EXTENSION, std::ios::in );
	// check opened correctly
	if ( !functionFile.is_open ( ) )
	{
		thrower ( "Function file " + function + "File passed test making sure the file exists, but it still "
				  "failed to open! (A low level bug, this shouldn't happen.)" );
	}
	// append __END__ to the end of the file for analysis purposes.
	std::stringstream buf;
	ScriptStream functionStream;
	buf << functionFile.rdbuf ( );
	functionStream << buf.str ( );
	functionStream << "\r\n\r\n__END__";
	functionFile.close ( );
	if ( functionStream.str ( ) == "" )
	{
		thrower ( "Function File for " + function + " function was empty! (A low level bug, this shouldn't happen" );
	}
	std::string word;
	// the following are used for repeat: functionality
	std::vector<ULONG> totalRepeatNum, currentRepeatNum;
	std::vector<std::streamoff> repeatPos;
	std::string scope = function;
	/// get the function arguments.
	std::string defLine, name;
	defLine = functionStream.getline ( ':' );
	std::vector<std::string> functionArgs;
	analyzeFunctionDefinition ( defLine, name, functionArgs );
	if ( functionArgs.size ( ) != args.size ( ) )
	{
		std::string functionArgsString;
		for ( auto elem : args )
		{
			functionArgsString += elem + ",";
		}
		thrower ( "incorrect number of arguments in the call for function " + function + ". Number in call was: "
				  + str ( args.size ( ) ) + ", number expected was " + str ( functionArgs.size ( ) ) 
				  + ". Function arguments were:" + functionArgsString + "." );
	}
	std::vector<std::pair<std::string, std::string>> replacements;
	for (auto replacementInc : range(args.size()))
	{
		replacements.push_back ( { functionArgs[ replacementInc ], args[ replacementInc ] } );
	}
	functionStream.loadReplacements ( replacements, params, function, callingScope, function );
	std::string currentFunctionText = functionStream.str ( );
	///
	functionStream >> word;
	while ( !( functionStream.peek ( ) == EOF ) || word != "__end__" )
	{
		if (handleTimeCommands (word, functionStream, params, scope, operationTime)){ /* got handled*/ }
		else if ( handleVariableDeclaration ( word, functionStream, params, scope, warnings ) ){}
		else if ( handleDoCommands ( word, functionStream, params, ttls, scope, operationTime) ){}
		else if ( handleAoCommands ( word, functionStream, params, aoSys, ttls, scope, operationTime) ){}
		else if ( word == "callcppcode" )
		{
			// and that's it... 
			callCppCodeFunction ( );
		}
		/// Handle RSG calls.
		else if ( word == "rsg:" )
		{
			thrower ("\"rsg:\" command is deprecated! Please use the microwave system listview instead.");
		}
		/// deal with function calls.
		else if ( handleFunctionCall ( word, functionStream, params, ttls, aoSys, warnings, function, operationTime ) ) {}
		else if ( word == "repeat:" )
		{
			std::string repeatStr;
			functionStream >> repeatStr;
			try
			{
				totalRepeatNum.push_back ( boost::lexical_cast<int> ( repeatStr ) );
			}
			catch ( boost::bad_lexical_cast& )
			{
				throwNested ( "the repeat number for a repeat structure inside the master script failed to convert "
						  "to an integer! Note that the repeat number can not currently be a variable." );
			}
			repeatPos.push_back ( functionStream.tellg ( ) );
			currentRepeatNum.push_back ( 1 );
		}
		else if ( word == "end" )
		{
			if ( currentRepeatNum.size ( ) == 0 )
			{
				thrower ( "mismatched \"end\" command for repeat structure in master script! there were more "
						  "\"end\" commands than \"repeat\" commands." );
			}
			if ( currentRepeatNum.back ( ) < totalRepeatNum.back ( ) )
			{
				functionStream.seekg ( repeatPos.back ( ) );
				currentRepeatNum.back ( )++;
			}
			else
			{
				// remove the entries corresponding to this repeat loop.
				currentRepeatNum.pop_back ( );
				repeatPos.pop_back ( );
				totalRepeatNum.pop_back ( );
				// and continue (no seekg)
			}
		}
		else
		{
			thrower ( "unrecognized master script command inside function analysis: " + word );
		}
		functionStream >> word;
	}
}


bool ExperimentThreadManager::getAbortStatus ( )
{
	return isAborting;
}

double ExperimentThreadManager::convertToTime( timeType time, std::vector<parameterType> variables, UINT variation )
{
	double variableTime = 0;
	// add together current values for all variable times.
	if ( time.first.size( ) != 0 )
	{
		for ( auto varTime : time.first )
		{
			variableTime += varTime.evaluate( variables, variation );
		}
	}
	return variableTime + time.second;
}


void ExperimentThreadManager::handleDebugPlots( debugInfo debugOptions, Communicator& comm, DoCore& ttls, AoSystem& aoSys,
									  std::vector<std::vector<pPlotDataVec>> ttlData, 
									  std::vector<std::vector<pPlotDataVec>> dacData )
{
	// handle on-screen plots.
	ttls.fillPlotData( 0, ttlData );
	aoSys.fillPlotData( 0, dacData );
	if ( debugOptions.showTtls )
	{
		comm.sendDebug( ttls.getTtlSequenceMessage( 0 ) );
	}
	if ( debugOptions.showDacs )
	{
		comm.sendDebug( aoSys.getDacSequenceMessage( 0 ) );
	}
	// no quiet on warnings or debug messages.
	comm.sendDebug( debugOptions.message );
}


bool ExperimentThreadManager::runningStatus()
{
	return experimentIsRunning;
}


/***
 * this function is very similar to startExperimentThread but instead of getting anything from the current profile, it
 * knows exactly where to look for the MOT profile. This is currently hard-coded.
 */
void ExperimentThreadManager::loadMotSettings(ExperimentThreadInput* input)
{	
	if ( experimentIsRunning )
	{
		delete input;
		thrower ( "Experiment is Running! Please abort the current run before setting the MOT settings." );
	}
	input->thisObj = this;
	runningThread = (HANDLE)_beginthreadex( NULL, NULL, &ExperimentThreadManager::experimentThreadProcedure, input, NULL, NULL );
}


HANDLE ExperimentThreadManager::startExperimentThread(ExperimentThreadInput* input)
{
	if ( !input )
	{
		thrower ( "Input to start experiment thread was null?!?!? (a Low level bug, this shouldn't happen)." );
	}
	if ( experimentIsRunning )
	{
		delete input;
		thrower ( "Experiment is already Running! You can only run one experiment at a time! Please abort before "
				 "running again." );
	}
	input->thisObj = this;
	runningThread = (HANDLE)_beginthreadex( NULL, NULL, &ExperimentThreadManager::experimentThreadProcedure, input, NULL, NULL );
	SetThreadPriority( runningThread, THREAD_PRIORITY_HIGHEST );
	return runningThread;
}


bool ExperimentThreadManager::getIsPaused()
{
	return isPaused;
}


void ExperimentThreadManager::pause()
{
	if ( !experimentIsRunning )
	{
		thrower ( "Can't pause the experiment if the experiment isn't running!" );
	}
	isPaused = true;
}


void ExperimentThreadManager::unPause()
{
	if ( !experimentIsRunning )
	{
		thrower ( "Can't unpause the experiment if the experiment isn't running!" );
	}
	isPaused = false;
}


void ExperimentThreadManager::abort()
{
	if ( !experimentIsRunning )
	{
		thrower ( "Can't abort the experiment if the experiment isn't running!" );
	}
	isAborting = true;
}

void ExperimentThreadManager::loadAgilentScript ( std::string scriptAddress, ScriptStream& agilentScript )
{
	std::ifstream scriptFile ( scriptAddress );
	if ( !scriptFile.is_open ( ) )
	{
		thrower ( "Scripted Agilent File \"" + scriptAddress + "\" failed to open!" );
	}
	agilentScript << scriptFile.rdbuf ( );
	agilentScript.seekg ( 0 );
	scriptFile.close ( );
}


void ExperimentThreadManager::loadNiawgScript ( std::string scriptAddress, ScriptStream& niawgScript )
{
	std::ifstream scriptFile;
	// check if file address is good.
	FILE *file;
	fopen_s ( &file, cstr ( scriptAddress ), "r" );
	if ( !file )
	{
		thrower ( "The Niawg Script File " + scriptAddress + " does not exist! The Master-Manager tried to "
				  "open this file before starting the script analysis." );
	}
	else
	{
		fclose ( file );
	}
	scriptFile.open ( cstr ( scriptAddress ) );
	// check opened correctly
	if ( !scriptFile.is_open ( ) )
	{
		thrower ( "File passed test making sure the file exists, but it still failed to open?!?! "
				  "(A low level-bug, this shouldn't happen.)" );
	}
	// dump the file into the stringstream.
	niawgScript << scriptFile.rdbuf ( );
	scriptFile.close ( );
}


void ExperimentThreadManager::loadMasterScript(std::string scriptAddress, ScriptStream& currentMasterScript )
{
	std::ifstream scriptFile;
	// check if file address is good.
	FILE *file;
	fopen_s( &file, cstr(scriptAddress), "r" );
	if ( !file )
	{
		thrower ("The Master Script File " + scriptAddress + " does not exist! The Master-Manager tried to "
				 "open this file before starting the script analysis.");
	}
	else
	{
		fclose( file );
	}
	scriptFile.open(cstr(scriptAddress));
	// check opened correctly
	if (!scriptFile.is_open())
	{
		thrower ("File passed test making sure the file exists, but it still failed to open?!?! "
				 "(A low level-bug, this shouldn't happen.)");
	}
	// dump the file into the stringstream.
	std::stringstream buf( std::ios_base::app | std::ios_base::out | std::ios_base::in );
	// IMPORTANT!
	buf << "\r\n t = 0.01 \r\n pulseon: " + str( OSCILLOSCOPE_TRIGGER ) + " 0.02\r\n t += 0.1\r\n";
	buf << scriptFile.rdbuf();
	// this is used to more easily deal some of the analysis of the script.
	buf << "\r\n\r\n__END__";
	// for whatever reason, after loading rdbuf into a stringstream, the stream seems to not 
	// want to >> into a string. tried resetting too using seekg, but whatever, this works.
	currentMasterScript.str("");
	currentMasterScript.str( buf.str());
	currentMasterScript.clear();
	currentMasterScript.seekg(0);
	scriptFile.close();
}


// makes sure formatting is correct, returns the arguments and the function name from reading the firs real line of a function file.
void ExperimentThreadManager::analyzeFunctionDefinition(std::string defLine, std::string& functionName, std::vector<std::string>& args)
{
	args.clear();
	ScriptStream defStream(defLine);
	std::string word;
	defStream >> word;
	if (word == "")
	{
		defStream >> word;
	}
	if (word != "def")
	{
		thrower ("Function file (extenion \".func\") in functions folder was not a function because it did not"
				 " start with \"def\"! Functions must start with this. Instead it started with \"" + word + "\".");
	}
	std::string functionDeclaration, functionArgumentList;
	functionDeclaration = defStream.getline( ':' );
	int initNamePos = defLine.find_first_not_of(" \t");
	functionName = functionDeclaration.substr(initNamePos, functionDeclaration.find_first_of("(") - initNamePos);

	if (functionName.find_first_of(" ") != std::string::npos)
	{
		thrower ("Function name included a space!");
	}
	int initPos = functionDeclaration.find_first_of("(");
	if (initPos == std::string::npos)
	{
		thrower ("No starting parenthesis \"(\" in function definition. Use \"()\" if no arguments.");
	}
	initPos++;
	int endPos = functionDeclaration.find_last_of(")");
	if (endPos == std::string::npos)
	{
		thrower ("No ending parenthesis \")\" in function definition. Use \"()\" if no arguments.");
	}
	functionArgumentList = functionDeclaration.substr(initPos, endPos - initPos);
	endPos = functionArgumentList.find_first_of(",");
	initPos = functionArgumentList.find_first_not_of(" \t");
	bool good = true;
	// fill out args.
	while (initPos != std::string::npos)
	{
		// get initial argument
		std::string tempArg = functionArgumentList.substr(initPos, endPos - initPos);
		if (endPos == std::string::npos)
		{
			functionArgumentList = "";
		}
		else
		{
			functionArgumentList.erase(0, endPos + 1);
		}
		// clean up any spaces on beginning and end.
		int lastChar = tempArg.find_last_not_of(" \t");
		int lastSpace = tempArg.find_last_of(" \t");
		if (lastSpace > lastChar)
		{
			tempArg = tempArg.substr(0, lastChar + 1);
		}
		// now it should be clean. Check if there are spaces in the middle.
		if (tempArg.find_first_of(" \t") != std::string::npos)
		{
			thrower ( "bad argument list in function. It looks like there might have been a space or tab inside "
					 "the function argument? (A low level bug, this shouldn't happen.)" );
		}
		if (tempArg == "")
		{
			thrower ("bad argument list in function. It looks like there might have been a stray \",\"?");
		}
		args.push_back(tempArg);
		endPos = functionArgumentList.find_first_of(",");
		initPos = functionArgumentList.find_first_not_of(" \t");
	}
}

// at least right now, this doesn't support varying any of the values of the constant vector. this could probably
// be sensibly changed at some point.
bool ExperimentThreadManager::handleVectorizedValsDeclaration ( std::string word, ScriptStream& stream, 
															std::vector<vectorizedNiawgVals>& constVecs, std::string& warnings )
{
	if ( word != "var_v" )
	{
		return false;
	}
	std::string vecLength;
	vectorizedNiawgVals tmpVec;
	stream >> vecLength >> tmpVec.name;
	for ( auto& cv : constVecs )
	{
		if ( tmpVec.name == cv.name )
		{
			thrower ( "Constant Vector name \"" + tmpVec.name + "\"being re-used! You may only declare one constant "
					  "vector with this name." );
		}
	}
	UINT vecLength_ui = 0;
	try
	{
		vecLength_ui = boost::lexical_cast<UINT>( vecLength );
	}
	catch ( boost::bad_lexical_cast )
	{
		thrower ( "Failed to convert constant vector length to an unsigned int!" );
	}
	if ( vecLength_ui == 0 || vecLength_ui > MAX_NIAWG_SIGNALS)
	{
		thrower ( "Invalid constant vector length: " + str ( vecLength_ui ) + ", must be greater than 0 and less than " 
				  + str ( MAX_NIAWG_SIGNALS ) );
	}
	std::string bracketDelims;
	stream >> bracketDelims;
	if ( bracketDelims != "[" )
	{
		thrower ( "Expected \"[\" after constant vector size and name." );
	}
	tmpVec.vals.resize ( vecLength_ui );
	for ( auto& val : tmpVec.vals )
	{
		stream >> val;
	}
	stream >> bracketDelims;
	if ( bracketDelims != "]" )
	{
		thrower ( "Expected \"]\" after constant vector values. Is the vector size right?" );
	}
	constVecs.push_back ( tmpVec );
	return true;
}

std::vector<parameterType> ExperimentThreadManager::getLocalParameters (ScriptStream& stream)
{
	std::string scriptText = stream.str ();
	if (scriptText == "")
	{
		return {};
	}
	std::string word;
	stream >> word;
	// the analysis loop.
	std::vector<parameterType> params;
	std::vector<vectorizedNiawgVals> niawgParams;
	std::string warnings="";
	while (!(stream.peek () == EOF) && !stream.eof() && word != "__end__")
	{
		auto peekpos = stream.peek ();
		try
		{
			if (handleVariableDeclaration (word, stream, params, GLOBAL_PARAMETER_SCOPE, warnings))
			{
			}
			else (handleVectorizedValsDeclaration (word, stream, niawgParams, warnings));
		}
		catch (Error & err) { /*Easy for this to happen. */}
		word = "";
		stream >> word;
	}
	for (auto& param : niawgParams)
	{
		parameterType temp;
		temp.name = param.name;
		params.push_back (temp);
	}
	return params;
}

bool ExperimentThreadManager::handleVariableDeclaration( std::string word, ScriptStream& stream, std::vector<parameterType>& vars,
													 std::string scope, std::string& warnings )
{
	if ( word != "var" )
	{
		return false;
	}
	// add to variables!
	std::string name, valStr;
	stream >> name >> valStr;
	parameterType tmpVariable;
	tmpVariable.constant = true;
	std::transform( name.begin( ), name.end( ), name.begin( ), ::tolower );
	tmpVariable.name = name;
	for ( auto var : vars )
	{
		if ( var.name == tmpVariable.name )
		{
			if ( var.parameterScope == GLOBAL_PARAMETER_SCOPE )
			{
				warnings += "Warning: local variable \"" + tmpVariable.name + "\" with scope \"" 
					+ scope + "\"is being overwritten by a parameter with the same name and "
					"global parameter scope. It is generally recommended to use the appropriate local scope when "
					"possible.\r\n";
				// this variable is being overwritten, so don't add this variable vector
				return true;
			}
			else if ( str( var.parameterScope, 13, false, true ) == str( scope, 13, false, true ) )
			{
				// being overwritten so don't add, but the variable was specific, so this must be fine.
				return true;
			}
		}
	}
	bool found = false;
	double val;
	try
	{
		val = boost::lexical_cast<double>( valStr );
	}
	catch ( boost::bad_lexical_cast& )
	{
		throwNested ( "Bad string for value of local variable " + str( name ) );
	}
	tmpVariable.constantValue = val;
	tmpVariable.scanDimension = 1;
	tmpVariable.parameterScope = str(scope, 13, false, true);
	tmpVariable.ranges.push_back ( { val, val } );
	// these are always constants, so just go ahead and set the keyvalue for use manually. 
	if ( vars.size( ) == 0 )
	{
		tmpVariable.keyValues = std::vector<double>( 1, val );
	}
	else
	{
		tmpVariable.keyValues = std::vector<double>( vars.front( ).keyValues.size( ), val );
	}	
	vars.push_back( tmpVariable );
	return true;
}


// if it handled it, returns true, else returns false.
bool ExperimentThreadManager::handleTimeCommands( std::string word, ScriptStream& stream, std::vector<parameterType>& vars, 
											  std::string scope, timeType& operationTime )
{
	try
	{
		if (word == "t")
		{
			std::string command;
			stream >> command;
			word += command;
		}
		//
		if (word == "t++")
		{
			operationTime.second++;
		}
		else if (word == "t+=")
		{
			Expression time;
			stream >> time;
			try
			{
				operationTime.second += time.evaluate ();
			}
			catch (Error&)
			{
				time.assertValid (vars, scope);
				operationTime.first.push_back (time);
			}
		}
		else if (word == "t=")
		{
			Expression time;
			stream >> time;
			try
			{
				operationTime.second = time.evaluate ();
			}
			catch (Error&)
			{
				time.assertValid (vars, scope);
				operationTime.first.push_back (time);
				// because it's equals. There shouldn't be any extra terms added to this now.
				operationTime.second = 0;
			}
		}
		else
		{
			return false;
		}
		return true;
	}
	catch (Error & err)
	{
		throwNested ("Error seen while handling time commands. Word was \"" + word + "\"");
	}
}

/* returns true if handles word, false otherwise. */
bool ExperimentThreadManager::handleDoCommands( std::string word, ScriptStream& stream, std::vector<parameterType>& vars,
											    DoCore& ttls, std::string scope, timeType& operationTime )
{
	if ( word == "on:" || word == "off:" )
	{
		std::string name;
		stream >> name;
		ttls.handleTtlScriptCommand( word, operationTime, name, vars, scope );
	}
	else if ( word == "pulseon:" || word == "pulseoff:" )
	{
		// this requires handling time as it is handled above.
		std::string name;
		Expression pulseLength;
		stream >> name >> pulseLength;
		ttls.handleTtlScriptCommand( word, operationTime, name, pulseLength, vars, scope );
	}
	else
	{
		return false;
	}
	return true;
}

/* returns true if handles word, false otherwise. */
bool ExperimentThreadManager::handleAoCommands( std::string word, ScriptStream& stream, std::vector<parameterType>& vars,
												AoSystem& aoSys, DoCore& ttls, std::string scope, timeType& operationTime )
{
	if ( word == "dac:" )
	{
		AoCommandForm command;
		std::string name;
		stream >> name >> command.finalVal;
		command.finalVal.assertValid( vars, scope );
		command.time = operationTime;
		command.commandName = "dac:";
		command.numSteps.expressionStr = command.initVal.expressionStr = "__NONE__";
		command.rampTime.expressionStr = command.rampInc.expressionStr = "__NONE__";
		try
		{
			aoSys.handleDacScriptCommand( command, name, vars, ttls );
		}
		catch ( Error&  )
		{
			throwNested( "Error handling \"dac:\" command." );
		}
	}
	else if ( word == "daclinspace:" )
	{
		AoCommandForm command;
		std::string name;
		stream >> name >> command.initVal >> command.finalVal >> command.rampTime >> command.numSteps;
		command.initVal.assertValid( vars, scope );
		command.finalVal.assertValid( vars, scope );
		command.rampTime.assertValid( vars, scope );
		command.numSteps.assertValid( vars, scope );
		command.time = operationTime;
		command.commandName = "daclinspace:";
		// not used here.
		command.rampInc.expressionStr = "__NONE__";
		//
		try
		{
			aoSys.handleDacScriptCommand( command, name, vars, ttls );
		}
		catch ( Error& )
		{
			throwNested(  "Error handling \"dacLinSpace:\" command." );
		}
	}
	else if ( word == "dacarange:" )
	{
		AoCommandForm command;
		std::string name;
		stream >> name >> command.initVal >> command.finalVal >> command.rampTime >> command.rampInc;
		command.initVal.assertValid( vars, scope );
		command.finalVal.assertValid( vars, scope );
		command.rampTime.assertValid( vars, scope );
		command.rampInc.assertValid( vars, scope );
		command.time = operationTime;
		command.commandName = "dacarange:";
		// not used here.
		command.numSteps.expressionStr = "__NONE__";
		try
		{
			aoSys.handleDacScriptCommand( command, name, vars, ttls );
		}
		catch ( Error& )
		{
			throwNested("Error handling \"dacArange:\" command." );
		}
	}
	else
	{
		return false;
	}
	return true;
}


/*
	this function can be called directly from scripts. Insert things inside the function to make it do something
	custom that's not possible inside the scripting language.
*/
void ExperimentThreadManager::callCppCodeFunction()
{
	// not used at the moment
}


bool ExperimentThreadManager::isValidWord( std::string word )
{
	if (word == "t" || word == "t++" || word == "t+=" || word == "t=" || word == "on:" || word == "off:"
		 || word == "dac:" || word == "dacarange:" || word == "daclinspace:" || word == "call" 
		 || word == "repeat:" || word == "end" || word == "pulseon:" || word == "pulseoff:" || word == "callcppcode")
	{
		return true;
	}
	return false;
}

UINT ExperimentThreadManager::determineVariationNumber( std::vector<parameterType> variables )
{
	int variationNumber;
	if ( variables.size() == 0)
	{
		variationNumber = 1;
	}
	else
	{
		variationNumber = variables.front().keyValues.size();
		if (variationNumber == 0)
		{
			variationNumber = 1;
		}
	}
	return variationNumber;
}


void ExperimentThreadManager::checkTriggerNumbers ( std::unique_ptr<ExperimentThreadInput>& input, 
													std::vector<parameterType>& expParams )
{
	/// check all trigger numbers between the DO system and the individual subsystems. These should almost always match,
	/// a mismatch is usually user error in writing the script.
	bool niawgMismatch = false, rsgMismatch=false;
	for ( auto variationInc : range ( determineVariationNumber(expParams) ) )
	{
		if ( input->runList.master)
		{
			UINT actualTrigs = input->ttls.countTriggers ( { DoRows::which::D,15 }, variationInc );
			UINT dacExpectedTrigs = input->aoSys.getNumberSnapshots ( variationInc );
			std::string infoString = "Actual/Expected DAC Triggers: " + str ( actualTrigs ) + "/" 
				+ str ( dacExpectedTrigs ) + ".";
			if ( actualTrigs != dacExpectedTrigs )
			{
				// this is a serious low level error. throw, don't warn.
				thrower ( "the number of dac triggers that the ttl system sends to the dac line does not "
						  "match the number of dac snapshots! " + infoString + ", seen in variation #" 
						  + str ( variationInc ) + "\r\n" );
			}
			if ( variationInc == 0 && input->debugOptions.outputExcessInfo )
			{
				input->debugOptions.message += infoString + "\n";
			}
		}
		auto& niawg = input->devices.getSingleDevice<NiawgCore> ();
		if (niawg.experimentActive && !niawgMismatch )
		{
			auto actualTrigs = input->ttls.countTriggers ( niawg.getTrigLines ( ), variationInc );
			auto niawgExpectedTrigs = niawg.getNumberTrigsInScript ( );
			std::string infoString = "Actual/Expected NIAWG Triggers: " + str ( actualTrigs ) + "/" 
				+ str ( niawgExpectedTrigs ) + ".";
			if ( actualTrigs != niawgExpectedTrigs )
			{
				input->comm.warnings += "WARNING: the NIAWG is not getting triggered by the ttl system the same number"
					" of times a trigger command appears in the NIAWG script. " + infoString + " First "
					"instance seen variation " + str ( variationInc ) 
					+ ".\r\n";
				niawgMismatch = true;
			}
			if ( variationInc == 0 && input->debugOptions.outputExcessInfo )
			{
				input->debugOptions.message += infoString + "\n";
			}
		}
		/// check RSG
		auto& rsg = input->devices.getSingleDevice< MicrowaveCore > ();
		if ( !rsgMismatch )
		{
			auto actualTrigs = input->ttls.countTriggers ( rsg.getRsgTriggerLine ( ), variationInc );
			auto rsgExpectedTrigs = rsg.getNumTriggers ( rsg.experimentSettings );
			std::string infoString = "Actual/Expected RSG Triggers: " + str ( actualTrigs ) + "/"
				+ str ( rsgExpectedTrigs ) + ".";
			if ( actualTrigs != rsgExpectedTrigs && rsgExpectedTrigs != 0 && rsgExpectedTrigs != 1 )
			{
				input->comm.warnings += "WARNING: the RSG is not getting triggered by the ttl system the same number"
					" of times a trigger command appears in the master script. " + infoString + " First "
					"instance seen variation " + str ( variationInc )
					+ ".\r\n";
				rsgMismatch = true;
			}
			if ( variationInc == 0 && input->debugOptions.outputExcessInfo )
			{
				input->debugOptions.message += infoString + "\n";
			}
			/// check Agilents
			for ( auto& agilent : input->devices.getDevicesByClass<AgilentCore>() )
			{
				agilent.get().checkTriggers ( variationInc, input->ttls, input->comm, input->debugOptions.outputExcessInfo );
			}
		}
	}
}


bool ExperimentThreadManager::handleFunctionCall( std::string word, ScriptStream& stream, std::vector<parameterType>& vars,
											      DoCore& ttls, AoSystem& aoSys, std::string& warnings, 
												  std::string callingFunction, timeType& operationTime )
{
	if ( word != "call" )
	{
		return false;
	}
	// calling a user-defined function. Get the name and the arguments to pass to the function handler.
	std::string functionCall, functionName, functionArgs;
	functionCall = stream.getline( '\n' );
	boost::erase_all ( functionCall, "\r" );
	int pos = functionCall.find_first_of( "(" ) + 1;
	int finalpos = functionCall.find_last_of( ")" );
	functionName = functionCall.substr( 0, pos - 1 );
	functionArgs = functionCall.substr( pos, finalpos - pos );
	std::string arg;
	std::vector<std::string> args;
	while ( true )
	{
		pos = functionArgs.find_first_of( ',' );
		if ( pos == std::string::npos )
		{
			arg = functionArgs.substr( 0, functionArgs.size( ) );
			if ( arg != "" )
			{
				args.push_back( arg );
			}
			break;
		}
		arg = functionArgs.substr( 0, pos );
		args.push_back( arg );
		// cut oinputut that argument off the string.
		functionArgs = functionArgs.substr( pos + 1, functionArgs.size( ) );
	}
	if ( functionName == callingFunction )
	{
		thrower ( "Recursive function call detected! " + callingFunction + " called itself! This is not allowed."
				 " There is no way to end a function call conditionally so this will necessarily result in an"
				 " infinite recursion\r\n" );
	}
	try
	{
		analyzeFunction( functionName, args, ttls, aoSys, vars, warnings, operationTime, callingFunction);
	}
	catch ( Error& )
	{
		throwNested( "Error handling Function call to function " + functionName + "." );
	}
	return true;
}


void ExperimentThreadManager::updatePlotX_vals ( std::unique_ptr<ExperimentThreadInput>& input, std::vector<parameterType>& expParams)
{
	// remove old plots that aren't trying to sustain.
	input->plotterInput->key = ParameterSystem::getKeyValues ( expParams );
	auto& pltInput = input->plotterInput;
	auto plotInc = 0;
	for ( auto plotParams : pltInput->plotInfo )
	{
		int which = pltInput->dataArrays.size () - pltInput->plotInfo.size () + plotInc;
		if (which < 0 || which >= pltInput->dataArrays.size ())
		{
			thrower ("Plotter data array access out of range?!");
		}
		auto& data = pltInput->dataArrays[which];
		for ( auto& line : data )
		{
			// initialize x axis for all data sets.
			UINT count = 0;
			for ( auto& keyItem : pltInput->key )
			{
				line->at ( count++ ).x = keyItem;
			}
		}
		plotInc++;
	}
}

void ExperimentThreadManager::calculateAdoVariations ( std::unique_ptr<ExperimentThreadInput>& input, 
													   std::vector<parameterType>& expParams, UINT repetitions, 
													   ScriptStream& masterStream, UINT skipThreshold )
{
	if (input->runList.master)
	{
		auto variations = determineVariationNumber (expParams);
		input->aoSys.resetDacEvents ();
		input->ttls.resetTtlEvents ();
		input->aoSys.initializeDataObjects (0);
		input->ttls.initializeDataObjects (0);
		input->thisObj->loadSkipTimes = std::vector<double> (variations);
		input->comm.expUpdate ("Analyzing Master Script...\n");
		input->comm.sendColorBox (System::Master, 'Y');
		input->thisObj->analyzeMasterScript (input->ttls, input->aoSys, expParams, masterStream,
			skipThreshold != UINT_MAX, input->comm.warnings, input->thisObj->operationTime, 
			input->thisObj->loadSkipTime);
		input->ttls.calculateVariations (expParams, input->comm);
		input->aoSys.calculateVariations (expParams,input->comm);
		for (auto variationInc : range (variations))
		{
			if (input->thisObj->isAborting) { thrower (abortString); }
			double& currLoadSkipTime = input->thisObj->loadSkipTimes[variationInc];
			currLoadSkipTime = ExperimentThreadManager::convertToTime (input->thisObj->loadSkipTime, expParams,
				variationInc);
			input->aoSys.standardExperimentPrep (variationInc, input->ttls, expParams, currLoadSkipTime);
			input->ttls.standardExperimentPrep (variationInc, currLoadSkipTime, expParams);
			input->aoSys.checkTimingsWork (variationInc);
		}
		input->comm.expUpdate ("Programmed time per repetition: " + str (input->ttls.getTotalTime (0)) + "\r\n");
		ULONGLONG totalTime = 0;
		for (auto variationNumber : range (variations))
		{
			totalTime += ULONGLONG (input->ttls.getTotalTime (variationNumber) * repetitions);
		}
		input->comm.expUpdate ("Programmed Total Experiment time: " + str (totalTime) + "\r\n");
		input->comm.expUpdate ("Number of TTL Events in experiment: " + str (input->ttls.getNumberEvents (0)) + "\r\n");
		input->comm.expUpdate ("Number of DAC Events in experiment: " + str (input->aoSys.getNumberEvents (0)) + "\r\n");
		handleDebugPlots (input->debugOptions, input->comm, input->ttls, input->aoSys, input->ttlData, input->dacData);
		input->comm.sendColorBox (System::Master, 'G');
	}
}

void ExperimentThreadManager::runConsistencyChecks ( std::unique_ptr<ExperimentThreadInput>& input, 
													 std::vector<parameterType> expParams )
{
	input->globalControl.setUsages (expParams);
	for (auto& var : expParams)
	{
		if (!var.constant && !var.active)
		{
			input->comm.warnings += "WARNING: Variable " + var.name + " is varied, but not being used?!?\r\n";
		}
	}
	ExperimentThreadManager::checkTriggerNumbers (input, expParams);
	if (input->comm.warnings != "")
	{
		input->comm.sendError (input->comm.warnings);
		auto response = promptBox ("WARNING: The following warnings were reported while preparing the experiment:\r\n"
			+ input->comm.warnings + "\r\nIs this acceptable? (press no to abort)", MB_YESNO);
		if (response == IDNO) { thrower (abortString); }
	}
	input->comm.warnings = ""; // then reset so as to not mindlessly repeat warnings from the experiment loop.
}


void ExperimentThreadManager::handlePause (Communicator& comm, std::atomic<bool>& isPaused, std::atomic<bool>& isAborting)
{
	if (isAborting) { thrower (abortString); }
	if (isPaused)
	{
		comm.expUpdate ("PAUSED\r\n!");
		while (isPaused)
		{
			Sleep (100);
			if (isAborting) { thrower (abortString); }
		}
		comm.expUpdate ("UN-PAUSED!\r\n");
	}
}

void ExperimentThreadManager::initVariation ( std::unique_ptr<ExperimentThreadInput>& input, UINT variationInc, 
											  std::vector<parameterType> expParams)
{
	auto variations = determineVariationNumber (expParams);
	input->comm.expUpdate ("Variation #" + str (variationInc + 1) + "/" + str (variations) + ": ");
	auto& aiSys = input->devices.getSingleDevice<AiSystem> ();
	auto& andorCamera = input->devices.getSingleDevice<AndorCameraCore> ();
	if (aiSys.wantsQueryBetweenVariations())
	{
		// the main gui thread does the whole measurement here. This probably makes less sense now. 
		input->comm.expUpdate ("Querying Voltages...\r\n");
		input->comm.sendLogVoltsMessage (variationInc);
	}
	if (input->debugOptions.sleepTime != 0) { Sleep (input->debugOptions.sleepTime); }
	for (auto param : expParams)
	{
		if (param.valuesVary)
		{
			if (param.keyValues.size () == 0)
			{
				thrower ("Variable " + param.name + " varies, but has no values assigned to "
					"it! (This shouldn't happen, it's a low-level bug...)");
			}
			input->comm.expUpdate (param.name + ": " + str (param.keyValues[variationInc], 12) + "\r\n");
		}
	}
	while (true)
	{
		if (andorCamera.queryStatus () == DRV_ACQUIRING)
		{
			Sleep (1000);
			input->comm.expUpdate ("Waiting for Andor camera to finish acquisition...");
			if (input->thisObj->isAborting) { thrower (abortString); }
		}
		else { break; }
	}
	input->comm.sendError (input->comm.warnings);
	input->comm.sendRepProgress (0);
}

void ExperimentThreadManager::errorFinish ( Communicator& comm, std::atomic<bool>& isAborting, Error& exception,
											std::chrono::time_point<chronoClock> startTime)
{
	if (isAborting)
	{
		comm.expUpdate (abortString);
		comm.sendColorBox (System::Master, 'B');
	}
	else
	{
		comm.sendColorBox (System::Master, 'R');
		comm.sendStatus ("Bad Exit!\r\n");
		auto txt = "Exited main experiment thread abnormally." + exception.trace ();
		comm.sendFatalError (txt);
	}
	{
		isAborting = false;
	}
	auto exp_t = std::chrono::duration_cast<std::chrono::seconds>((chronoClock::now () - startTime)).count ();
	comm.expUpdate ( "Experiment took " + str (int (exp_t) / 3600) + " hours, " + str (int (exp_t) % 3600 / 60) + " minutes, "
					 + str (int (exp_t) % 60) + " seconds.\r\n");
}

void ExperimentThreadManager::normalFinish ( Communicator& comm, ExperimentType& expType, bool runMaster, 
											 std::chrono::time_point<chronoClock> startTime, AoSystem& aoSys)
{
	if (runMaster) {
		try {
			aoSys.stopDacs ();
			aoSys.setDacStatusNoForceOut (aoSys.getFinalSnapshot ());
		}
		catch (Error&) { /* this gets thrown if no dac events. just continue.*/ }
	}
	switch (expType)
	{
		case ExperimentType::CameraCal:
			comm.sendCameraCalFin ();
			break;
		case ExperimentType::LoadMot:
		case ExperimentType::MachineOptimization:
		case ExperimentType::AutoCal:
			comm.sendFinish (expType);
			break;
		default:
			comm.sendFinish (ExperimentType::Normal);
	}
	comm.expUpdate ("\r\nExperiment Finished Normally.\r\n");
	comm.sendColorBox (System::Master, 'B');
	auto exp_t = std::chrono::duration_cast<std::chrono::seconds>((chronoClock::now () - startTime)).count ();
	comm.expUpdate ( "Experiment took " + str (int (exp_t) / 3600) + " hours, " + str (int (exp_t) % 3600 / 60) + " minutes, "
					 + str (int (exp_t) % 60) + " seconds.\r\n");
}

void ExperimentThreadManager::adoStart (std::unique_ptr<ExperimentThreadInput>& input, UINT repInc, UINT variationInc, bool skip)
{
	if (input->runList.master)
	{
		input->comm.sendRepProgress (repInc + 1);
		input->aoSys.resetDacs (variationInc, skip);
		input->ttls.ftdi_trigger ();
		input->ttls.FtdiWaitTillFinished (variationInc);
	}
}