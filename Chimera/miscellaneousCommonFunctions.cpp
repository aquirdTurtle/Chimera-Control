#include "stdafx.h"
#include "miscellaneousCommonFunctions.h"
#include <Windows.h>
#include <string>
#include <boost/tokenizer.hpp>


// for mfc edits
void appendText(std::string newText, CEdit& edit)
{
	// get the initial text length
	int nLength = edit.GetWindowTextLength();
	// put the selection at the end of text
	edit.SetSel(nLength, nLength);
	// replace the selection
	edit.ReplaceSel(cstr(newText));
	edit.LineScroll(INT_MAX);
}

// for mfc rich edits
void appendText(std::string newText, Control<CRichEditCtrl>& edit)
{
	// get the initial text length
	int nLength = edit.GetWindowTextLength();
	// put the selection at the end of text
	edit.SetSel(nLength, nLength);
	// replace the selection
	edit.ReplaceSel(cstr(newText));
	edit.SetFocus();
	nLength = edit.GetWindowTextLength();
	// put the selection at the end of text
	edit.SetSel(nLength, nLength);
	//	edit.LineScroll(INT_MAX);
}


std::string doubleToString( double number, long precision )
{
	std::stringstream stream;
	stream << std::fixed << std::setprecision( precision ) << number;
	return stream.str();
}


double reduce(std::string expression, key variationKey, UINT variation, std::vector<variable>& vars)
{
	double resultOfReduction = 0;
	try
	{
		// try the simple thing.
		size_t num;
		resultOfReduction = std::stod(expression, &num);
		if (num != expression.size())
		{
			thrower("started with number");
		}
		return resultOfReduction;
	}
	catch (std::invalid_argument&) {	/* that's fine, just means it needs actual reducing.*/ }
	catch (Error&) { /* Same. */ }
	std::string workingExp = expression;
	workingExp = "(" + workingExp + ")";
	std::vector<std::string> terms;
	// separate terms out.
	// specify only the kept separators
	boost::char_separator<char> sep("", " \t+-*/()");
	boost::tokenizer<boost::char_separator<char>> tokens(workingExp, sep);
	for (std::string t : tokens)
	{
		if (t != " " && t != "\t")
		{
			// don't include whitespace.
			terms.push_back(t);
		}
	}

	// if not the default value,
	if (variation != -1)
	{
		// substitute all variables within the expression.
		for (auto& term : terms)
		{
			for (auto var : variationKey)
			{
				if (term == var.first)
				{
					term = str(var.second.first[variation]);
					// find the variable 
					bool foundVariable = false;
					for (auto& variable : vars)
					{
						if (var.first == variable.name)
						{
							variable.active = true;
							foundVariable = true;
						}
					}
					if (!foundVariable)
					{
						thrower("ERROR: Variable existed in key but not in variable vector. Low level bug. Ask Mark.");
					}
				}
			}
		}
	}

	// do math.
	bool parenthesisExists = true;
	bool atLeastOneTerm = false;
	/// parenthesis
	while (parenthesisExists)
	{
		// this will store all the terms in the right-most parenthesis enclosure.
		// arguably it's a little more intuitive, as to how a person would actually simplify,
		// to do the innermost first. But it shouldn't make a mathematical difference, and this
		// is easier to code.
		std::vector<std::string> rightmostParenthesisTerms;
		bool leftExists = false;
		// find innermost parenthesis		
		UINT count = 0;
		int leftPos = 0, rightPos = 0;
		for (auto& elem : terms)
		{
			if (elem == "(")
			{
				// just always reset it
				leftExists = true;
				leftPos = count;
			}
			count++;
		}
		if (leftExists)
		{
			rightmostParenthesisTerms = std::vector<std::string>(&terms[leftPos] + 1, &terms.back() + 1);
			bool rightExists = false;
			count = 0;
			for (auto& elem : rightmostParenthesisTerms)
			{
				if (elem == ")")
				{
					// rightPos is set in terms of the original vector for future reference,
					// not the new substring I just created.
					rightPos = leftPos + count;
					rightExists = true;
					break;
				}
				count++;
			}
			if (!rightExists)
			{
				thrower("ERROR: Unmatched \"(\" in math expression!\r\n");
			}
			rightmostParenthesisTerms = std::vector<std::string>(&terms[leftPos + 1], &terms[rightPos + 1]);
			// now I have a term which I can analyze.
		}
		else
		{
			// I'm finished.
			parenthesisExists = false;
			break;
		}
		atLeastOneTerm = true;
		// I now I have a subvector with no parenthesis in it. Verify that there's no parenthesis in it.
		for (auto elem : rightmostParenthesisTerms)
		{
			if (elem == "(" || elem == ")")
			{
				thrower("ERROR: Math evaluation failed, there are parenthesis in an innermost term which should not"
						" have parenthesis in it! This is a bug.");
			}
		}
		/// find mult do mult
		// this can be done as just a scan from left to right.
		count = 0;
		for (UINT count = 0; count < rightmostParenthesisTerms.size(); count++)
		{
			if (rightmostParenthesisTerms[count] == "*" || rightmostParenthesisTerms[count] == "/")
			{
				std::string individualResult;
				double leftTerm = 0, rightTerm = 0;
				try
				{
					if (count <= 0)
					{
						thrower("ERROR: Operator " + rightmostParenthesisTerms[count] + " has no value on its"
								" left!");
					}
					leftTerm = std::stod(rightmostParenthesisTerms[count - 1]);
				}
				catch (std::invalid_argument&)
				{
					thrower("ERROR: Tried and failed to evaluate string " + rightmostParenthesisTerms[count - 1]
							+ " to a double (error in for multiplication / division section)!");
				}
				try
				{
					rightTerm = std::stod(rightmostParenthesisTerms[count + 1]);
				}
				catch (std::invalid_argument&)
				{
					thrower("ERROR: Tried and failed to convert string " + rightmostParenthesisTerms[count + 1]
							+ " to a double (error in for multiplication / division section)!");
				}
				// caliculate the result
				if (rightmostParenthesisTerms[count] == "/")
				{
					individualResult = str(leftTerm / rightTerm);
				}
				else
				{
					individualResult = str(leftTerm * rightTerm);
				}
				// replace the * expression with the result.
				rightmostParenthesisTerms.erase(rightmostParenthesisTerms.begin() + (count - 1),
												rightmostParenthesisTerms.begin() + (count + 2));
				rightmostParenthesisTerms.insert(rightmostParenthesisTerms.begin() + (count - 1), individualResult);
				// this accounts for the fact that we just deleted several terms from the vector, making sure that
				// the function looks at the correct next term afterwards.
				count -= 1;
			}
		}

		/// find add do add
		for (UINT count = 0; count < rightmostParenthesisTerms.size(); count++)
		{
			if (rightmostParenthesisTerms[count] == "+" || rightmostParenthesisTerms[count] == "-")
			{
				std::string individualResult;
				double leftTerm = 0, rightTerm = 0;
				try
				{
					if (count == 0 && rightmostParenthesisTerms[count] == "-")
					{
						// this will convert (-1) to (0-1) which will evaluate correctly.
						leftTerm = 0;
						rightmostParenthesisTerms.insert(rightmostParenthesisTerms.begin(), "0");
						count = 1;
					}
					else
					{
						if (count <= 0)
						{
							thrower("ERROR: Operator " + rightmostParenthesisTerms[count] + " has no value on its"
									" left!");
						}
						leftTerm = std::stod(rightmostParenthesisTerms[count - 1]);
					}
				}
				catch (std::invalid_argument&)
				{
					thrower("ERROR: Tried and failed to evaluate string " + rightmostParenthesisTerms[count - 1]
							+ " to a double (error in for addition/subtraction section)!");
				}
				try
				{
					rightTerm = std::stod(rightmostParenthesisTerms[count + 1]);
				}
				catch (std::invalid_argument&)
				{
					thrower("ERROR: Tried and failed to evaluate string " + rightmostParenthesisTerms[count + 1]
							+ " to a double (error in for addition/subtraction section)!");
				}
				// caliculate the result
				if (rightmostParenthesisTerms[count] == "+")
				{
					individualResult = str(leftTerm + rightTerm);
				}
				else
				{
					individualResult = str(leftTerm - rightTerm);
				}
				// replace the expression with the result.
				rightmostParenthesisTerms.erase(rightmostParenthesisTerms.begin() + (count - 1),
												rightmostParenthesisTerms.begin() + (count + 2));
				rightmostParenthesisTerms.insert(rightmostParenthesisTerms.begin() + (count - 1), individualResult);
				// this accounts for the fact that we just deleted several terms from the vector, making sure that
				// the function looks at the correct next term afterwards.
				count -= 1;
			}
		}
		if (rightmostParenthesisTerms.size() != 1)
		{
			std::string msg("ERROR: Math evaluation failed! After a complete run-through of a term, there was more than one "
							"element left! Terms were:\n");
			std::string termString;
			for (auto term : terms)
			{
				termString += term + "\n";
			}
			thrower(msg + termString);
		}
		// and replace parenthesis term in the main set of terms.
		std::string result = rightmostParenthesisTerms[0];
		terms.erase(terms.begin() + leftPos, terms.begin() + (rightPos + 2));
		terms.insert(terms.begin() + leftPos, result);
	}
	if (!atLeastOneTerm)
	{
		thrower("ERROR: No terms detected in expression! Expression was " + expression + "Make sure a set of "
				"parenthesis surrounds the expression.\n");
	}
	if (terms.size() != 1)
	{
		std::string msg("ERROR: Math evaluation failed! After a complete run-through of a term, there was more than one "
						"element left! Terms were:\n");
		std::string termString;
		for (auto term : terms)
		{
			termString += term + "\n";
		}
		thrower(msg + termString);
	}

	try
	{
		resultOfReduction = std::stod(terms[0]);
	}
	catch (std::invalid_argument&)
	{
		std::string msg("ERROR: Math evaluation failed! Final result failed to reduce to double! Original Expression was "
						+ expression + ". Result of reduction string was \"" + terms[0] + "\" \r\nVariables: \r\n");
		for (auto var : vars)
		{
			msg += ", " + var.name;
		}
		thrower(msg);
	}
	return resultOfReduction;
}

