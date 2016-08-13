#include "stdafx.h"
#include "KeyHandler.h"
#include <unordered_map>
#include <algorithm>
#include <random>
#include <fstream>
#include <iomanip>

std::unordered_map<std::string, std::vector<double>> KeyHandler::getKey()
{
	return this->keyValues;
}
bool KeyHandler::loadVariables(std::vector<variable> newVariables)
{
	this->variables = newVariables;
	return true;
}

bool KeyHandler::generateKey()
{
	// get information from variables.
	this->keyValues.clear();
	std::vector<int> variations;
	variations.push_back(1);
	std::vector<int> variableIndexes;
	for (int varInc = 0; varInc < variables.size(); varInc++)
	{
		// find a varying parameter.
		if (!variables[varInc].singleton)
		{
			variableIndexes.push_back(varInc);
			if (variations.size() != variables[varInc].ranges.size())
			{
				if (variations.size() != 1)
				{
					errBox("ERROR: Not all variables seem to have the same number of ranges for their parameters!");
					return false;
				}
				variations.resize(variables[varInc].ranges.size());
			}
			// make sure the variations number is consistent.
			for (int rangeInc = 0; rangeInc < variations.size(); rangeInc++)
			{
				if ((rangeInc == 0 && variations[rangeInc] != 1) || (rangeInc != 0 && variations[rangeInc] != 0))
				{
					if (variables[varInc].ranges[rangeInc].variations != variations[rangeInc])
					{
						errBox("ERROR: not all ranges of variables have the same number of variations!");
						return false;
					}
				}
				variations[rangeInc] = variables[varInc].ranges[rangeInc].variations;
			}
		}
	}

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
	std::random_device rng;
	std::mt19937 urng(rng());
	std::shuffle(randomizerKey.begin(), randomizerKey.end(), urng);
	// we now have a random key for the shuffling which every variable will follow
	// initialize this to one so that singletons always get at least one value.
	int totalSize = 1;
	for (int variableInc = 0; variableInc < variableIndexes.size(); variableInc++)
	{
		int varIndex = variableIndexes[variableInc];
		// calculate all values for a given variable
		std::vector<double> tempKey, tempKeyRandomized;
		for (int rangeInc = 0; rangeInc < variations.size(); rangeInc++)
		{
			for (int variationInc = 0; variationInc < variations[rangeInc]; variationInc++)
			{
				double value = (variables[varIndex].ranges[rangeInc].finalValue - variables[varIndex].ranges[rangeInc].initialValue) * ((variationInc + 1)/ double(variations[rangeInc])) 
					+ variables[varIndex].ranges[rangeInc].initialValue;
				tempKey.push_back(value);
			}
		}
		// now shuffle these values
		for (int randomizerInc = 0; randomizerInc < randomizerKey.size(); randomizerInc++)
		{
			tempKeyRandomized.push_back(tempKey[randomizerKey[randomizerInc]]);
		}
		// now, finally, add to the actual key object.
		this->keyValues[variables[varIndex].name] = tempKeyRandomized;
		totalSize = tempKeyRandomized.size();
	}
	// now add all singleton objects.
	for (variable var : variables)
	{
		if (var.singleton)
		{
			std::vector<double> tempKey;
			for (int variationInc = 0; variationInc < totalSize; variationInc++)
			{
				// the only singleton value is stored as the initial value here.
				tempKey.push_back(var.ranges[0].initialValue);
			}
			this->keyValues[var.name] = tempKey;
		}
	}
	return true;
}

bool KeyHandler::exportKey()
{
	// TODO
	std::fstream keyFile(KEY_ADDRESS, std::ios::out);
	if (!keyFile.is_open())
	{
		errBox("ERROR: Exporting Key File Failed!");
		return false;
	}
	for (int variableInc = 0; variableInc < variables.size(); variableInc++)
	{
		keyFile << std::setw(15) << variables[variableInc].name + ":";
		for (int keyInc = 0; keyInc < keyValues[variables[variableInc].name].size(); keyInc++)
		{
			keyFile << std::setprecision(12) << std::setw(15) << this->keyValues[variables[variableInc].name][keyInc];
		}
		keyFile << "\n";
	}
	keyFile.close();
	// export this to the andor.
	return true;
}