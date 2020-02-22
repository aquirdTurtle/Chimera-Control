// created by Mark O. Brown
#include "stdafx.h"
#include "ParameterSystem.h"
#include "Scripts/Script.h"
#include "DigitalOutput/DioSystem.h"
#include "PrimaryWindows/AuxiliaryWindow.h"
#include "ExcessDialogs/TextPromptDialog.h"
#include "GeneralObjects/multiDimensionalKey.h"
#include "GeneralUtilityFunctions/cleanString.h"

#include <iomanip>
#include <unordered_map>
#include <random>
#include "afxcmn.h"
#include <boost/lexical_cast.hpp>

ParameterSystem::ParameterSystem ( std::string configurationFileDelimiter ) : configDelim ( configurationFileDelimiter )
{ }


void ParameterSystem::initialize( POINT& pos, cToolTips& toolTips, CWnd* parent, int& id, std::string title,
								  UINT listviewId, ParameterSysType type )
{
	paramSysType = type;
	scanDimensions = 1;
	rangeInfo.defaultInit ( );
	rangeInfo.setNumScanDimensions ( 1 );
	rangeInfo.setNumRanges ( 0, 1 );

	parametersHeader.sPos = { pos.x, pos.y, pos.x + 480, pos.y += 25 };
	parametersHeader.Create( cstr( title ), NORM_HEADER_OPTIONS, parametersHeader.sPos, parent, id++ );
	parametersHeader.fontType = fontTypes::HeadingFont;
	
	parametersListview.sPos = { pos.x, pos.y, pos.x + 480, pos.y + 200 };
	parametersListview.Create( NORM_LISTVIEW_OPTIONS, parametersListview.sPos, parent, listviewId );
	
	RECT r;
	parametersListview.GetClientRect (&r);
	auto width = r.right - r.left;
	if ( paramSysType == ParameterSysType::global )
	{
		parametersListview.InsertColumn( 0, "Symbol", int (0.5*width));
		parametersListview.InsertColumn( 1, "Value", int (0.3*width ));
	}
	else 
	{
		parametersListview.InsertColumn( 0, "Symbol", int (0.2*width));
		parametersListview.InsertColumn( 1, "Type", int(0.1*width));
		parametersListview.InsertColumn( 2, "Dim");
		parametersListview.InsertColumn( 3, "Value");
		parametersListview.InsertColumn ( 4, "Scope" );
		setVariationRangeColumns ( 1, 0.05*width );
	}
	parametersListview.insertBlankRow ( );
	parametersListview.fontType = fontTypes::SmallCodeFont;
	parametersListview.SetTextBkColor ( _myRGBs["Interactable-Bkgd"] );
	parametersListview.SetTextColor ( _myRGBs["AuxWin-Text"] );
	parametersListview.SetBkColor( _myRGBs["Interactable-Bkgd"] );
	pos.y += 200;
}


void ParameterSystem::setVariationRangeColumns ( int num, int width )
{
	if ( currentParameters.size ( ) == 0 )
	{
		return;
	}
	auto nrange = int ( parametersListview.GetHeaderCtrl ( )->GetItemCount ( ) ) - int ( preRangeColumns );
	if ( nrange < 0 )
	{
		thrower ( "Somehow control thinks there are a negative number of range columns" );
	}
	for ( auto col : range ( nrange ) )
	{
		parametersListview.DeleteColumn ( preRangeColumns );
	}
	auto cInc = 5;
	for ( auto rInc : range ( (num==-1) ? currentParameters.front ( ).ranges.size ( ) : num) )
	{
		parametersListview.InsertColumn ( cInc++, str(rInc+1) + ". {", width );
		parametersListview.InsertColumn ( cInc++, "}", width );
		parametersListview.InsertColumn ( cInc++, "#", width );
	}
	parametersListview.InsertColumn ( cInc++, "+{}", width );
	parametersListview.InsertColumn ( cInc++, "-{}", width );
}


/*
 * The "normal" function, used for config and global variable systems.
 */
void ParameterSystem::handleOpenConfig( std::ifstream& configFile, Version ver )
{
	clearParameters( );
	/// 
	rangeInfo = getRangeInfoFromFile ( configFile, ver );
	std::vector<parameterType> fileParams;
	try
	{
		fileParams = getParametersFromFile( configFile, ver, rangeInfo );
	}
	catch ( Error& )
	{/*??? Shouldn't I handle something here?*/}
	currentParameters = fileParams;
	flattenScanDimensions ( );	
	redrawListview ( );
}


void ParameterSystem::redrawListview ( )
{
	if ( parametersListview.m_hWnd == NULL )
	{
		return;
	}
	parametersListview.DeleteAllItems ( );
	if ( paramSysType == ParameterSysType::config )
	{
		RECT r;
		parametersListview.GetClientRect (&r);
		setVariationRangeColumns (-1, 0.08*(r.right - r.left ));
		if ( currentParameters.size ( ) != 0 )
		{
			UINT varInc = 0;
			for ( auto param : currentParameters )
			{
				addParamToListview ( param, varInc++ );
			}
		}
	}
	else
	{
		UINT varInc = 0;
		for ( auto param : currentParameters )
		{
			addParamToListview ( param, varInc++ );
		}
	}
	parametersListview.insertBlankRow ( );
	updateVariationNumber ( );
}


ScanRangeInfo ParameterSystem::getRangeInfoFromFile ( std::ifstream& configFile, Version ver )
{
	ScanRangeInfo rInfo;
	UINT numRanges;
	if ( ver > Version ( "3.4" ) )
	{
		ProfileSystem::checkDelimiterLine ( configFile, "RANGE-INFO" );
		UINT numDimensions;
		if ( ver > Version ( "4.2" ) )
		{
			configFile >> numDimensions;
		}
		else
		{
			numDimensions = 1;
		}
		rInfo.setNumScanDimensions ( numDimensions );
		for ( auto dim : range( numDimensions ))
		{
			configFile >> numRanges;
			rInfo.setNumRanges ( dim, numRanges );
			for ( auto& range : rInfo.dimensionInfo(dim) )
			{	
				configFile >> range.leftInclusive >> range.rightInclusive >> range.variations;
			}
		}
	}
	else
	{
		rInfo.reset ( );
		rInfo.defaultInit ( );
	}
	return rInfo;
}


std::vector<parameterType> ParameterSystem::getParametersFromFile( std::ifstream& configFile, Version ver, 
																  ScanRangeInfo rangeInfo )
{
	UINT variableNumber;
	configFile >> variableNumber;
	if ( variableNumber > 100 )
	{
		int answer = promptBox( "variable number retrieved from file appears suspicious. The number is "
								+ str( variableNumber ) + ". Is this accurate?", MB_YESNO );
		if ( answer == IDNO )
		{
			// don't try to load anything.
			variableNumber = 0;
		}
	}
	std::vector<parameterType> tempVariables;
	for ( const UINT varInc : range( variableNumber ) )
	{
		tempVariables.push_back( loadParameterFromFile( configFile, ver, rangeInfo ) );
	}
	return tempVariables;
}


void ParameterSystem::updateVariationNumber( )
{
	// if no variables, or all are constants, it will stay at 1. else, it will get set to the # of variations
	// of the first variable that it finds.
	std::vector<ULONG> dimVariations;
	std::vector<bool> dimsSeen;
	for ( auto tempParam : currentParameters )
	{
		if ( !tempParam.constant )
		{
			if ( dimsSeen.size( ) <= tempParam.scanDimension )
			{
				dimVariations.resize( tempParam.scanDimension+1, 0 );
				dimsSeen.resize( tempParam.scanDimension+1, false );
			}
			if ( dimsSeen[tempParam.scanDimension] )
			{
				// already seen.
				continue;
			}
			else
			{
				// now it's been seen, don't add it again.
				dimsSeen[tempParam.scanDimension] = true;
			}
			for ( auto range : rangeInfo.dimensionInfo(tempParam.scanDimension) )
			{
				dimVariations[ tempParam.scanDimension ] += range.variations;
			}
		}
	}
	currentVariations = 1;
	for ( auto val : dimVariations )
	{
		currentVariations *= val;
	}
}

 
void ParameterSystem::handleNewConfig( std::ofstream& newFile )
{
	newFile << configDelim + "\n";
	// Number of functions with variables saved
	newFile << 0 << "\n";
	newFile << "END_" + configDelim + "\n";
}

double ParameterSystem::getVariableValue ( std::string paramName )
{
	if ( paramSysType != ParameterSysType::global )
	{
		thrower ( "adjusting variable values in the code like this is only meant to be used with global variables!" );
	}

	bool found = false;
	for ( auto& param : currentParameters )
	{
		if ( param.name == paramName )
		{
			return param.constantValue;
		}
	}
	if ( !found )
	{
		thrower ( "variable \"" + paramName + "\" not found in global varable control!" );
	}
}


void ParameterSystem::adjustVariableValue( std::string paramName, double value )
{
	if ( paramSysType != ParameterSysType::global )
	{
		thrower ( "adjusting variable values in the code like this is only meant to be used with global variables!" );
	}
	bool found = false;
	for ( auto& param : currentParameters )
	{
		if ( param.name == paramName )
		{
			param.constantValue = value;
			found = true;
			break;
		}
	}
	if ( !found )
	{
		thrower ( "variable \"" + paramName + "\" not found in global varable control!" );
	}
	redrawListview ( );
}


parameterType ParameterSystem::loadParameterFromFile( std::ifstream& openFile, Version ver, ScanRangeInfo rangeInfo )
{
	parameterType tempParam;
	std::string paramName, typeText, valueString;
	openFile >> paramName >> typeText;
	std::transform( paramName.begin( ), paramName.end( ), paramName.begin( ), ::tolower );
	tempParam.name = paramName;
	if ( typeText == "Constant" )
	{
		tempParam.constant = true;
	}
	else if ( typeText == "Variable" )
	{
		tempParam.constant = false;
	}
	else
	{
		thrower ( "unknown parameter type option: \"" + typeText + "\" for parameter \"" + paramName 
				 + "\". Check the formatting of the configuration file." );
	}
	if (ver > Version("2.7" ) )
	{
		openFile >> tempParam.scanDimension;
		if ( ver < Version ( "4.2" ) )
		{
			if ( tempParam.scanDimension > 0 )
			{
				tempParam.scanDimension--;
			}
		}
	}
	else
	{
		tempParam.scanDimension = 0;
	}
	
	if ( ver <= Version ( "3.4" ) )
	{
		UINT rangeNumber = 1;
		openFile >> rangeNumber;
		// I think it's unlikely to ever need more than 2 or 3 ranges.
		if ( rangeNumber < 1 || rangeNumber > 100 )
		{
			errBox ( "Bad range number! setting it to 1, but found " + str ( rangeNumber ) + " in the file." );
			rangeNumber = 1;
		}
		rangeInfo.setNumRanges ( tempParam.scanDimension, rangeNumber );
	}
	UINT totalVariations = 0;
	for ( auto rangeInc : range( rangeInfo.numRanges(tempParam.scanDimension ) ))
	{
		double initValue = 0, finValue = 0;
		unsigned int variations = 0;
		bool leftInclusive = 0, rightInclusive = 0;
		openFile >> initValue >> finValue;
		if ( ver <= Version ( "3.4" ) )
		{
			openFile >> variations >> leftInclusive >> rightInclusive;
		}		
		totalVariations += variations;
		tempParam.ranges.push_back ( { initValue, finValue } );
	}
	// shouldn't be because of 1 forcing earlier.
	if ( tempParam.ranges.size( ) == 0 )
	{
		// make sure it has at least one entry.
		tempParam.ranges.push_back ( { 0,0 } );
	}
	if (ver >= Version("2.14") )
	{
		openFile >> tempParam.constantValue;
	}
	else
	{
		tempParam.constantValue = tempParam.ranges[0].initialValue;
	}
	if (ver > Version("3.2"))
	{
		openFile >> tempParam.parameterScope;
	}
	else
	{
		tempParam.parameterScope = GLOBAL_PARAMETER_SCOPE;
	}
	return tempParam;
}


void ParameterSystem::saveParameter( std::ofstream& saveFile, parameterType parameter )
{
	saveFile << parameter.name << " " << ( parameter.constant ? "Constant " : "Variable " ) << parameter.scanDimension << "\n";
	for ( auto& range : parameter.ranges )
	{
		saveFile << range.initialValue << "\n" << range.finalValue << "\n";
	}
	saveFile << parameter.constantValue << "\n" << parameter.parameterScope << "\n";
}


void ParameterSystem::handleSaveConfig ( std::ofstream& saveFile )
{
	checkScanDimensionConsistency ( ); 
	checkVariationRangeConsistency ( );
	
	saveFile << configDelim + "\n";
	saveFile << "RANGE-INFO\n";
	saveFile << rangeInfo.numScanDimensions ( ) << "\n";
	for ( auto dimNum : range(rangeInfo.numScanDimensions ( )) )
	{
		saveFile << rangeInfo.numRanges ( dimNum ) << "\n";
		for ( auto range : rangeInfo.dimensionInfo (dimNum) )
		{
			saveFile << range.leftInclusive << "\n" << range.rightInclusive << "\n" << range.variations << "\n";
		}
	}
	saveFile << getCurrentNumberOfVariables ( ) << "\n";
	for ( UINT varInc = 0; varInc < getCurrentNumberOfVariables( ); varInc++ )
	{
		saveParameter(saveFile, getVariableInfo( varInc ));
	}
	saveFile << "END_" + configDelim + "\n";
}


void ParameterSystem::rearrange ( UINT width, UINT height, fontMap fonts )
{
	parametersHeader.rearrange ( width, height, fonts );
	parametersListview.rearrange ( width, height, fonts );
}


void ParameterSystem::removeVariableDimension ( )
{
	if ( scanDimensions == 1 )
	{
		thrower  ( "Can't delete last variable scan dimension." );
	}
	// change all variables in the last dimension to be in the second-to-last dimension.
	// TODO: I'm gonna have to check variation numbers here or change them to be compatible.
	for ( auto& variable : currentParameters )
	{
		if ( variable.scanDimension == scanDimensions-2 )
		{
			variable.scanDimension--;
		}
	}
	scanDimensions--;
	redrawListview ( );
}


void ParameterSystem::flattenScanDimensions ( )
{
	while ( true )
	{
		UINT maxDim = 0;
		for ( auto& var : currentParameters )
		{
			maxDim = var.scanDimension > maxDim ? var.scanDimension : maxDim;
		}
		for ( auto dim : range ( maxDim ) )
		{
			bool found = false;
			for ( auto param : currentParameters )
			{
				if ( param.scanDimension == dim )
				{
					found = true;
				}
			}
			if ( !found )
			{
				for ( auto& param : currentParameters )
				{
					if ( param.scanDimension > dim )
					{
						param.scanDimension--;
					}
				}
				// and do the same thing again.
				continue;
			}
		}
		break;
	}
}

void ParameterSystem::checkScanDimensionConsistency( )
{
	rangeInfo.numScanDimensions ( );
	for ( auto& param : currentParameters )
	{
		if ( param.scanDimension >= rangeInfo.numScanDimensions ( ) )
		{
			param.scanDimension = 0;
		}
	}
}


void ParameterSystem::checkVariationRangeConsistency ( )
{
	UINT dum = 0;
	for ( auto& var : currentParameters )
	{
		if ( var.ranges.size ( ) != rangeInfo.numRanges(var.scanDimension) )
		{
			if ( dum == 0 )
			{
				errBox ( "The number of variation ranges of a parameter, " + var.name +
						 ", (and perhaps others) did not match the official number. The code will force the parameter "
						 "to match the official number." );
				dum++; // only dislpay the error message once.
			}
			var.ranges.resize ( rangeInfo.numRanges ( var.scanDimension ) );
		}
	}
}


void ParameterSystem::setVariationRangeNumber ( int num, USHORT dimNumber )
{
	auto columnCount = parametersListview.GetHeaderCtrl ( )->GetItemCount ( );
	// -2 for the two +- columns
	int currentVariableRangeNumber = ( columnCount - preRangeColumns - 2 ) / 3;
	checkScanDimensionConsistency ( );
	checkVariationRangeConsistency ( );
	if ( rangeInfo.numRanges(dimNumber) != currentVariableRangeNumber )
	{
		errBox ( "somehow, the number of ranges the ParameterSystem object thinks there are and the actual number "
				 "displayed are off! The numbers are " + str ( rangeInfo.numRanges ( dimNumber ) ) + " and "
				 + str ( currentVariableRangeNumber ) + " respectively. The program will attempt to fix this, but "
				 "data may be lost." );
		while ( rangeInfo.numRanges ( dimNumber ) != currentVariableRangeNumber )
		{
			if ( rangeInfo.numRanges ( dimNumber ) > currentVariableRangeNumber )
			{
				rangeInfo.dimensionInfo ( dimNumber ).pop_back ( );
				for ( auto& param : currentParameters )
				{
					param.ranges.pop_back( );
				}
			}
			else
			{
				rangeInfo.dimensionInfo ( dimNumber ).push_back ( defaultRangeInfo );
				for ( auto& param : currentParameters )
				{
					param.ranges.pop_back ( );
				}
			}
		}
	}
	if (currentVariableRangeNumber < num)
	{
		while (currentVariableRangeNumber < num)
		{
			/// add a range.
			// edit all variables
			rangeInfo.dimensionInfo ( dimNumber ).push_back ( defaultRangeInfo );
			for (UINT varInc = 0; varInc < currentParameters.size(); varInc++)
			{
				indvParamRangeInfo tempInfo{ 0,0 };
				currentParameters[varInc].ranges.push_back( tempInfo );
			}
			currentVariableRangeNumber++;
		}
	}
	else if (currentVariableRangeNumber > num)
	{
		while (currentVariableRangeNumber > num)
		{
			// delete a range.
			if ( rangeInfo.dimensionInfo ( dimNumber ).size() == 1)
			{
				// can't delete last range
				return;
			}
			// edit all variables
			for (auto& param : currentParameters )
			{
				param.ranges.pop_back();
			}
			rangeInfo.dimensionInfo ( dimNumber ).pop_back ( );
			currentVariableRangeNumber--;
		}
	}
	redrawListview ( );
}


void ParameterSystem::handleColumnClick(NMHDR * pNotifyStruct, LRESULT * result)
{
	if ( mostRecentlySelectedParam == -1 || currentParameters.size() <= mostRecentlySelectedParam )
	{
		// must have selected an item in order to determine which scan dimension to change.
		return;
	}
	auto dimNum = currentParameters[ mostRecentlySelectedParam ].scanDimension;
	POINT cursorPos;
	GetCursorPos(&cursorPos);
	parametersListview.ScreenToClient(&cursorPos);
	LVHITTESTINFO myItemInfo = { 0 };
	myItemInfo.pt = cursorPos;
	parametersListview.SubItemHitTest(&myItemInfo);
	if (myItemInfo.iSubItem == preRangeColumns + 3 * rangeInfo.numRanges(0))
	{
		// add a range.
		setVariationRangeNumber( rangeInfo.numRanges ( 0 ) + 1, 0);
	}
	else if (myItemInfo.iSubItem == preRangeColumns + 1 + 3 * rangeInfo.numRanges ( 0 ) )
	{
		// delete a range.
		setVariationRangeNumber( rangeInfo.numRanges ( 0 ) - 1, 0);
	}
	else if (myItemInfo.iSubItem >= preRangeColumns && (myItemInfo.iSubItem - preRangeColumns) % 3 == 0)
	{
		// switch between [ and (
		if ( currentParameters.size( ) == 0 )
		{
			return;
		}
		UINT rangeNum = (myItemInfo.iSubItem - preRangeColumns) / 3;
		setRangeInclusivity( rangeNum, dimNum, true, !rangeInfo(0,rangeNum).leftInclusive);
	}
	else if (myItemInfo.iSubItem >= preRangeColumns && (myItemInfo.iSubItem- preRangeColumns) % 3 == 1)
	{
		// switch between ] and )
		if ( currentParameters.size( ) == 0 )
		{
			return;
		}
		UINT rangeNum = (myItemInfo.iSubItem - preRangeColumns) / 3;
		setRangeInclusivity( rangeNum, dimNum,  false, !rangeInfo ( 0, rangeNum ).rightInclusive);
	}
	redrawListview ( );
}


void ParameterSystem::setRangeInclusivity( UINT rangeNum, UINT dimNum, bool isLeft, bool inclusive )
{
	if ( rangeNum >= rangeInfo.numRanges(0) )
	{
		thrower  ( "tried to set the border inclusivity of a range that does not exist!" );
	}
	( isLeft ? rangeInfo ( dimNum, rangeNum ).leftInclusive : rangeInfo ( dimNum, rangeNum ).rightInclusive ) = inclusive;
}


void ParameterSystem::handleDraw(NMHDR* pNMHDR, LRESULT* pResult )
{
	NMLVCUSTOMDRAW* pLVCD = reinterpret_cast<NMLVCUSTOMDRAW*>(pNMHDR);
	*pResult = CDRF_DODEFAULT;
	// First thing - check the draw stage. If it's the control's prepaint
	// stage, then tell Windows we want messages for every item.
	if (CDDS_PREPAINT == pLVCD->nmcd.dwDrawStage)
	{
		*pResult = CDRF_NOTIFYITEMDRAW;
	}
	else if (CDDS_ITEMPREPAINT == pLVCD->nmcd.dwDrawStage)
	{
		int item = pLVCD->nmcd.dwItemSpec;
		if (item < 0)
		{
			return;
		}
		if (item >= currentParameters.size())
		{
			pLVCD->clrText = _myRGBs["AuxWin-Text"];
			pLVCD->clrTextBk = _myRGBs["Interactable-Bkgd"];
		}
		else
		{
			if (currentParameters[item].active)
			{
				pLVCD->clrTextBk = _myRGBs["Solarized Orange"];
			}
			else
			{
				pLVCD->clrTextBk = _myRGBs["Interactable-Bkgd"];
			}
			if (currentParameters[item].overwritten)
			{
				pLVCD->clrText = _myRGBs["Solarized Red"];
			}
			else
			{
				pLVCD->clrText = _myRGBs["Text"];
			}
		}
		// Tell Windows to paint the control itself.
		*pResult = CDRF_DODEFAULT;
	}
}

BOOL ParameterSystem::handleAccelerators( HACCEL m_haccel, LPMSG lpMsg )
{
	for ( auto& dlg : childDlgs )
	{
		if ( ::TranslateAccelerator( dlg->m_hWnd, m_haccel, lpMsg ) )
		{
			return TRUE;
		}
	}
	return FALSE;
}


void ParameterSystem::handleSingleClick ()
{
	if ( !controlActive )
	{
		return;
	}
	/// get the item and subitem
	LVHITTESTINFO myItemInfo = { 0 };
	GetCursorPos ( &myItemInfo.pt );
	parametersListview.ScreenToClient ( &myItemInfo.pt );
	parametersListview.SubItemHitTest ( &myItemInfo );
	mostRecentlySelectedParam = myItemInfo.iItem;
}

void ParameterSystem::handleDblClick( std::vector<Script*> scripts, MainWindow* mainWin, AuxiliaryWindow* auxWin,
										   DioSystem* ttls, AoSystem* aoSys )
{
	if ( !controlActive )
	{
		return;
	}
	/// get the item and subitem
	LVHITTESTINFO myItemInfo = { 0 };
	GetCursorPos(&myItemInfo.pt);
	parametersListview.ScreenToClient(&myItemInfo.pt);
	parametersListview.SubItemHitTest(&myItemInfo);
	int subitem = myItemInfo.iSubItem, itemIndicator = mostRecentlySelectedParam = myItemInfo.iItem;
	if (itemIndicator == -1)
	{
		return;
	}
	/// check if adding new parameter
	CString text = parametersListview.GetItemText(itemIndicator, 0);
	if (text == "___")
	{
		// add a parameter
		currentParameters.resize(currentParameters.size() + 1);
		currentParameters.back().name = "";
		currentParameters.back().constant = true;
		currentParameters.back().active = false;
		currentParameters.back().overwritten = false;
		currentParameters.back().scanDimension = 0;
		for ( auto rangeVariations : rangeInfo.dimensionInfo( 0 ))
		{
			currentParameters.back ( ).ranges.push_back ( { 0, 0 } );
		}
		redrawListview ( );
	}
	auto& param = currentParameters[ itemIndicator ];
	/// Handle different subitem clicks
	switch (subitem)
	{
		case 0:
		{
			/// person name
			std::string newName;
			TextPromptDialog dialog(&newName, "Please enter a name for the Parameter:", param.name);
			dialog.DoModal();
			newName = str ( newName, 13, false, true );
			if (newName == "")
			{
				break; // probably canceled.
			}
			for (auto param : currentParameters)
			{
				if (param.name == newName)
				{
					thrower ( "A parameter with name " + newName + " already exists!" );
				}
			}
			if ( ttls->isValidTTLName( newName ) )
			{
				thrower ( "the name " + newName + " is already a ttl Name!" );
			}
			if ( aoSys->isValidDACName( newName ) )
			{
				thrower ( "the name " + newName + " is already a dac name!" );
			}
			param.name = newName;
			redrawListview ( );
			break;
		}
		case 1:
		{
			if ( paramSysType == ParameterSysType::global )
			{
				/// global value
				std::string newValue;
				TextPromptDialog dialog(&newValue, "Please enter a value for the global parameter "
										+ param.name + ". Value will be formatted as a double.", 
					str(param.constantValue));
				childDlgs.push_back( &dialog );
				dialog.DoModal();
				childDlgs.pop_back();
				if (newValue == "")
				{
					// probably canceled.
					break;
				}
				try
				{
					param.constantValue = boost::lexical_cast<double>( newValue );
				}
				catch ( boost::bad_lexical_cast&)
				{
					throwNested ( "the value entered, " + newValue + ", failed to convert to a double! "
							      "Check for invalid characters." );
				}
				redrawListview ( );
				break;
			}
			else
			{
				/// constant or variable?
				param.constant = !param.constant;
				redrawListview ( );
			}
			break;
		}
		case 2:
		{
			/// variable scan dimension
			if ( param.constant )
			{
				break;
			}
			UINT maxDim = 0;
			for ( auto& variable : currentParameters )
			{
				if ( variable.name == param.name || variable.constant )
				{
					// don't count the one being changed.
					continue;
				}
				if ( variable.scanDimension > maxDim )
				{
					maxDim = variable.scanDimension;
				}
			}
			param.scanDimension++;
			// handle "wrapping" of the dimension.
			if ( param.scanDimension > maxDim+1 )
			{
				param.scanDimension = 0;
			}
			reorderVariableDimensions( );
			redrawListview ( );
			break;
		}
		case 3:
		{
			/// constant value
			if ( !param.constant )
			{
				// In this case the extra boxes are unresponsive.
				break;
			}
			std::string newValue;
			TextPromptDialog dialog( &newValue, "Please enter an initial value for the variable "
									 + param.name + ". Value will be formatted as a double.", str(param.constantValue));
			dialog.DoModal( );
			if ( newValue == "" )
			{
				// probably canceled.
				break;
			}
			try
			{
				param.constantValue = boost::lexical_cast<double>( newValue );
			}
			catch ( boost::bad_lexical_cast& )
			{
				throwNested ( "the value entered, " + newValue + ", failed to convert to a double! "
						 "Check for invalid characters." );
			}
			redrawListview ( );
			break;
		}
		case 4:
		{
			/// scope
			std::string newScope;
			TextPromptDialog dialog( &newScope, "Please enter a the scope for the variable: \""
									 + param.name + "\". You may enter a function name, "
									 "\"parent\", or \"global\".", param.parameterScope );
			dialog.DoModal( );
			if ( newScope == "" )
			{
				// probably canceled.
				break;
			}
			newScope = str ( newScope, 13, false, true );
			param.parameterScope = newScope;
			redrawListview ( );
			break;
		}
		default:
		{
			// if it's a constant, you can only set the first range initial value.
			if ( param.constant )
			{
				// then no final value to be set. In this case the extra boxes are unresponsive.
				break;
			}
			UINT rangeNum = (subitem - preRangeColumns) / 3;
			std::string newValue;
			auto& whichVal = ((subitem - preRangeColumns) % 3 == 0) ? param.ranges[rangeNum].initialValue : param.ranges[rangeNum].finalValue;
			TextPromptDialog dialog( &newValue, "Please enter a value for the variable " + param.name + ".", str(whichVal) );
			dialog.DoModal( );
			if ( newValue == "" )
			{
				// probably canceled.
				break;
			}
			if ((subitem - preRangeColumns) % 3 == 0 || (subitem - preRangeColumns) % 3 == 1)
			{
				try 
				{
					whichVal = boost::lexical_cast<double>(newValue);
				}
				catch ( boost::bad_lexical_cast&)
				{
					throwNested ("the value entered, " + newValue + ", failed to convert to a double! "
							 "Check for invalid characters.");
				}
				redrawListview ( );
				break;
			}
			else if((subitem - preRangeColumns) % 3 == 2)
			{
				try
				{
					for (auto& variable : currentParameters)
					{
						if (!variable.constant)
						{
							// make sure all variables have the same number of variations.
							if ( variable.scanDimension != param.scanDimension )
							{
								continue;
							}
							rangeInfo(variable.scanDimension,rangeNum).variations = boost::lexical_cast<int> ( newValue );
						}
					}
				}
				catch ( boost::bad_lexical_cast&)
				{
					throwNested ("the value entered, " + newValue + ", failed to convert to a double! Check "
									"for invalid characters.");
				}
				redrawListview ( );
				break;
			}
		}
	}
	updateVariationNumber( );
}


void ParameterSystem::setParameterControlActive(bool active)
{
	controlActive = active;
}


void ParameterSystem::deleteVariable()
{
	if ( !controlActive )
	{
		return;
	}
	/// get the item and subitem
	POINT cursorPos;
	GetCursorPos(&cursorPos);
	parametersListview.ScreenToClient(&cursorPos);
	int subitemIndicator = parametersListview.HitTest(cursorPos);
	LVHITTESTINFO myItemInfo = { 0 };
	myItemInfo.pt = cursorPos;
	int itemIndicator = parametersListview.SubItemHitTest(&myItemInfo);
	if (itemIndicator == -1 || itemIndicator == currentParameters.size())
	{
		// user didn't click in a deletable item.
		return;
	}
	int answer;
	if (UINT(itemIndicator) < currentParameters.size())
	{
		answer = promptBox("Delete variable " + currentParameters[itemIndicator].name + "?", MB_YESNO);
		if (answer == IDYES)
		{
			currentParameters.erase(currentParameters.begin() + itemIndicator);
			redrawListview ( );
		}
	}
	else if (UINT(itemIndicator) > currentParameters.size())
	{
		answer = promptBox("You appear to have found a bug with the listview control... there are too many lines "
							 "in this control. Clear this line?", MB_YESNO);
		if (answer == IDYES)
		{
			redrawListview ( );
		}
	}
	updateVariationNumber( );
}


parameterType ParameterSystem::getVariableInfo(int varNumber)
{
	return currentParameters[varNumber];
}


UINT ParameterSystem::getCurrentNumberOfVariables()
{
	return currentParameters.size();
}


// takes as input variables, but just looks at the name and usage stats. When it finds matches between the variables,
// it takes the usage of the input and saves it as the usage of the real inputVar. 
void ParameterSystem::setUsages(std::vector<std::vector<parameterType>> vars)
{
	for ( auto& seqVars : vars )
	{
		for ( auto inputVar : seqVars )
		{
			for ( auto& realVar : currentParameters )
			{
				if ( inputVar.name == realVar.name )
				{
					realVar.overwritten = inputVar.overwritten;
					realVar.active = inputVar.active;
					break;
				}
			}
		}
	}
	parametersListview.RedrawWindow();
}


void ParameterSystem::clearParameters()
{
	currentParameters.clear();
	redrawListview ( );
}


std::vector<parameterType> ParameterSystem::getAllParams()
{
	return currentParameters;
}


std::vector<parameterType> ParameterSystem::getAllConstants()
{
	std::vector<parameterType> constants;
	for (UINT varInc = 0; varInc < currentParameters.size(); varInc++)
	{
		if (currentParameters[varInc].constant)
		{
			constants.push_back(currentParameters[varInc]);
		}
	}
	return constants;
}

// this function returns the compliment of the variables that "getAllConstants" returns.
std::vector<parameterType> ParameterSystem::getAllVariables()
{
	// opposite of get constants.
	std::vector<parameterType> varyingParameters;
	for (UINT varInc = 0; varInc < currentParameters.size(); varInc++)
	{
		if (!currentParameters[varInc].constant)
		{
			varyingParameters.push_back(currentParameters[varInc]);
		}
	}
	return varyingParameters;
}


void ParameterSystem::addParamToListview ( parameterType param, UINT item )
{
	// only to be called by redrawListview
	if ( param.name == "" )
	{
		parametersListview.insertBlankRow ( );
		parametersListview.RedrawWindow ( );
		return;
	}
	/// else...
	if ( parametersListview.m_hWnd == NULL )
	{
		return;
	}

	parametersListview.InsertItem ( param.name, item, 0 );
	if ( paramSysType == ParameterSysType::global )
	{
		parametersListview.SetItem ( str ( param.constantValue ), item, 1 );
		parametersListview.RedrawWindow ( );
		return;
	}
	// else config system...
	if ( param.constant )
	{
		parametersListview.SetItem ( "Constant", item, 1 );
		parametersListview.SetItem ( str ( param.constantValue ), item, 3 );
	}
	else
	{
		parametersListview.SetItem ( "Variable", item, 1 );
		parametersListview.SetItem ( "---", item, 3 );
	}
	parametersListview.SetItem ( str ( char ( 'A' + param.scanDimension ) ), item, 2 );
	parametersListview.SetItem ( param.parameterScope, item, 4 );
	// make sure there are enough ranges.
	UINT currentRanges = currentParameters.front ( ).ranges.size ( );
	if ( param.ranges.size ( ) < currentRanges )
	{
		param.ranges.resize ( currentRanges );
	}
	// update ranges
	auto info = rangeInfo.dimensionInfo ( param.scanDimension );
	for ( auto rangeInc : range ( param.ranges.size ( ) ) )
	{
		auto col = preRangeColumns + rangeInc * 3;
		auto& range = currentParameters[ item ].ranges[ rangeInc ];
		parametersListview.SetItem ( param.constant ? "---" : ( info[ rangeInc ].leftInclusive ? "[ " : "( " )
									 + str ( range.initialValue, 13, true ), item, col++ );
		parametersListview.SetItem ( param.constant ? "---" : str ( range.finalValue, 13, true ) 
									 + ( info[ rangeInc ].rightInclusive ? " ]" : " )" ), item, col++ );
		parametersListview.SetItem ( param.constant ? "---" : str ( info[rangeInc].variations, 13, true ), item, col );
	}
	parametersListview.RedrawWindow ( );
	updateVariationNumber ( );
}


void ParameterSystem::addParameter(parameterType variableToAdd)
{
	// make name lower case.
	std::transform(variableToAdd.name.begin(), variableToAdd.name.end(), variableToAdd.name.begin(), ::tolower);
	if (isdigit(variableToAdd.name[0]))
	{
		thrower ("" + variableToAdd.name + " is an invalid name; names cannot start with numbers.");
	}
	// check for forbidden (math) characters
	if (variableToAdd.name.find_first_of(" \t\r\n()*+/-%") != std::string::npos)
	{
		thrower ("Forbidden character in variable name! you cannot use spaces, tabs, newlines, or any of "
				"\"()*+/-%\" in a variable name.");
	}
	if (variableToAdd.name == "" )
	{
		return;
	}
	/// else...
	for (auto currentVar : currentParameters)
	{
		if (currentVar.name == variableToAdd.name)
		{
			thrower ("A variable with the name " + variableToAdd.name + " already exists!");
		}
	}
	currentParameters.push_back(variableToAdd);
	redrawListview ( );
}


ScanRangeInfo ParameterSystem::getRangeInfo ( )
{
	return rangeInfo;
}


void ParameterSystem::reorderVariableDimensions( )
{
	/// find the maximum dimension
	UINT maxDim = 0;
	for ( auto& variable : currentParameters )
	{
		if ( variable.scanDimension > maxDim )
		{
			maxDim = variable.scanDimension;
		}
	}
	if ( rangeInfo.numScanDimensions ( ) != maxDim+1 )
	{
		rangeInfo.setNumScanDimensions ( maxDim+1 );
	}
	/// flatten the dimension numbers.
	UINT flattenNumber = 0;
	for ( auto dimInc : range( maxDim+1 ) )
	{
		bool found = false;
		for ( auto& variable : currentParameters )
		{
			if ( variable.scanDimension == dimInc )
			{
				variable.scanDimension = dimInc - flattenNumber;
				found = true;
			}
		}
		if ( !found )
		{
			rangeInfo.removeDim ( dimInc );
			flattenNumber++;
		}
	}
	/// reset variables ??? why?
	std::vector<parameterType> varCopy = currentParameters; 
	clearParameters( );
	for ( auto& variable : varCopy )
	{
		addParameter( variable);
	}
	addParameter( {} );
}


INT_PTR ParameterSystem::handleColorMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, brushMap brushes)
{
	HDC hdcStatic = (HDC)wParam;
	if ( GetDlgCtrlID( (HWND)lParam ) == parametersHeader.GetDlgCtrlID())
	{
		SetTextColor(hdcStatic, _myRGBs["AuxWin-Text"]);
		SetBkColor(hdcStatic, _myRGBs[ "Interactable-Text" ] );
		return (LRESULT)_myBrushes["Interactable-Text"];
	}
	else
	{
		return NULL;
	}
}


std::vector<double> ParameterSystem::getKeyValues( std::vector<parameterType> variables )
{
	for ( auto variable : variables )
	{
		if ( variable.valuesVary )
		{
			return variable.keyValues;
		}
	}
	// no varying variables found.
	return std::vector<double>( 1, 0 );
}


std::vector<parameterType> ParameterSystem::getConfigParamsFromFile( std::string configFileName )
{
	std::ifstream file(configFileName);
	Version ver;
	std::vector<parameterType> configParams;
	try
	{
		ProfileSystem::initializeAtDelim (file, "CONFIG_PARAMETERS", ver, Version ( "4.0" ) );
		auto rInfo = getRangeInfoFromFile (file, ver );
		configParams = getParametersFromFile (file, ver, rInfo );
		ProfileSystem::checkDelimiterLine (file, "END_CONFIG_PARAMETERS" );
	}
	catch ( Error& )
	{
		throwNested ( "Failed to get configuration parameters from the configuration file!" );
	}
	return configParams;
}


ScanRangeInfo ParameterSystem::getRangeInfoFromFile ( std::string configFileName )
{
	std::ifstream file ( configFileName );
	Version ver;
	ScanRangeInfo rInfo;
	try
	{
		ProfileSystem::initializeAtDelim (file, "CONFIG_PARAMETERS", ver, Version ( "4.0" ) );
		rInfo = getRangeInfoFromFile (file, ver );
		auto configVariables = getParametersFromFile (file, ver, rInfo );
		ProfileSystem::checkDelimiterLine (file, "END_CONFIG_PARAMETERS" );
	}
	catch ( Error& )
	{
		throwNested ( "Failed to get configuration parameter range info from file!" );
	}
	return rInfo;
}


void ParameterSystem::generateKey( std::vector<std::vector<parameterType>>& variables, bool randomizeVariationsOption,
								   ScanRangeInfo inputRangeInfo )
{
	for ( auto& seqVariables : variables )
	{
		for ( auto& variable : seqVariables )
		{
			variable.keyValues.clear( );
		}
	}
	// find the maximum scan dimension.
	UINT maxDim = 0;
	for ( auto seqVariables : variables )
	{
		for ( auto variable : seqVariables )
		{
			maxDim = ( variable.scanDimension > maxDim ? variable.scanDimension : maxDim );
		}
	}
	// each element of the vector refers to the number of variations of a given range of a given scan dimension of a 
	// given sequence element. i.e.: variations[seqNumber][dimNumber][rangeNumber]
	std::vector<std::vector<std::vector<int>>> variationNums( variables.size( ), std::vector<std::vector<int>>(maxDim+1));
	std::vector<std::vector<UINT>> totalSeqDimVariationsList ( variationNums.size ( ), std::vector<UINT> ( maxDim+1 ) );
	// for randomizing...
	std::vector<std::vector<int>> variableIndexes(variables.size());
	for (auto seqInc : range(variables.size()) )
	{
		for ( auto dimInc : range( maxDim+1 ) )
		{
			variationNums[seqInc][dimInc].resize( variables[seqInc].front( ).ranges.size( ) );
			for ( auto paramInc : range(variables[seqInc].size() ) )
			{
				auto& parameter = variables[seqInc][paramInc];
				// find a varying parameter in this scan dimension
				if ( parameter.scanDimension != dimInc || parameter.constant )
				{
					continue; 
				}
				variableIndexes[seqInc].push_back( paramInc );
				if ( variationNums[seqInc][dimInc].size( ) != parameter.ranges.size( ) )
				{
					// if its zero its just the initial size on the initial variable. Else something has gone wrong.
					if ( variationNums[seqInc].size( ) != 0 )
					{
						thrower ( "Not all variables seem to have the same number of ranges for their parameters!" );
					}
					variationNums[seqInc][dimInc].resize( parameter.ranges.size( ) );
				}
				totalSeqDimVariationsList[ seqInc ][ dimInc ] = 0;
				for ( auto rangeInc : range( variationNums[seqInc][dimInc].size( ) ) )
				{
					variationNums[ seqInc ][ dimInc ][ rangeInc ] = inputRangeInfo(dimInc, rangeInc).variations;
					if ( variationNums[ seqInc ][ dimInc ][ rangeInc ] == 1 )
					{
						thrower ( "You need more than one variation in every range." );
					}
					totalSeqDimVariationsList[ seqInc ][ dimInc ] += inputRangeInfo ( dimInc, rangeInc ).variations;
				}
			}
		}
	}
	// create a key which will be randomized and then used to randomize other things the same way.
	multiDimensionalKey<int> randomizerMultiKey( maxDim+1 );
	randomizerMultiKey.resize( totalSeqDimVariationsList );
	UINT count = 0;
	for ( auto& keyElem : randomizerMultiKey.values[0] )
	{
		keyElem = count++;
	}
	if ( randomizeVariationsOption )
	{
		std::random_device rng;
		std::mt19937 twister( rng( ) );
		std::shuffle( randomizerMultiKey.values[0].begin( ), randomizerMultiKey.values[0].end( ), twister );
	}
	// initialize this to one so that constants always get at least one value.
	int totalSize = 1;
	for ( auto seqInc : range( variableIndexes.size( ) ) )
	{
		for ( auto variableInc : range( variableIndexes[seqInc].size( ) ) )
		{
			auto& variable = variables[seqInc][ variableIndexes[ seqInc ][ variableInc ] ];
			// calculate all values for a given variable
			multiDimensionalKey<double> tempKey( maxDim+1 ), tempKeyRandomized( maxDim+1 );
			tempKey.resize( totalSeqDimVariationsList );
			tempKeyRandomized.resize( totalSeqDimVariationsList );
			/* Suppose you have a three dimensional scan with variation numbers 3, 2, and 3 in each dimension. Then, 
			keyValueIndexes will take on the following sequence of values:
			{0,0,0} -> {1,0,0} -> {2,0,0} ->
			{0,1,0} -> {1,1,0} -> {2,1,0} ->
			{0,0,1} -> {1,0,1} -> {2,0,1} ->
			{0,1,1} -> {1,1,1} -> {2,1,1} ->
			{0,0,2} -> {1,0,2} -> {2,0,2} ->
			{0,1,2} -> {1,1,2} -> {2,1,2}.
			at which point the while loop will notice that all values turn over at the same time and leave the loop.
			*/
			std::vector<UINT> keyValueIndexes( maxDim+1 );
			while ( true )
			{
				UINT rangeIndex = 0, varDim = variable.scanDimension, tempShrinkingIndex = keyValueIndexes[ varDim ],
					 rangeCount = 0, rangeOffset = 0;
				// calculate which range it is and how many values have already been calculated for the variable 
				// (i.e. the rangeOffset).
				for ( auto range : inputRangeInfo.dimensionInfo( varDim ) )
				{
					if ( tempShrinkingIndex >= range.variations )
					{
						// then should have already gone through all that range's variations
						tempShrinkingIndex -= range.variations;
						rangeOffset += range.variations;
						rangeCount++;
					}
					else
					{
						rangeIndex = rangeCount;
						break;
					}
				}
				auto& currRange = variable.ranges[rangeIndex];
				// calculate the parameters for the variation range
				bool lIncl = inputRangeInfo(varDim, rangeIndex).leftInclusive, 
					rIncl = inputRangeInfo(varDim, rangeIndex).rightInclusive;
				int spacings = variationNums[ seqInc ][ varDim ][ rangeIndex ] + ( !lIncl && !rIncl ) - ( lIncl && rIncl );
				double valueRange = ( currRange.finalValue - currRange.initialValue );
				double initVal = ( lIncl ? currRange.initialValue : currRange.initialValue + valueRange / spacings);
				double value = valueRange * ( keyValueIndexes[ varDim ] - rangeOffset) / spacings + initVal;
				tempKey.setValue( keyValueIndexes, seqInc, value );
				bool isAtEnd = true;
				for ( auto indexInc : range( keyValueIndexes.size( ) ) )
				{
					// if at end of cycle for this index
					if ( keyValueIndexes[indexInc] == totalSeqDimVariationsList[seqInc][indexInc] - 1 )
 					{
						keyValueIndexes[indexInc] = 0;
 					}
					else
					{
						keyValueIndexes[indexInc]++;
						isAtEnd = false;
						break;
					}
				}
				if ( isAtEnd )
				{
					break;
				}
			}

			for ( auto keyInc : range( randomizerMultiKey.values[seqInc].size( ) ) )
			{
				tempKeyRandomized.values[seqInc][keyInc] = tempKey.values[seqInc][randomizerMultiKey.values[seqInc][keyInc]];
			}
			variable.keyValues = tempKeyRandomized.values[seqInc];
			variable.valuesVary = true;
			totalSize = tempKeyRandomized.values[seqInc].size( );
		}
	}
	// now add all constant objects.
	for ( auto& seqParams: variables )
	{
		for ( parameterType& param : seqParams)
		{
			if ( param.constant )
			{
				param.keyValues.clear ( );
				param.keyValues.resize( totalSize, param.constantValue );
				param.valuesVary = false;
			}
		}
	}
}


/*
 * takes global params, config params, and function params, and reorganizes them to form a "parameters" object and a 
 * "constants" objects. The "parameters" object includes everything, variables and otherwise. the "constants" object 
 * includes only parameters that don't vary. 
 */
std::vector<parameterType> ParameterSystem::combineParamsForExpThread( std::vector<parameterType>& configParams, 
																	   std::vector<parameterType>& globalParams )
{
	std::vector<parameterType> combinedParams;
	combinedParams = configParams;
	for ( auto& sub : globalParams )
	{
		sub.overwritten = false;
		bool nameExists = false;
		for ( auto& var : combinedParams )
		{
			if ( var.name == sub.name )
			{
				sub.overwritten = true;
				var.overwritten = true;
			}
		}
		if ( !sub.overwritten )
		{
			combinedParams.push_back( sub );
		}
	}
	for ( auto& var : combinedParams )
	{
		// set the default scope for the variables set in the normal parameter listviews. There might be a better place
		// to set this.
		if ( var.parameterScope == "" )
		{
			var.parameterScope = GLOBAL_PARAMETER_SCOPE;
		}
	}
	return combinedParams;
}


UINT ParameterSystem::getTotalVariationNumber( )
{
	return currentVariations;
}

