// created by Mark O. Brown
#include "stdafx.h"
#include "ParameterSystem.h"
#include "Script.h"
#include "DioSystem.h"
#include "AuxiliaryWindow.h"
#include "TextPromptDialog.h"
#include "multiDimensionalKey.h"
#include "cleanString.h"
#include "Thrower.h"

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
	rangeInfo.resize ( 1, defaultRangeInfo );

	parametersHeader.sPos = { pos.x, pos.y, pos.x + 480, pos.y += 25 };
	parametersHeader.Create( cstr( title ), NORM_HEADER_OPTIONS, parametersHeader.sPos, parent, id++ );
	parametersHeader.fontType = fontTypes::HeadingFont;
	
	parametersListview.sPos = { pos.x, pos.y, pos.x + 480, pos.y + 300 };
	parametersListview.Create( NORM_LISTVIEW_OPTIONS, parametersListview.sPos,
							  parent, listviewId );

	RECT r;
	parent->GetClientRect( &r );
	if ( paramSysType == ParameterSysType::global )
	{
		parametersListview.InsertColumn( 0, "Symbol", 3 * r.right / 5 );
		parametersListview.InsertColumn( 1, "Value", r.right / 6 );
	}
	else 
	{
		parametersListview.InsertColumn( 0, "Symbol", r.right/4 );
		parametersListview.InsertColumn( 1,"Type", r.right/10);
		parametersListview.InsertColumn( 2,"Dim" );
		parametersListview.InsertColumn( 3, "Value" );
		parametersListview.InsertColumn ( 4, "Scope" );
		parametersListview.InsertColumn ( 5, "1:(", r.right / 15 );
		parametersListview.InsertColumn( 6, "]" );
		parametersListview.InsertColumn ( 7, "#" );
		parametersListview.InsertColumn ( 8, "+()" );
		parametersListview.InsertColumn( 9, "-()" );
	}
	parametersListview.insertBlankRow ( );
	parametersListview.fontType = fontTypes::SmallFont;
	parametersListview.SetTextBkColor ( _myRGBs["Interactable-Bkgd"] );
	parametersListview.SetTextColor ( _myRGBs["AuxWin-Text"] );
	parametersListview.SetBkColor( _myRGBs["Interactable-Bkgd"] );
	pos.y += 300;
}


/*
 * The "normal" function, used for config and global variable systems.
 */
void ParameterSystem::handleOpenConfig( std::ifstream& configFile, Version ver )
{
	clearVariables( );
	/// 
	rangeInfo = getRangeInfoFromFile ( configFile, ver );
	std::vector<parameterType> variables;
	try
	{
		variables = getVariablesFromFile( configFile, ver, rangeInfo.size() );
	}
	catch ( Error& )
	{}
	if ( variables.size( ) == 0 )
	{
		setVariationRangeNumber( 1, 1 );
	}
	else
	{
		setVariationRangeNumber( variables.front( ).ranges.size(), 1 );
	}
	UINT varInc = 0;
	for ( auto var : variables )
	{
		addConfigParameter( var, varInc++ );
	}
	if ( currentParameters.size( ) != 0 )
	{
		for ( auto rangeInc : range( rangeInfo.size() ) )
		{
			//bool leftInclusivity = currentParameters.front( ).ranges[rangeInc].leftInclusive;
			//bool rightInclusivity = currentParameters.front( ).ranges[rangeInc].rightInclusive;
			setRangeInclusivity( rangeInc, true, rangeInfo[rangeInc].leftInclusive, preRangeColumns + rangeInc * 3 );
			setRangeInclusivity( rangeInc, false, rangeInfo[ rangeInc ].rightInclusive, preRangeColumns + 1 + rangeInc * 3 );
		}
	}
	else
	{
		setRangeInclusivity( 0, true, false, preRangeColumns );
		setRangeInclusivity( 0, false, true, preRangeColumns + 1 );
	}
	// add a blank line
	parameterType var;
	var.name = "";
	var.constant = false;
	var.ranges.push_back ( { 0,0 } );
	addConfigParameter( var, currentParameters.size( ) );
	updateVariationNumber( );
}


std::vector<variationRangeInfo> ParameterSystem::getRangeInfoFromFile ( std::ifstream& configFile, Version ver )
{
	std::vector<variationRangeInfo> rInfo;
	UINT numRanges;
	if ( ver > Version ( "3.4" ) )
	{
		ProfileSystem::checkDelimiterLine ( configFile, "RANGE-INFO" );
		configFile >> numRanges;
		rInfo.resize ( numRanges );
		for ( auto& range : rInfo )
		{
			configFile >> range.leftInclusive >> range.rightInclusive >> range.variations;
		}
	}
	else
	{
		rInfo.clear ( );
		rInfo.push_back ( { 2, false, true } );
	}
	return rInfo;
}


std::vector<parameterType> ParameterSystem::getVariablesFromFile( std::ifstream& configFile, Version ver, UINT rangeNum )
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
		tempVariables.push_back( loadVariableFromFile( configFile, ver, rangeNum ) );
	}
	return tempVariables;
}


void ParameterSystem::updateVariationNumber( )
{
	// if no variables, or all are constants, it will stay at 1. else, it will get set to the # of variations
	// of the first variable that it finds.
	std::vector<ULONG> dimVariations;
	std::vector<bool> dimsSeen;
	for ( auto tempVariable : currentParameters )
	{
		if ( !tempVariable.constant )
		{
			if ( dimsSeen.size( ) < tempVariable.scanDimension )
			{
				dimVariations.resize( tempVariable.scanDimension, 0 );
				dimsSeen.resize( tempVariable.scanDimension, false );
			}
			if ( dimsSeen[tempVariable.scanDimension-1] )
			{
				// already seen.
				continue;
			}
			else
			{
				// now it's been seen, don't add it again.
				dimsSeen[tempVariable.scanDimension - 1] = true;
			}
			for ( auto range : rangeInfo )
			{
				dimVariations[ tempVariable.scanDimension - 1 ] += range.variations;
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
	// adjust text.
	LVFINDINFO param = { 0 };
	param.flags = LVFI_STRING;
	param.psz = (LPCSTR)paramName.c_str();
	auto item = parametersListview.FindItem( &param );
	if ( item == -1 )
	{
		thrower ( "parameter named \"" + paramName
				 + "\" was found in list of parameters, but on in parameter control???" );
	}
	parametersListview.SetItem ( str ( value ), item, 1 );
}


parameterType ParameterSystem::loadVariableFromFile( std::ifstream& openFile, Version ver, UINT rangeNum )
{
	parameterType tempVar;
	std::string varName, typeText, valueString;
	openFile >> varName >> typeText;
	std::transform( varName.begin( ), varName.end( ), varName.begin( ), ::tolower );
	tempVar.name = varName;
	if ( typeText == "Constant" )
	{
		tempVar.constant = true;
	}
	else if ( typeText == "Variable" )
	{
		tempVar.constant = false;
	}
	else
	{
		thrower ( "unknown variable type option: \"" + typeText + "\" for variable \"" + varName 
				 + "\". Check the formatting of the configuration file." );
	}
	if (ver > Version("2.7" ) )
	{
		openFile >> tempVar.scanDimension;
	}
	else
	{
		tempVar.scanDimension = 1;
	}
	UINT rangeNumber;
	if ( ver <= Version ( "3.4" ) )
	{
		openFile >> rangeNumber;
		// I think it's unlikely to ever need more than 2 or 3 ranges.
		if ( rangeNumber < 1 || rangeNumber > 100 )
		{
			errBox ( "Bad range number! setting it to 1, but found " + str ( rangeNumber ) + " in the file." );
			rangeNumber = 1;
		}
	}
	UINT totalVariations = 0;
	for ( auto rangeInc : range( rangeNum ) )
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
		tempVar.ranges.push_back ( { initValue, finValue } );
	}
	// shouldn't be because of 1 forcing earlier.
	if ( tempVar.ranges.size( ) == 0 )
	{
		// make sure it has at least one entry.
		tempVar.ranges.push_back ( { 0,0 } );
	}
	if (ver >= Version("2.14") )
	{
		openFile >> tempVar.constantValue;
	}
	else
	{
		tempVar.constantValue = tempVar.ranges[0].initialValue;
	}
	if (ver > Version("3.2"))
	{
		openFile >> tempVar.parameterScope;
	}
	else
	{
		tempVar.parameterScope = GLOBAL_PARAMETER_SCOPE;
	}
	return tempVar;
}


void ParameterSystem::saveVariable( std::ofstream& saveFile, parameterType variable )
{
	saveFile << variable.name << " " << ( variable.constant ? "Constant " : "Variable " ) << variable.scanDimension << "\n";
	for ( auto& range : variable.ranges )
	{
		saveFile << range.initialValue << "\n" << range.finalValue << "\n";
	}
	saveFile << variable.constantValue << "\n" << variable.parameterScope << "\n";
}


void ParameterSystem::handleSaveConfig(std::ofstream& saveFile)
{
	saveFile << configDelim + "\n";
	saveFile << "RANGE-INFO\n";
	saveFile << rangeInfo.size ( ) << "\n";
	for ( auto range : rangeInfo )
	{
		saveFile << range.leftInclusive << "\n" << range.rightInclusive << "\n" << range.variations << "\n";
	}
	saveFile << getCurrentNumberOfVariables ( ) << "\n";
	for ( UINT varInc = 0; varInc < getCurrentNumberOfVariables( ); varInc++ )
	{
		saveVariable(saveFile, getVariableInfo( varInc ));
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
		if ( variable.scanDimension == scanDimensions )
		{
			variable.scanDimension--;
		}
	}
	// find the last such dimension border item.
	UINT itemNumber = parametersListview.GetItemCount ( );
	for ( UINT item = itemNumber; item >= 0; item-- )
	{
		CString text;
		text = parametersListview.GetItemText ( item, 0 );
		if ( text == "Symbol" )
		{
			// delete "new" for this border range
			parametersListview.DeleteItem ( parametersListview.GetItemCount ( ) - 1 );
			parametersListview.DeleteItem ( item );
			break;
		}
	}
	scanDimensions--;
}


void ParameterSystem::checkVariationRangeConsistency ( )
{
	UINT dum = -1;
	for ( auto& var : currentParameters )
	{
		if ( var.ranges.size ( ) != rangeInfo.size ( ) )
		{
			if ( dum == 0 )
			{
				errBox ( "The number of variation ranges of a parameter, " + var.name +
						 ", did not match the official number. The code will force the parameter to match the official"
						 " number." );
				dum++;
				// only dislpay the error message once.
			}
			var.ranges.resize ( rangeInfo.size ( ) );
		}
	}
}


void ParameterSystem::setVariationRangeNumber ( int num, USHORT dimNumber )
{
	auto columnCount = parametersListview.GetHeaderCtrl ( )->GetItemCount ( );
	// -2 for the two +- columns
	int currentVariableRangeNumber = ( columnCount - preRangeColumns - 2 ) / 3;
	checkVariationRangeConsistency ( );
	if ( rangeInfo.size ( ) != currentVariableRangeNumber )
	{
		errBox ( "somehow, the number of ranges the ParameterSystem object thinks there are and the actual number "
				 "displayed are off! The numbers are " + str ( rangeInfo.size ( ) ) + " and "
				 + str ( currentVariableRangeNumber ) + " respectively. The program will attempt to fix this, but "
				 "data may be lost." );
		while ( rangeInfo.size ( ) != currentVariableRangeNumber )
		{
			if ( rangeInfo.size ( ) < currentVariableRangeNumber )
			{
				rangeInfo.pop_back ( );
				for ( auto& param : currentParameters )
				{
					param.ranges.pop_back( );
				}
			}
			else
			{
				rangeInfo.push_back ( defaultRangeInfo );
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
			parametersListview.InsertColumn( preRangeColumns + 3 * currentVariableRangeNumber, 
											 str ( currentVariableRangeNumber + 1 ) + ":(", 0x20 );
			parametersListview.InsertColumn( preRangeColumns + 1 + 3 * currentVariableRangeNumber, "]");
			parametersListview.InsertColumn( preRangeColumns + 2 + 3 * currentVariableRangeNumber, "#");
			// edit all variables
			rangeInfo.push_back ( defaultRangeInfo );
			for (UINT varInc = 0; varInc < currentParameters.size(); varInc++)
			{
				indvParamRangeInfo tempInfo{ 0,0 };
				currentParameters[varInc].ranges.push_back( tempInfo );
				std::string txt = currentParameters[ varInc ].constant ? "---" : "0";
				parametersListview.SetItem ( txt, varInc, preRangeColumns + 3 * currentVariableRangeNumber );
				parametersListview.SetItem ( txt, varInc, preRangeColumns + 1 + 3 * currentVariableRangeNumber );
				parametersListview.SetItem ( txt, varInc, preRangeColumns + 2 + 3 * currentVariableRangeNumber );
			}
			int newRangeNum = (parametersListview.GetHeaderCtrl()->GetItemCount() - 5) / 3;
			// make sure this makes sense.
			if (currentVariableRangeNumber != newRangeNum - 1)
			{
				errBox("Error! Range numbers after new range don't make sense!");
			}
			currentVariableRangeNumber = newRangeNum;
		}
	}
	else if (currentVariableRangeNumber > num)
	{
		while (currentVariableRangeNumber > num)
		{
			// delete a range.
			if (rangeInfo.size() == 1)
			{
				// can't delete last set...
				return;
			}
			parametersListview.DeleteColumn( preRangeColumns + 3 * (currentVariableRangeNumber - 1));
			parametersListview.DeleteColumn( preRangeColumns + 3 * (currentVariableRangeNumber - 1));
			parametersListview.DeleteColumn( preRangeColumns + 3 * (currentVariableRangeNumber - 1));
			// edit all variables
			for (UINT varInc = 0; varInc < currentParameters.size(); varInc++)
			{
				currentParameters[varInc].ranges.pop_back();
			}
			rangeInfo.pop_back ( );
			// account for the first 2 columns (name, constant) and the last three (currently the extra dimension stuff
			int newRangeNum = (parametersListview.GetHeaderCtrl()->GetItemCount() - 5) / 3;
			currentVariableRangeNumber = newRangeNum;
		}
	}
	// if equal, nothing needs to be done.
}


void ParameterSystem::handleColumnClick(NMHDR * pNotifyStruct, LRESULT * result)
{
	POINT cursorPos;
	GetCursorPos(&cursorPos);
	parametersListview.ScreenToClient(&cursorPos);
	LVHITTESTINFO myItemInfo;
	memset(&myItemInfo, 0, sizeof(LVHITTESTINFO));
	myItemInfo.pt = cursorPos;
	parametersListview.SubItemHitTest(&myItemInfo);
	if (myItemInfo.iSubItem == preRangeColumns + 3 * rangeInfo.size())
	{
		// add a range.
		setVariationRangeNumber( rangeInfo.size ( ) + 1, 1);
	}
	else if (myItemInfo.iSubItem == preRangeColumns + 1 + 3 * rangeInfo.size ( ) )
	{
		// delete a range.
		setVariationRangeNumber( rangeInfo.size ( ) - 1, 1);
	}
	else if (myItemInfo.iSubItem >= preRangeColumns && (myItemInfo.iSubItem - preRangeColumns) % 3 == 0)
	{
		// switch between [ and (
		if ( currentParameters.size( ) == 0 )
		{
			return;
		}
		UINT rangeNum = (myItemInfo.iSubItem - preRangeColumns) / 3;
		setRangeInclusivity( rangeNum, true, !rangeInfo[rangeNum].leftInclusive, myItemInfo.iSubItem );
	}
	else if (myItemInfo.iSubItem >= preRangeColumns && (myItemInfo.iSubItem- preRangeColumns) % 3 == 1)
	{
		// switch between ] and )
		if ( currentParameters.size( ) == 0 )
		{
			return;
		}
		UINT rangeNum = (myItemInfo.iSubItem - preRangeColumns) / 3;
		setRangeInclusivity( rangeNum, false, !rangeInfo[ rangeNum ].rightInclusive, myItemInfo.iSubItem );
	}
}


void ParameterSystem::setRangeInclusivity( UINT rangeNum, bool leftBorder, bool inclusive, UINT column )
{
	if ( rangeNum >= rangeInfo.size ( ) )
	{
		thrower  ( "tried to set the border inclusivity of a range that does not exist!" );
	}
	if ( leftBorder )
	{
		rangeInfo[ rangeNum ].leftInclusive = inclusive;
		LVCOLUMNA colInfo = { 0 };
		colInfo.cchTextMax = 100;
		colInfo.mask = LVCF_TEXT;
		parametersListview.GetColumn( column, &colInfo );
		std::string text;
		if ( inclusive )
		{
			text = str( rangeNum + 1 ) + ". [";
		}
		else
		{
			text = str( rangeNum + 1 ) + ". (";
		}
		colInfo.pszText = &text[0];
		parametersListview.SetColumn( column, &colInfo );
	}
	else
	{
		// it's the right border then.
		rangeInfo[ rangeNum ].rightInclusive = inclusive;
		LVCOLUMNA colInfo = { 0 };
		colInfo.cchTextMax = 100;
		colInfo.mask = LVCF_TEXT;
		parametersListview.GetColumn( column, &colInfo );
		colInfo.pszText = inclusive ? "]" : ")";
		parametersListview.SetColumn( column, &colInfo );
	}
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
				pLVCD->clrTextBk = _myRGBs["Interactive-Bkgd"];
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


void ParameterSystem::updateParameterInfo( std::vector<Script*> scripts, MainWindow* mainWin, AuxiliaryWindow* auxWin,
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
	int subitem, itemIndicator;
	itemIndicator = myItemInfo.iItem;
	if (itemIndicator == -1)
	{
		return;
	}
	subitem = myItemInfo.iSubItem;
	/// check if adding new variable
	CString text = parametersListview.GetItemText(itemIndicator, 0);
	if (text == "___")
	{
		// add a variable
		currentParameters.resize(currentParameters.size() + 1);
		currentParameters.back().name = "";
		currentParameters.back().constant = true;
		currentParameters.back().active = false;
		currentParameters.back().overwritten = false;
		currentParameters.back().scanDimension = 1;

		for ( auto rangeVariations : rangeInfo )
		{
			currentParameters.back ( ).ranges.push_back ( { 0, 0 } );
		}
		// make a new "new" row.
		parametersListview.InsertItem ( "___", itemIndicator, 0 );
		if ( paramSysType == ParameterSysType::global )
		{			
			parametersListview.SetItem ( "0", itemIndicator, 1 );
		}
		else
		{
			parametersListview.SetItem ( "Constant", itemIndicator, 1 );
			parametersListview.SetItem ( "A", itemIndicator, 2 );
			parametersListview.SetItem ( "0", itemIndicator, 3 );
			parametersListview.SetItem ( GLOBAL_PARAMETER_SCOPE, itemIndicator, 4 );
			for ( int rangeInc = 0; rangeInc < rangeInfo.size(); rangeInc++ )
			{
				for ( auto inc : range( 3 ) )
				{
					parametersListview.SetItem ( "---", itemIndicator, preRangeColumns + 3 * rangeInc + inc );
				}
			}
		}
	}
	auto& param = currentParameters[ itemIndicator ];
	/// Handle different subitem clicks
	switch (subitem)
	{
		case 0:
		{
			/// person name
			std::string newName;
			TextPromptDialog dialog(&newName, "Please enter a name for the variable:");
			dialog.DoModal();
			// make name lower case
			newName = str ( newName, 13, false, true );
			if (newName == "")
			{
				// probably canceled.
				break;
			}
			for (auto variable : currentParameters)
			{
				if (variable.name == newName)
				{
					thrower ( "A varaible with name " + newName + " already exists!" );
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
			parametersListview.SetItem ( newName, itemIndicator, subitem );
			break;
		}
		case 1:
		{
			if ( paramSysType == ParameterSysType::global )
			{
				/// global value
				std::string newValue;
				TextPromptDialog dialog(&newValue, "Please enter a value for the global variable "
										+ param.name + ". Value will be formatted as a double.");
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
				std::string temp(str( param.constantValue));
				parametersListview.SetItem ( temp, itemIndicator, subitem );
				break;
			}
			else
			{
				/// constant or variable?
				// this is just a binary switch.
				if ( param.constant)
				{
					// switch to variable.
					param.constant = false;
					parametersListview.SetItem ( "Variable", itemIndicator, subitem );
					parametersListview.SetItem ( "---", itemIndicator, 3 );
					for (UINT rangeInc = 0; rangeInc < param.ranges.size(); rangeInc++)
					{
						parametersListview.SetItem ( str ( param.ranges[ rangeInc ].initialValue, 13 ),
													 itemIndicator, preRangeColumns + rangeInc * 3 );
						parametersListview.SetItem ( str ( param.ranges[ rangeInc ].finalValue, 13 ),
													 itemIndicator, preRangeColumns + 1 + rangeInc * 3 );
						parametersListview.SetItem ( str ( rangeInfo[ rangeInc ].variations ),
													 itemIndicator, preRangeColumns + 2 + rangeInc * 3 );
					}
				}
				else
				{
					/// switch to constant.
					parametersListview.SetItem ( "Constant", itemIndicator, subitem );
					parametersListview.SetItem ( str ( param.constantValue ), itemIndicator, 3 );
					param.constant = true;
					for (int rangeInc = 0; rangeInc < rangeInfo.size(); rangeInc++)
					{
						// set the value to be dashes on the screen. no value for "Variable".
						parametersListview.SetItem ( "---", itemIndicator, preRangeColumns + rangeInc * 3 );
						parametersListview.SetItem ( "---", itemIndicator, 1 + preRangeColumns + rangeInc * 3 );
						parametersListview.SetItem ( "---", itemIndicator, 2 + preRangeColumns + rangeInc * 3 );
					}
				}
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
			if ( param.scanDimension > maxDim + 1 )
			{
				param.scanDimension = 1;
			}
			reorderVariableDimensions( );
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
									 + param.name + ". Value will be formatted as a double." );
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
			// update the listview
			parametersListview.SetItem ( str ( param.constantValue, 13 ), itemIndicator, subitem );
			break;
		}
		case 4:
		{
			/// scope
			std::string newScope;
			TextPromptDialog dialog( &newScope, "Please enter a the scope for the variable: \""
									 + param.name + "\". You may enter a function name, "
									 "\"parent\", or \"global\"." );
			dialog.DoModal( );
			if ( newScope == "" )
			{
				// probably canceled.
				break;
			}
			newScope = str ( newScope, 13, false, true );
			// update the listview
			param.parameterScope = newScope;
			parametersListview.SetItem ( param.parameterScope, itemIndicator, subitem );
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
			TextPromptDialog dialog( &newValue, "Please enter a value for the variable " + param.name + "." );
			dialog.DoModal( );
			if ( newValue == "" )
			{
				// probably canceled.
				break;
			}
			if ((subitem - preRangeColumns) % 3 == 0 || (subitem - preRangeColumns) % 3 == 1)
			{
				double val;
				try 
				{
					val = boost::lexical_cast<double>(newValue);
				}
				catch ( boost::bad_lexical_cast&)
				{
					throwNested ("the value entered, " + newValue + ", failed to convert to a double! "
							 "Check for invalid characters.");
				}
				if ( (subitem - preRangeColumns) % 3 == 0 )
				{
					param.ranges[rangeNum].initialValue = val;
				}
				else
				{
					param.ranges[rangeNum].finalValue = val;
				}
				parametersListview.SetItem ( str ( val, 13 ), itemIndicator, subitem );
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
							rangeInfo[ rangeNum ].variations = boost::lexical_cast<int> ( newValue );
						}
					}
				}
				catch ( boost::bad_lexical_cast&)
				{
					throwNested ("the value entered, " + newValue + ", failed to convert to a double! Check "
									"for invalid characters.");
				}
				for (auto varInc : range(currentParameters.size()))
				{
					if (!currentParameters[varInc].constant 
						 && (currentParameters[varInc].scanDimension == param.scanDimension))
					{
						parametersListview.SetItem ( str ( rangeInfo[ rangeNum ].variations ), varInc, subitem);
					}
				}
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
	LVHITTESTINFO myItemInfo;
	memset(&myItemInfo, 0, sizeof(LVHITTESTINFO));
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
			parametersListview.DeleteItem(itemIndicator);
			currentParameters.erase(currentParameters.begin() + itemIndicator);
		}

	}
	else if (UINT(itemIndicator) > currentParameters.size())
	{
		answer = promptBox("You appear to have found a bug with the listview control... there are too many lines "
							 "in this control. Clear this line?", MB_YESNO);
		if (answer == IDYES)
		{
			parametersListview.DeleteItem(itemIndicator);
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


void ParameterSystem::clearVariables()
{
	currentParameters.clear();
	int itemCount = parametersListview.GetItemCount();
	for (int itemInc = 0; itemInc < itemCount; itemInc++)
	{
		parametersListview.DeleteItem(0);
	}
}


std::vector<parameterType> ParameterSystem::getEverything()
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
	std::vector<parameterType> varyingParameters;
	for (UINT varInc = 0; varInc < currentParameters.size(); varInc++)
	{
		// opposite of get constants.
		if (!currentParameters[varInc].constant)
		{
			varyingParameters.push_back(currentParameters[varInc]);
		}
	}
	return varyingParameters;
}


void ParameterSystem::addGlobalParameter( parameterType variable, UINT item )
{
	// convert name to lower case.
	std::transform( variable.name.begin(), variable.name.end(), variable.name.begin(), ::tolower );
	if (isdigit(variable.name[0]))
	{
		thrower ("" + variable.name + " is an invalid name; names cannot start with numbers.");
	}
	if (variable.name.find_first_of(" \t\r\n()*+/-%") != std::string::npos)
	{
		thrower ("Forbidden character in variable name! you cannot use spaces, tabs, newlines, or any of "
				"\"()*+/-%\" in a variable name.");
	}
	if (variable.name == "")
	{
		parametersListview.insertBlankRow ( );
		parametersListview.RedrawWindow();
		return;
	}
	/// else...
	if (variable.constant == false)
	{
		thrower ( "attempted to add a non-constant to the global variable control!" );
	}
	for (auto currentVar : currentParameters)
	{
		if (currentVar.name == variable.name)
		{
			thrower ( "A variable with the name " + variable.name + " already exists!" );
		}
	}
	// add it to the internal structure that keeps track of variables
	currentParameters.push_back( variable );
	/// add the entry to the listview.
	parametersListview.InsertItem ( variable.name, item, 0 );
	parametersListview.SetItem( str ( variable.ranges.front ( ).initialValue, 13, true ), item, 1 );
}


void ParameterSystem::addConfigParameter(parameterType variableToAdd, UINT item)
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
		parametersListview.insertBlankRow ( );
		parametersListview.RedrawWindow();
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
	// add it to the internal structure that keeps track of variables
	currentParameters.push_back(variableToAdd);
	if ( parametersListview.m_hWnd == NULL )
	{
		return;
	}

	parametersListview.InsertItem ( variableToAdd.name, item, 0 );
	if (variableToAdd.constant)
	{
		parametersListview.SetItem ( "Constant", item, 1 );
		parametersListview.SetItem ( str(variableToAdd.constantValue), item, 3 );
	}
	else
	{
		parametersListview.SetItem ( "Variable", item, 1 );
		parametersListview.SetItem ( "---", item, 3 );
	}
	parametersListview.SetItem ( str(char ( 'A' + variableToAdd.scanDimension - 1 ) ), item, 2 );
	parametersListview.SetItem ( variableToAdd.parameterScope, item, 4 );
	// make sure there are enough currentRanges.
	UINT currentRanges = currentParameters.front( ).ranges.size( );
	// not sure why this would happen, but was bug.
	if ( variableToAdd.ranges.size( ) < currentRanges )
	{
		variableToAdd.ranges.resize( currentRanges );
	}

	for (auto rangeAddInc : range(variableToAdd.ranges.size() - currentRanges))
	{
		// add a range.
		parametersListview.InsertColumn ( preRangeColumns + 3 * rangeInfo.size(), str ( rangeInfo.size ( ) + 1 ) + ":(", 0x20 );
		parametersListview.InsertColumn ( preRangeColumns + 1 + 3 * rangeInfo.size ( ), "]" );
		parametersListview.InsertColumn ( preRangeColumns + 2 + 3 * rangeInfo.size ( ), "#" );
		// edit all variables
		for (auto varInc : range(currentParameters.size()))
		{
			currentParameters[ varInc ].ranges.push_back ( { 0,0 } );
			for ( auto inc : range( 3 ) )
			{
				parametersListview.SetItem ( currentParameters[ varInc ].constant ? "---" : "0", varInc, 
											 preRangeColumns + 3 * rangeInfo.size ( ) + inc);
			}
		}
		rangeInfo.push_back ( defaultRangeInfo );
	}
	// update ranges
	for (auto rangeInc : range(variableToAdd.ranges.size()))
	{
		if (!variableToAdd.constant)
		{
			// variable case.
			auto& range = currentParameters[ item ].ranges[ rangeInc ];
			parametersListview.SetItem ( str ( range.initialValue, 13, true ), item, preRangeColumns + rangeInc * 3 );
			parametersListview.SetItem ( str ( range.finalValue, 13, true ), item, preRangeColumns + 1 + rangeInc * 3 );
			parametersListview.SetItem ( str ( rangeInfo[rangeInc].variations, 13, true ), item, preRangeColumns + 2 + rangeInc * 3 );
		}
		else
		{
			// constant case.
			parametersListview.SetItem ( "---", item, preRangeColumns + rangeInc * 3 );
			parametersListview.SetItem ( "---", item, preRangeColumns + 1 + rangeInc * 3 );
			parametersListview.SetItem ( "---", item, preRangeColumns + 2 + rangeInc * 3 );
		}
	}
	parametersListview.RedrawWindow();
	updateVariationNumber( );
}


std::vector<variationRangeInfo> ParameterSystem::getRangeInfo ( )
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
	/// flatten the dimension numbers.
	UINT flattenNumber = 0;
	for ( auto dimInc : range( maxDim ) )
	{
		bool found = false;
		for ( auto& variable : currentParameters )
		{
			if ( variable.scanDimension == dimInc+1 )
			{
				variable.scanDimension = dimInc + 1 - flattenNumber;
				found = true;
			}
		}
		if ( !found )
		{
			flattenNumber++;
		}
	}
	/// reset variables
	std::vector<parameterType> varCopy = currentParameters;
	clearVariables( );
	UINT variableInc=0;
	for ( auto& variable : varCopy )
	{
		addConfigParameter( variable, variableInc++ );
	}
	addConfigParameter( {}, -1 );
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
	std::ifstream f(configFileName);
	Version ver;
	//ProfileSystem::getVersionFromFile( f, ver );
	std::vector<parameterType> configVariables;
	try
	{
		ProfileSystem::initializeAtDelim ( f, "CONFIG_PARAMETERS", ver, Version ( "4.0" ) );
		auto rInfo = getRangeInfoFromFile ( f, ver );
		configVariables = getVariablesFromFile ( f, ver, rInfo.size ( ) );
		ProfileSystem::checkDelimiterLine ( f, "END_CONFIG_PARAMETERS" );
	}
	catch ( Error& )
	{
		throwNested ( "Failed to get configuration parameters from the configuration file!" );
	}
	return configVariables;
}


std::vector<variationRangeInfo> ParameterSystem::getRangeInfoFromFile ( std::string configFileName )
{
	std::ifstream f ( configFileName );
	Version ver;
	std::vector<variationRangeInfo> rInfo;
	try
	{
		ProfileSystem::initializeAtDelim ( f, "CONFIG_PARAMETERS", ver, Version ( "4.0" ) );
		rInfo = getRangeInfoFromFile ( f, ver );
		auto configVariables = getVariablesFromFile ( f, ver, rInfo.size ( ) );
		ProfileSystem::checkDelimiterLine ( f, "END_CONFIG_PARAMETERS" );
	}
	catch ( Error& )
	{
		throwNested ( "Failed to get configuration parameter range info from file!" );
	}
	return rInfo;
}


void ParameterSystem::generateKey( std::vector<std::vector<parameterType>>& variables, bool randomizeVariablesOption,
								   std::vector<variationRangeInfo> inputRangeInfo )
{
	// get information from variables.
	for ( auto& seqVariables : variables )
	{
		for ( auto& variable : seqVariables )
		{
			variable.keyValues.clear( );
		}
	}
	// get maximum dimension.
	UINT maxDim = 0;
	for ( auto seqVariables : variables )
	{
		for ( auto variable : seqVariables )
		{
			if ( variable.scanDimension > maxDim )
			{
				maxDim = variable.scanDimension;
			}
		}
	}
	// each element of the vector refers to the number of variations within a given variation range.
	// variations[seqNumber][dimNumber][rangeNumber]
	std::vector<std::vector<std::vector<int>>> variations( variables.size( ), std::vector<std::vector<int>>(maxDim));
	std::vector<std::vector<int>> variableIndexes(variables.size());
	for (auto seqInc : range(variables.size()) )
	{
		for ( UINT dimInc : range( maxDim ) )
		{
			variations[seqInc][dimInc].resize( variables[seqInc].front( ).ranges.size( ) );
			for ( UINT varInc = 0; varInc < variables[seqInc].size( ); varInc++ )
			{
				auto& variable = variables[seqInc][varInc];
				if ( variable.scanDimension != dimInc + 1 )
				{
					continue;
				}
				// find a varying parameter.
				if ( variable.constant )
				{
					continue;
				}
				// then this variable varies in this dimension. 
				variableIndexes[seqInc].push_back( varInc );
				// variations.size is the number of ranges currently.
				if ( variations[seqInc][dimInc].size( ) != variable.ranges.size( ) )
				{
					// if its zero its just the initial size on the initial variable.
					if ( variations[seqInc].size( ) != 0 )
					{
						thrower ( "Not all variables seem to have the same number of ranges for their parameters!" );
					}
					variations[seqInc][dimInc].resize( variable.ranges.size( ) );
				}
				// make sure the variations number is consistent between
				for ( auto rangeInc : range( variations[seqInc][dimInc].size( ) ) )
				{
					auto& variationNum = variations[seqInc][dimInc][rangeInc];
					if ( variable.scanDimension != dimInc + 1 )
					{
						continue;
					}
					// can break here...?
					variationNum = inputRangeInfo[ rangeInc ].variations;
				}
			}
		}
	}
	std::vector<std::vector<UINT>> totalVariations( variations.size(), std::vector<UINT>(maxDim) );
	for ( auto seqInc : range( variations.size( ) ) )
	{
		for ( auto dimInc : range( variations[seqInc].size( ) ) )
		{
			totalVariations[seqInc][dimInc] = 0;
			for ( auto variationsInRange : variations[seqInc][dimInc] )
			{
				totalVariations[seqInc][dimInc] += variationsInRange;
			}
		}
	}
	// create a key which will be randomized and then used to randomize other things the same way.
	multiDimensionalKey<int> randomizerMultiKey( maxDim );
	randomizerMultiKey.resize( totalVariations );
	UINT count = 0;
	for ( auto& keyElem : randomizerMultiKey.values[0] )
	{
		keyElem = count++;
	}
	if ( randomizeVariablesOption )
	{
		std::random_device rng;
		std::mt19937 twister( rng( ) );
		std::shuffle( randomizerMultiKey.values[0].begin( ), randomizerMultiKey.values[0].end( ), twister );
		// we now have a random key for the shuffling which every variable will follow
		// initialize this to one so that constants always get at least one value.
	}
	int totalSize = 1;
	for ( auto seqInc : range( variableIndexes.size( ) ) )
	{
		if ( variableIndexes[ seqInc ].size ( ) == 0 && inputRangeInfo[seqInc].variations != 1 )
		{
			//thrower  ( "Key generator thinks that there are variations but no variables vary?!?! Low level bug"
			//		  ", this shouldn't happen." );
		}
		for ( auto variableInc : range( variableIndexes[seqInc].size( ) ) )
		{
			int varIndex = variableIndexes[seqInc][variableInc];
			auto& variable = variables[seqInc][varIndex];
			// calculate all values for a given variable
			multiDimensionalKey<double> tempKey( maxDim ), tempKeyRandomized( maxDim );
			tempKey.resize( totalVariations );
			tempKeyRandomized.resize( totalVariations );
			std::vector<UINT> rangeOffset( totalVariations.size( ), 0 );
			std::vector<UINT> indexes( maxDim );
			while ( true )
			{
				UINT rangeIndex = 0, variationInc = 0;
				UINT varDim = variable.scanDimension - 1;
				UINT relevantIndex = indexes[varDim];
				UINT tempShrinkingIndex = relevantIndex;
				UINT rangeCount = 0, rangeOffset = 0;
				// calculate which range it is and what the index offset should be as a result.
				for ( auto range : inputRangeInfo )
				{
					if ( tempShrinkingIndex >= range.variations )
					{
						tempShrinkingIndex -= range.variations;
						rangeOffset += range.variations;
					}
					else
					{
						rangeIndex = rangeCount;
						break;
					}
					rangeCount++;
				}

				auto& currRange = variable.ranges[rangeIndex];
				if ( variations[seqInc][varDim][rangeIndex] <= 1 )
				{
					thrower ( "You need more than one variation in every range." );
				}
				// calculate the parameters for the variation range
				double valueRange = (currRange.finalValue - currRange.initialValue);
				int spacings;
				
				if ( inputRangeInfo[rangeIndex].leftInclusive && inputRangeInfo[ rangeIndex ].rightInclusive )
				{
					spacings = variations[seqInc][variables[seqInc][varIndex].scanDimension - 1][rangeIndex] - 1;
				}
				else if ( !inputRangeInfo[ rangeIndex ].leftInclusive && !inputRangeInfo[ rangeIndex ].rightInclusive )
				{
					spacings = variations[seqInc][varDim][rangeIndex] + 1;
				}
				else
				{
					spacings = variations[seqInc][varDim][rangeIndex];
				}

				double initVal;
				if ( inputRangeInfo[ rangeIndex ].leftInclusive )
				{
					initVal = currRange.initialValue;
				}
				else
				{
					initVal = currRange.initialValue + valueRange / spacings;
				}
				// calculate values.
				variationInc = indexes[varDim];
				double value = valueRange * (variationInc - rangeOffset) / spacings + initVal;
				tempKey.setValue( indexes, seqInc, value );
				// increment. This part effectively makes this infinite while an arbitrary-dimensional loop.
				bool isAtEnd = true;
				for ( auto indexInc : range( indexes.size( ) ) )
				{
					// if at end of cycle for this index in this range
					if ( indexes[indexInc] == totalVariations[seqInc][indexInc] - 1 )
					{
						indexes[indexInc] = 0;
						continue;
					}
					else
					{
						indexes[indexInc]++;
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
	for ( auto& seqVariables : variables )
	{
		for ( parameterType& variable : seqVariables )
		{
			if ( variable.constant )
			{
				variable.keyValues.clear( );
				variable.keyValues.resize( totalSize );
				for ( auto& val : variable.keyValues )
				{
					// the only constant value is stored as the initial value here.
					val = variable.constantValue;
				}
				variable.valuesVary = false;
			}
		}
	}
}


/*
 * takes global params, config params, and function params, and reorganizes them to form a "parameters" object and a 
 * "constants" objects. The "parameters" object includes everything, variables and otherwise. the "constants" object 
 * includes only parameters that don't vary. 
 */
std::vector<parameterType> ParameterSystem::combineParametersForExperimentThread( std::vector<parameterType>& configParams, 
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

