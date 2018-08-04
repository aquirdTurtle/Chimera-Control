#include "stdafx.h"
#include "MachineOptimizer.h"
#include "MasterThreadInput.h"
#include "TextPromptDialog.h"
#include <algorithm>
#include <memory>
#include "afxwin.h"


void MachineOptimizer::initialize ( POINT& pos, cToolTips& toolTips, CWnd* parent, int& id )
{
	header.sPos = { pos.x, pos.y, pos.x + 480, pos.y += 25 };
	header.Create ( "AUTO-OPTIMIZATION-CONTROL", NORM_HEADER_OPTIONS, header.sPos, parent, id++ );

	optimizeButton.sPos = { pos.x, pos.y, pos.x + 480, pos.y += 25 };
	optimizeButton.Create ( "Optimize", NORM_PUSH_OPTIONS, optimizeButton.sPos, parent, IDC_MACHINE_OPTIMIZE );

	algorithmsHeader.sPos = { pos.x, pos.y, pos.x + 80, pos.y + 25 };
	algorithmsHeader.Create ( "Algorithm:", NORM_STATIC_OPTIONS, algorithmsHeader.sPos, parent, id++ );
	
	//algorithmRadios;

	pos.y += 25;

	optParamsHeader.sPos = { pos.x, pos.y, pos.x + 480, pos.y += 25 };
	optParamsHeader.Create ( "Optimization Parameters:", NORM_STATIC_OPTIONS, optParamsHeader.sPos, parent, id++ );

	optParamsListview.sPos = { pos.x, pos.y, pos.x + 480, pos.y + 100 };
	optParamsListview.Create ( NORM_LISTVIEW_OPTIONS, optParamsListview.sPos, parent, IDC_MACHINE_OPTIMIZE_LISTVIEW );
	optParamsListview.fontType = fontTypes::SmallFont;
	optParamsListview.SetBkColor ( RGB ( 15, 15, 15 ) );
	optParamsListview.SetTextBkColor ( RGB ( 15, 15, 15 ) );
	optParamsListview.SetTextColor ( RGB ( 150, 150, 150 ) );
	optParamsListview.InsertColumn ( 0, "Name", 200 );
	optParamsListview.InsertColumn ( 1, "Current-Value", 200 );
	optParamsListview.InsertColumn ( 2, "[", 100 );
	optParamsListview.InsertColumn ( 3, "]", 100 );
	optParamsListview.insertBlankRow ( );
}


void MachineOptimizer::rearrange ( UINT width, UINT height, fontMap fonts )
{
	header.rearrange ( width, height, fonts );
	optimizeButton.rearrange ( width, height, fonts );
	algorithmsHeader.rearrange ( width, height, fonts );
	optParamsListview.rearrange ( width, height, fonts );
	optParamsHeader.rearrange ( width, height, fonts );
}


std::vector<std::shared_ptr<optParamSettings>> MachineOptimizer::getOptParams ( )
{
	return optParams;
}


/*
	resultValue: the value of the metric that was measured in the last round.
 */
void MachineOptimizer::updateParams ( ExperimentInput input, dataPoint resultValue )
{
	if ( optCount == 0 )
	{
		paramHistory.clear ( );
		measuredResultHistory.clear ( );
		// it's the first one.
		isOptimizing = true;
		// initialize the current values with the constant values from the main parameter controls
		for ( auto& optParam : optParams )
		{
			for ( auto& expParam : input.masterInput->variables[0] )
			{
				if ( optParam->name == expParam.name )
				{
					optParam->currentValue = expParam.constantValue;
				}
			}
		}
	}
	switch ( currentSettings.alg )
	{
		case optimizationAlgorithm::which::HillClimbing:
		{
			hillClimbingUpdate ( input, resultValue );
		}
	}
	// update the history with the new values.
	paramHistory.resize ( paramHistory.size ( ) +1 );
	for ( auto param : optParams )
	{
		paramHistory.back ( ).push_back ( param->currentValue );
	}
	// update variable values...
	optCount++;  
}


void MachineOptimizer::hillClimbingUpdate ( ExperimentInput input, dataPoint resultValue )
{
	// handle first value case, should only happen once in entire experiment.
	double newVal;
	auto& param = optStatus.currParam;
	if ( param == NULL )
	{
		// initial 
		param = optParams.front ( ); 
		param->index = 0;
		param->valueHist.clear ( ); 
		param->resultHist.clear ( ); 
	}
	if ( param->valueHist.size ( ) == 0 )
	{
		optStatus.scanDir = 1;
	}
	else
	{
		/// get the best value so far
		dataPoint bestResult;
		double bestSetting;
		if ( param->resultHist.size ( ) == 0 )
		{
			bestResult = resultValue;
			bestSetting = param->currentValue;
		}
		else
		{
			UINT bestLoc = 0, resultCount = 0;
			bestResult = param->resultHist.front ( );
			for ( auto res : param->resultHist )
			{
				if ( res.y > bestResult.y )
				{
					bestLoc = resultCount;
					bestResult = res;
				}
				resultCount = 0;
			}
			bestSetting = param->valueHist[ bestLoc ];
		}
		/// determine way to update
		if ( resultValue.y - resultValue.err > bestResult.y + bestResult.err || param->valueHist.size ( ) <= 1 )
		{
			// the most recent result was the best number OR not enough values to determine trend. 
			// keep moving in this direction.
		}
		else
		{
			if ( optStatus.scanDir == -1 || param->valueHist.size ( ) > 2 )
			{
				// finished with this variable for now!
				auto index = param->index;
				if ( index + 1 >= optParams.size ( ) )
				{
					/// finished with round... TODO
					thrower ( "BREAK!" );
				}
				else
				{
					/// next variable
					// order is important here.
					param->currentValue = bestSetting;
					param.reset ( );
					param = optParams[ index + 1 ];
					param->index = index + 1;
					param->valueHist.clear ( );
					param->resultHist.clear ( );
					// give it the first data point as the end point from the previous round.
					param->resultHist.push_back ( bestResult );
					param->valueHist.push_back ( param->currentValue );
					optStatus.scanDir = 1;
				}
			}
			else
			{
				// first data point was in wrong direction.
				optStatus.scanDir = -1;
				param->currentValue = param->valueHist.front ( );
			}
		}
		param->currentValue = param->currentValue + optStatus.scanDir * currentSettings.gain * param->limitSizeRange ( );
	}
	optStatus.currParam->valueHist.push_back ( param->currentValue);
	optStatus.currParam->resultHist.push_back ( resultValue );
	for ( auto& param : optParams )
	{
		for ( auto& expParam : input.masterInput->variables[0] )
		{
			if ( expParam.name == param->name )
			{
				// change value
				expParam.constantValue = param->currentValue;
			}
		}
	}
}


void MachineOptimizer::deleteParam ( )
{
	/// get the item and subitem
	POINT cursorPos;
	GetCursorPos ( &cursorPos );
	optParamsListview.ScreenToClient ( &cursorPos );
	int subitemIndicator = optParamsListview.HitTest ( cursorPos );
	LVHITTESTINFO myItemInfo;
	memset ( &myItemInfo, 0, sizeof ( LVHITTESTINFO ) );
	myItemInfo.pt = cursorPos;
	int itemIndicator = optParamsListview.SubItemHitTest ( &myItemInfo );
	if ( itemIndicator == -1 || itemIndicator == optParams.size ( ) )
	{
		// user didn't click in a deletable item.
		return;
	}
	int answer;
	if ( UINT ( itemIndicator ) < optParams.size ( ) )
	{
		answer = promptBox ( "Delete variable " + optParams[ itemIndicator ]->name + "?", MB_YESNO );
		if ( answer == IDYES )
		{
			optParamsListview.DeleteItem ( itemIndicator );
			optParams.erase ( optParams.begin ( ) + itemIndicator );
		}

	}
	else if ( UINT ( itemIndicator ) > optParams.size ( ) )
	{
		answer = promptBox ( "You appear to have found a bug with the listview control... there are too many lines "
							 "in this control. Clear this line?", MB_YESNO );
		if ( answer == IDYES )
		{
			optParamsListview.DeleteItem ( itemIndicator );
		}
	}
}


void MachineOptimizer::handleListViewClick ( )
{
	LVHITTESTINFO myItemInfo = { 0 };
	GetCursorPos ( &myItemInfo.pt );
	optParamsListview.ScreenToClient ( &myItemInfo.pt );
	optParamsListview.SubItemHitTest ( &myItemInfo );
	int subitem, itemIndicator;
	itemIndicator = myItemInfo.iItem;
	if ( itemIndicator == -1 )
	{
		return;
	}
	CString text = optParamsListview.GetItemText ( itemIndicator, 0 );
	if ( text == "___" )
	{
		// add a variable
		optParams.resize ( optParams.size ( ) + 1, std::make_shared<optParamSettings>());
		optParams.back ( )->name = "-";
		// make a new "new" row.
		optParamsListview.InsertItem ( "___", itemIndicator, 0 );
		optParamsListview.SetItem ( "-", itemIndicator, 1 );
	}
	auto& param = optParams[itemIndicator];
	subitem = myItemInfo.iSubItem;
	/// Handle different subitem clicks
	switch ( subitem )
	{
		case 0:
		{
			/// person name
			std::string newName;
			TextPromptDialog dialog ( &newName, "Please enter a name for the Optimization Parameter. This should match"
									  " the name of a parameter in either the global or config parameter systems." );
			dialog.DoModal ( );
			// make name lower case
			std::transform ( newName.begin ( ), newName.end ( ), newName.begin ( ), ::tolower );
			if ( newName == "" )
			{
				// probably canceled.
				break;
			}
			for ( auto variable : optParams )
			{
				if ( variable->name == newName )
				{
					thrower ( "ERROR: An optimization parameter with name " + newName + " already exists!" );
				}
			}
			param->name = newName;
			optParamsListview.SetItem ( newName, itemIndicator, subitem );
			break;
		}
		case 1:
		{
			// this box is not responsive.
			break;
		}
		case 2:
		{
			// clicked left border.
			std::string lowerLim;
			TextPromptDialog dialog ( &lowerLim, "Please enter a value for the lower search limit for this "
									  "optimization parameter." );
			dialog.DoModal ( );
			try
			{
				param->lowerLim = std::stod ( lowerLim );
			}
			catch ( std::invalid_argument )
			{
				thrower ( "ERROR: Failed to convert lower limit text to a double!" );
			}
			optParamsListview.SetItem ( str ( param->lowerLim ), itemIndicator, subitem );
			break;
		}
		case 3:
		{
			// clicked right border.
			std::string upperLim;
			TextPromptDialog dialog ( &upperLim, "Please enter a value for the upper search limit for this "
									  "optimization parameter." );
			dialog.DoModal ( );
			try
			{
				param->upperLim = std::stod ( upperLim );
			}
			catch ( std::invalid_argument )
			{
				thrower ( "ERROR: Failed to convert lower limit text to a double!" );
			}
			optParamsListview.SetItem ( str ( param->upperLim ), itemIndicator, subitem );
			break;
		}
	}
}


void MachineOptimizer::reset ( )
{
	optStatus.currParam.reset ( );
	optStatus.currParam = NULL;
	optCount = 0;
}


void MachineOptimizer::verifyOptInput ( ExperimentInput input )
{
	if ( !input.includesCameraRun )
	{
		thrower ( "ERROR: Cannot optimize parameters if not taking camera data!" );
	}
	UINT count = 0;
	for ( auto& info : input.plotterInput->plotInfo )
	{
		count += info.isActive;
	}
	if ( count != 1 )
	{
		thrower ( "ERROR: for machine optimization, you need one and only one active data analyzer "
				  "(e.g. loading, survival)" );
	}
	for ( auto param : optParams )
	{
		bool found = false;
		for ( auto& expParam : input.masterInput->variables[0] )
		{
			if ( expParam.name == param->name )
			{
				if ( !expParam.constant )
				{
					thrower ( "ERROR: optimization parameter matched an experiment parameter, but the experiment "
							  "parameter is varying! Optimization parameters must be constants in the main experiment." );
				}
				found = true;
			}
		}
		if ( !found )
		{
			thrower ( "ERROR: A Parameter in the optimization control didn't match any constants in the actual "
					  "experiment!" );
		}
	}
}


