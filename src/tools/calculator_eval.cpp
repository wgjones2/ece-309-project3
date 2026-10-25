// src/tools/calculator_eval.cpp — YOU IMPLEMENT (spec §4.6).
//
// calc_evaluate: evaluate a well-formed token list with two stacks
// (one for numbers, one for operators and '('). See spec §4.6 and the
// worked trace in Appendix C. std::stack or std::vector are both fine.

#include "tools/calculator.h"

#include <stack>

namespace {

// precedence helper
//takes an operator kind and returns its precedence
//times and divide are 2, plus and minus are 1, and a left parenthesis is 0

// apply helper
//takes the values stack, the ops stack, and the error string
//if the ops stack is empty or there are fewer than two values,
//set the error to "malformed expression" and return false
//pop the operator, then pop the right value, then pop the left value
//(the right value comes off first)
//for plus, minus, and times, push the result of left op right onto the values stack
//for divide, if the right value is 0, set the error to "division by zero" and return false
//otherwise push left divided by right onto the values stack
//if the operator is a left parenthesis (one that was never closed),
//set the error to "malformed expression" and return false
//return true

} // namespace

bool calc_evaluate(const std::vector<CalcToken>& /*tokens*/, double* /*out*/,
                   std::string* error) {
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
    *error = "calc_evaluate not implemented yet";   // TODO
    return false;
}
