// src/tools/calculator_eval.cpp — YOU IMPLEMENT (spec §4.6).
//
// calc_evaluate: evaluate a well-formed token list with two stacks
// (one for numbers, one for operators and '('). See spec §4.6 and the
// worked trace in Appendix C. std::stack or std::vector are both fine.

#include "tools/calculator.h"
#include <stack>

////////////////////////////////////////////////////////////////////////////////////////////////////
//Precedence helper for operators:
//times and divide are 2, plus and minus are 1, a left parenthesis is 0, and undefined is -1
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
		*error = "malformed expression"; //set the error (no exception): malformed expression
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
				*error = "division by zero"; //set the error (no exception): division by zero
				return false; //exit early due to division by zero
			}
			values.push(left / right); //otherwise, push the result of left / right onto the values stack
			break;
		case CalcToken::Kind::LParen: //if the operator is a left parenthesis (one that was never closed)
			*error = "malformed expression"; //set the error (no exception): malformed expression
			return false; //exit early due to malformed expression
		default: //error by default: unknown operator
			*error = "malformed expression";
			return false;
	}
	return true; //true for successful application of the operator - any failure returns early
}
} // namespace end
////////////////////////////////////////////////////////////////////////////////////////////////////


// Evaluates a sequence of CalcTokens and stores the result in "out".
//returns true if the evaluation is successful, false otherwise
//if an error occurs, 'error' is set to a descriptive message
//results should match the table in the log
bool calc_evaluate(const std::vector<CalcToken>& tokens, double* out, std::string* error) {
	std::stack<double> values; //stack of numbers
	std::stack<CalcToken::Kind> ops; //stack of operators and left parentheses
	for (std::size_t i = 0; i < tokens.size(); ++i) { //go through each token
		const CalcToken token = tokens[i]; //current token
		if (token.kind == CalcToken::Kind::Number) { //if the token is a number
			values.push(token.value); //push its value onto the values stack
		} else if (token.kind == CalcToken::Kind::LParen) { //if the token is a left parenthesis
			ops.push(token.kind); //push it onto the ops stack
		} else if (token.kind == CalcToken::Kind::RParen) { //if the token is a right parenthesis
			while (!ops.empty() && ops.top() != CalcToken::Kind::LParen) { //apply operators until the top is a left parenthesis
				if (!apply(values, ops, error)) { //apply pops the operator itself
					return false; //return false if apply fails
				}
			}
			if (ops.empty()) { //if there is no matching left parenthesis
				*error = "malformed expression";
				return false;
			}
			ops.pop(); //pop the left parenthesis
		//if the token is an operator (plus, minus, times, divide)
		} else if (token.kind == CalcToken::Kind::Plus || token.kind == CalcToken::Kind::Minus || token.kind == CalcToken::Kind::Times || token.kind == CalcToken::Kind::Divide) {
			while (!ops.empty() && precedence(ops.top()) >= precedence(token.kind)) { //while the top of the ops stack has greater or equal precedence
				if (!apply(values, ops, error)) { //apply pops the operator itself
					return false; //return false if apply fails
				}
			}
			ops.push(token.kind); //push the current operator
		} else {
			*error = "malformed expression";
			return false;
		}
	}
	//after the loop, apply every operator left on the ops stack
	while (!ops.empty()) { //while there are still operators on the stack
		if (!apply(values, ops, error)) { //apply pops the operator itself
			return false; //return false if apply fails
		}
	}
	//if there is not exactly one value left, set the error to "malformed expression" and return false
	if (values.size() != 1) {
		*error = "malformed expression";
		return false;
	}
	//store the last value in out and return true
	*out = values.top();
	return true;
}
