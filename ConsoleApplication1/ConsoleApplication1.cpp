// ConsoleApplication1.cpp : Defines the entry point for the console application.
//


#include "stdafx.h"
#include "boost/algorithm/string/replace.hpp"
#include <boost/algorithm/string.hpp>
#include <vector>
#include <iostream>
#include <boost/tokenizer.hpp>

int main()
{
	std::cout << "Welcome to my little calculator. \nIt can understand and simplify algebraic expressions. \n"
		<< "It supports parentheses, addition, subtraction, multiplication, and division.\n"
		<< "Inputs must be surrounded by parentheses. For example:\n"
		<< "(5+4*(-3+2)).\n\n";
	while (true)
	{
		std::string workingExp;
		std::cout << "Enter an expression for evaluation:\n";
		std::getline(std::cin, workingExp);
		std::vector<std::string> terms;

		// separate terms out.
		boost::char_separator<char> sep("", " \t+-*/()"); // specify only the kept separators
		boost::tokenizer<boost::char_separator<char>> tokens(workingExp, sep);
		//boost::split(terms, workingExp, boost::is_any_of("\t +-*/()"));
		for (std::string t : tokens)
		{
			if (t != " " && t != "\t")
			{
				// don't include whitespace.
				terms.push_back(t);
			}
		}
		// do math.
		bool parenthesisExists = true;
		bool multExists;
		bool addExists;
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
			unsigned long long count = 0, leftPos = 0, rightPos = 0;
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
					std::cout << "ERROR: Unmatched \"(\" in math expression!\n";
					break;
				}
				rightmostParenthesisTerms = std::vector<std::string>(&terms[leftPos+1], &terms[rightPos+1]);
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
					std::cout << "ERROR: Math evaluation failed, there are parenthesis in an innermost term which should not"
						" have parenthesis in it! This is a bug.";
				}
			}
			/// find mult do mult
			// this can be done as just a scan from left to right.
			count = 0;
			for (int count = 0; count < rightmostParenthesisTerms.size(); count++)
			{
				if (rightmostParenthesisTerms[count] == "*" || rightmostParenthesisTerms[count] == "/")
				{
					std::string individualResult;
					double leftTerm = 0, rightTerm = 0;
					try
					{
						if (count <= 0)
						{
							std::cout << "ERROR: Operator " + rightmostParenthesisTerms[count] + " has no value on its"
								" left!";
						}
						leftTerm = std::stod(rightmostParenthesisTerms[count - 1]);
					}
					catch (std::invalid_argument& err)
					{
						std::cout << "ERROR: Tried and failed to convert string " + rightmostParenthesisTerms[count - 1]
							+ " to a double for multiplication/division!";
					}
					try
					{
						rightTerm = std::stod(rightmostParenthesisTerms[count + 1]);
					}
					catch (std::invalid_argument& err)
					{
						std::cout << "ERROR: Tried and failed to convert string " + rightmostParenthesisTerms[count + 1]
							+ " to a double for multiplication/division!";
					}
					// caliculate the result
					if (rightmostParenthesisTerms[count] == "/")
					{
						individualResult = std::to_string(leftTerm / rightTerm);
					}
					else
					{
						individualResult = std::to_string(leftTerm * rightTerm);
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
			for (int count = 0; count < rightmostParenthesisTerms.size(); count++)
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
								std::cout << "ERROR: Operator " + rightmostParenthesisTerms[count] + " has no value on its"
									" left!";
							}
							leftTerm = std::stod(rightmostParenthesisTerms[count - 1]);
						}
					}
					catch (std::invalid_argument& err)
					{
						std::cout << "ERROR: Tried and failed to convert string " + rightmostParenthesisTerms[count - 1]
							+ " to a double for addition/subtraction!";
					}
					try
					{
						rightTerm = std::stod(rightmostParenthesisTerms[count + 1]);
					}
					catch (std::invalid_argument& err)
					{
						std::cout << "ERROR: Tried and failed to convert string " + rightmostParenthesisTerms[count + 1]
							+ " to a double for addition/subtraction!";
					}
					// caliculate the result
					if (rightmostParenthesisTerms[count] == "+")
					{
						individualResult = std::to_string(leftTerm + rightTerm);
					}
					else
					{
						individualResult = std::to_string(leftTerm - rightTerm);
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
				std::cout << ("ERROR: Math evaluation failed! After a complete run-through of a term, there was more than one "
							  "element left! Terms were:\n");
				std::string termString;
				for (auto term : terms)
				{
					termString += term + "\n";
				}
				std::cout << termString;
			}
			// and replace parenthesis term in the main set of terms.
			std::string result = rightmostParenthesisTerms[0];
			terms.erase(terms.begin() + leftPos, terms.begin() + (rightPos+2));
			terms.insert(terms.begin() + leftPos, result);
		}
		if (!atLeastOneTerm)
		{
			std::cout << "ERROR: No terms detected in expression! Make sure a set of parenthesis surrounds the "
						 "expression.\n";
			continue;
		}
		if (terms.size() != 1)
		{
			std::cout << ("ERROR: Math evaluation failed! After a complete run-through of a term, there was more than one "
						  "element left! Terms were:\n");
			std::string termString;
			for (auto term : terms)
			{
				termString += term + "\n";
			}
			std::cout << termString;
		}
		double resultOfReduction = 0;
		try
		{
			resultOfReduction = std::stod(terms[0]);
		}
		catch (std::invalid_argument& err)
		{
			std::cout << ("ERROR: Math evaluation failed! Final result failed to reduce to double!");
		}
		std::cout << "\t\t= " << resultOfReduction << "\n\n";
	}
	return 0;
}
