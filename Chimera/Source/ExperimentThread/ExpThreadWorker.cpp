#include "stdafx.h"
#include "ExpThreadWorker.h"
#include <ConfigurationSystems/ConfigSystem.h>
#include <MiscellaneousExperimentOptions/Repetitions.h>
#include <Scripts/Script.h>
#include <boost/algorithm/string/replace.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

ExpThreadWorker::ExpThreadWorker (ExperimentThreadInput* input_, std::atomic<bool>& expRunning) 
	: experimentIsRunning(expRunning){
	input = std::unique_ptr< ExperimentThreadInput >((ExperimentThreadInput*)input_);
}

ExpThreadWorker::~ExpThreadWorker () {
}

void ExpThreadWorker::process (){
	experimentThreadProcedure();
	emit mainProcessFinish ();
}

/*
 * The workhorse of actually running experiments. This thread procedure analyzes all of the GUI settings and current
 * configuration settings to determine how to program and run the experiment.
 */
void ExpThreadWorker::experimentThreadProcedure () {
	auto startTime = chronoClock::now ();
	experimentIsRunning = true;
	notify(qstr("Starting Experiment " + input->profile.configuration + "...\n"));
	ExpRuntimeData expRuntime;
	isPaused = false;
	try {
	 	for (auto& device : input->devices.list) {
	 		emit updateBoxColor ("Black", device.get ().getDelim ().c_str ());
	 	}
		emit updateBoxColor ("Black", "Other");
		notify ("Loading Experiment Settings...\n");
		ConfigStream cStream (input->profile.configFilePath (), true);
		loadExperimentRuntime (cStream, expRuntime);
		if (input->expType != ExperimentType::LoadMot) {
			notify({ "Loading Master Runtime...\n", 1 });
			input->logger.logMasterRuntime (expRuntime.repetitions, expRuntime.expParams);
		}
		for (auto& device : input->devices.list) {
			deviceLoadExpSettings (device, cStream);
		}
		/// The Variation Calculation Step.
		notify ("Calculating All Variation Data...\r\n");
		for (auto& device : input->devices.list) {
			deviceCalculateVariations (device, expRuntime.expParams);
		}
		calculateAdoVariations (expRuntime);
		runConsistencyChecks (expRuntime.expParams);
		if (input->expType != ExperimentType::LoadMot) {
			for (auto& device : input->devices.list) {
				if (device.get ().experimentActive) {
					notify({ qstr("Logging Devce " + device.get().getDelim()
						+ " Settings...\n"), 1 });
					device.get ().logSettings (input->logger, this);
				}
			}
		}
		/// Begin experiment 
		for (const auto& variationInc : range (determineVariationNumber (expRuntime.expParams))) {
			initVariation (variationInc, expRuntime.expParams);
			notify ("Programming Devices for Variation...\n");
			for (auto& device : input->devices.list) {
				deviceProgramVariation (device, expRuntime.expParams, variationInc);
			}
			notify ("Running Experiment.\n");
			for (const auto& repInc : range (expRuntime.repetitions)) {
				handlePause (isPaused, isAborting);
				startRep (repInc, variationInc, input->skipNext == nullptr ? false : input->skipNext->load ());
			}
		}
		waitForAndorFinish ();
		for (auto& device : input->devices.list) {
			deviceNormalFinish (device);
		}
		normalFinish (input->expType, true /*runmaster*/, startTime);
	}
	catch (ChimeraError & exception) {
		for (auto& device : input->devices.list) {
			// don't change the colors, the colors should reflect the end state before the error. 
			if (device.get ().experimentActive) {
				device.get ().errorFinish ();
			}
		}
		errorFinish (isAborting, exception, startTime);
	}
	experimentIsRunning = false;
}


void ExpThreadWorker::analyzeMasterScript (DoCore& ttls, AoSystem& aoSys, std::vector<parameterType>& vars,
	ScriptStream& currentMasterScript, bool expectsLoadSkip,
	std::string& warnings, timeType& operationTime,
	timeType& loadSkipTime) {
	std::string currentMasterScriptText = currentMasterScript.str ();
	loadSkipTime.first.clear ();
	loadSkipTime.second = 0;
	// starts at 0.1 if not initialized by the user.
	operationTime.second = 0.1;
	operationTime.first.clear ();
	if (currentMasterScript.str () == "") {
		thrower ("Master script is empty! (A low level bug, this shouldn't happen)");
	}
	std::string word;
	currentMasterScript >> word;
	std::vector<unsigned> totalRepeatNum, currentRepeatNum;
	std::vector<std::streamoff> repeatPos;
	// the analysis loop.
	bool loadSkipFound = false;
	std::string scope = PARENT_PARAMETER_SCOPE;
	try {
		while (!(currentMasterScript.peek () == EOF) || word != "__end__") {
			if (handleTimeCommands (word, currentMasterScript, vars, scope, operationTime)) {
				// got handled, so break out of the if-else by entering this scope.
			}
			else if (handleVariableDeclaration (word, currentMasterScript, vars, scope, warnings)) {}
			else if (handleDoCommands (word, currentMasterScript, vars, ttls, scope, operationTime)) {}
			else if (handleAoCommands (word, currentMasterScript, vars, aoSys, ttls, scope, operationTime)) {}
			else if (word == "callcppcode") {
				// and that's it... 
				callCppCodeFunction ();
			}
			else if (word == "loadskipentrypoint!") {
				loadSkipFound = true;
				loadSkipTime = operationTime;
			}
			/// Deal with RSG calls
			else if (word == "rsg:") {
				thrower ("\"rsg:\" command is deprecated! Please use the microwave system listview instead.");
			}
			else if (handleFunctionCall (word, currentMasterScript, vars, ttls, aoSys, warnings,
				PARENT_PARAMETER_SCOPE, operationTime)) {
			}
			else if (word == "repeat:") {
				Expression repeatStr;
				currentMasterScript >> repeatStr;
				try {
					totalRepeatNum.push_back (repeatStr.evaluate ());
				}
				catch (ChimeraError&) {
					throwNested ("the repeat number failed to convert to an integer! Note that the repeat number can not"
						" currently be a variable.");
				}
				repeatPos.push_back (currentMasterScript.tellg ());
				currentRepeatNum.push_back (1);
			}
			else if (word == "end") {
				// handle end of repeat
				if (currentRepeatNum.size () == 0) {
					thrower ("ERROR! Tried to end repeat structure in master script, but you weren't repeating!");
				}
				if (currentRepeatNum.back () < totalRepeatNum.back ()) {
					currentMasterScript.seekg (repeatPos.back ());
					currentRepeatNum.back ()++;
				}
				else {
					currentRepeatNum.pop_back ();
					repeatPos.pop_back ();
					totalRepeatNum.pop_back ();
				}
			}
			else {
				word = (word == "") ? "[EMPTY-STRING]" : word;
				thrower ("unrecognized master script command: \"" + word + "\"");
			}
			word = "";
			currentMasterScript >> word;
		}
	}
	catch (ChimeraError&) {
		throwNested ("Error Seen While Analyzing Master Script!");
	}
	if (expectsLoadSkip && !loadSkipFound) {
		thrower ("Expected load skip in script, but the load skip command was not found during script analysis!");
	}
}


void ExpThreadWorker::analyzeFunction (std::string function, std::vector<std::string> args, DoCore& ttls,
	AoSystem& aoSys, std::vector<parameterType>& params, std::string& warnings, timeType& operationTime,
	std::string callingScope) {
	/// load the file
	std::fstream functionFile;
	// check if file address is good.
	FILE* file;
	fopen_s (&file, cstr (FUNCTIONS_FOLDER_LOCATION + function + "." + Script::FUNCTION_EXTENSION), "r");
	if (!file) {
		thrower ("Function " + function + " does not exist! The master script tried to open this function, it"
			" tried and failed to open the location " + FUNCTIONS_FOLDER_LOCATION + function + "."
			+ Script::FUNCTION_EXTENSION + ".");
	}
	else {
		fclose (file);
	}
	functionFile.open (FUNCTIONS_FOLDER_LOCATION + function + "." + Script::FUNCTION_EXTENSION, std::ios::in);
	// check opened correctly
	if (!functionFile.is_open ()) {
		thrower ("Function file " + function + "File passed test making sure the file exists, but it still "
			"failed to open! (A low level bug, this shouldn't happen.)");
	}
	// append __END__ to the end of the file for analysis purposes.
	std::stringstream buf;
	ScriptStream functionStream;
	buf << functionFile.rdbuf ();
	functionStream << buf.str ();
	functionStream << "\r\n\r\n__END__";
	functionFile.close ();
	if (functionStream.str () == "") {
		thrower ("Function File for " + function + " function was empty! (A low level bug, this shouldn't happen");
	}
	std::string word;
	// the following are used for repeat: functionality
	std::vector<unsigned long> totalRepeatNum, currentRepeatNum;
	std::vector<std::streamoff> repeatPos;
	std::string scope = function;
	/// get the function arguments.
	std::string defLine, name;
	defLine = functionStream.getline (':');
	std::vector<std::string> functionArgs;
	analyzeFunctionDefinition (defLine, name, functionArgs);
	if (functionArgs.size () != args.size ()) {
		std::string functionArgsString;
		for (auto elem : args) {
			functionArgsString += elem + ",";
		}
		thrower ("incorrect number of arguments in the call for function " + function + ". Number in call was: "
			+ str (args.size ()) + ", number expected was " + str (functionArgs.size ())
			+ ". Function arguments were:" + functionArgsString + ".");
	}
	std::vector<std::pair<std::string, std::string>> replacements;
	for (auto replacementInc : range (args.size ())) {
		replacements.push_back ({ functionArgs[replacementInc], args[replacementInc] });
	}
	functionStream.loadReplacements (replacements, params, function, callingScope, function);
	std::string currentFunctionText = functionStream.str ();
	///
	functionStream >> word;
	try {
		while (!(functionStream.peek () == EOF) || word != "__end__") {
			if (handleTimeCommands (word, functionStream, params, scope, operationTime)) { /* got handled*/ }
			else if (handleVariableDeclaration (word, functionStream, params, scope, warnings)) {}
			else if (handleDoCommands (word, functionStream, params, ttls, scope, operationTime)) {}
			else if (handleAoCommands (word, functionStream, params, aoSys, ttls, scope, operationTime)) {}
			else if (word == "callcppcode") {
				// and that's it... 
				callCppCodeFunction ();
			}
			/// Handle RSG calls.
			else if (word == "rsg:") {
				thrower ("\"rsg:\" command is deprecated! Please use the microwave system listview instead.");
			}
			/// deal with function calls.
			else if (handleFunctionCall (word, functionStream, params, ttls, aoSys, warnings, function, operationTime)) {}
			else if (word == "repeat:") {
				std::string repeatStr;
				functionStream >> repeatStr;
				try {
					totalRepeatNum.push_back (boost::lexical_cast<int> (repeatStr));
				}
				catch (boost::bad_lexical_cast&) {
					throwNested ("the repeat number for a repeat structure inside the master script failed to convert "
						"to an integer! Note that the repeat number can not currently be a variable.");
				}
				repeatPos.push_back (functionStream.tellg ());
				currentRepeatNum.push_back (1);
			}
			else if (word == "end") {
				if (currentRepeatNum.size () == 0) {
					thrower ("mismatched \"end\" command for repeat structure in master script! there were more "
						"\"end\" commands than \"repeat\" commands.");
				}
				if (currentRepeatNum.back () < totalRepeatNum.back ()) {
					functionStream.seekg (repeatPos.back ());
					currentRepeatNum.back ()++;
				}
				else {
					// remove the entries corresponding to this repeat loop.
					currentRepeatNum.pop_back ();
					repeatPos.pop_back ();
					totalRepeatNum.pop_back ();
					// and continue (no seekg)
				}
			}
			else {
				thrower ("unrecognized master script command inside function analysis: " + word);
			}
			functionStream >> word;
		}
	}
	catch (ChimeraError&) {
		throwNested ("Failed to analyze function \"" + function + "\"");
	}
}


bool ExpThreadWorker::getAbortStatus () {
	return isAborting;
}

double ExpThreadWorker::convertToTime (timeType time, std::vector<parameterType> variables, unsigned variation) {
	double variableTime = 0;
	// add together current values for all variable times.
	if (time.first.size () != 0) {
		for (auto varTime : time.first) {
			variableTime += varTime.evaluate (variables, variation);
		}
	}
	return variableTime + time.second;
}

void ExpThreadWorker::handleDebugPlots (DoCore& ttls, AoSystem& aoSys, unsigned variation) {
	emit doAoData (ttls.getPlotData (variation), aoSys.getPlotData (variation));
	notify({ qstr(ttls.getTtlSequenceMessage(variation)), 2 });
	notify({ qstr(aoSys.getDacSequenceMessage(variation)), 2 });
}

bool ExpThreadWorker::runningStatus () {
	return experimentIsRunning;
}

void ExpThreadWorker::startExperimentThread (ExperimentThreadInput* input, IChimeraQtWindow* parent) {
}

bool ExpThreadWorker::getIsPaused () {
	return isPaused;
}

void ExpThreadWorker::pause () {
	if (!experimentIsRunning) {
		thrower ("Can't pause the experiment if the experiment isn't running!");
	}
	isPaused = true;
}

void ExpThreadWorker::unPause () {
	if (!experimentIsRunning) {
		thrower ("Can't unpause the experiment if the experiment isn't running!");
	}
	isPaused = false;
}

void ExpThreadWorker::abort () {
	if (!experimentIsRunning) {
		thrower ("Can't abort the experiment if the experiment isn't running!");
	}
	isAborting = true;
}

void ExpThreadWorker::loadAgilentScript (std::string scriptAddress, ScriptStream& agilentScript) {
	std::ifstream scriptFile (scriptAddress);
	if (!scriptFile.is_open ()) {
		thrower ("Scripted Agilent File \"" + scriptAddress + "\" failed to open!");
	}
	agilentScript << scriptFile.rdbuf ();
	agilentScript.seekg (0);
	scriptFile.close ();
}

void ExpThreadWorker::loadNiawgScript (std::string scriptAddress, ScriptStream& niawgScript) {
	std::ifstream scriptFile;
	// check if file address is good.
	FILE* file;
	fopen_s (&file, cstr (scriptAddress), "r");
	if (!file) {
		thrower ("The Niawg Script File \"" + scriptAddress + "\" does not exist! The Master-Manager tried to "
			"open this file before starting the script analysis.");
	}
	else {
		fclose (file);
	}
	scriptFile.open (cstr (scriptAddress));
	// check opened correctly
	if (!scriptFile.is_open ()) {
		thrower ("File passed test making sure the file exists, but it still failed to open?!?! "
			"(A low level-bug, this shouldn't happen.)");
	}
	// dump the file into the stringstream.	
	niawgScript.str ("");
	niawgScript.clear ();
	niawgScript << scriptFile.rdbuf ();
	niawgScript.seekg (0);
	scriptFile.close ();
}

void ExpThreadWorker::loadMasterScript (std::string scriptAddress, ScriptStream& currentMasterScript) {
	std::ifstream scriptFile;
	// check if file address is good.
	FILE* file;
	fopen_s (&file, cstr (scriptAddress), "r");
	if (!file) {
		thrower ("The Master Script File " + scriptAddress + " does not exist! The Master-Manager tried to "
			"open this file before starting the script analysis.");
	}
	else {
		fclose (file);
	}
	scriptFile.open (cstr (scriptAddress));
	// check opened correctly
	if (!scriptFile.is_open ()) {
		thrower ("File passed test making sure the file exists, but it still failed to open?!?! "
			"(A low level-bug, this shouldn't happen.)");
	}
	// dump the file into the stringstream.
	std::stringstream buf (std::ios_base::app | std::ios_base::out | std::ios_base::in);
	// IMPORTANT!
	buf << "\r\n t = 0.01 \r\n pulseon: " + str (OSCILLOSCOPE_TRIGGER) + " 0.02\r\n t += 0.1\r\n";
	buf << scriptFile.rdbuf ();
	// this is used to more easily deal some of the analysis of the script.
	buf << "\r\n\r\n__END__";
	// for whatever reason, after loading rdbuf into a stringstream, the stream seems to not 
	// want to >> into a string. tried resetting too using seekg, but whatever, this works.
	currentMasterScript.str ("");
	currentMasterScript.str (buf.str ());
	currentMasterScript.clear ();
	currentMasterScript.seekg (0);
	scriptFile.close ();
}

// makes sure formatting is correct, returns the arguments and the function name from reading the firs real line of a function file.
void ExpThreadWorker::analyzeFunctionDefinition (std::string defLine, std::string& functionName,
												 std::vector<std::string>& args) {
	args.clear ();
	ScriptStream defStream (defLine);
	std::string word;
	defStream >> word;
	if (word == "") {
		defStream >> word;
	}
	if (word != "def") {
		thrower ("Function file (extenion \".func\") in functions folder was not a function because it did not"
			" start with \"def\"! Functions must start with this. Instead it started with \"" + word + "\".");
	}
	std::string functionDeclaration, functionArgumentList;
	functionDeclaration = defStream.getline (':');
	int initNamePos = defLine.find_first_not_of (" \t");
	functionName = functionDeclaration.substr (initNamePos, functionDeclaration.find_first_of ("(") - initNamePos);

	if (functionName.find_first_of (" ") != std::string::npos) {
		thrower ("Function name included a space!");
	}
	int initPos = functionDeclaration.find_first_of ("(");
	if (initPos == std::string::npos) {
		thrower ("No starting parenthesis \"(\" in function definition. Use \"()\" if no arguments.");
	}
	initPos++;
	int endPos = functionDeclaration.find_last_of (")");
	if (endPos == std::string::npos) {
		thrower ("No ending parenthesis \")\" in function definition. Use \"()\" if no arguments.");
	}
	functionArgumentList = functionDeclaration.substr (initPos, endPos - initPos);
	endPos = functionArgumentList.find_first_of (",");
	initPos = functionArgumentList.find_first_not_of (" \t");
	bool good = true;
	// fill out args.
	while (initPos != std::string::npos) {
		// get initial argument
		std::string tempArg = functionArgumentList.substr (initPos, endPos - initPos);
		if (endPos == std::string::npos) {
			functionArgumentList = "";
		}
		else {
			functionArgumentList.erase (0, endPos + 1);
		}
		// clean up any spaces on beginning and end.
		int lastChar = tempArg.find_last_not_of (" \t");
		int lastSpace = tempArg.find_last_of (" \t");
		if (lastSpace > lastChar){
			tempArg = tempArg.substr (0, lastChar + 1);
		}
		// now it should be clean. Check if there are spaces in the middle.
		if (tempArg.find_first_of (" \t") != std::string::npos){
			thrower ("bad argument list in function. It looks like there might have been a space or tab inside "
				"the function argument? (A low level bug, this shouldn't happen.)");
		}
		if (tempArg == ""){
			thrower ("bad argument list in function. It looks like there might have been a stray \",\"?");
		}
		args.push_back (tempArg);
		endPos = functionArgumentList.find_first_of (",");
		initPos = functionArgumentList.find_first_not_of (" \t");
	}
}

// at least right now, this doesn't support varying any of the values of the constant vector. this could probably
// be sensibly changed at some point.
bool ExpThreadWorker::handleVectorizedValsDeclaration (std::string word, ScriptStream& stream,
	std::vector<vectorizedNiawgVals>& constVecs, std::string& warnings) {
	if (word != "var_v") {
		return false;
	}
	std::string vecLength;
	vectorizedNiawgVals tmpVec;
	stream >> vecLength >> tmpVec.name;
	for (auto& cv : constVecs) {
		if (tmpVec.name == cv.name) {
			thrower ("Constant Vector name \"" + tmpVec.name + "\"being re-used! You may only declare one constant "
				"vector with this name.");
		}
	}
	unsigned vecLength_ui = 0;
	try {
		vecLength_ui = boost::lexical_cast<unsigned>(vecLength);
	}
	catch (boost::bad_lexical_cast) {
		thrower ("Failed to convert constant vector length to an unsigned int!");
	}
	if (vecLength_ui == 0 || vecLength_ui > NiawgConstants::MAX_NIAWG_SIGNALS) {
		thrower ("Invalid constant vector length: " + str (vecLength_ui) + ", must be greater than 0 and less than "
			+ str (NiawgConstants::MAX_NIAWG_SIGNALS));
	}
	std::string bracketDelims;
	stream >> bracketDelims;
	if (bracketDelims != "[") {
		thrower ("Expected \"[\" after constant vector size and name (make sure it's separated by spaces).");
	}
	tmpVec.vals.resize (vecLength_ui);
	for (auto& val : tmpVec.vals) {
		stream >> val;
	}
	stream >> bracketDelims;
	if (bracketDelims != "]") {
		thrower ("Expected \"]\" after constant vector values (make sure it's separated by spaces). Is the vector size right?");
	}
	constVecs.push_back (tmpVec);
	return true;
}

std::vector<parameterType> ExpThreadWorker::getLocalParameters (ScriptStream& stream) {
	std::string scriptText = stream.str ();
	if (scriptText == "") {
		return {};
	}
	std::string word;
	stream >> word;
	// the analysis loop.
	std::vector<parameterType> params;
	std::vector<vectorizedNiawgVals> niawgParams;
	std::string warnings = "";
	while (!(stream.peek () == EOF) && !stream.eof () && word != "__end__") {
		auto peekpos = stream.peek ();
		try {
			if (handleVariableDeclaration (word, stream, params, GLOBAL_PARAMETER_SCOPE, warnings)) {
			}
			else (handleVectorizedValsDeclaration (word, stream, niawgParams, warnings));
		}
		catch (ChimeraError&) { /*Easy for this to happen. */ }
		word = "";
		stream >> word;
	}
	for (auto& param : niawgParams) {
		parameterType temp;
		temp.name = param.name;
		params.push_back (temp);
	}
	return params;
}

bool ExpThreadWorker::handleVariableDeclaration (std::string word, ScriptStream& stream, std::vector<parameterType>& vars,
	std::string scope, std::string& warnings) {
	if (word != "var") {
		return false;
	}
	// add to variables!
	std::string name, valStr;
	stream >> name >> valStr;
	parameterType tmpVariable;
	tmpVariable.constant = true;
	std::transform (name.begin (), name.end (), name.begin (), ::tolower);
	tmpVariable.name = name;
	for (auto var : vars) {
		if (var.name == tmpVariable.name) {
			if (var.parameterScope == GLOBAL_PARAMETER_SCOPE) {
				warnings += "Warning: local variable \"" + tmpVariable.name + "\" with scope \""
					+ scope + "\"is being overwritten by a parameter with the same name and "
					"global parameter scope. It is generally recommended to use the appropriate local scope when "
					"possible.\r\n";
				// this variable is being overwritten, so don't add this variable vector
				return true;
			}
			else if (str (var.parameterScope, 13, false, true) == str (scope, 13, false, true)) {
				// being overwritten so don't add, but the variable was specific, so this must be fine.
				return true;
			}
		}
	}
	bool found = false;
	double val;
	try {
		val = boost::lexical_cast<double>(valStr);
	}
	catch (boost::bad_lexical_cast&) {
		throwNested ("Bad string for value of local variable " + str (name));
	}
	tmpVariable.constantValue = val;
	tmpVariable.scanDimension = 1;
	tmpVariable.parameterScope = str (scope, 13, false, true);
	tmpVariable.ranges.push_back ({ val, val });
	// these are always constants, so just go ahead and set the keyvalue for use manually. 
	if (vars.size () == 0) {
		tmpVariable.keyValues = std::vector<double> (1, val);
	}
	else {
		tmpVariable.keyValues = std::vector<double> (vars.front ().keyValues.size (), val);
	}
	vars.push_back (tmpVariable);
	return true;
}


// if it handled it, returns true, else returns false.
bool ExpThreadWorker::handleTimeCommands (std::string word, ScriptStream& stream, std::vector<parameterType>& vars,
	std::string scope, timeType& operationTime) {
	try {
		if (word == "t") {
			std::string command;
			stream >> command;
			word += command;
		}
		//
		if (word == "t++") {
			operationTime.second++;
		}
		else if (word == "t+=") {
			Expression time;
			stream >> time;
			try {
				operationTime.second += time.evaluate ();
			}
			catch (ChimeraError&) {
				time.assertValid (vars, scope);
				operationTime.first.push_back (time);
			}
		}
		else if (word == "t=") {
			Expression time;
			stream >> time;
			try {
				operationTime.first.clear();
				operationTime.second = time.evaluate ();
			}
			catch (ChimeraError&) {
				time.assertValid (vars, scope);
				operationTime.first.clear();
				operationTime.first.push_back (time);
				// because it's equals. There shouldn't be any extra terms added to this now.
				operationTime.second = 0;
			}
		}
		else {
			return false;
		}
		return true;
	}
	catch (ChimeraError&) {
		throwNested ("Error seen while handling time commands. Word was \"" + word + "\"");
	}
}

/* returns true if handles word, false otherwise. */
bool ExpThreadWorker::handleDoCommands (std::string word, ScriptStream& stream, std::vector<parameterType>& vars,
	DoCore& ttls, std::string scope, timeType& operationTime) {
	if (word == "on:" || word == "off:") {
		std::string name;
		stream >> name;
		ttls.handleTtlScriptCommand (word, operationTime, name, vars, scope);
	}
	else if (word == "pulseon:" || word == "pulseoff:") {
		// this requires handling time as it is handled above.
		std::string name;
		Expression pulseLength;
		stream >> name >> pulseLength;
		ttls.handleTtlScriptCommand (word, operationTime, name, pulseLength, vars, scope);
	}
	else {
		return false;
	}
	return true;
}

/* returns true if handles word, false otherwise. */
bool ExpThreadWorker::handleAoCommands (std::string word, ScriptStream& stream,	std::vector<parameterType>& params, AoSystem& aoSys, DoCore& ttls,
	std::string scope, timeType& operationTime) {
	if (word == "cao:") {
		// calibrated analog out. Right now, the code doesn't having a variable calibration setting, as this would
		// require the calibration to... do something funny. 
		AoCommandForm command;
		std::string dacName, calibrationName, calibratedVal;
		stream >> calibrationName >> dacName >> calibratedVal;

		command.finalVal.assertValid (params, scope);
		command.time = operationTime;
		command.commandName = "dac:";
		command.numSteps.expressionStr = command.initVal.expressionStr = "__NONE__";
		command.rampTime.expressionStr = command.rampInc.expressionStr = "__NONE__";
		try {
			aoSys.handleDacScriptCommand (command, dacName, params, ttls);
		}
		catch (ChimeraError&) {
			throwNested ("Error handling \"cao:\" command.");
		}
	}
	if (word == "dac:") {
		AoCommandForm command;
		std::string name;
		stream >> name >> command.finalVal;
		command.finalVal.assertValid (params, scope);
		command.time = operationTime;
		command.commandName = "dac:";
		command.numSteps.expressionStr = command.initVal.expressionStr = "__NONE__";
		command.rampTime.expressionStr = command.rampInc.expressionStr = "__NONE__";
		try {
			aoSys.handleDacScriptCommand (command, name, params, ttls);
		}
		catch (ChimeraError&) {
			throwNested ("Error handling \"dac:\" command.");
		}
	}
	else if (word == "daclinspace:") {
		AoCommandForm command;
		std::string name;
		stream >> name >> command.initVal >> command.finalVal >> command.rampTime >> command.numSteps;
		command.initVal.assertValid (params, scope);
		command.finalVal.assertValid (params, scope);
		command.rampTime.assertValid (params, scope);
		command.numSteps.assertValid (params, scope);
		command.time = operationTime;
		command.commandName = "daclinspace:";
		// not used here.
		command.rampInc.expressionStr = "__NONE__";
		//
		try {
			aoSys.handleDacScriptCommand (command, name, params, ttls);
		}
		catch (ChimeraError&) {
			throwNested ("Error handling \"dacLinSpace:\" command.");
		}
	}
	else if (word == "dacarange:")
	{
		AoCommandForm command;
		std::string name;
		stream >> name >> command.initVal >> command.finalVal >> command.rampTime >> command.rampInc;
		command.initVal.assertValid (params, scope);
		command.finalVal.assertValid (params, scope);
		command.rampTime.assertValid (params, scope);
		command.rampInc.assertValid (params, scope);
		command.time = operationTime;
		command.commandName = "dacarange:";
		// not used here.
		command.numSteps.expressionStr = "__NONE__";
		try {
			aoSys.handleDacScriptCommand (command, name, params, ttls);
		}
		catch (ChimeraError&) {
			throwNested ("Error handling \"dacArange:\" command.");
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
void ExpThreadWorker::callCppCodeFunction () {
	// not used at the moment
}


bool ExpThreadWorker::isValidWord (std::string word) {
	if (word == "t" || word == "t++" || word == "t+=" || word == "t=" || word == "on:" || word == "off:"
		|| word == "dac:" || word == "dacarange:" || word == "daclinspace:" || word == "call"
		|| word == "repeat:" || word == "end" || word == "pulseon:" || word == "pulseoff:" || word == "callcppcode") {
		return true;
	}
	return false;
}

unsigned ExpThreadWorker::determineVariationNumber (std::vector<parameterType> variables) {
	int variationNumber;
	if (variables.size () == 0) {
		variationNumber = 1;
	}
	else {
		variationNumber = variables.front ().keyValues.size ();
		if (variationNumber == 0) {
			variationNumber = 1;
		}
	}
	return variationNumber;
}

void ExpThreadWorker::checkTriggerNumbers (std::vector<parameterType>& expParams) {
	/// check all trigger numbers between the DO system and the individual subsystems. These should almost always match,
	/// a mismatch is usually user error in writing the script.
	bool niawgMismatch = false, uwaveMismatch = false;
	for (auto variationInc : range (determineVariationNumber (expParams))) {
		if (true /*runMaster*/) {
			unsigned actualTrigs = input->ttls.countTriggers ({ DoRows::which::D,15 }, variationInc);
			unsigned dacExpectedTrigs = input->aoSys.getNumberSnapshots (variationInc);
			std::string infoString = "Actual/Expected DAC Triggers: " + str (actualTrigs) + "/"
				+ str (dacExpectedTrigs) + ".";
			if (actualTrigs != dacExpectedTrigs) {
				// this is a serious low level error. throw, don't warn.
				thrower ("the number of dac triggers that the ttl system sends to the dac line does not "
					"match the number of dac snapshots! " + infoString + ", seen in variation #"
					+ str (variationInc) + "\r\n");
			}
			if (variationInc == 0) {
				notify({ qstr(infoString), 2 });
			}
		}
		auto& niawg = input->devices.getSingleDevice<NiawgCore> ();
		if (niawg.experimentActive && !niawgMismatch) {
			auto actualTrigs = input->ttls.countTriggers (niawg.getTrigLines (), variationInc);
			auto niawgExpectedTrigs = niawg.getNumberTrigsInScript ();
			std::string infoString = "Actual/Expected NIAWG Triggers: " + str (actualTrigs) + "/"
				+ str (niawgExpectedTrigs) + ".";
			if (actualTrigs != niawgExpectedTrigs)
			{
				emit warn (
					cstr ("WARNING: the NIAWG is not getting triggered by the ttl system the same number"
						" of times a trigger command appears in the NIAWG script. " + infoString + " First "
						"instance seen variation " + str (variationInc) + ".\r\n"));
				niawgMismatch = true;
			}
			if (variationInc == 0) {
				notify({ qstr(infoString), 2 });
			}
		}
		/// check Microwave System
		auto& uwaveCore = input->devices.getSingleDevice< MicrowaveCore > ();
		if (!uwaveMismatch) {
			auto actualTrigs = input->ttls.countTriggers (uwaveCore.getUWaveTriggerLine (), variationInc);
			auto uwaveExpectedTrigs = uwaveCore.getNumTriggers (uwaveCore.experimentSettings);
			statusMsg infoMsg("",2,"");
			infoMsg.msg = "Actual/Expected Microwave Triggers: " + qstr (actualTrigs) + "/"
				+ qstr (uwaveExpectedTrigs) + ".";
			if (actualTrigs != uwaveExpectedTrigs && uwaveExpectedTrigs != 0 && uwaveExpectedTrigs != 1) {
				emit warn (qstr (
					"WARNING: the Microwave System is not getting triggered by the ttl system the same number"
					" of times a trigger command appears in the master script. " + infoMsg.msg + " First "
					"instance seen variation " + qstr (variationInc) + ".\r\n"));
				uwaveMismatch = true;
			}
			if (variationInc == 0) {
				notify (infoMsg);
			}
			/// check Agilents
			for (auto& agilent : input->devices.getDevicesByClass<AgilentCore> ()) {
				agilent.get ().checkTriggers (variationInc, input->ttls, this);
			}
		}
	}
}

bool ExpThreadWorker::handleFunctionCall (std::string word, ScriptStream& stream, std::vector<parameterType>& vars,
	DoCore& ttls, AoSystem& aoSys, std::string& warnings,
	std::string callingFunction, timeType& operationTime) {
	if (word != "call") {
		return false;
	}
	// calling a user-defined function. Get the name and the arguments to pass to the function handler.
	std::string functionCall, functionName, functionArgs;
	functionCall = stream.getline ('\n');
	//boost::erase_all (functionCall, "\r");
	int pos = functionCall.find_first_of ("(") + 1;
	int finalpos = functionCall.find_last_of (")");
	functionName = functionCall.substr (0, pos - 1);
	functionArgs = functionCall.substr (pos, finalpos - pos);
	std::string arg;
	std::vector<std::string> args;
	while (true) {
		pos = functionArgs.find_first_of (',');
		if (pos == std::string::npos) {
			arg = functionArgs.substr (0, functionArgs.size ());
			if (arg != "") {
				args.push_back (arg);
			}
			break;
		}
		arg = functionArgs.substr (0, pos);
		args.push_back (arg);
		// cut oinputut that argument off the string.
		functionArgs = functionArgs.substr (pos + 1, functionArgs.size ());
	}
	if (functionName == callingFunction) {
		thrower ("Recursive function call detected! " + callingFunction + " called itself! This is not allowed."
			" There is no way to end a function call conditionally so this will necessarily result in an"
			" infinite recursion\r\n");
	}
	try {
		analyzeFunction (functionName, args, ttls, aoSys, vars, warnings, operationTime, callingFunction);
	}
	catch (ChimeraError&) {
		throwNested ("Error handling Function call to function " + functionName + ".");
	}
	return true;
}

void ExpThreadWorker::calculateAdoVariations (ExpRuntimeData& runtime) {
	if (true /*runMaster*/) {
		auto variations = determineVariationNumber (runtime.expParams);
		input->aoSys.resetDacEvents ();
		input->ttls.resetTtlEvents ();
		input->aoSys.initializeDataObjects (0);
		input->ttls.initializeDataObjects (0);
		loadSkipTimes = std::vector<double> (variations);
		notify ("Analyzing Master Script...\n");
		std::string warnings;
		analyzeMasterScript (input->ttls, input->aoSys, runtime.expParams, runtime.masterScript,
			runtime.mainOpts.atomSkipThreshold != UINT_MAX, warnings, operationTime, loadSkipTime);
		emit warn (cstr (warnings));
		notify({ "Calcualting DO system variations...\n", 1 });
		input->ttls.calculateVariations (runtime.expParams, this);
		notify({ "Calcualting AO system variations...\n", 1 });
		input->aoSys.calculateVariations (runtime.expParams, this, input->calibrations);
		notify({ "Running final ado checks...\n",1});
		for (auto variationInc : range (variations)) {
			if (isAborting) { thrower (abortString); }
			double& currLoadSkipTime = loadSkipTimes[variationInc];
			currLoadSkipTime = convertToTime (loadSkipTime, runtime.expParams, variationInc);
			input->aoSys.standardExperimentPrep (variationInc, input->ttls, runtime.expParams, currLoadSkipTime);
			input->ttls.standardExperimentPrep (variationInc, currLoadSkipTime, runtime.expParams);
			input->aoSys.checkTimingsWork (variationInc);
		}
		notify({ ("Programmed time per repetition: " + str(input->ttls.getTotalTime(0)) + "\r\n").c_str(), 1 });
		unsigned __int64 totalTime = 0;
		for (auto variationNumber : range (variations)) {
			totalTime += unsigned __int64 (input->ttls.getTotalTime (variationNumber) * runtime.repetitions);
		}
		notify({ ("Programmed Total Experiment time: " + str(totalTime) + "\r\n").c_str(), 1 });
		notify({ ("Number of TTL Events in experiment: " + str(input->ttls.getNumberEvents(0)) + "\r\n").c_str(), 1 });
		notify({ ("Number of DAC Events in experiment: " + str(input->aoSys.getNumberEvents(0)) + "\r\n").c_str(), 1 });
	}
}

void ExpThreadWorker::runConsistencyChecks (std::vector<parameterType> expParams) {
	Sleep (1000);
	emit plot_Xvals_determined (ParameterSystem::getKeyValues (expParams));
	emit expParamsSet (expParams);
	//input->globalControl.setUsages (expParams);
	for (auto& var : expParams) {
		if (!var.constant && !var.active) {
			emit warn (cstr ("WARNING: Variable " + var.name + " is varied, but not being used?!?\r\n"));
		}
	}
	checkTriggerNumbers (expParams);
}

void ExpThreadWorker::handlePause (std::atomic<bool>& isPaused, std::atomic<bool>& isAborting) {
	if (isAborting) { thrower (abortString); }
	if (isPaused) {
		notify ("PAUSED\r\n!");
		while (isPaused) {
			Sleep (100);
			if (isAborting) { thrower (abortString); }
		}
		notify ("UN-PAUSED!\r\n");
	}
}

void ExpThreadWorker::initVariation (unsigned variationInc,std::vector<parameterType> expParams) {
	auto variations = determineVariationNumber (expParams);
	notify (("\nVariation #" + str (variationInc + 1) + "/" + str (variations) + ": ").c_str ());
	auto& aiSys = input->devices.getSingleDevice<AiSystem> ();
	if (aiSys.wantsQueryBetweenVariations ()) {
		// the main gui thread does the whole measurement here. This probably makes less sense now. 
		notify ("Querying Voltages...\r\n");
		//input->comm.sendLogVoltsMessage (variationInc);
	}
	if (input->sleepTime != 0) { Sleep (input->sleepTime); }
	for (auto param : expParams) {
		if (param.valuesVary) {
			if (param.keyValues.size () == 0) {
				thrower ("Variable " + param.name + " varies, but has no values assigned to "
					"it! (This shouldn't happen, it's a low-level bug...)");
			}
			notify ((param.name + ": " + str (param.keyValues[variationInc], 12) + "\r\n").c_str ());
		}
	}
	waitForAndorFinish ();
	bool skipOption = input->skipNext == nullptr ? false : input->skipNext->load ();
	if (true /*runMaster*/) { input->ttls.ftdi_write (variationInc, skipOption); }
	handleDebugPlots ( input->ttls, input->aoSys, variationInc );
}

void ExpThreadWorker::waitForAndorFinish () {
	auto& andorCamera = input->devices.getSingleDevice<AndorCameraCore> ();
	while (true) {
		if (andorCamera.queryStatus () == DRV_ACQUIRING) {
			Sleep (1000);
			notify ("Waiting for Andor camera to finish acquisition...\n");
			if (isAborting) { thrower (abortString); }
		}
		else { break; }
	}
}

void ExpThreadWorker::errorFinish (std::atomic<bool>& isAborting, ChimeraError& exception,
	std::chrono::time_point<chronoClock> startTime) {
	std::string finMsg;
	if (isAborting) {
		emit updateBoxColor ("Grey", "Other");
		finMsg = abortString.c_str ();
	}
	else {
		emit updateBoxColor ("Red", "Other");
		finMsg = "Bad Exit!\r\nExited main experiment thread abnormally." + exception.trace ();
	}
	isAborting = false;
	auto exp_t = std::chrono::duration_cast<std::chrono::seconds>((chronoClock::now () - startTime)).count ();
	emit errorExperimentFinish ((finMsg + "\r\nExperiment took " + str (int (exp_t) / 3600) + " hours, "
		+ str (int (exp_t) % 3600 / 60) + " minutes, "+ str (int (exp_t) % 60) + " seconds.\r\n").c_str (), 
		input->profile);
}

void ExpThreadWorker::normalFinish (ExperimentType& expType, bool runMaster,
	std::chrono::time_point<chronoClock> startTime) {
	auto exp_t = std::chrono::duration_cast<std::chrono::seconds>((chronoClock::now () - startTime)).count ();
	switch (expType) {
	case ExperimentType::AutoCal:
		emit calibrationFinish (("\r\nCalibration Finished Normally.\r\nExperiment took "
			+ str (int (exp_t) / 3600) + " hours, " + str (int (exp_t) % 3600 / 60)
			+ " minutes, " + str (int (exp_t) % 60) + " seconds.\r\n").c_str (),
			input->profile);
		break;
	default:
		emit normalExperimentFinish (("\r\nExperiment Finished Normally.\r\nExperiment took "
			+ str (int (exp_t) / 3600) + " hours, " + str (int (exp_t) % 3600 / 60)
			+ " minutes, " + str (int (exp_t) % 60) + " seconds.\r\n").c_str (), input->profile);
	}
}

void ExpThreadWorker::startRep (unsigned repInc, unsigned variationInc, bool skip) {
	if (true /*runMaster*/) {
		notify({ qstr("Starting Repetition #" + qstr(repInc) + "\n"), 2 });
		emit repUpdate (repInc + 1);
		input->aoSys.resetDacs (variationInc, skip);
		input->ttls.ftdi_trigger ();
		input->ttls.FtdiWaitTillFinished (variationInc);
	}
}

void ExpThreadWorker::deviceLoadExpSettings (IDeviceCore& device, ConfigStream& cStream) {
	try {
		notify({ "Loading Settings for device " + qstr(device.getDelim()) + "...\n", 1 });
		emit updateBoxColor ("White", device.getDelim ().c_str ());
		device.loadExpSettings (cStream);
	}
	catch (ChimeraError&) {
		emit updateBoxColor ("Red", device.getDelim ().c_str ());
		throwNested ("Error seen while loading experiment settings for system: " + device.getDelim ());
	}
}

void ExpThreadWorker::deviceProgramVariation (IDeviceCore& device, std::vector<parameterType>& expParams, 
	unsigned variationInc) {
	if (device.experimentActive) {
		try {
			notify({ qstr("Programming Devce " + device.getDelim() + "...\n"), 1 });
			device.programVariation (variationInc, expParams, this);
			emit updateBoxColor ("Green", device.getDelim ().c_str ());
		}
		catch (ChimeraError&) {
			emit updateBoxColor ("Red", device.getDelim ().c_str ());
			throwNested ("Error seen while programming variation for system: " + device.getDelim ());
		}
	}
}

void ExpThreadWorker::deviceCalculateVariations (IDeviceCore& device, std::vector<parameterType>& expParams) {
	if (device.experimentActive) {
		try {
			notify({ qstr("Calculating Variations for device " + device.getDelim()
				+ "...\n"), 1 });
			emit updateBoxColor ("Yellow", device.getDelim ().c_str ());
			device.calculateVariations (expParams, this);
		}
		catch (ChimeraError&) {
			emit updateBoxColor ("Red", device.getDelim ().c_str ());
			throwNested ("Error Seen while calculating variations for system: " + device.getDelim ());
		}
		if (isAborting) thrower (abortString);
	}
}

void ExpThreadWorker::notify(statusMsg msg) {
	if (msg.systemDelim == "") {
		msg.systemDelim = "EXP_THREAD_WORKER";
	}
	emit notification(msg);
}

void ExpThreadWorker::deviceNormalFinish (IDeviceCore& device) {
	if (device.experimentActive) {
		emit updateBoxColor ("Blue", device.getDelim ().c_str ());
		device.normalFinish ();
	}
	else {
		emit updateBoxColor ("Black", device.getDelim ().c_str ());
	}
}

void ExpThreadWorker::loadExperimentRuntime (ConfigStream& config, ExpRuntimeData& runtime) {
	runtime.expParams = ParameterSystem::combineParams (
		ParameterSystem::getConfigParamsFromFile (input->profile.configFilePath ()), input->globalParameters);
	ScanRangeInfo varRangeInfo = ParameterSystem::getRangeInfoFromFile (input->profile.configFilePath ());
	loadMasterScript (ConfigSystem::getMasterAddressFromConfig (input->profile), runtime.masterScript);
	runtime.mainOpts = ConfigSystem::stdConfigGetter (config, "MAIN_OPTIONS", MainOptionsControl::getSettingsFromConfig);
	runtime.repetitions = ConfigSystem::stdConfigGetter (config, "REPETITIONS", Repetitions::getSettingsFromConfig);
	ParameterSystem::generateKey (runtime.expParams, runtime.mainOpts.randomizeVariations, varRangeInfo);
}
