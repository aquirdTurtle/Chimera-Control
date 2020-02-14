// created by Mark O. Brown
#include "stdafx.h"
#include "DataLogging/DataLogger.h"
#include "MachineOptimizer.h"
#include "ExperimentThread/ExperimentThreadInput.h"
#include "ExcessDialogs/TextPromptDialog.h"
#include <algorithm>
#include <memory>
#include "afxwin.h"
#include <boost/lexical_cast.hpp>


void MachineOptimizer::initialize ( POINT& pos, cToolTips& toolTips, CWnd* parent, int& id )
{
	header.sPos = { pos.x, pos.y, pos.x + 300, pos.y + 25 };
	header.Create ( "AUTO-OPTIMIZATION-CONTROL", NORM_HEADER_OPTIONS, header.sPos, parent, id++ );
	optimizeButton.sPos = { pos.x + 300, pos.y, pos.x + 480, pos.y += 25 };
	optimizeButton.Create ( "Optimize", NORM_PUSH_OPTIONS, optimizeButton.sPos, parent, IDC_MACHINE_OPTIMIZE );

	maxRoundsTxt.sPos = { pos.x, pos.y, pos.x + 120, pos.y + 25 };
	maxRoundsTxt.Create ( "Max Rounds:", NORM_STATIC_OPTIONS, maxRoundsTxt.sPos, parent, id++ );
	
	maxRoundsEdit.sPos = { pos.x + 120, pos.y, pos.x + 240, pos.y + 25 };
	maxRoundsEdit.Create ( NORM_EDIT_OPTIONS, maxRoundsEdit.sPos, parent, id++ );

	currRoundTxt.sPos = { pos.x + 240, pos.y, pos.x + 400, pos.y + 25 };
	currRoundTxt.Create ( "Current Round:", NORM_STATIC_OPTIONS, currRoundTxt.sPos, parent, id++ );

	currRoundDisp.sPos = { pos.x + 400, pos.y, pos.x + 480, pos.y += 25 };
	currRoundDisp.Create ( "", NORM_STATIC_OPTIONS, currRoundTxt.sPos, parent, id++ );

	//algorithmsHeader.sPos = { pos.x, pos.y, pos.x + 80, pos.y + 25 };
	//algorithmsHeader.Create ( "Algorithm:", NORM_STATIC_OPTIONS, algorithmsHeader.sPos, parent, id++ );
	//algorithmRadios;

	bestResultTxt.sPos = { pos.x, pos.y, pos.x + 180, pos.y + 25 };
	bestResultTxt.Create ( "Best Result:", NORM_STATIC_OPTIONS, bestResultTxt.sPos, parent, id++);
	bestResultVal.sPos = { pos.x + 180, pos.y, pos.x + 330, pos.y + 25 };
	bestResultVal.Create ( "---", NORM_STATIC_OPTIONS, bestResultVal.sPos, parent, id++ );
	bestResultErr.sPos = { pos.x + 330, pos.y, pos.x + 480, pos.y + 25 };
	bestResultErr.Create ( "---", NORM_STATIC_OPTIONS, bestResultErr.sPos, parent, id++ );

	pos.y += 25;

	optParamsHeader.sPos = { pos.x, pos.y, pos.x + 480, pos.y += 25 };
	optParamsHeader.Create ( "Optimization Parameters:", NORM_STATIC_OPTIONS, optParamsHeader.sPos, parent, id++ );

	optParamsListview.sPos = { pos.x, pos.y, pos.x + 480, pos.y + 100 };
	optParamsListview.Create ( NORM_LISTVIEW_OPTIONS, optParamsListview.sPos, parent, IDC_MACHINE_OPTIMIZE_LISTVIEW );
	optParamsListview.fontType = fontTypes::SmallFont;
	optParamsListview.SetBkColor ( RGB ( 15, 15, 15 ) );
	optParamsListview.SetTextBkColor ( RGB ( 15, 15, 15 ) );
	optParamsListview.SetTextColor ( RGB ( 150, 150, 150 ) );
	optParamsListview.InsertColumn ( 0, "Name", 100 );
	optParamsListview.InsertColumn ( 1, "Current-Val.", 100 );
	optParamsListview.InsertColumn ( 2, "[", 50 );
	optParamsListview.InsertColumn ( 3, "]", 50 );
	optParamsListview.InsertColumn ( 4, "Best-Val.", 100 );
	optParamsListview.InsertColumn (5, "Incr.", 50);
	optParamsListview.insertBlankRow ( );
}


void MachineOptimizer::updateCurrRoundDisplay ( std::string round )
{
	currRoundDisp.SetWindowText ( round.c_str ( ) );
}


void MachineOptimizer::updateBestResult ( std::string val, std::string err )
{
	bestResultVal.SetWindowText ( val.c_str ( ) );
	bestResultErr.SetWindowText ( err.c_str ( ) );
}

void MachineOptimizer::rearrange ( UINT width, UINT height, fontMap fonts )
{
	header.rearrange ( width, height, fonts );
	optimizeButton.rearrange ( width, height, fonts );
	//algorithmsHeader.rearrange ( width, height, fonts );
	maxRoundsTxt.rearrange ( width, height, fonts );
	maxRoundsEdit.rearrange ( width, height, fonts );
	currRoundTxt.rearrange ( width, height, fonts );
	currRoundDisp.rearrange ( width, height, fonts );
	
	bestResultTxt.rearrange ( width, height, fonts );
	bestResultVal.rearrange ( width, height, fonts );
	bestResultErr.rearrange ( width, height, fonts );
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
// TODO: FIX THIS SINCE MOVING PARAMS OUT OF INPUT
/*
void MachineOptimizer::updateParams ( AllExperimentInput& input, dataPoint resultValue, DataLogger& logger )
{
	if ( optCount == 0 )
	{
		optStatus.optimizationHistory.clear ( );
		// it's the first one.
		isOptimizing = true;
		// initialize the current values with the constant values from the main parameter controls
		
		for ( auto& optParam : optParams )
		{
			for ( auto& expParam : input.masterInput->parameters[0] )
			{
				if ( optParam->name == expParam.name )
				{
					optParam->currentValue = expParam.constantValue;
				}
			}
		}
		logger.initOptimizationFile ( );
		std::string initString = "Parameters: ";
		for ( auto& param : optParams )
		{
			initString += param->name + " ";
		}
		initString += "\n";
		logger.updateOptimizationFile ( initString );
	}
	else
	{
		// update the history with the new values.
		optStatus.optimizationHistory.resize ( optStatus.optimizationHistory.size ( ) + 1 );
		for ( auto param : optParams )
		{
			optStatus.optimizationHistory.back ( ).paramValues.push_back ( param->currentValue );
		}
		optStatus.optimizationHistory.back ( ).value = resultValue.y;
		optStatus.optimizationHistory.back ( ).yerr = resultValue.err;
		double bestVal = -DBL_MAX, bestErr;
		for ( auto& point : optStatus.optimizationHistory )
		{
			if ( point.value > bestVal )
			{
				bestVal = point.value;
				bestErr = point.yerr;
			}
		}
		std::string pointString = "Point_Number " + str(optCount) + " ";
		for ( auto& param : optParams )
		{
			pointString += str(param->currentValue) + " ";
		}
		pointString += str ( resultValue.y ) + " " + str ( resultValue.err );
		pointString += "\n";
		logger.updateOptimizationFile ( pointString );
		updateBestResult ( str ( bestVal ), str ( bestErr ) );
	}
	switch ( currentSettings.alg )
	{
		case optimizationAlgorithm::which::HillClimbing:
		{
			hillClimbingUpdate ( input, resultValue, logger );
		}
	}
	// update variable values...
	optCount++;  
	updateCurrentValueDisplays ( );
	updateBestValueDisplays ( );
}
*/

void MachineOptimizer::updateCurrentValueDisplays ( )
{
	UINT count = 0;
	for ( auto& param : optParams )
	{
		optParamsListview.SetItem (str(param->currentValue), count, 1 );
		count++;
	}	
}


void MachineOptimizer::updateBestValueDisplays ( )
{
	UINT count = 0;
	for ( auto& param : optParams )
	{
		optParamsListview.SetItem ( str ( param->bestResult.x), count, 4 );
		count++;
	}
}


bool MachineOptimizer::isInMiddleOfOptimizing ( )
{
	return isOptimizing;
}


void MachineOptimizer::hillClimbingUpdate ( AllExperimentInput& input, dataPoint resultValue, DataLogger& logger )
{
	// handle first value case, should only happen once in entire experiment.
	auto& param = optStatus.currParam;
	if ( param == NULL )
	{
		updateCurrRoundDisplay ( str ( roundCount ) );
		// initial 
		param = optParams.front ( ); 
		param->index = 0;
		param->resultHist.clear ( ); 
		logger.updateOptimizationFile ( "Variable: " + param->name + "\n" );
	}
	resultValue.x = param->currentValue;
	if ( param->resultHist.size ( ) == 0 )
	{
		optStatus.scanDir = 1;
	}
	else
	{
		/// get the best value so far
		if ( param->resultHist.size ( ) == 0 )
		{
			param->bestResult = resultValue;
		}
		else 
		{
			UINT bestLoc = 0, resultCount = 0;
			param->bestResult = param->resultHist.front ( );
			for ( auto res : param->resultHist )
			{
				if ( res.y > param->bestResult.y )
				{
					bestLoc = resultCount;
					param->bestResult = res;
				}
				resultCount = 0;
			}
		}
		param->currentValue = param->currentValue + double(optStatus.scanDir) * param->increment;
		/// determine way to update
		if ( (resultValue.y + 2 * resultValue.err > param->bestResult.y - 2 * param->bestResult.err 
			   || param->resultHist.size ( ) <= 1) && ! (param->currentValue > param->upperLim || param->currentValue < param->lowerLim ) )
		{
			// the most recent result was the best (or close to best) number OR not enough values to determine trend, and still within 
			// range. Keep moving in this direction.
		}
		else
		{
			if ( optStatus.scanDir == -1 || param->resultHist.size ( ) > 2 || param->currentValue < param->lowerLim )
			{
				// finished with this variable for now!
				auto index = param->index;
				if ( index + 1 >= optParams.size ( ) )
				{
					// finished round!
					if ( roundCount >= getMaxRoundNum ( ) )
					{
						logger.finOptimizationFile ( );
						onFinOpt ( );
						thrower ( "Finished Optimization!" );
					}
					else
					{
						logger.updateOptimizationFile ( "Finished round " + str ( roundCount ) + "\n" );
						roundCount++;
						updateCurrRoundDisplay ( str ( roundCount ) );
						/// go back to first variable.
						// order is important here.
						param->currentValue = param->bestResult.x;
						auto tempResult = param->bestResult;
						param.reset ( );
						index = 0; 
						param = optParams[ index ];
						param->index = index;
						param->resultHist.clear ( );
						// give it the first data point as the end point from the previous round.
						param->resultHist.push_back ( { param->currentValue, tempResult.y, tempResult.err } );
						optStatus.scanDir = 1;
						param->currentValue = param->currentValue + double ( optStatus.scanDir ) * param->increment;
						logger.updateOptimizationFile ( "Variable: " + param->name + "\n" );
					}
				}
				else
				{
					/// next variable
					// order is important here.
					param->currentValue = param->bestResult.x;
					auto tempResult = param->bestResult;
					param.reset ( );
					param = optParams[ index + 1 ];
					param->index = index + 1;
					param->resultHist.clear ( );
					// give it the first data point as the end point from the previous round.
					param->resultHist.push_back ( { param->currentValue, tempResult.y, tempResult.err } );
					optStatus.scanDir = 1;
					param->currentValue = param->currentValue + double ( optStatus.scanDir ) * param->increment;
					logger.updateOptimizationFile ( "Variable: " + param->name + "\n" );
				}
			}
			else
			{
				// first data point was in wrong direction. Go back to beginning and change direction.
				optStatus.scanDir = -1;
				param->currentValue = param->resultHist.front ( ).x + double ( optStatus.scanDir ) * param->increment;
				logger.updateOptimizationFile ( "Switched_Direction\n");
			}
		}
	}
	optStatus.currParam->resultHist.push_back ( resultValue );
	for ( auto& param : optParams )
	{
		/*for ( auto& expParam : input.masterInput->parameters[0] )	
		{
			if ( expParam.name == param->name )
			{
				// change value
				expParam.constantValue = param->currentValue;
			}
		}*/
	}
}


void MachineOptimizer::onFinOpt ( )
{
	isOptimizing = false;
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
					thrower ( "An optimization parameter with name " + newName + " already exists!" );
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
				param->lowerLim = boost::lexical_cast<double> ( lowerLim );
			}
			catch ( boost::bad_lexical_cast& )
			{
				throwNested ( "Failed to convert lower limit text to a double!" );
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
				param->upperLim = boost::lexical_cast<double> ( upperLim );
			}
			catch ( boost::bad_lexical_cast& )
			{
				throwNested ( "Failed to convert lower limit text to a double!" );
			}
			optParamsListview.SetItem ( str ( param->upperLim ), itemIndicator, subitem );
			break;
		}
		case 4:
		{
			// clicked best value.
			break;
		}
		case 5:
		{
			// clicked Increment.
			std::string inc_str;
			TextPromptDialog dialog ( &inc_str, "Please enter a value for the increment for this "
									  "optimization parameter." );
			dialog.DoModal ( );
			try
			{
				param->increment = boost::lexical_cast<double> ( inc_str );
			}
			catch ( boost::bad_lexical_cast& )
			{
				throwNested ( "Failed to convert lower limit text to a double!" );
			}
			optParamsListview.SetItem ( str ( param->increment ), itemIndicator, subitem );
			break;
		}
	}
}


void MachineOptimizer::reset ( )
{
	optStatus.currParam.reset ( );
	optStatus.currParam = NULL;
	optCount = 0;
	roundCount = 1;
	updateCurrRoundDisplay ( "---" );
}


void MachineOptimizer::verifyOptInput ( AllExperimentInput& input )
{
	if ( !input.includesAndorRun )
	{
		thrower ( "Cannot optimize parameters if not taking camera data!" );
	}
	UINT count = 0;
	for ( auto& info : input.masterInput->plotterInput->plotInfo )
	{
		count += info.isActive;
	}
	if ( count != 1 )
	{
		thrower ( "for machine optimization, you need one and only one active data analyzer "
				  "(e.g. loading, survival)" );
	}
	for ( auto param : optParams )
	{
		bool found = false;
		/*
		for ( auto& expParam : input.masterInput->parameters[0] )
		{
			if ( expParam.name == param->name )
			{
				if ( !expParam.constant )
				{
					thrower ( "optimization parameter matched an experiment parameter, but the experiment "
							  "parameter is varying! Optimization parameters must be constants in the main experiment." );
				}
				found = true;
			}
		}
		*/
		if ( !found )
		{
			thrower ( "A Parameter in the optimization control didn't match any constants in the actual "
					  "experiment!" );
		}
	}
	updateBestResult ( "---", "---" );
}

UINT MachineOptimizer::getMaxRoundNum ( )
{
	UINT res;
	CString txt;
	maxRoundsEdit.GetWindowText ( txt);
	try
	{
		res = boost::lexical_cast<UINT>(str ( txt ));
	}
	catch (boost::bad_lexical_cast& )
	{
		throwNested ( "failed to convert max optimization count to an unsigned integer!" );
	}
	return res;		
}
