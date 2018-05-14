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


void ParameterSystem::initialize( POINT& pos, cToolTips& toolTips, CWnd* parent, int& id, std::string title,
								  rgbMap rgbs, UINT listviewId, ParameterSysType type )
{
	paramSysType = type;
	scanDimensions = 1;
	variableRanges = 1;

	parametersHeader.sPos = { pos.x, pos.y, pos.x + 480, pos.y += 25 };
	parametersHeader.Create( cstr( title ), NORM_HEADER_OPTIONS, parametersHeader.sPos, parent, id++ );
	parametersHeader.fontType = fontTypes::HeadingFont;
	
	parametersListview.sPos = { pos.x, pos.y, pos.x + 480, pos.y + 300 };
	parametersListview.Create( NORM_LISTVIEW_OPTIONS, parametersListview.sPos,
							  parent, listviewId );
	parametersListview.fontType = fontTypes::SmallFont;
	parametersListview.SetBkColor( RGB( 15, 15, 15 ) );
	parametersListview.SetTextBkColor( RGB( 15, 15, 15 ) );
	parametersListview.SetTextColor( RGB( 150, 150, 150 ) );
	LV_COLUMN listViewDefaultCollumn;
	// Zero Members
	memset( &listViewDefaultCollumn, 0, sizeof( listViewDefaultCollumn ) );
	listViewDefaultCollumn.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	listViewDefaultCollumn.pszText = "Symbol";
	RECT r;
	parent->GetClientRect( &r );
	if ( paramSysType == ParameterSysType::global )
	{
		listViewDefaultCollumn.cx = 3 * r.right / 5;
		parametersListview.InsertColumn( 0, &listViewDefaultCollumn );
		listViewDefaultCollumn.cx = r.right / 6;
		listViewDefaultCollumn.pszText = "Value";
		parametersListview.InsertColumn( 1, &listViewDefaultCollumn );
		// Make First Blank row.
		LVITEM listViewDefaultItem;
		memset( &listViewDefaultItem, 0, sizeof( listViewDefaultItem ) );
		listViewDefaultItem.mask = LVIF_TEXT; 
		listViewDefaultItem.cchTextMax = 256; 
		listViewDefaultItem.pszText = "___";
		listViewDefaultItem.iItem = 0;     
		listViewDefaultItem.iSubItem = 0;  
		parametersListview.InsertItem( &listViewDefaultItem );
		listViewDefaultItem.iSubItem = 1;
		parametersListview.SetItem( &listViewDefaultItem );
	}
	else 
	{
		listViewDefaultCollumn.cx = r.right/4;
		parametersListview.InsertColumn( 0, &listViewDefaultCollumn );
		listViewDefaultCollumn.pszText = "Type";
		listViewDefaultCollumn.cx = r.right / 10;
		parametersListview.InsertColumn( 1, &listViewDefaultCollumn );
		listViewDefaultCollumn.pszText = "Dim";
		parametersListview.InsertColumn( 2, &listViewDefaultCollumn );
		listViewDefaultCollumn.pszText = "Value";
		parametersListview.InsertColumn( 3, &listViewDefaultCollumn );
		listViewDefaultCollumn.pszText = "Scope";
		parametersListview.InsertColumn( 4, &listViewDefaultCollumn );
		listViewDefaultCollumn.cx = r.right / 15;
		listViewDefaultCollumn.pszText = "1:(";
		parametersListview.InsertColumn( 5, &listViewDefaultCollumn );
		listViewDefaultCollumn.pszText = "]";
		parametersListview.InsertColumn( 6, &listViewDefaultCollumn );
		listViewDefaultCollumn.pszText = "#";
		parametersListview.InsertColumn( 7, &listViewDefaultCollumn );
		listViewDefaultCollumn.pszText = "+()";
		parametersListview.InsertColumn( 8, &listViewDefaultCollumn );
		listViewDefaultCollumn.pszText = "-()";
		parametersListview.InsertColumn( 9, &listViewDefaultCollumn );
		if ( paramSysType == ParameterSysType::config )
		{
			// Make First Blank row.
			LVITEM listViewDefaultItem;
			memset( &listViewDefaultItem, 0, sizeof( listViewDefaultItem ) );
			listViewDefaultItem.mask = LVIF_TEXT;
			listViewDefaultItem.cchTextMax = 256;
			listViewDefaultItem.pszText = "___";
			listViewDefaultItem.iItem = 0;
			listViewDefaultItem.iSubItem = 0;
			parametersListview.InsertItem( &listViewDefaultItem );
			for ( int itemInc = 1; itemInc < 7; itemInc++ )
			{
				listViewDefaultItem.iSubItem = itemInc;
				parametersListview.SetItem( &listViewDefaultItem );
			}
		}
	}
	parametersListview.SetBkColor( rgbs["Solarized Base02"] );
	pos.y += 300;
}


/*
 * The "normal" function, used for config and global variable systems.
 */
void ParameterSystem::normHandleOpenConfig( std::ifstream& configFile, Version ver )
{
	ProfileSystem::checkDelimiterLine( configFile, "VARIABLES" );
	// handle variables
	clearVariables( );
	std::vector<parameterType> variables;
	try
	{
		variables = getVariablesFromFile( configFile, ver );
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
		for ( auto rangeInc : range( currentParameters.front( ).ranges.size( ) ) )
		{
			bool leftInclusivity = currentParameters.front( ).ranges[rangeInc].leftInclusive;
			bool rightInclusivity = currentParameters.front( ).ranges[rangeInc].rightInclusive;
			setRangeInclusivity( rangeInc, true, leftInclusivity, preRangeColumns + rangeInc * 3 );
			setRangeInclusivity( rangeInc, false, rightInclusivity, preRangeColumns + 1 + rangeInc * 3 );
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
	var.ranges.push_back( { 0, 0, 1, false, true } );
	addConfigParameter( var, currentParameters.size( ) );
	ProfileSystem::checkDelimiterLine( configFile, "END_VARIABLES" );
	updateVariationNumber( );
}


std::vector<parameterType> ParameterSystem::getVariablesFromFile( std::ifstream& configFile, Version ver )
{
	UINT variableNumber;
	configFile >> variableNumber;
	if ( variableNumber > 100 )
	{
		int answer = promptBox( "ERROR: variable number retrieved from file appears suspicious. The number is "
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
		tempVariables.push_back( loadVariableFromFile( configFile, ver ) );
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
			for ( auto range : tempVariable.ranges )
			{
				dimVariations[tempVariable.scanDimension - 1] += range.variations;
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
	newFile << "VARIABLES\n";
	// Number of functions with variables saved
	newFile << 0 << "\n";
	newFile << "END_VARIABLES\n";
}


void ParameterSystem::adjustVariableValue( std::string paramName, double value )
{
	if ( paramSysType != ParameterSysType::global )
	{
		thrower( "ERROR: adjusting variable values in the code like this is only meant to be used with global variables!" );
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
		thrower( "ERROR: variable \"" + paramName + "\" not found in global varable control!" );
	}
	// adjust text.
	LVITEM listViewItem;
	memset( &listViewItem, 0, sizeof( listViewItem ) );
	listViewItem.mask = LVIF_TEXT;
	listViewItem.cchTextMax = 256;

	LVFINDINFO param = { 0 };
	param.flags = LVFI_STRING;
	param.psz = (LPCSTR)paramName.c_str();
	listViewItem.iItem = parametersListview.FindItem( &param );
	if ( listViewItem.iItem == -1 )
	{
		thrower( "ERROR: parameter named \"" + paramName
				 + "\" was found in list of parameters, but on in parameter control???" );
	}
	listViewItem.iSubItem = 1;
	std::string txt = str( value );
	listViewItem.pszText = (LPSTR)txt.c_str( );

	parametersListview.SetItem( &listViewItem );
}


parameterType ParameterSystem::loadVariableFromFile( std::ifstream& openFile, Version ver )
{

	parameterType tempVar;
	std::string varName, typeText, valueString;
	bool constant;
	openFile >> varName >> typeText;
	std::transform( varName.begin( ), varName.end( ), varName.begin( ), ::tolower );
	tempVar.name = varName;
	if ( typeText == "Constant" )
	{
		constant = true;
		tempVar.constant = true;
	}
	else if ( typeText == "Variable" )
	{
		constant = false;
		tempVar.constant = false;
	}
	else
	{
		thrower( "ERROR: unknown variable type option: \"" + typeText + "\" for variable \"" + varName 
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
	openFile >> rangeNumber;
	// I think it's unlikely to ever need more than 2 or 3 ranges.
	if ( rangeNumber < 1 || rangeNumber > 100 )
	{
		errBox( "ERROR: Bad range number! setting it to 1, but found " + str( rangeNumber ) + " in the file." );
		rangeNumber = 1;
	}
	//setVariationRangeNumber( rangeNumber, 1 );
	UINT totalVariations = 0;
	for ( auto rangeInc : range( rangeNumber ) )
	{
		double initValue = 0, finValue = 0;
		unsigned int variations = 0;
		bool leftInclusive = 0, rightInclusive = 0;
		openFile >> initValue >> finValue >> variations >> leftInclusive >> rightInclusive;
		totalVariations += variations;
		tempVar.ranges.push_back( { initValue, finValue, variations, leftInclusive, rightInclusive } );
	}
	// shouldn't be because of 1 forcing earlier.
	if ( tempVar.ranges.size( ) == 0 )
	{
		// make sure it has at least one entry.
		tempVar.ranges.push_back( { 0,0,1, false, true } );
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
	saveFile << variable.name << " " << (variable.constant ? "Constant " : "Variable ") << variable.scanDimension << "\n"
		<< variable.ranges.size( ) << "\n";
	for ( auto& range : variable.ranges )
	{
		saveFile << range.initialValue << "\n" << range.finalValue << "\n" << range.variations << "\n"
			<< range.leftInclusive << "\n" << range.rightInclusive << "\n";
	}
	saveFile << variable.constantValue << "\n" << variable.parameterScope << "\n";
}


void ParameterSystem::handleSaveConfig(std::ofstream& saveFile)
{
	saveFile << "VARIABLES\n";
	saveFile << getCurrentNumberOfVariables( ) << "\n";
	for ( UINT varInc = 0; varInc < getCurrentNumberOfVariables( ); varInc++ )
	{
		saveVariable(saveFile, getVariableInfo( varInc ));

	}
	saveFile << "END_VARIABLES\n";
}


void ParameterSystem::rearrange(UINT width, UINT height, fontMap fonts)
{
	parametersHeader.rearrange( width, height, fonts);
	parametersListview.rearrange( width, height, fonts);
}


void loadVariablesFromFunc( std::string funcName )
{
	//clearVariables( );
	std::string funcAddress = FUNCTIONS_FOLDER_LOCATION + funcName + "." + FUNCTION_EXTENSION;
	std::ifstream file;
	ScriptStream stream;
	file.open( funcAddress, std::ios::in );
	if ( !file.is_open( ) )
	{
		thrower( "ERROR: Failed to open function file: " + funcAddress + " for function variables." );
	}
	std::string tempLine;
	std::string fileText;
	while ( std::getline( file, tempLine ) )
	{
		cleanString( tempLine );
		fileText += tempLine;
	}
	file.close( );
	stream.str( fileText );
	stream.clear( );
	stream.seekg( 0 );

	while ( true )
	{
		std::string tmp;
		stream >> tmp;
		if ( tmp == "var" )
		{
			// add to variables!
			std::string name, valStr;
			stream >> name >> valStr;
			parameterType tmpVariable;
			tmpVariable.constant = true;
			std::transform( name.begin( ), name.end( ), name.begin( ), ::tolower );
			tmpVariable.name = name;
			bool found = false;
			double val;
			try
			{
				val = std::stod( valStr );
			}
			catch ( std::invalid_argument& )
			{
				thrower( "ERROR: Bad string for value of local variable " + str( name ) );
			}
			tmpVariable.constantValue = val;
			tmpVariable.scanDimension = 1;
			tmpVariable.ranges.push_back( { val, val, 1, true, true } );
		}
		if ( stream.peek( ) == EOF )
		{
			break;
		}
	}
}


void ParameterSystem::removeVariableDimension()
{
	if (scanDimensions == 1)
	{
		thrower("ERROR: Can't delete last variable scan dimension.");
	}
	// change all variables in the last dimension to be in the second-to-last dimension.
	// TODO: I'm gonna have to check variation numbers here or change them to be compatible.
	for (auto& variable : currentParameters)
	{
		if (variable.scanDimension == scanDimensions)
		{
			variable.scanDimension--;
		}
	}
	// find the last such dimension border item.
	UINT itemNumber = parametersListview.GetItemCount();
	for (UINT item = itemNumber; item >= 0; item--)
	{
		CString text;
		text = parametersListview.GetItemText(item, 0);
		if (text == "Symbol")
		{
			// delete "new" for this border range
			parametersListview.DeleteItem(parametersListview.GetItemCount()-1);
			parametersListview.DeleteItem(item);
			break;
		}
	}
	scanDimensions--;
}


void ParameterSystem::updateCurrentVariationsNum( )
{
	UINT dum = -1;
	for ( auto& var : currentParameters )
	{
		if ( dum == -1 )
		{
			variableRanges = var.ranges.size( );
			dum++;
		}
		else
		{
			if ( variableRanges != var.ranges.size( ) )
			{
				if ( dum == 0 )
				{
					errBox( "ERROR: While loading variables from file, a variable did not have same number of ranges as "
							"first variable loaded. The range number will be changed to make the ranges consistent." );
					dum++;
					// only dislpay the error message once.
				}
				var.ranges.resize( variableRanges );
			}
		}
	}
}


void ParameterSystem::setVariationRangeNumber(int num, USHORT dimNumber)
{
	auto columnCount = parametersListview.GetHeaderCtrl( )->GetItemCount( );
	// -2 for the two +- columns
	int currentVariableRangeNumber = (columnCount - preRangeColumns - 2 ) / 3;
	updateCurrentVariationsNum( );
	if (variableRanges != currentVariableRangeNumber)
	{
		errBox( "ERROR: somehow, the number of ranges the ParameterSystem object thinks there are and the actual number "
				"displayed are off! The numbers are " + str(variableRanges) + " and "
				+ str(currentVariableRangeNumber) + " respectively. The program will attempt to fix this, but " 
				"data may be lost." );
		variableRanges = currentVariableRangeNumber;
		for (auto& variable : currentParameters)
		{
			variable.ranges.resize(currentVariableRangeNumber);
		}
	}
	if (currentVariableRangeNumber < num)
	{
		while (currentVariableRangeNumber < num)
		{
			/// add a range.
			LV_COLUMN listViewDefaultCollumn;
			memset(&listViewDefaultCollumn, 0, sizeof(listViewDefaultCollumn));
			listViewDefaultCollumn.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
			std::string temp(str(str(variableRanges + 1) + ":("));
			listViewDefaultCollumn.pszText = &temp[0];
			listViewDefaultCollumn.cx = 0x20;
			parametersListview.InsertColumn( preRangeColumns + 3 * currentVariableRangeNumber, &listViewDefaultCollumn);
			listViewDefaultCollumn.pszText = "]";
			parametersListview.InsertColumn( preRangeColumns+1 + 3 * currentVariableRangeNumber, &listViewDefaultCollumn);
			listViewDefaultCollumn.pszText = "#";
			parametersListview.InsertColumn( preRangeColumns+2 + 3 * currentVariableRangeNumber, &listViewDefaultCollumn);
			// edit all variables
			LVITEM listViewItem;
			memset(&listViewItem, 0, sizeof(listViewItem));
			listViewItem.mask = LVIF_TEXT; 
			listViewItem.cchTextMax = 256; 
			for (UINT varInc = 0; varInc < currentParameters.size(); varInc++)
			{
				variationRangeInfo tempInfo{ 0,0,0, false, true };
				currentParameters[varInc].ranges.push_back( tempInfo );
				if (currentParameters[varInc].constant)
				{
					listViewItem.pszText = "---";
				}
				else
				{
					listViewItem.pszText = "0";
				}
				listViewItem.iItem = varInc;
				listViewItem.iSubItem = preRangeColumns + 3 * currentVariableRangeNumber;
				parametersListview.SetItem(&listViewItem);
				listViewItem.iSubItem = preRangeColumns + 1 + 3 * currentVariableRangeNumber;
				parametersListview.SetItem(&listViewItem);
				listViewItem.iSubItem = preRangeColumns + 2+ 3 * currentVariableRangeNumber;
				parametersListview.SetItem(&listViewItem);
			}
			int newRangeNum = (parametersListview.GetHeaderCtrl()->GetItemCount() - 5) / 3;
			// make sure this makes sense.
			if (currentVariableRangeNumber != newRangeNum - 1)
			{
				errBox("Error! Range numbers after new range don't make sense!");
			}
			currentVariableRangeNumber = newRangeNum;
			variableRanges = currentVariableRangeNumber;
		}
	}
	else if (currentVariableRangeNumber > num)
	{
		while (currentVariableRangeNumber > num)
		{
			// delete a range.
			if (variableRanges == 1)
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
			// account for the first 2 columns (name, constant) and the last three (currently the extra dimension stuff
			int newRangeNum = (parametersListview.GetHeaderCtrl()->GetItemCount() - 5) / 3;
			currentVariableRangeNumber = newRangeNum;
			variableRanges = currentVariableRangeNumber;
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
	if (myItemInfo.iSubItem == preRangeColumns + 3 * variableRanges)
	{
		// add a range.
		setVariationRangeNumber(variableRanges + 1, 1);
	}
	else if (myItemInfo.iSubItem == preRangeColumns + 1 + 3 * variableRanges )
	{
		// delete a range.
		setVariationRangeNumber(variableRanges - 1, 1);
	}
	else if (myItemInfo.iSubItem >= preRangeColumns && (myItemInfo.iSubItem - preRangeColumns) % 3 == 0)
	{
		// switch between [ and (
		if ( currentParameters.size( ) == 0 )
		{
			return;
		}
		UINT rangeNum = (myItemInfo.iSubItem - preRangeColumns) / 3;
		setRangeInclusivity( rangeNum, true, !currentParameters.front( ).ranges[rangeNum].leftInclusive, 
							 myItemInfo.iSubItem );
	}
	else if (myItemInfo.iSubItem >= preRangeColumns && (myItemInfo.iSubItem- preRangeColumns) % 3 == 1)
	{
		// switch between ] and )
		if ( currentParameters.size( ) == 0 )
		{
			return;
		}
		UINT rangeNum = (myItemInfo.iSubItem - preRangeColumns) / 3;
		setRangeInclusivity( rangeNum, false, !currentParameters.front( ).ranges[rangeNum].rightInclusive, 
							 myItemInfo.iSubItem );
	}
}


void ParameterSystem::setRangeInclusivity( UINT rangeNum, bool leftBorder, bool inclusive, UINT column )
{
	for ( auto& variable : currentParameters )
	{
		if ( leftBorder )
		{
			variable.ranges[rangeNum].leftInclusive = inclusive;
		}
		else
		{
			// it's the right border then.
			variable.ranges[rangeNum].rightInclusive = inclusive;
		}
	}

	if ( leftBorder )
	{
		LVCOLUMNA colInfo;
		ZeroMemory( &colInfo, sizeof( colInfo ) );
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
		LVCOLUMNA colInfo;
		colInfo.cchTextMax = 100;
		ZeroMemory( &colInfo, sizeof( colInfo ) );
		colInfo.mask = LVCF_TEXT;
		parametersListview.GetColumn( column, &colInfo );
		if ( inclusive )
		{
			colInfo.pszText = "]";
		}
		else
		{
			colInfo.pszText = ")";
		}
		parametersListview.SetColumn( column, &colInfo );
	}
}


void ParameterSystem::handleDraw(NMHDR* pNMHDR, LRESULT* pResult, rgbMap rgbs)
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
		if (item == currentParameters.size())
		{
			pLVCD->clrText = rgbs["Solarized Base1"];
			pLVCD->clrTextBk = rgbs["Solarized Base02"];
		}
		else
		{
			if (currentParameters[item].active)
			{
				pLVCD->clrTextBk = rgbs["Solarized Blue"];
			}
			else
			{
				pLVCD->clrTextBk = rgbs["Solarized Base04"];
			}
			if (currentParameters[item].overwritten)
			{
				pLVCD->clrText = rgbs["Solarized Red"];
			}
			else
			{
				pLVCD->clrText = rgbs["Solarized Base1"];
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
	POINT cursorPos;
	GetCursorPos(&cursorPos);
	parametersListview.ScreenToClient(&cursorPos);
	int subitem, itemIndicator;
	LVHITTESTINFO myItemInfo;
	memset(&myItemInfo, 0, sizeof(LVHITTESTINFO));
	myItemInfo.pt = cursorPos;
	parametersListview.SubItemHitTest(&myItemInfo);
	itemIndicator = myItemInfo.iItem;
	if (itemIndicator == -1)
	{
		return;
	}
	UINT borderCount = 0;
	for (auto rowCount : range(itemIndicator))
	{
		if (parametersListview.GetItemText(rowCount, 0) == "___" ||parametersListview.GetItemText(rowCount, 0) == "Symbol"  )
		{
			borderCount++;
		}
	}
	UINT scanDim = (borderCount + 1) / 2;
	UINT varNumber = itemIndicator - borderCount;
	subitem = myItemInfo.iSubItem;
	if (itemIndicator == -1)
	{
		// user didn't click in an item.
		return;
	}
	LVITEM listViewItem;
	memset(&listViewItem, 0, sizeof(listViewItem));
	listViewItem.mask = LVIF_TEXT; 
	listViewItem.cchTextMax = 256; 
	/// check if adding new variable
	listViewItem.iItem = itemIndicator;
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
		currentParameters.back().ranges.push_back({0,0,1, false, true});
		for (auto rangeInc : range(variableRanges))
		{
			currentParameters.back().ranges.push_back({ 0,0,0, false, true });
		}
		// make a new "new" row.
		listViewItem.iItem = itemIndicator;
		listViewItem.pszText = "___";
		listViewItem.iSubItem = 0;       // Put in first coluom
		parametersListview.InsertItem(&listViewItem);
		listViewItem.iSubItem = 1;
		if ( paramSysType == ParameterSysType::global )
		{			
			listViewItem.pszText = "0";
			parametersListview.SetItem( &listViewItem );
		}
		else
		{
			listViewItem.pszText = "Constant";
			parametersListview.SetItem( &listViewItem );
			listViewItem.pszText = "A";
			listViewItem.iSubItem = 2;
			parametersListview.SetItem( &listViewItem );
			listViewItem.iSubItem = 3;
			listViewItem.pszText = "0";
			parametersListview.SetItem( &listViewItem );
			listViewItem.iSubItem = 4;
			listViewItem.pszText = GLOBAL_PARAMETER_SCOPE;
			parametersListview.SetItem( &listViewItem );
			for ( int rangeInc = 0; rangeInc < variableRanges; rangeInc++ )
			{
				listViewItem.pszText = "---";
				for ( auto inc : range( 3 ) )
				{
					listViewItem.iSubItem = preRangeColumns + 3 * rangeInc + inc;
					parametersListview.SetItem( &listViewItem );
				}
			}
		}
	}
	listViewItem.iSubItem = subitem;
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
			std::transform( newName.begin(), newName.end(), newName.begin(), ::tolower );
			if (newName == "")
			{
				// probably canceled.
				break;
			}
			for (auto variable : currentParameters)
			{
				if (variable.name == newName)
				{
					thrower( "ERROR: A varaible with name " + newName + " already exists!" );
				}
			}
			if ( ttls->isValidTTLName( newName ) )
			{
				thrower( "ERROR: the name " + newName + " is already a ttl Name!" );
			}
			if ( aoSys->isValidDACName( newName ) )
			{
				thrower( "ERROR: the name " + newName + " is already a dac name!" );
			}
			currentParameters[varNumber].name = newName;
			listViewItem.pszText = &newName[0];
			parametersListview.SetItem(&listViewItem);
			break;
		}
		case 1:
		{
			if ( paramSysType == ParameterSysType::global )
			{
				/// global value
				std::string newValue;
				TextPromptDialog dialog(&newValue, "Please enter a value for the global variable "
										+ currentParameters[varNumber].name + ". Value will be formatted as a double.");
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
					currentParameters[varNumber].constantValue = std::stod( newValue );
				}
				catch (std::invalid_argument&)
				{
					thrower( "ERROR: the value entered, " + newValue + ", failed to convert to a double! "
							"Check for invalid characters." );
				}
				std::string temp(str(currentParameters[varNumber].constantValue));
				listViewItem.pszText = &temp[0];
				parametersListview.SetItem( &listViewItem );
				break;
			}
			else
			{
				/// constant or variable?
				// this is just a binary switch.
				if (currentParameters[varNumber].constant)
				{
					// switch to variable.
					currentParameters[varNumber].constant = false;
					listViewItem.pszText = "Variable";
					parametersListview.SetItem( &listViewItem );
					listViewItem.pszText = "---";
					listViewItem.iSubItem = 3;
					parametersListview.SetItem( &listViewItem );
					for (UINT rangeInc = 0; rangeInc < currentParameters[varNumber].ranges.size(); rangeInc++)
					{
						// set lower end of range
						std::string temp(str(currentParameters[varNumber].ranges[rangeInc].initialValue, 13));
						listViewItem.pszText = &temp[0];
						listViewItem.iSubItem = preRangeColumns + rangeInc * 3;
						parametersListview.SetItem( &listViewItem );
						// set higher end of range
						temp = str(currentParameters[varNumber].ranges[rangeInc].finalValue, 13);
						listViewItem.pszText = &temp[0];
						listViewItem.iSubItem = preRangeColumns+1 + rangeInc * 3;
						parametersListview.SetItem( &listViewItem );
						// set number of variations in this range
						temp = str( currentParameters[varNumber].ranges[rangeInc].variations);
						listViewItem.pszText = &temp[0];
						//currentParameters[varNumber].ranges[rangeInc].variations = currentVariations;
						// TODO: Handle this better. 
						UINT totalVariations = 0;
						for (auto range : currentParameters[varNumber].ranges)
						{
							totalVariations += range.variations;
						}
						if (totalVariations == 0)
						{
							errBox("WARNING: variable has zero variations in a certain range! "
								   "There needs to be at least one.");
						}
						listViewItem.iSubItem = preRangeColumns+2 + rangeInc * 3;
						parametersListview.SetItem( &listViewItem );
					}
				}
				else
				{
					/// switch to constant.
					listViewItem.pszText = "Constant";
					parametersListview.SetItem( &listViewItem );
					std::string tmpStr = str( currentParameters[varNumber].constantValue );
					listViewItem.pszText = &tmpStr[0];
					listViewItem.iSubItem = 3;
					parametersListview.SetItem( &listViewItem );
					currentParameters[varNumber].constant = true;
					for (int rangeInc = 0; rangeInc < variableRanges; rangeInc++)
					{
						// set the value to be dashes on the screen. no value for "Variable".
						listViewItem.pszText = "---";
						listViewItem.iSubItem = preRangeColumns + rangeInc * 3;
						parametersListview.SetItem( &listViewItem );
						listViewItem.iSubItem = preRangeColumns+1 + rangeInc * 3;
						parametersListview.SetItem( &listViewItem );
						listViewItem.iSubItem = preRangeColumns+2 + rangeInc * 3;
						parametersListview.SetItem( &listViewItem );
					}
				}
			}
			break;
		}
		case 2:
		{
			/// variable scan dimension
			if ( currentParameters[varNumber].constant )
			{
				break;
			}
			UINT maxDim = 0;
			for ( auto& variable : currentParameters )
			{
				if ( variable.name == currentParameters[varNumber].name || variable.constant )
				{
					// don't count the one being changed.
					continue;
				}
				if ( variable.scanDimension > maxDim )
				{
					maxDim = variable.scanDimension;
				}
			}
			currentParameters[varNumber].scanDimension++;
			// handle "wrapping" of the dimension.
			if ( currentParameters[varNumber].scanDimension > maxDim + 1 )
			{
				currentParameters[varNumber].scanDimension = 1;
			}
			reorderVariableDimensions( );
		}
		case 3:
		{
			/// constant value
			if ( !currentParameters[varNumber].constant )
			{
				// In this case the extra boxes are unresponsive.
				break;
			}
			std::string newValue;
			TextPromptDialog dialog( &newValue, "Please enter an initial value for the variable "
									 + currentParameters[varNumber].name + ". Value will be formatted as a double." );
			dialog.DoModal( );
			if ( newValue == "" )
			{
				// probably canceled.
				break;
			}
			try
			{
				currentParameters[varNumber].constantValue = std::stod( newValue );
			}
			catch ( std::invalid_argument& )
			{
				thrower( "ERROR: the value entered, " + newValue + ", failed to convert to a double! "
						 "Check for invalid characters." );
			}
			// update the listview
			std::string tempStr( str( currentParameters[varNumber].constantValue, 13 ) );
			listViewItem.pszText = &tempStr[0];
			parametersListview.SetItem( &listViewItem );
			break;
		}
		case 4:
		{
			/// scope
			std::string newScope;
			TextPromptDialog dialog( &newScope, "Please enter a the scope for the variable: \""
									 + currentParameters[varNumber].name + "\". You may enter a function name, "
									 "\"parent\", or \"global\"." );
			dialog.DoModal( );
			if ( newScope == "" )
			{
				// probably canceled.
				break;
			}
			// update the listview
			currentParameters[varNumber].parameterScope = newScope;
			std::string tempStr( str( currentParameters[varNumber].parameterScope, 0, false, true ) );
			listViewItem.pszText = &tempStr[0];
			parametersListview.SetItem( &listViewItem );
			break;
		}
		default:
		{
			// if it's a constant, you can only set the first range initial value.
			if ( currentParameters[varNumber].constant )
			{
				// then no final value to be set. In this case the extra boxes are unresponsive.
				break;
			}
			UINT rangeNum = (subitem - preRangeColumns) / 3;
			std::string newValue;
			TextPromptDialog dialog( &newValue, "Please enter a value for the variable "
									 + currentParameters[varNumber].name + "." );
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
					val = std::stod(newValue);
				}
				catch (std::invalid_argument&)
				{
					thrower("ERROR: the value entered, " + newValue + ", failed to convert to a double! "
							 "Check for invalid characters.");
				}
				if ( (subitem - preRangeColumns) % 3 == 0 )
				{
					currentParameters[varNumber].ranges[rangeNum].initialValue = val;
				}
				else
				{
					currentParameters[varNumber].ranges[rangeNum].finalValue = val;
				}
				std::string tempStr(str(val, 13));
				listViewItem.pszText = &tempStr[0];
				parametersListview.SetItem(&listViewItem);
				break;
			}
			else if((subitem - preRangeColumns) % 3 == 2)
			{
				// else there's something there.
				try
				{
					for (auto& variable : currentParameters)
					{
						if (!variable.constant)
						{
							// make sure all variables have the same number of variations.
							if ( variable.scanDimension != currentParameters[varNumber].scanDimension )
							{
								continue;
							}
							variable.ranges[rangeNum].variations = std::stoi(newValue);
							UINT totalVariations = 0;
							for (auto range : variable.ranges)
							{
								totalVariations += range.variations;
							}
							if (totalVariations == 0)
							{
								errBox("WARNING: there needs to be at least one variation for a variable.");
							}
						}
					}
				}
				catch (std::invalid_argument&)
				{
					thrower("ERROR: the value entered, " + newValue + ", failed to convert to a double! Check "
									"for invalid characters.");
				}
				std::string tempStr(str(currentParameters[varNumber].ranges[rangeNum].variations));
				listViewItem.pszText = &tempStr[0];
				for (auto varInc : range(currentParameters.size()))
				{
					if (!currentParameters[varInc].constant 
						 && (currentParameters[varInc].scanDimension == currentParameters[varNumber].scanDimension))
					{
						listViewItem.iItem = varInc;
						parametersListview.SetItem(&listViewItem);
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
		thrower("ERROR: " + variable.name + " is an invalid name; names cannot start with numbers.");
	}

	if (variable.name.find_first_of(" \t\r\n()*+/-%") != std::string::npos)
	{
		thrower("ERROR: Forbidden character in variable name! you cannot use spaces, tabs, newlines, or any of "
				"\"()*+/-%\" in a variable name.");
	}

	if (variable.name == "")
	{
		// then add empty variable slot
		LVITEM listViewDefaultItem;
		memset( &listViewDefaultItem, 0, sizeof( listViewDefaultItem ) );
		listViewDefaultItem.mask = LVIF_TEXT; 
		listViewDefaultItem.cchTextMax = 256; 
		listViewDefaultItem.pszText = "___";
		if (item == -1)
		{
			// at end.
			listViewDefaultItem.iItem = parametersListview.GetItemCount();
		}
		else
		{
			listViewDefaultItem.iItem = item;          // choose item  
		}
		listViewDefaultItem.iSubItem = 0;       // Put in first collumn
		parametersListview.InsertItem( &listViewDefaultItem );
		listViewDefaultItem.iSubItem = 1;
		parametersListview.SetItem( &listViewDefaultItem );
		parametersListview.RedrawWindow();
		return;
	}
	/// else...
	if (variable.constant == false)
	{
		thrower( "ERROR: attempted to add a non-constant to the global variable control!" );
	}
	for (auto currentVar : currentParameters)
	{
		if (currentVar.name == variable.name)
		{
			thrower( "ERROR: A variable with the name " + variable.name + " already exists!" );
		}
	}
	// add it to the internal structure that keeps track of variables
	currentParameters.push_back( variable );
	/// add the entry to the listview.
	LVITEM listViewItem;
	memset( &listViewItem, 0, sizeof( listViewItem ) );
	listViewItem.mask = LVIF_TEXT;   
	listViewItem.cchTextMax = 256; 
	listViewItem.iItem = item;
	std::string tempStr(str(variable.name));
	listViewItem.pszText = &tempStr[0];
	listViewItem.iSubItem = 0;
	parametersListview.InsertItem(&listViewItem);
	listViewItem.iSubItem = 1;
	tempStr = str(variable.ranges.front().initialValue, 13, true);
	listViewItem.pszText = &tempStr[0];
	parametersListview.SetItem(&listViewItem);
}


void ParameterSystem::addConfigParameter(parameterType variableToAdd, UINT item)
{
	// make name lower case.
	std::transform(variableToAdd.name.begin(), variableToAdd.name.end(), variableToAdd.name.begin(), ::tolower);
	if (isdigit(variableToAdd.name[0]))
	{
		thrower("ERROR: " + variableToAdd.name + " is an invalid name; names cannot start with numbers.");
	}
	// check for forbidden (math) characters
	if (variableToAdd.name.find_first_of(" \t\r\n()*+/-%") != std::string::npos)
	{
		thrower("ERROR: Forbidden character in variable name! you cannot use spaces, tabs, newlines, or any of "
				"\"()*+/-%\" in a variable name.");
	}
	if (variableToAdd.name == "" )
	{
		// then add empty variable slot
		LVITEM listViewDefaultItem;
		memset(&listViewDefaultItem, 0, sizeof(listViewDefaultItem));
		listViewDefaultItem.mask = LVIF_TEXT;   // Text Style
		listViewDefaultItem.cchTextMax = 256; // Max size of test
		listViewDefaultItem.pszText = "___";
		if (item == -1)
		{
			// at end.
			listViewDefaultItem.iItem = parametersListview.GetItemCount();
		}
		else
		{
			listViewDefaultItem.iItem = item;
		}
		listViewDefaultItem.iSubItem = 0;
		parametersListview.InsertItem(&listViewDefaultItem);
		for (int itemInc = 1; itemInc < 7; itemInc++) // Add SubItems in a loop
		{
			listViewDefaultItem.iSubItem = itemInc;
			parametersListview.SetItem(&listViewDefaultItem);
		}
		parametersListview.RedrawWindow();
		return;
	}

	/// else...
	for (auto currentVar : currentParameters)
	{
		if (currentVar.name == variableToAdd.name)
		{
			thrower("ERROR: A variable with the name " + variableToAdd.name + " already exists!");
		}
	}
	// add it to the internal structure that keeps track of variables
	currentParameters.push_back(variableToAdd);
	LVITEM listViewItem;
	memset(&listViewItem, 0, sizeof(listViewItem));
	listViewItem.mask = LVIF_TEXT;
	listViewItem.cchTextMax = 256;
	listViewItem.iItem = item;
	std::string tempStr(str(variableToAdd.name));
	listViewItem.pszText = &tempStr[0];
	listViewItem.iSubItem = 0;
	parametersListview.InsertItem(&listViewItem);
	listViewItem.iSubItem = 1;
	if (variableToAdd.constant)
	{
		listViewItem.pszText = "Constant";
		parametersListview.SetItem( &listViewItem );
		listViewItem.iSubItem = 3;
		std::string s = str( variableToAdd.constantValue );
		listViewItem.pszText = &s[0];
		parametersListview.SetItem( &listViewItem );
	}
	else
	{
		listViewItem.pszText = "Variable";
		parametersListview.SetItem( &listViewItem );
		listViewItem.iSubItem = 3;
		listViewItem.pszText = "---";
		parametersListview.SetItem( &listViewItem );
	}
	listViewItem.iSubItem = 2;
	std::string s( str( char('A' + variableToAdd.scanDimension-1) ) );
	listViewItem.pszText = &s[0];
	parametersListview.SetItem( &listViewItem );
	listViewItem.iSubItem = 4;
	listViewItem.pszText = (LPSTR)variableToAdd.parameterScope.c_str();
	parametersListview.SetItem( &listViewItem );

	// make sure there are enough currentRanges.
	UINT currentRanges = currentParameters.front( ).ranges.size( );
	//UINT columns = parametersListview.GetHeaderCtrl()->GetItemCount();
	//UINT currentRanges = (columns - preRangeColumns - 2) / 3;
	// not sure why this would happen, but was bug.
	if ( variableToAdd.ranges.size( ) < currentRanges )
	{
		variableToAdd.ranges.resize( currentRanges );
	}

	for (auto rangeAddInc : range(variableToAdd.ranges.size() - currentRanges))
	{
		// add a range.
		LV_COLUMN listViewDefaultCollumn;
		memset(&listViewDefaultCollumn, 0, sizeof(listViewDefaultCollumn));
		listViewDefaultCollumn.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
		// width between each coloum
		std::string tempStr(str(variableRanges + 1) + ":(");
		listViewDefaultCollumn.pszText = &tempStr[0];
		listViewDefaultCollumn.cx = 0x20;
		parametersListview.InsertColumn( preRangeColumns + 3 * variableRanges, &listViewDefaultCollumn);
		listViewDefaultCollumn.pszText = "]";
		parametersListview.InsertColumn( preRangeColumns + 1 + 3 * variableRanges, &listViewDefaultCollumn);
		listViewDefaultCollumn.pszText = "#";
		parametersListview.InsertColumn( preRangeColumns + 2 + 3 * variableRanges, &listViewDefaultCollumn);
		// edit all variables
		LVITEM listViewItem;
		memset(&listViewItem, 0, sizeof(listViewItem));
		listViewItem.mask = LVIF_TEXT;   
		listViewItem.cchTextMax = 256; 
		// make sure all variables have some text for the new columns.
		for (auto varInc : range(currentParameters.size()))
		{
			currentParameters[varInc].ranges.push_back({0,0,0, false, true});
			if (currentParameters[varInc].constant)
			{
				listViewItem.pszText = "---";
			}
			else
			{
				listViewItem.pszText = "0";
			}
			listViewItem.iItem = varInc;
			for ( auto inc : range( 3 ) )
			{
				listViewItem.iSubItem = preRangeColumns + 3 * variableRanges + inc;
				parametersListview.SetItem( &listViewItem );
			}
		}
		variableRanges++;
	}


	for (auto rangeInc : range(variableToAdd.ranges.size()))
	{
		if (!variableToAdd.constant)
		{
			// variable case.
			std::string tempStr(str(currentParameters[item].ranges[rangeInc].initialValue, 13, true));
			listViewItem.pszText = &tempStr[0];
			listViewItem.iSubItem = preRangeColumns + rangeInc * 3;

			parametersListview.SetItem(&listViewItem);
			tempStr = str(currentParameters[item].ranges[rangeInc].finalValue, 13, true);
			listViewItem.pszText = &tempStr[0];
			listViewItem.iSubItem = preRangeColumns + 1 + rangeInc * 3;

			parametersListview.SetItem(&listViewItem);
			tempStr = str(currentParameters[item].ranges[rangeInc].variations, 13, true);
			listViewItem.pszText = &tempStr[0];
			listViewItem.iSubItem = preRangeColumns + 2 + rangeInc * 3;
			parametersListview.SetItem(&listViewItem);
		}
		else
		{
			// constant case.
			listViewItem.pszText = "---";
			listViewItem.iSubItem = preRangeColumns + rangeInc * 3;
			parametersListview.SetItem( &listViewItem );
			listViewItem.iSubItem = preRangeColumns + 1 + rangeInc * 3;
			parametersListview.SetItem(&listViewItem);
			listViewItem.iSubItem = preRangeColumns + 2 + rangeInc * 3;
			parametersListview.SetItem(&listViewItem);
		}
	}
	parametersListview.RedrawWindow();
	updateVariationNumber( );
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


INT_PTR ParameterSystem::handleColorMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, 
											brushMap brushes)
{
	HDC hdcStatic = (HDC)wParam;
	if ( GetDlgCtrlID( (HWND)lParam ) == parametersHeader.GetDlgCtrlID())
	{
		SetTextColor(hdcStatic, RGB(218, 165, 32));
		SetBkColor(hdcStatic, RGB(30, 30, 30));
		return (LRESULT)brushes["Medium Grey"];
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


std::vector<parameterType> ParameterSystem::getConfigVariablesFromFile( std::string configFileName )
{
	std::ifstream f(configFileName);
	Version ver;
	ProfileSystem::getVersionFromFile( f, ver );
	std::vector<parameterType> configVariables;
	while ( f )
	{
		try
		{
			ProfileSystem::checkDelimiterLine( f, "VARIABLES" );
		}
		catch ( Error& )
		{
			continue;
		}
		configVariables = getVariablesFromFile( f, ver );

		ProfileSystem::checkDelimiterLine( f, "END_VARIABLES" );
		break;
	}
	return configVariables;
}


void ParameterSystem::generateKey( std::vector<std::vector<parameterType>>& variables,
								  bool randomizeVariablesOption )
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
	std::vector<std::vector<std::vector<int>>> variations( variables.size( ), 
														   std::vector<std::vector<int>>(maxDim));
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
						thrower( "ERROR: Not all variables seem to have the same number of ranges for their parameters!" );
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
					// avoid the case of zero as this just hasn't been set yet.
					if ( variationNum != 0 )
					{
						if ( variable.ranges[rangeInc].variations != variationNum )
						{
							thrower( "ERROR: not all ranges of variables have the same number of variations!" );
						}
					}
					variationNum = variable.ranges[rangeInc].variations;
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
				for ( auto range : variable.ranges )
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
				// calculate the parameters for the variation range
				double valueRange = (currRange.finalValue - currRange.initialValue);
				int spacings;
				if ( currRange.leftInclusive && currRange.rightInclusive )
				{
					spacings = variations[seqInc][variables[seqInc][varIndex].scanDimension - 1][rangeIndex] - 1;
				}
				else if ( !currRange.leftInclusive && !currRange.rightInclusive )
				{
					spacings = variations[seqInc][varDim][rangeIndex] + 1;
				}
				else
				{
					spacings = variations[seqInc][varDim][rangeIndex];
				}
				double initVal;
				if ( currRange.leftInclusive )
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
				for ( auto& indexInc : range( indexes.size( ) ) )
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

