#include "stdafx.h"
#include "VariableSystem.h"
#include "fonts.h"
#include "textPromptDialogProcedure.h"
#include <iomanip>
#include <sstream>
#include "Script.h"
#include "constants.h"
#include <unordered_map>
#include "afxcmn.h"
#include "TTL_System.h"

#include "MasterWindow.h"

void VariableSystem::setVariationRangeNumber(int num)
{
	int currentRanges = (this->variablesListview.GetHeaderCtrl()->GetItemCount() - 3) / 3;
	if (this->variableRangeSets != currentRanges)
	{
		errBox("ERROR: somehow, the number of ranges the VariableSystem object thinks there are and the actual number "
				"are off! The numbes are " + std::to_string(this->variableRangeSets) + " and " 
				+ std::to_string(currentRanges) + " respectively. The program will attempt to fix this, but " 
				"data may be lost.");
		this->variableRangeSets = currentRanges;
		for (int variableInc = 0; variableInc < this->currentVariables.size(); variableInc++)
		{
			this->currentVariables[variableInc].ranges.resize(currentRanges);
		}
	}
	if (currentRanges < num)
	{
		while (currentRanges < num)
		{
			/// add a range.
			LV_COLUMN listViewDefaultCollumn;
			// Zero Members
			memset(&listViewDefaultCollumn, 0, sizeof(listViewDefaultCollumn));
			// Type of mask
			listViewDefaultCollumn.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
			// width between each coloum
			CString varnum((std::to_string(this->variableRangeSets + 1) + ":(").c_str());
			listViewDefaultCollumn.pszText = varnum.GetBuffer();
			listViewDefaultCollumn.cx = 0x20;
			this->variablesListview.InsertColumn(3 + 3 * currentRanges, &listViewDefaultCollumn);
			listViewDefaultCollumn.pszText = "]";
			this->variablesListview.InsertColumn(4 + 3 * currentRanges, &listViewDefaultCollumn);
			listViewDefaultCollumn.pszText = "#";
			this->variablesListview.InsertColumn(5 + 3 * currentRanges, &listViewDefaultCollumn);
			// edit all variables
			LVITEM listViewItem;
			memset(&listViewItem, 0, sizeof(listViewItem));
			listViewItem.mask = LVIF_TEXT; 
			listViewItem.cchTextMax = 256; 
			for (int varInc = 0; varInc < this->currentVariables.size(); varInc++)
			{
				variationRangeInfo tempInfo{ 0,0,0 };
				this->currentVariables[varInc].ranges.push_back( tempInfo );
				if (this->currentVariables[varInc].singleton)
				{
					listViewItem.pszText = "---";
				}
				else
				{
					listViewItem.pszText = "0";
				}
				listViewItem.iItem = varInc;
				listViewItem.iSubItem = 3 + 3 * currentRanges;
				this->variablesListview.SetItem(&listViewItem);
				listViewItem.iSubItem = 4 + 3 * currentRanges;
				this->variablesListview.SetItem(&listViewItem);
				listViewItem.iSubItem = 5 + 3 * currentRanges;
				this->variablesListview.SetItem(&listViewItem);
			}
			int newRangeNum = (this->variablesListview.GetHeaderCtrl()->GetItemCount() - 3) / 3;
			// make sure this makes sense.
			if (currentRanges != newRangeNum - 1)
			{
				errBox("Error!");
			}
			currentRanges = newRangeNum;
			this->variableRangeSets = currentRanges;
		}
	}
	else if (currentRanges > num)
	{
		while (currentRanges > num)
		{
			// delete a range.
			if (this->variableRangeSets == 1)
			{
				// can't delete last set...
				return;
			}

			this->variablesListview.DeleteColumn(3 + 3 * (currentRanges - 1));
			this->variablesListview.DeleteColumn(3 + 3 * (currentRanges - 1));
			this->variablesListview.DeleteColumn(3 + 3 * (currentRanges - 1));
			// edit all variables
			for (int varInc = 0; varInc < this->currentVariables.size(); varInc++)
			{
				this->currentVariables[varInc].ranges.pop_back();
			}
			int newRangeNum = (this->variablesListview.GetHeaderCtrl()->GetItemCount() - 3) / 3;
			// make sure this makes sense.
			if (currentRanges != newRangeNum + 1)
			{
				errBox("Error!");
			}
			// continue...
			currentRanges = newRangeNum;
			this->variableRangeSets = currentRanges;
		}
	}
	// if equal, nothing needs to be done.
	return;
}


void VariableSystem::handleColumnClick(NMHDR * pNotifyStruct, LRESULT * result)
{
	POINT cursorPos;
	GetCursorPos(&cursorPos);
	variablesListview.ScreenToClient(&cursorPos);
	LVHITTESTINFO myItemInfo;
	memset(&myItemInfo, 0, sizeof(LVHITTESTINFO));
	myItemInfo.pt = cursorPos;
	variablesListview.SubItemHitTest(&myItemInfo);
	if (myItemInfo.iSubItem == 3 + 3 * this->variableRangeSets)
	{
		// add a range.
		this->setVariationRangeNumber(this->variableRangeSets + 1);
	}
	else if (myItemInfo.iSubItem == 4 + 3 * this->variableRangeSets)
	{	
		// delete a range.
		this->setVariationRangeNumber(this->variableRangeSets - 1);
	}
	return;
}

void VariableSystem::updateVariableInfo(std::vector<Script*> scripts, MasterWindow* Master)
{
	/// get the item and subitem
	POINT cursorPos;
	GetCursorPos(&cursorPos);
	variablesListview.ScreenToClient(&cursorPos);
	//NMITEMACTIVATE itemClicked = *(NMITEMACTIVATE*)lParamOfMessage;
	int subitemIndicator;// = variablesListview.HitTest(cursorPos);
	LVHITTESTINFO myItemInfo;
	memset(&myItemInfo, 0, sizeof(LVHITTESTINFO));
	myItemInfo.pt = cursorPos;
	int itemIndicator;
	variablesListview.SubItemHitTest(&myItemInfo);
	itemIndicator = myItemInfo.iItem;
	subitemIndicator = myItemInfo.iSubItem;
	if (itemIndicator == -1)
	{
		// user didn't click in an item.
		return;
	}
	LVITEM listViewItem;
	memset(&listViewItem, 0, sizeof(listViewItem));
	listViewItem.mask = LVIF_TEXT;   // Text Style
	listViewItem.cchTextMax = 256; // Max size of test
	// choose item
	/// check if adding new variable
	listViewItem.iItem = itemIndicator;
	if (itemIndicator == currentVariables.size())
	{
		// add a variable
		currentVariables.resize(currentVariables.size() + 1);
		currentVariables.back().name = "";
		currentVariables.back().timelike = false;
		currentVariables.back().singleton = true;
		currentVariables.back().ranges.push_back({0,0,1});
		for (int rangeInc = 1; rangeInc < this->variableRangeSets; rangeInc++)
		{
			currentVariables.back().ranges.push_back({ 0,0,0 });
		}
		// add an item to the control
		// choose item
		listViewItem.iItem = itemIndicator;
		// initialize this row.
		listViewItem.pszText = "___";
		listViewItem.iSubItem = 0;       // Put in first coluom
		variablesListview.InsertItem(&listViewItem);
		if (isGlobal)
		{
			listViewItem.iSubItem = 1;
			listViewItem.pszText = "0";
			variablesListview.SetItem( &listViewItem );
		}
		else
		{
			listViewItem.iSubItem = 1;
			listViewItem.pszText = "Singleton";
			variablesListview.SetItem( &listViewItem );
			listViewItem.iSubItem = 2;
			listViewItem.pszText = "No";
			variablesListview.SetItem( &listViewItem );
			for (int rangeInc = 0; rangeInc < this->variableRangeSets; rangeInc++)
			{
				listViewItem.iSubItem = 3 + 3 * rangeInc;
				if (rangeInc == 0)
				{
					listViewItem.pszText = "0";
				}
				else
				{
					listViewItem.pszText = "---";
				}
				variablesListview.SetItem( &listViewItem );
				listViewItem.iSubItem = 4 + 3 * rangeInc;
				listViewItem.pszText = "---";
				variablesListview.SetItem( &listViewItem );
				listViewItem.iSubItem = 5 + 3 * rangeInc;
				listViewItem.pszText = "---";
				variablesListview.SetItem( &listViewItem );
			}
		}
	}
	/// Handle different subitem clicks
	switch (subitemIndicator)
	{
		case 0:
		{
			/// person name
			// prompt for a name
			std::string newName = (const char*)DialogBoxParam(this->programInstance, 
															   MAKEINTRESOURCE(IDD_TEXT_PROMPT_DIALOG), 0, 
															   (DLGPROC)textPromptDialogProcedure, 
															   (LPARAM)"Please enter a name for the variable:");
			// make name lower case
			std::transform( newName.begin(), newName.end(), newName.begin(), ::tolower );
			if (newName == "")
			{
				// probably canceled.
				break;
			}
			for (auto var : this->currentVariables)
			{
				if (var.name == newName)
				{
					thrower( "ERROR: A varaible with name " + newName + " already exists!" );
					return;
				}
			}
			// update the info inside
			currentVariables[itemIndicator].name = newName;
			// update the screen
			listViewItem.iItem = itemIndicator;
			listViewItem.iSubItem = subitemIndicator;
			listViewItem.pszText = (LPSTR)newName.c_str();
			variablesListview.SetItem(&listViewItem);
			// recolor to catch any new variable names needing to change color.
			for (int scriptInc = 0; scriptInc < scripts.size(); scriptInc++)
			{
				scripts[scriptInc]->colorEntireScript(Master);
			}
			break;
		}
		case 1:
		{
			if (isGlobal)
			{
				listViewItem.iItem = itemIndicator;
				listViewItem.iSubItem = subitemIndicator;
				std::string newValue = (const char*)DialogBoxParam( this->programInstance,
																	MAKEINTRESOURCE( IDD_TEXT_PROMPT_DIALOG ), 0,
																	(DLGPROC)textPromptDialogProcedure,
																	(LPARAM)"Please enter a value for this global "
																	"variable. Value will be formatted as a double." );
				if (newValue == "")
				{
					// probably canceled.
					break;
				}
				// else there's something there.
				try
				{
					currentVariables[itemIndicator].ranges.front().initialValue =
						currentVariables[itemIndicator].ranges.front().finalValue = std::stod( newValue );
				}
				catch (std::invalid_argument &exception)
				{
					errBox( "ERROR: the value entered, " + newValue + ", failed to convert to a double! "
							"Check for invalid characters." );
					break;
				}
				// update the listview
				listViewItem.iItem = itemIndicator;
				listViewItem.iSubItem = subitemIndicator;
				std::ostringstream out;
				out << std::setprecision( 12 ) << currentVariables[itemIndicator].ranges.front().initialValue;
				std::string tempstr = out.str();
				listViewItem.pszText = (LPSTR)tempstr.c_str();
				variablesListview.SetItem( &listViewItem );
				break;
			}
			else
			{
				/// singleton or variable?
				listViewItem.iItem = itemIndicator;
				listViewItem.iSubItem = subitemIndicator;
				// this is just a binary switch.
				if (currentVariables[itemIndicator].singleton)
				{
					// switch to variable.
					currentVariables[itemIndicator].singleton = false;
					listViewItem.iSubItem = subitemIndicator;
					listViewItem.pszText = "Variable";
					variablesListview.SetItem( &listViewItem );
					for (int rangeInc = 0; rangeInc < currentVariables[itemIndicator].ranges.size(); rangeInc++)
					{
						std::ostringstream out;
						out << std::setprecision( 12 ) << currentVariables[itemIndicator].ranges[(subitemIndicator - 3) / 3].initialValue;
						std::string tempstr = out.str();
						out.clear();
						out.str( "" );
						listViewItem.pszText = (LPSTR)tempstr.c_str();
						listViewItem.iSubItem = 3 + rangeInc * 3;
						variablesListview.SetItem( &listViewItem );
						out << std::setprecision( 12 ) << currentVariables[itemIndicator].ranges[(subitemIndicator - 3) / 3].finalValue;
						tempstr = out.str();
						out.clear();
						out.str( "" );
						listViewItem.pszText = (LPSTR)tempstr.c_str();
						listViewItem.iSubItem = 4 + rangeInc * 3;
						variablesListview.SetItem( &listViewItem );
						out << this->currentVariations;
						currentVariables[itemIndicator].ranges[(subitemIndicator - 3) / 3].variations = this->currentVariations;
						tempstr = out.str();
						out.clear();
						out.str( "" );
						listViewItem.pszText = (LPSTR)tempstr.c_str();
						listViewItem.iSubItem = 5 + rangeInc * 3;
						variablesListview.SetItem( &listViewItem );
					}
				}
				else
				{
					/// switch to singleton.
					currentVariables[itemIndicator].singleton = true;
					for (int rangeInc = 0; rangeInc < this->variableRangeSets; rangeInc++)
					{
						if (rangeInc == 0)
						{
							std::ostringstream out;
							out << std::setprecision( 12 ) << this->currentVariables[itemIndicator].ranges[0].initialValue;
							std::string tempStr = out.str();
							listViewItem.pszText = (LPSTR)tempStr.c_str();
							listViewItem.iSubItem = 3 + rangeInc * 3;
							variablesListview.SetItem( &listViewItem );
						}
						else
						{
							listViewItem.pszText = "---";
							listViewItem.iSubItem = 3 + rangeInc * 3;
							variablesListview.SetItem( &listViewItem );
						}
						// set the value to be dashes on the screen. no value for "from master".
						listViewItem.pszText = "---";
						listViewItem.iSubItem = 4 + rangeInc * 3;
						variablesListview.SetItem( &listViewItem );
						listViewItem.pszText = "---";
						listViewItem.iSubItem = 5 + rangeInc * 3;
						variablesListview.SetItem( &listViewItem );
					}
					listViewItem.iSubItem = subitemIndicator;
					listViewItem.pszText = "Singleton";
					variablesListview.SetItem( &listViewItem );
				}
			}
			break;
		}
		case 2:
		{
			/// timelike?
			listViewItem.iItem = itemIndicator;
			listViewItem.iSubItem = subitemIndicator;
			// this is just a binary switch.
			if (currentVariables[itemIndicator].timelike)
			{
				currentVariables[itemIndicator].timelike = false;
				listViewItem.pszText = "No";
			}
			else
			{
				currentVariables[itemIndicator].timelike = true;
				listViewItem.pszText = "Yes";
			}
			variablesListview.SetItem(&listViewItem);
			break;
		}
		default:
		{
			if ((subitemIndicator - 3) % 3 == 0)
			{
				// if it's a singleton, you can only set the first range initial value.
				if (currentVariables[itemIndicator].singleton && (subitemIndicator - 3) != 0)
				{
					// then no final value to be set.
					break;
				}
				std::string newValue = (const char*)DialogBoxParam(this->programInstance, 
																	MAKEINTRESOURCE(IDD_TEXT_PROMPT_DIALOG), 0, 
																	(DLGPROC)textPromptDialogProcedure,
																	(LPARAM)"Please enter an initial value for this "
																	"variable. Value will be formatted as a double.");
				if (newValue == "")
				{
					// probably canceled.
					break;
				}
				// else there's something there.
				try 
				{
					currentVariables[itemIndicator].ranges[(subitemIndicator - 3) / 3].initialValue = std::stod(newValue);
				}
				catch (std::invalid_argument &exception)
				{
					MessageBox(0, ("ERROR: the value entered, " + newValue + ", failed to convert to a double! "
									"Check for invalid characters.").c_str(), 0, 0);
					break;
				}
				// update the listview
				listViewItem.iItem = itemIndicator;
				listViewItem.iSubItem = subitemIndicator;
				std::ostringstream out;
				out << std::setprecision(12) << currentVariables[itemIndicator].ranges[(subitemIndicator - 3) / 3].initialValue;
				std::string tempstr = out.str();
				listViewItem.pszText = (LPSTR)tempstr.c_str();
				variablesListview.SetItem(&listViewItem);
				break;
			}
			else if ((subitemIndicator - 3) % 3 == 1)
			{
				if (currentVariables[itemIndicator].singleton)
				{
					// then no final value to be set.
					break;
				}
				// else singleton.
				std::string newValue = (const char*)DialogBoxParam(this->programInstance, 
																	MAKEINTRESOURCE(IDD_TEXT_PROMPT_DIALOG), 0, 
																	(DLGPROC)textPromptDialogProcedure,
																	(LPARAM)"Please enter a final value for this "
																	"variable. Value will be formatted as a double.");
				if (newValue == "")
				{
					// probably canceled.
					break;
				}
				// else there's something there.
				try
				{
					currentVariables[itemIndicator].ranges[(subitemIndicator - 3) / 3].finalValue = std::stod(newValue);
				}
				catch (std::invalid_argument &exception)
				{
					MessageBox(0, ("ERROR: the value entered, " + newValue + ", failed to convert to a double! Check "
									"for invalid characters.").c_str(), 0, 0);
					break;
				}
				// update the listview
				listViewItem.iItem = itemIndicator;
				listViewItem.iSubItem = subitemIndicator;
				std::ostringstream out;
				out << std::setprecision(12) << currentVariables[itemIndicator].ranges[(subitemIndicator - 3) / 3].finalValue;
				std::string tempstr = out.str();
				listViewItem.pszText = (LPSTR)tempstr.c_str();
				variablesListview.SetItem(&listViewItem);
				break;
			}
			else if((subitemIndicator - 3) % 3 == 2)
			{
				if (currentVariables[itemIndicator].singleton)
				{
					// then must be 1.
					break;
				}
				// else singleton.
				std::string newValue = (const char*)DialogBoxParam(this->programInstance, 
																	MAKEINTRESOURCE(IDD_TEXT_PROMPT_DIALOG), 0, 
																	(DLGPROC)textPromptDialogProcedure,
																	(LPARAM)"Please enter the number of variations of "
																	"this variable. Value will be formatted as an "
																	"integer.");
				if (newValue == "")
				{
					// probably canceled.
					break;
				}
				// else there's something there.
				try
				{
					for (int varInc = 0; varInc < currentVariables.size(); varInc++)
					{
						if (!currentVariables[varInc].singleton)
						{
							// make sure all variables have the same number of variations.
							currentVariables[varInc].ranges[(subitemIndicator - 3) / 3].variations = std::stoi(newValue);
						}
					}
					this->currentVariations = std::stoi(newValue);
				}
				catch (std::invalid_argument &exception)
				{
					MessageBox(0, ("ERROR: the value entered, " + newValue + ", failed to convert to a double! Check "
									"for invalid characters.").c_str(), 0, 0);
					break;
				}
				// update the listview
				
				listViewItem.iSubItem = subitemIndicator;
				std::ostringstream out;
				out << std::setprecision(12) << currentVariables[itemIndicator].ranges[(subitemIndicator - 3) / 3].variations;
				std::string tempstr = out.str();
				listViewItem.pszText = (LPSTR)tempstr.c_str();
				for (int varInc = 0; varInc < this->currentVariables.size(); varInc++)
				{
					if (!currentVariables[varInc].singleton)
					{
						listViewItem.iItem = varInc;
						variablesListview.SetItem(&listViewItem);
					}
				}
				break;
			}
		}
	}
	return;
}


void VariableSystem::deleteVariable()
{

	/// get the item and subitem
	POINT cursorPos;
	GetCursorPos(&cursorPos);
	variablesListview.ScreenToClient(&cursorPos);
	int subitemIndicator = variablesListview.HitTest(cursorPos);
	LVHITTESTINFO myItemInfo;
	memset(&myItemInfo, 0, sizeof(LVHITTESTINFO));
	myItemInfo.pt = cursorPos;
	int itemIndicator = variablesListview.SubItemHitTest(&myItemInfo);
	if (itemIndicator == -1 || itemIndicator == currentVariables.size())
	{
		// user didn't click in a deletable item.
		return;
	}
	int answer;
	if (itemIndicator < currentVariables.size())
	{
		answer = MessageBox(0, ("Delete variable " + currentVariables[itemIndicator].name + "?").c_str(), 0, MB_YESNO);
		if (answer == IDYES)
		{
			variablesListview.DeleteItem(itemIndicator);
			currentVariables.erase(currentVariables.begin() + itemIndicator);
		}

	}
	else if (itemIndicator > currentVariables.size())
	{
		answer = MessageBox(0, "You appear to have found a bug with the listview control... there are too many lines "
							 "in this control. Clear this line?", 0, MB_YESNO);
		if (answer == IDYES)
		{
			variablesListview.DeleteItem(itemIndicator);
		}
	}
	return;
}


variable VariableSystem::getVariableInfo(int varNumber)
{
	return currentVariables[varNumber];
}


unsigned int VariableSystem::getCurrentNumberOfVariables()
{
	return currentVariables.size();
}



void VariableSystem::clearVariables()
{
	// clear the internal info holder;
	currentVariables.clear();
	// clear the listview
	int itemCount = variablesListview.GetItemCount();
	// take the bottoms out of the variable listview
	for (int itemInc = 0; itemInc < itemCount; itemInc++)
	{
		variablesListview.DeleteItem(0);
	}
	return;
}


std::vector<variable> VariableSystem::getEverything()
{
	return this->currentVariables;
}


std::vector<variable> VariableSystem::getAllSingletons()
{
	std::vector<variable> singletons;
	for (int varInc = 0; varInc < currentVariables.size(); varInc++)
	{
		if (currentVariables[varInc].singleton)
		{
			singletons.push_back(currentVariables[varInc]);
		}
	}
	return singletons;
}

// this function returns the compliment of the variables that "getAllSingletons" returns.
std::vector<variable> VariableSystem::getAllVaryingParameters()
{
	std::vector<variable> varyingParameters;
	for (int varInc = 0; varInc < currentVariables.size(); varInc++)
	{
		// opposite of get singletons.
		if (!currentVariables[varInc].singleton)
		{
			varyingParameters.push_back(currentVariables[varInc]);
		}
	}
	return varyingParameters;
}


void VariableSystem::addGlobalVariable( variable var, int item )
{
	// convert name to lower case.
	std::transform( var.name.begin(), var.name.end(), var.name.begin(), ::tolower );

	if (var.name == "")
	{
		// then add empty variable slot
		// Make First Blank row.
		LVITEM listViewDefaultItem;
		memset( &listViewDefaultItem, 0, sizeof( listViewDefaultItem ) );
		listViewDefaultItem.mask = LVIF_TEXT;   // Text Style
		listViewDefaultItem.cchTextMax = 256; // Max size of test
		listViewDefaultItem.pszText = "___";
		if (item == -1)
		{
			// at end.
			listViewDefaultItem.iItem = this->variablesListview.GetItemCount();          // choose item  
		}
		else
		{
			listViewDefaultItem.iItem = item;          // choose item  
		}
		listViewDefaultItem.iSubItem = 0;       // Put in first collumn
		variablesListview.InsertItem( &listViewDefaultItem );
		listViewDefaultItem.iSubItem = 1;
		variablesListview.SetItem( &listViewDefaultItem );
		variablesListview.RedrawWindow();
		return;
	}
	/// else...
	if (var.singleton == false)
	{
		thrower( "ERROR: attempted to add a non-singleton to the global variable control!" );
		return;
	}
	for (auto currentVar : currentVariables)
	{
		if (currentVar.name == var.name)
		{
			thrower( "ERROR: A variable with the name " + var.name + " already exists!" );
		}
	}
	// add it to the internal structure that keeps track of variables
	currentVariables.push_back( var );
	/// add the entry to the listview.
	LVITEM listViewItem;
	memset( &listViewItem, 0, sizeof( listViewItem ) );
	listViewItem.mask = LVIF_TEXT;   // Text Style
	listViewItem.cchTextMax = 256; // Max size of test
								   // choose item  
								   // initialize this row.
	listViewItem.iItem = item;
	listViewItem.pszText = (LPSTR)var.name.c_str();
	// Put in first column
	listViewItem.iSubItem = 0;
	variablesListview.InsertItem( &listViewItem );
	listViewItem.iSubItem = 1;
	std::string text = std::to_string(var.ranges.front().initialValue);
	listViewItem.pszText = (LPSTR)text.c_str();
	variablesListview.SetItem( &listViewItem );
}


void VariableSystem::addConfigVariable(variable var, int item)
{
	// make name lower case.
	std::transform( var.name.begin(), var.name.end(), var.name.begin(), ::tolower );

	if (var.name == "")
	{
		// then add empty variable slot
		// Make First Blank row.
		LVITEM listViewDefaultItem;
		memset(&listViewDefaultItem, 0, sizeof(listViewDefaultItem));
		listViewDefaultItem.mask = LVIF_TEXT;   // Text Style
		listViewDefaultItem.cchTextMax = 256; // Max size of test
		listViewDefaultItem.pszText = "___";
		if (item == -1)
		{
			// at end.
			listViewDefaultItem.iItem = this->variablesListview.GetItemCount();          // choose item  
		}
		else
		{
			listViewDefaultItem.iItem = item;          // choose item  
		}
		listViewDefaultItem.iSubItem = 0;       // Put in first collumn
		variablesListview.InsertItem(&listViewDefaultItem);
		for (int itemInc = 1; itemInc < 6; itemInc++) // Add SubItems in a loop
		{
			listViewDefaultItem.iSubItem = itemInc;
			variablesListview.SetItem(&listViewDefaultItem);
		}
		variablesListview.RedrawWindow();
		return;
	}

	/// else...
	for (auto currentVar : currentVariables)
	{
		if (currentVar.name == var.name)
		{
			thrower( "ERROR: A variable with the name " + var.name + " already exists!" );
			return;
		}
	}

	/// add it to the internal structure that keeps track of variables
	currentVariables.push_back(var);
	/// add the entry to the listview.
	LVITEM listViewItem;
	memset(&listViewItem, 0, sizeof(listViewItem));
	listViewItem.mask = LVIF_TEXT;   // Text Style
	listViewItem.cchTextMax = 256; // Max size of test
	// choose item  
	// initialize this row.
	listViewItem.iItem = item;          
	listViewItem.pszText = (LPSTR)var.name.c_str();
	// Put in first column
	listViewItem.iSubItem = 0;
	variablesListview.InsertItem(&listViewItem);
	listViewItem.iSubItem = 1;
	if (var.singleton)
	{
		listViewItem.pszText = "Singleton";
	}
	else
	{
		listViewItem.pszText = "From Master";
	}
	variablesListview.SetItem(&listViewItem);
	// make sure there are enough collumns.
	int collumns = this->variablesListview.GetItemCount();
	for (int rangeAddInc = 0; rangeAddInc < int(var.ranges.size()) - collumns; rangeAddInc++)
	{
		// add a range.
		LV_COLUMN listViewDefaultCollumn;
		// Zero Members
		memset(&listViewDefaultCollumn, 0, sizeof(listViewDefaultCollumn));
		// Type of mask
		listViewDefaultCollumn.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
		// width between each coloum
		CString varnum((std::to_string(this->variableRangeSets + 1) + ":(").c_str());
		listViewDefaultCollumn.pszText = varnum.GetBuffer();
		listViewDefaultCollumn.cx = 0x20;
		this->variablesListview.InsertColumn(3 + 3 * this->variableRangeSets, &listViewDefaultCollumn);
		listViewDefaultCollumn.pszText = "]";
		this->variablesListview.InsertColumn(4 + 3 * this->variableRangeSets, &listViewDefaultCollumn);
		listViewDefaultCollumn.pszText = "#";
		this->variablesListview.InsertColumn(5 + 3 * this->variableRangeSets, &listViewDefaultCollumn);
		// edit all variables
		LVITEM listViewItem;
		memset(&listViewItem, 0, sizeof(listViewItem));
		listViewItem.mask = LVIF_TEXT;   // Text Style
		listViewItem.cchTextMax = 256; // Max size of test
									   // choose item
									   /// check if adding new variable
		for (int varInc = 0; varInc < this->currentVariables.size(); varInc++)
		{
			this->currentVariables[varInc].ranges.push_back({0,0,0});
			if (this->currentVariables[varInc].singleton)
			{
				listViewItem.pszText = "---";
			}
			else
			{
				listViewItem.pszText = "0";
			}
			listViewItem.iItem = varInc;
			listViewItem.iSubItem = 3 + 3 * this->variableRangeSets;
			this->variablesListview.SetItem(&listViewItem);
			listViewItem.iSubItem = 4 + 3 * this->variableRangeSets;
			this->variablesListview.SetItem(&listViewItem);
			listViewItem.iSubItem = 5 + 3 * this->variableRangeSets;
			this->variablesListview.SetItem(&listViewItem);
		}
		this->variableRangeSets++;
	}


	for (int rangeInc = 0; rangeInc < var.ranges.size(); rangeInc++)
	{
		if (!var.singleton)
		{
			// variable case.
			std::ostringstream out;
			out << std::setprecision(12) << currentVariables[item].ranges[rangeInc].initialValue;
			std::string tempstr = out.str();
			out.clear();
			out.str("");
			listViewItem.pszText = (LPSTR)tempstr.c_str();
			listViewItem.iSubItem = 3 + rangeInc * 3;
			variablesListview.SetItem(&listViewItem);
			out << std::setprecision(12) << currentVariables[item].ranges[rangeInc].finalValue;
			tempstr = out.str();
			out.clear();
			out.str("");
			listViewItem.pszText = (LPSTR)tempstr.c_str();
			listViewItem.iSubItem = 4 + rangeInc * 3;
			variablesListview.SetItem(&listViewItem);
			out << currentVariables[item].ranges[rangeInc].variations;
			tempstr = out.str();
			out.clear();
			out.str("");
			listViewItem.pszText = (LPSTR)tempstr.c_str();
			listViewItem.iSubItem = 5 + rangeInc * 3;
			variablesListview.SetItem(&listViewItem);
		}
		else
		{
			// singleton case.
			if (rangeInc == 0)
			{
				// get the first value, this is the value of the singleton.
				std::ostringstream out;
				out << std::setprecision(12) << currentVariables[item].ranges[rangeInc].initialValue;
				std::string tempstr = out.str();
				out.clear();
				out.str("");
				listViewItem.pszText = (LPSTR)tempstr.c_str();
				listViewItem.iSubItem = 3 + rangeInc * 3;
				variablesListview.SetItem(&listViewItem);
			}
			else
			{
				listViewItem.pszText = "---";
				listViewItem.iSubItem = 3 + rangeInc * 3;
				variablesListview.SetItem(&listViewItem);
			}
			listViewItem.pszText = "---";
			listViewItem.iSubItem = 4 + rangeInc * 3;
			variablesListview.SetItem(&listViewItem);
			listViewItem.pszText = "---";
			listViewItem.iSubItem = 5 + rangeInc * 3;
			variablesListview.SetItem(&listViewItem);
		}
	}
	if (var.timelike)
	{
		listViewItem.pszText = "Yes";
	}
	else
	{
		listViewItem.pszText = "No";
	}

	// Enter text to SubItems
	listViewItem.iSubItem = 2;
	variablesListview.SetItem(&listViewItem);
	variablesListview.RedrawWindow();
	return;
}


void VariableSystem::initialize( POINT& pos, std::vector<CToolTipCtrl*>& toolTips, MasterWindow* master, int& id, 
								 std::string title )
{
	if (title == "GLOBAL VARIABLES")
	{
		this->isGlobal = true;
	}
	else
	{
		this->isGlobal = false;
	}
	// initialize this number to reflect the initial status of the window.
	this->variableRangeSets = 1;
	// controls
	variablesHeader.position = { pos.x, pos.y, pos.x + 480, pos.y + 25 };
	variablesHeader.ID = id++;
	variablesHeader.Create( title.c_str(), WS_BORDER | WS_CHILD | WS_VISIBLE | ES_CENTER | ES_READONLY,
							variablesHeader.position, master, variablesHeader.ID );
	variablesHeader.SetFont(CFont::FromHandle(sHeadingFont));
	pos.y += 25;

	variablesListview.position = { pos.x, pos.y, pos.x + 480, pos.y + 125 };
	variablesListview.ID = id++;
	if (!	 ((variablesListview.ID == IDC_CONFIG_VARS_LISTVIEW && isGlobal == false) 
		   || (variablesListview.ID == IDC_GLOBAL_VARS_LISTVIEW && isGlobal == true)))
	{
		throw;
	}
	variablesListview.Create(WS_VISIBLE | WS_CHILD | LVS_REPORT | LVS_EDITLABELS | WS_BORDER, 
							  variablesListview.position, master, variablesListview.ID);
	variablesListview.SetFont(CFont::FromHandle(sSmallFont));
	variablesListview.SetBkColor(RGB(15, 15, 15));
	variablesListview.SetTextBkColor(RGB(15, 15, 15));
	variablesListview.SetTextColor(RGB(255, 255, 255));
	LV_COLUMN listViewDefaultCollumn;
	// Zero Members
	memset(&listViewDefaultCollumn, 0, sizeof(listViewDefaultCollumn));
	listViewDefaultCollumn.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;


	listViewDefaultCollumn.pszText = "Symbol";
	if (isGlobal)
	{
		listViewDefaultCollumn.cx = 200;
		variablesListview.InsertColumn( 0, &listViewDefaultCollumn );

		listViewDefaultCollumn.pszText = "Value";
		listViewDefaultCollumn.cx = 200;
		variablesListview.InsertColumn( 1, &listViewDefaultCollumn );
		// Make First Blank row.
		LVITEM listViewDefaultItem;
		memset( &listViewDefaultItem, 0, sizeof( listViewDefaultItem ) );
		listViewDefaultItem.mask = LVIF_TEXT;   // Text Style
		listViewDefaultItem.cchTextMax = 256; // Max size of test
		listViewDefaultItem.pszText = "___";
		listViewDefaultItem.iItem = 0;          // choose item  
		listViewDefaultItem.iSubItem = 0;       // Put in first coluom
		variablesListview.InsertItem( &listViewDefaultItem );
		listViewDefaultItem.iSubItem = 1;
		variablesListview.SetItem( &listViewDefaultItem );
	}
	else
	{
		listViewDefaultCollumn.cx = 50;
		variablesListview.InsertColumn( 0, &listViewDefaultCollumn );
		listViewDefaultCollumn.pszText = "Type";
		listViewDefaultCollumn.cx = 0x30;
		variablesListview.InsertColumn( 1, &listViewDefaultCollumn );
		listViewDefaultCollumn.pszText = "Timelike?";
		listViewDefaultCollumn.cx = 0x42;
		variablesListview.InsertColumn( 2, &listViewDefaultCollumn );
		listViewDefaultCollumn.pszText = "1:(";
		listViewDefaultCollumn.cx = 0x20;
		variablesListview.InsertColumn( 3, &listViewDefaultCollumn );
		listViewDefaultCollumn.pszText = "]";
		variablesListview.InsertColumn( 4, &listViewDefaultCollumn );
		listViewDefaultCollumn.pszText = "#";
		variablesListview.InsertColumn( 5, &listViewDefaultCollumn );
		listViewDefaultCollumn.pszText = "+";
		listViewDefaultCollumn.cx = 0x16;
		variablesListview.InsertColumn( 6, &listViewDefaultCollumn );
		listViewDefaultCollumn.pszText = "-";
		variablesListview.InsertColumn( 7, &listViewDefaultCollumn );
		// Make First Blank row.
		LVITEM listViewDefaultItem;
		memset( &listViewDefaultItem, 0, sizeof( listViewDefaultItem ) );
		listViewDefaultItem.mask = LVIF_TEXT;   // Text Style
		listViewDefaultItem.cchTextMax = 256; // Max size of test
		listViewDefaultItem.pszText = "___";
		listViewDefaultItem.iItem = 0;          // choose item  
		listViewDefaultItem.iSubItem = 0;       // Put in first coluom
		variablesListview.InsertItem( &listViewDefaultItem );
		for (int itemInc = 1; itemInc < 6; itemInc++) // Add SubItems in a loop
		{
			listViewDefaultItem.iSubItem = itemInc;
			variablesListview.SetItem( &listViewDefaultItem );
		}

	}
	pos.y += 125;
	return;
}

INT_PTR VariableSystem::handleColorMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, 
											std::unordered_map<std::string, HBRUSH> brushes)
{
	DWORD controlID = GetDlgCtrlID((HWND)lParam);
	HDC hdcStatic = (HDC)wParam;
	if (controlID == this->variablesHeader.ID)
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

