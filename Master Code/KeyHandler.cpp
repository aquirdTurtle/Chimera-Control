#include "stdafx.h"
#include "KeyHandler.h"
#include <unordered_map>
#include <algorithm>
#include <random>
#include <fstream>
#include <iomanip>


key KeyHandler::getKey()
{
	return keyValues;
}


void KeyHandler::initialize(POINT loc, CWnd* parent, int& id)
{
	randomizeVariablesButton.sPos = { loc.x, loc.y, loc.x + 480, loc.y += 20 };
	randomizeVariablesButton.ID = id++;
	randomizeVariablesButton.Create( "Randomize Variations", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 
									 randomizeVariablesButton.sPos, parent, randomizeVariablesButton.ID );
	randomizeVariablesButton.SetCheck(true);

	randomizeRepetitionsButton.sPos = { loc.x, loc.y, loc.x + 480, loc.y += 20 };
	randomizeRepetitionsButton.ID = id++;
	randomizeRepetitionsButton.Create( "Randomize Repetitions", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
									   randomizeVariablesButton.sPos, parent, randomizeVariablesButton.ID);

}



void KeyHandler::loadVariables(std::vector<variable> newVariables)
{
	variables = newVariables;
}


void KeyHandler::generateKey()
{
	// get information from variables.
	keyValues.clear();
	// each element of the vector refers to the number of variations within a given variation range.
	std::vector<int> variations;
	std::vector<int> variableIndexes;
	for (int varInc = 0; varInc < variables.size(); varInc++)
	{
		// find a varying parameter.
		if (!variables[varInc].constant)
		{
			// then this variable varies. 

			variableIndexes.push_back(varInc);
			// variations.size is the number of ranges currently.
			if (variations.size() != variables[varInc].ranges.size())
			{
				// if its zero its just the initial size on the initial variable.
				if (variations.size() != 0)
				{
					thrower("ERROR: Not all variables seem to have the same number of ranges for their parameters!");
				}
				variations.resize(variables[varInc].ranges.size());
			}

			// make sure the variations number is consistent between 
			for (int rangeInc = 0; rangeInc < variations.size(); rangeInc++)
			{
				// avoid the case of zero as this just hasn't been set yet.
				if (variations[rangeInc] != 0)
				{
					if (variables[varInc].ranges[rangeInc].variations != variations[rangeInc])
					{
						thrower("ERROR: not all ranges of variables have the same number of variations!");
					}
				}
				variations[rangeInc] = variables[varInc].ranges[rangeInc].variations;
			}
		}
	}
	// create a key which will be randomized and then used to randomize other things the same way.
	std::vector<int> randomizerKey;
	int counter = 0;
	for (int rangeInc = 0; rangeInc < variations.size(); rangeInc++)
	{
		for (int variationInc = 0; variationInc < variations[rangeInc]; variationInc++)
		{
			randomizerKey.push_back(counter);
			counter++;
		}
	}
	if (randomizeVariablesButton.GetCheck())
	{
		// initialize rng
		std::random_device rng;
		std::mt19937 urng(rng());
		// and shuffle.
		std::shuffle(randomizerKey.begin(), randomizerKey.end(), urng);
		// we now have a random key for the shuffling which every variable will follow
		// initialize this to one so that constants always get at least one value.
	}
	int totalSize = 1;
	for (int variableInc = 0; variableInc < variableIndexes.size(); variableInc++)
	{
		int varIndex = variableIndexes[variableInc];
		// calculate all values for a given variable
		std::vector<double> tempKey, tempKeyRandomized;
		for (int rangeInc = 0; rangeInc < variations.size(); rangeInc++)
		{
			double range = (variables[varIndex].ranges[rangeInc].finalValue - variables[varIndex].ranges[rangeInc].initialValue);
			int spacings;

			if (variables[varIndex].ranges[rangeInc].leftInclusive && variables[varIndex].ranges[rangeInc].rightInclusive)
			{
				spacings = variations[rangeInc] - 1;
			}
			else if (variables[varIndex].ranges[rangeInc].leftInclusive && variables[varIndex].ranges[rangeInc].rightInclusive)
			{
				spacings = variations[rangeInc] + 1;
			}
			else
			{
				spacings = variations[rangeInc];
			}
			double initVal;
			if (variables[varIndex].ranges[rangeInc].leftInclusive)
			{
				initVal = variables[varIndex].ranges[rangeInc].initialValue;
			}
			else
			{
				initVal = variables[varIndex].ranges[rangeInc].initialValue + range / spacings;
			}

			for (int variationInc = 0; variationInc < variations[rangeInc]; variationInc++)
			{
				tempKey.push_back(range * variationInc / spacings + initVal);
			}
		}
		// now shuffle these values
		for (int randomizerInc = 0; randomizerInc < randomizerKey.size(); randomizerInc++)
		{
			tempKeyRandomized.push_back(tempKey[randomizerKey[randomizerInc]]);
		}
		// now, finally, add to the actual key object.
		keyValues[variables[varIndex].name].first = tempKeyRandomized;
		// varies
		keyValues[variables[varIndex].name].second = true;
		totalSize = tempKeyRandomized.size();
	}
	// now add all constant objects.
	for (variable var : variables)
	{
		if (var.constant)
		{
			std::vector<double> tempKey;
			for (int variationInc = 0; variationInc < totalSize; variationInc++)
			{
				// the only constant value is stored as the initial value here.
				tempKey.push_back(var.ranges[0].initialValue);
			}
			keyValues[var.name].first = tempKey;
			// does not vary
			keyValues[var.name].second = false;
		}
	}
}


// creates the key file.
void KeyHandler::exportKey()
{
	std::fstream keyFile(KEY_ADDRESS, std::ios::out);
	if (!keyFile.is_open())
	{
		thrower("ERROR: Exporting Key File Failed to open!");
	}
	for (int variableInc = 0; variableInc < variables.size(); variableInc++)
	{
		if (!variables[variableInc].constant)
		{
			keyFile << std::setw(15) << variables[variableInc].name + ":";
			for (int keyInc = 0; keyInc < keyValues[variables[variableInc].name].first.size(); keyInc++)
			{
				keyFile << std::setprecision(12) << std::setw(15) << keyValues[variables[variableInc].name].first[keyInc];
				keyFile << " ";
			}
			keyFile << "\n";
		}
	}
	keyFile.close();
}


void KeyHandler::rearrange(int width, int height, std::unordered_map<std::string, CFont*> fonts)
{
	randomizeVariablesButton.rearrange(width, height, fonts);
	randomizeRepetitionsButton.rearrange(width, height, fonts);
}
