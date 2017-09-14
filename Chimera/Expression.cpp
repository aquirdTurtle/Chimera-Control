#include "stdafx.h"
#include "Expression.h"
#include <vector>
#include <boost/tokenizer.hpp>

Expression::Expression( )
{
	// in this case, the expression must be explicitly initialized using init() later.
}


Expression::Expression( std::string expressionString )
{
	expressionStr = expressionString;
}

/*
* proper order:
* deal with fuctions
- Find innermost function
- reduce argument expression
- apply function
- find next function
* reduce remaining math
*/


/*
Split the input into all math objects and variables. The separated such objects are stored and returned in a vector of
strings.
*/
std::vector<std::string> Expression::splitString( std::string workingString )
{
	//workingString = "(" + workingString + ")";
	std::vector<std::string> terms;
	// separate terms out.
	// specify only the kept separators
	boost::char_separator<char> sep( "", " \t+-*/()" );
	boost::tokenizer<boost::char_separator<char>> tokens( workingString, sep );
	for ( std::string t : tokens )
	{
		if ( t != " " && t != "\t" )
		{
			// don't include whitespace.
			terms.push_back( t );
		}
	}
	return terms;
}


void Expression::doMultAndDiv( std::vector<std::string>& terms )
{
	/// find mult do mult
	// this can be done as just a scan from left to right.
	for ( UINT count = 0; count < terms.size( ); count++ )
	{
		if ( terms[count] == "*" || terms[count] == "/" )
		{
			std::string individualResult;
			double leftTerm = 0, rightTerm = 0;
			try
			{
				if ( count <= 0 )
				{
					thrower( "ERROR: Operator " + terms[count] + " has no value on its left!" );
				}
				leftTerm = std::stod( terms[count - 1] );
			}
			catch ( std::invalid_argument& )
			{
				thrower( "ERROR: Tried and failed to evaluate string " + terms[count - 1]
						 + " to a double (error in for multiplication / division section)!" );
			}
			try
			{
				rightTerm = std::stod( terms[count + 1] );
			}
			catch ( std::invalid_argument& )
			{
				thrower( "ERROR: Tried and failed to convert string " + terms[count + 1] + " to a double (error in for"
						 " multiplication / division section)!" );
			}
			// calculate the result
			if ( terms[count] == "/" )
			{
				individualResult = str( leftTerm / rightTerm );
			}
			else
			{
				individualResult = str( leftTerm * rightTerm );
			}
			// replace the * expression with the result.
			terms.erase( terms.begin( ) + (count - 1), terms.begin( ) + (count + 2) );
			terms.insert( terms.begin( ) + (count - 1), individualResult );
			// this accounts for the fact that we just deleted several terms from the vector, making sure that
			// the function looks at the correct next term afterwards.
			count -= 1;
		}
	}
}


void Expression::doAddAndSub( std::vector<std::string>& terms )
{
	/// find add do add
	for ( UINT count = 0; count < terms.size( ); count++ )
	{
		if ( terms[count] == "+" || terms[count] == "-" )
		{
			std::string individualResult;
			double leftTerm = 0, rightTerm = 0;
			try
			{
				if ( count == 0 && terms[count] == "-" )
				{
					// this will convert (-1) to (0-1) which will evaluate correctly.
					leftTerm = 0;
					terms.insert( terms.begin( ), "0" );
					count = 1;
				}
				else
				{
					if ( count <= 0 )
					{
						thrower( "ERROR: Operator " + terms[count] + " has no value on its"
								 " left!" );
					}
					leftTerm = std::stod( terms[count - 1] );
				}
			}
			catch ( std::invalid_argument& )
			{
				thrower( "ERROR: Tried and failed to evaluate string " + terms[count - 1]
						 + " to a double (error in for addition/subtraction section)!" );
			}
			try
			{
				rightTerm = std::stod( terms[count + 1] );
			}
			catch ( std::invalid_argument& )
			{
				thrower( "ERROR: Tried and failed to evaluate string " + terms[count + 1]
						 + " to a double (error in for addition/subtraction section)!" );
			}
			// caliculate the result
			if ( terms[count] == "+" )
			{
				individualResult = str( leftTerm + rightTerm );
			}
			else
			{
				individualResult = str( leftTerm - rightTerm );
			}
			// replace the expression with the result.
			terms.erase( terms.begin( ) + (count - 1),
						 terms.begin( ) + (count + 2) );
			terms.insert( terms.begin( ) + (count - 1), individualResult );
			// this accounts for the fact that we just deleted several terms from the vector, making sure that
			// the function looks at the correct next term afterwards.
			count -= 1;
		}
	}
}


// reduce handles reduction of standard math operations (i.e. not functions) only. 
double Expression::reduce( std::vector<std::string> terms )
{
	bool parenthesisExists = true;
	// parenthesis loop
	while ( parenthesisExists )
	{
		// this will store all the terms in the right-most parenthesis enclosure.
		// arguably it's a little more intuitive, as to how a person would actually simplify,
		// to do the innermost first. But it shouldn't make a mathematical difference, and this
		// is easier to code.
		std::vector<std::string> rightmostParenthesisTerms;
		bool leftExists = false;
		// find rightmost parenthesis
		UINT count = 0;
		int leftPos = 0, rightPos = 0;
		for ( auto& elem : terms )
		{
			if ( elem == "(" )
			{
				// just always reset it
				leftExists = true;
				leftPos = count;
			}
			count++;
		}
		//
		if ( leftExists )
		{
			rightmostParenthesisTerms = std::vector<std::string>( &terms[leftPos] + 1, &terms.back( ) + 1 );
			bool rightExists = false;
			count = 0;
			for ( auto& elem : rightmostParenthesisTerms )
			{
				if ( elem == ")" )
				{
					// rightPos is set in terms of the original vector for future reference,
					// not the new substring I just created.
					rightPos = leftPos + count;
					rightExists = true;
					break;
				}
				count++;
			}
			if ( !rightExists )
			{
				thrower( "ERROR: Unmatched \"(\" in math expression!\r\n" );
			}
			rightmostParenthesisTerms = std::vector<std::string>( &terms[leftPos + 1], &terms[rightPos + 1] );
			// now I have a term which I can analyze.
		}
		else
		{
			// no left parenthesis left, so I'm finished.
			parenthesisExists = false;
			break;
		}
		// I now I have a subvector with no parenthesis in it. Verify that there's no parenthesis in it.
		for ( auto elem : rightmostParenthesisTerms )
		{
			if ( elem == "(" || elem == ")" )
			{
				thrower( "ERROR: Math evaluation failed, there are parenthesis in an innermost term which should not"
						 " have parenthesis in it! This is a bug." );
			}
		}

		doMultAndDiv( rightmostParenthesisTerms );

		doAddAndSub( rightmostParenthesisTerms );

		// check that the list of terms has been reduced.
		if ( rightmostParenthesisTerms.size( ) != 1 )
		{
			std::string msg( "ERROR: Math evaluation failed! After a complete run-through of a term, there was more than one "
							 "element left! Terms were:\n" );
			std::string termString;
			for ( auto term : terms )
			{
				termString += term + "\n";
			}
			thrower( msg + termString );
		}
		// and replace parenthesis term in the main set of terms.
		std::string result = rightmostParenthesisTerms[0];
		terms.erase( terms.begin( ) + leftPos, terms.begin( ) + (rightPos + 2) );
		terms.insert( terms.begin( ) + leftPos, result );
	} // end parenthesis loop
	if ( terms.size( ) != 1 )
	{
		thrower( "ERROR: \"reduce\" function failed to reduce its arguments to a single term!" );
	}
	double finalResult;
	try
	{
		finalResult = std::stod( terms[0] );
	}
	catch ( std::invalid_argument& )
	{
		thrower( "ERROR: \"reduce\" failed convert its reduction to a double! Result of reduction was " + terms[0] );
	}
	return finalResult;
}


// this function evaluates functions and leaves the input as only standard math operations and objects. There are many parallels between
// this function and reduce.
void Expression::evaluateFunctions( std::vector<std::string>& terms )
{
	// list of supported functions.
	std::vector<std::string> functionList = { "sin", "cos", "exp", "ln", "log10" };
	bool functionExists = true;
	while ( functionExists )
	{
		std::vector<std::string> functionArgUnevaluated;
		std::string funcName;
		UINT count = 0;
		int funcPos = -1;
		// find the rightmost function.
		for ( auto& elem : terms )
		{
			for ( auto func : functionList )
			{
				if ( elem == func )
				{
					// just always reset it
					funcPos = count;
					funcName = elem;
				}
			}
			count++;
		}

		if ( funcPos == -1 )
		{
			// then the loop found no functions.
			functionExists = false;
			break;
		}
		if ( funcPos >= terms.size( ) - 3 )
		{
			// the function position needs to have at least 3 terms after it (2 for parenthesis, 1 for the arg). 
			// the last value is at position terms.size() - 1.
			thrower( "ERROR: function " + funcName + " detected with no arguments!" );
		}
		if ( terms[funcPos + 1] != "(" )
		{
			thrower( "ERROR: function " + funcName + " detected with enclosing parenthesis () to hold its arguments! "
					 "all function arguments must be enclosed in parenthesis." );
		}

		int funcArgPosLeft = funcPos + 1;
		int funcArgPosRight = 0;
		// find the function argument's enclosing parenthesis
		// first make a subvector that holds everything past the function's first parenthesis.
		std::vector<std::string> maxArg = std::vector<std::string>( &terms[funcArgPosLeft] + 1, &terms.back( ) + 1 );
		bool closingParenthesisExists = false;
		int subParenthesisCount = 0;
		count = 0;
		for ( auto& elem : maxArg )
		{
			if ( elem == "(" )
			{
				subParenthesisCount++;
			}
			else if ( elem == ")" )
			{

				if ( subParenthesisCount == 0 )
				{
					// then I've found the closing parenthesis of the function arguments.
					funcArgPosRight = funcArgPosLeft + count;
					closingParenthesisExists = true;
					break;
				}
				subParenthesisCount--;
			}
			count++;
		}
		if ( !closingParenthesisExists )
		{
			thrower( "Function argument's enclosing () were unclosed! The initial ( didn't have a matching )!" );
		}
		// now I have a term which I can analyze.
		functionArgUnevaluated = std::vector<std::string>( &terms[funcArgPosLeft], &terms[funcArgPosRight + 2] );

		// reduce whatever's in the function argument
		double functionArg = reduce( functionArgUnevaluated );

		// evaluate the function.
		double functionResult;

		// { "sin", "cos", "exp", "ln", "log10" };
		if ( funcName == "sin" )
		{
			functionResult = sin( functionArg );
		}
		else if ( funcName == "cos" )
		{
			functionResult = cos( functionArg );
		}
		else if ( funcName == "exp" )
		{
			functionResult = exp( functionArg );
		}
		else if ( funcName == "ln" )
		{
			// log in cmath is the natural log.
			functionResult = log( functionArg );
		}
		else if ( funcName == "log10" )
		{
			functionResult = log10( functionArg );
		}
		std::string resultStr = str( functionResult, 13 );
		terms.erase( terms.begin( ) + funcPos, terms.begin( ) + (funcArgPosRight + 2) );
		terms.insert( terms.begin( ) + funcPos, resultStr );
	}
	// since only evaluating functions, terms may still be a full vector after this loop.
}


/*
Evaluate takes in an expression, which can be a combination of variables, standard math operations, and standard
math functions, and evaluates it to a double.
*/
double Expression::evaluate( key variationKey, UINT variation, std::vector<variableType>& vars )
{
	// make a constant copy of the original string to use during the evaluation.
	const std::string originalExpression( expressionStr );

	double resultOfReduction = 0;
	try
	{
		// try the simple thing.
		size_t num;
		resultOfReduction = std::stod( originalExpression, &num );
		if ( num != originalExpression.size( ) )
		{
			// ???
			thrower( "started with number" );
		}
		return resultOfReduction;
	}
	catch ( std::invalid_argument& ) {	/* that's fine, just means it needs actual reducing.*/ }
	catch ( Error& ) { /* Same. */ }

	std::vector<std::string> terms = splitString( "(" + originalExpression + ")" );

	// if not the default value (see header)
	if ( variation != -1 )
	{
		// substitute all variables within the expression.
		for ( auto& term : terms )
		{
			for ( auto variableVariationInfo : variationKey )
			{
				if ( term == variableVariationInfo.first )
				{
					if ( variableVariationInfo.second.first.size( ) == 0 )
					{
						thrower( "ERROR: Attmepting to use key that hasn't been generated yet!" );
					}
					term = str( variableVariationInfo.second.first[variation], 12 );
					// find the variable 
					bool foundVariable = false;
					for ( auto& variable : vars )
					{
						if ( variableVariationInfo.first == variable.name )
						{
							variable.active = true;
							foundVariable = true;
						}
					}
					if ( !foundVariable )
					{
						thrower( "ERROR: Variable existed in key but not in variable vector. Low level bug. Ask Mark." );
					}
				}
			}
		}
	}

	evaluateFunctions( terms );

	/// do math.
	return reduce( terms );
}

// this function checks whether the string "item" is usable as a double, either by direct reduction to double without
// variables, or if it is a variable.
void Expression::assertValid( std::vector<variableType>& vars )
{
	double value;
	try
	{
		value = evaluate();
	}
	catch ( Error& )
	{
		bool isVar = false;
		for ( UINT varInc = 0; varInc < vars.size( ); varInc++ )
		{
			if ( vars[varInc].name == expressionStr )
			{
				vars[varInc].active = true;
				isVar = true;
				break;
			}
		}
		if ( !isVar )
		{
			// check if its a usable math expression. I.e. is composed of numbers, variables, or math symbols.
			bool failed = false;
			std::vector<std::string> terms = splitString( expressionStr );
			for ( auto elem : terms )
			{
				if ( elem == "(" || elem == "+" || elem == "-" || elem == "*" || elem == "/" || elem == ")" )
				{
					// it's a valid math symbol.
					continue;
				}
				if ( elem == "sin" || elem == "cos" || elem == "exp" || elem == "ln" || elem == "log10" )
				{
					// it's a supported math function.
					continue;
				}

				try
				{
					value = std::stod( elem );
					continue;
				}
				catch ( std::invalid_argument& ) {/* term is not a double.*/ }


				isVar = false;
				for ( UINT varInc = 0; varInc < vars.size( ); varInc++ )
				{
					if ( vars[varInc].name == elem )
					{
						vars[varInc].active = true;
						isVar = true;
						break;
					}
				}
				if ( isVar )
				{
					continue;
				}
				// it reached the end, that means the term isn't a variable, isn't a math symbol, and isn't a double.
				// It's not usable then.
				failed = true;
				break;
			}

			if ( failed )
			{
				thrower( "ERROR: " + expressionStr + " is not a valid expression. It's not a double, a variable, "
						 "and it wont evaluate as a mathematical expression." );
			}
		}
	}
}
