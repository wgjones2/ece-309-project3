// src/tools/calculator_eval.cpp — YOU IMPLEMENT (spec §4.6).
//
// calc_evaluate: evaluate a well-formed token list with two stacks
// (one for numbers, one for operators and '('). See spec §4.6 and the
// worked trace in Appendix C. std::stack or std::vector are both fine.

#include "tools/calculator.h"
#include <stack>

////////////////////////////////////////////////////////////////////////////////////////////////////
//Precedence helper for operators:
//times and divide are 2, plus and minus are 1, and a left parenthesis is 0 undefined is -1
namespace {
	int precedence(CalcToken::Kind kind) { //check the precedence of the operator
		if (kind == CalcToken::Kind::Times || kind == CalcToken::Kind::Divide) { //if the operator is times or divide
			return 2; //times and divide have precedence 2
		} else if (kind == CalcToken::Kind::Plus || kind == CalcToken::Kind::Minus) { //if the operator is plus or minus
			return 1; //plus and minus have precedence 1
		} else if (kind == CalcToken::Kind::LParen) { //if the operator is a left parenthesis
			return 0; //left parenthesis has precedence 0
		}
		return -1; //otherwise, the operator has no defined precedence - return -1
	}


// Apply helper:
//applies the operator on top of the ops stack to the top two values on the values stack.
//returns true if successful, false if there is an error (malformed expression or division by zero).
bool apply(std::stack<double>& values, std::stack<CalcToken::Kind>& ops, std::string* error) { //take in the values stack, the ops stack, and the error string
	if (ops.empty() || values.size() < 2) { //if the ops stack is empty or there are fewer than two values
		*error = "malformed expression"; //throw an error: malformed expression
		return false; //exit early due to malformed expression
	}
	CalcToken::Kind op = ops.top(); //get the operator from the top of the ops stack
	ops.pop(); //remove the operator from the ops stack
	double right = values.top(); //get the right value from the top of the values stack
	values.pop(); //remove the right value from the values stack
	double left = values.top(); //get the left value from the top of the values stack
	values.pop(); //remove the left value from the values stack
	switch (op) { //depending on op:
		case CalcToken::Kind::Plus: //if the operator is plus
			values.push(left + right); //push the result of left + right onto the values stack
			break;
		case CalcToken::Kind::Minus: //if the operator is minus
			values.push(left - right); //push the result of left - right onto the values stack
			break;
		case CalcToken::Kind::Times: //if the operator is times
			values.push(left * right); //push the result of left * right onto the values stack
			break;
		case CalcToken::Kind::Divide: //if the operator is divide
			if (right == 0.0) { //check for division by zero
				*error = "division by zero"; //throw an error: division by zero
				return false; //exit early due to division by zero
			}
			values.push(left / right); //otherwise, push the result of left / right onto the values stack
			break;
		case CalcToken::Kind::LParen: //if the operator is a left parenthesis (one that was never closed)
			*error = "malformed expression"; //throw an error: malformed expression
			return false; //exit early due to malformed expression
		default: //error by default: unknown operator
			*error = "malformed expression";
			return false;
	}
	return true; //true for successful application of the operator - any fail breaks early
}
} // namespace end
////////////////////////////////////////////////////////////////////////////////////////////////////



bool calc_evaluate(const std::vector<CalcToken>& tokens, double* out, std::string* error) {
	//make two stacks: one for values (doubles) and one for operators (CalcToken::Kind)
	//any time the apply helper returns false, return false right away
	//go through the tokens one at a time with a loop (no recursion):
		//number: push its value onto the values stack
		//left parenthesis: push it onto the ops stack
		//right parenthesis: apply operators until the top of the ops stack is a left parenthesis
			//if the ops stack runs out first, set the error to "malformed expression" and return false
			//pop the left parenthesis off the ops stack
		//plus, minus, times, or divide: while the ops stack is not empty and its top has a
		//precedence greater than or equal to this operator's, apply it
			//then push this operator onto the ops stack
	//after the loop, apply every operator left on the ops stack
	//if there is not exactly one value left, set the error to "malformed expression" and return false
	//store the last value in out and return true
    std::stack<double> values;
    std::stack<CalcToken::Kind> ops;
}
