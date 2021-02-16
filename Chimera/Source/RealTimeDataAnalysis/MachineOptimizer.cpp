// created by Mark O. Brown
#include "stdafx.h"
#include "DataLogging/DataLogger.h"
#include "MachineOptimizer.h"
#include "ExperimentThread/ExperimentThreadInput.h"
#include <algorithm>
#include <memory>
#include <boost/lexical_cast.hpp>
#include <qheaderview.h>
#include <qmenu.h>
#include <PrimaryWindows/QtMainWindow.h>

void MachineOptimizer::handleContextMenu (const QPoint& pos){
	QTableWidgetItem* item = optParamsListview->itemAt (pos);
	QMenu menu;
	menu.setStyleSheet (chimeraStyleSheets::stdStyleSheet ());
	auto* deleteAction = new QAction ("Delete This Item", optParamsListview);
	optParamsListview->connect (deleteAction, &QAction::triggered, [this, item]() {
		optParams.erase (optParams.begin () + item->row ());
		});
	auto* newPerson = new QAction ("New Item", optParamsListview);
	optParamsListview->connect (newPerson, &QAction::triggered,
		[this]() {
			optParams.push_back (std::make_shared<optParamSettings> ());
		});
	if (item) { menu.addAction (deleteAction); }
	menu.addAction (newPerson);
	menu.exec (optParamsListview->mapToGlobal (pos));
}

void MachineOptimizer::initialize ( QPoint& pos, IChimeraQtWindow* parent ){
	auto& px = pos.rx (), & py = pos.rx ();
	header = new QLabel ("AUTO-OPTIMIZATION-CONTROL", parent);
	header->setGeometry (px, py, 300, 25);

	optimizeButton = new QPushButton ("Optimize", parent);
	optimizeButton->setGeometry (px + 300, py, 180, 25);
	parent->connect (optimizeButton, &QPushButton::released, 
		[this, parent]() {
			try	{
				auto res = QMessageBox::question (parent, "Start Opt.",
					"Start Machine optimization using the currently selected configuration parameters?");
				if (res == QMessageBox::No) {
					return;
				}
				reset ();
				//commonFunctions::handleCommonMessage (ID_MACHINE_OPTIMIZATION, parent);
			}
			catch (ChimeraError& err) {
				// catch any extra errors that handleCommonMessage doesn't explicitly handle.
				parent->reportErr(err.qtrace ());
			}
		});

	maxRoundsTxt = new QLabel ("Max Rounds:", parent);
	maxRoundsTxt->setGeometry (px, py += 25, 120, 25);

	maxRoundsEdit = new QLineEdit ("", parent);
	maxRoundsEdit->setGeometry (px + 120, py, 120, 25);

	currRoundTxt = new QLabel ("Current Round:", parent);
	currRoundTxt->setGeometry (px+240, py, 160, 25);

	currRoundDisp = new QLabel ("", parent);
	currRoundDisp->setGeometry (px + 400, py, 80, 25);

	bestResultTxt = new QLabel ("Best Result:", parent);
	bestResultTxt->setGeometry (px, py+=25, 180, 25);
	bestResultVal = new QLabel ("---", parent);
	bestResultVal->setGeometry (px + 180, py, 150, 25);
	bestResultErr = new QLabel ("---", parent);
	bestResultErr->setGeometry (px + 330, py, 150, 25);
	optParamsHeader = new QLabel ("Optimization Parameters:", parent);
	optParamsHeader->setGeometry (px, py+=25, 480, 25);
	optParamsListview = new QTableWidget (parent);
	optParamsListview->setGeometry (px, py += 25, 480, 100);
	py += 160;
	optParamsListview->horizontalHeader ()->setFixedHeight (30);
	optParamsListview->verticalHeader ()->setFixedWidth (40);
	optParamsListview->verticalHeader ()->setDefaultSectionSize (22);

	optParamsListview->setContextMenuPolicy (Qt::CustomContextMenu);
	parent->connect (optParamsListview, &QTableWidget::customContextMenuRequested,
		[this](const QPoint& pos) {handleContextMenu (pos); });
	optParamsListview->setShowGrid (true);

	QStringList labels;
	labels << "Name" << "Current-Val." << "[" << "]" << "Best-Val" << "Incr.";
	optParamsListview->setColumnCount (labels.size ());
	optParamsListview->setHorizontalHeaderLabels (labels);
}


void MachineOptimizer::updateCurrRoundDisplay ( std::string round )
{
	currRoundDisp->setText ( round.c_str ( ) );
}


void MachineOptimizer::updateBestResult ( std::string val, std::string err )
{
	bestResultVal->setText ( val.c_str ( ) );
	bestResultErr->setText ( err.c_str ( ) );
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
	unsigned count = 0;
	for ( auto& param : optParams )
	{
		optParamsListview->setItem (count, 1, new QTableWidgetItem( cstr(param->currentValue) ));
		count++;
	}	
}


void MachineOptimizer::updateBestValueDisplays ( )
{
	unsigned count = 0;
	for ( auto& param : optParams )
	{
		optParamsListview->setItem ( count, 4, new QTableWidgetItem(cstr ( param->bestResult.x)) );
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
	if ( param == nullptr )
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
			unsigned bestLoc = 0, resultCount = 0;
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


void MachineOptimizer::deleteParam ( ){
	/*
	/// get the item and subitem
	QPoint cursorPos;
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
	if ( unsigned ( itemIndicator ) < optParams.size ( ) )
	{
		answer = promptBox ( "Delete variable " + optParams[ itemIndicator ]->name + "?", MB_YESNO );
		if ( answer == IDYES )
		{
			optParamsListview.DeleteItem ( itemIndicator );
			optParams.erase ( optParams.begin ( ) + itemIndicator );
		}

	}
	else if ( unsigned ( itemIndicator ) > optParams.size ( ) )
	{
		answer = promptBox ( "You appear to have found a bug with the listview control... there are too many lines "
							 "in this control. Clear this line?", MB_YESNO );
		if ( answer == IDYES )
		{
			optParamsListview.DeleteItem ( itemIndicator );
		}
	}
	*/
}


void MachineOptimizer::handleListViewClick ( )
{
	/*
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
									  " the name of a parameter in either the global or config parameter systems.", 
									  param->name );
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
									  "optimization parameter.", str(param->lowerLim));
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
									  "optimization parameter.", str(param->upperLim ));
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
									  "optimization parameter.", str(param->increment ));
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
	*/
}


void MachineOptimizer::reset ( )
{
	optStatus.currParam.reset ( );
	optStatus.currParam = nullptr;
	optCount = 0;
	roundCount = 1;
	updateCurrRoundDisplay ( "---" );
}


void MachineOptimizer::verifyOptInput ( AllExperimentInput& input )
{

	unsigned count = 0;
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
    
unsigned MachineOptimizer::getMaxRoundNum ( )
{
	unsigned res;
	try
	{
		res = boost::lexical_cast<unsigned>(str(maxRoundsEdit->text()));
	}
	catch (boost::bad_lexical_cast& )
	{
		throwNested ( "failed to convert max optimization count to an unsigned integer!" );
	}
	return res;		
}
