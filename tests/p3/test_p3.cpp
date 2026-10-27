// tests/p3/test_p3.cpp — YOUR P3 TEST SUITE.
//
// The five tests below are worked examples. Keep them, and add your own
// until you have AT LEAST 15 (spec §6 lists what must be covered).
// Name every test <area>_<what>: reg_ for the registry, ws_ for the
// workspace, calc_ for the calculator.
//
// Build & run:   cmake --build build && ./build/test_p3
// One area:      ./build/test_p3 reg_        (names starting with "reg_")
// One test:      ./build/test_p3 =reg_add_and_find

#include "../mini_test.h"
#include "../test_helpers.h"     // TempDir, write_text, args1, ...
#include "tools/calculator.h"
#include "tools/tool_registry.h"
#include "tools/workspace.h"
#include <algorithm>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace fs = std::filesystem; // for convenience

namespace {

// A tool whose name you choose — use it to fill the registry in tests.
class NamedTool : public Tool {
public:
    explicit NamedTool(std::string n) : n_(std::move(n)), schema_(JsonValue::object()) {}
    std::string name() const override { return n_; }
    std::string description() const override { return "test tool"; }
    const JsonValue& schema() const override { return schema_; }
    ToolResult invoke(const JsonValue&, ToolContext&) override { return ToolResult::success(n_); }
private:
    std::string n_;
    JsonValue   schema_;   // an empty JSON object: {}
};

// Tokenize with the provided tokenizer, then run YOUR calc_evaluate.
bool eval(const std::string& expr, double* value, std::string* error) {
    std::vector<CalcToken> tokens;
    if (!calc_tokenize(expr, &tokens, error)) return false;
    return calc_evaluate(tokens, value, error);
}

}  // namespace

// ---------------------------------------------------------------- examples given

TEST(reg_add_and_find) {
    ToolRegistry r;
    auto t = std::make_unique<NamedTool>("alpha");
    Tool* raw = t.get();                        // remember the address before moving
    REQUIRE(r.add(std::move(t)));
    CHECK(r.size() == 1);
    CHECK(r.find("alpha") == raw);              // same object, not a copy
    CHECK(r.find("beta") == nullptr);
}

TEST(reg_duplicate_keeps_original) {
    ToolRegistry r;
    auto first = std::make_unique<NamedTool>("alpha");
    Tool* original = first.get();
    REQUIRE(r.add(std::move(first)));
    CHECK(!r.add(std::make_unique<NamedTool>("alpha")));
    CHECK(r.find("alpha") == original);
    CHECK(r.size() == 1);
}

TEST(ws_rejects_parent_escape) {
    TempDir d;
    Workspace ws(d.path());
    fs::path out;
    std::string reason;
    REQUIRE(ws.resolve("notes/todo.txt", &out, &reason));   // sanity: accepts *something*
    CHECK(out == ws.root() / "notes" / "todo.txt");
    CHECK(!ws.resolve("../etc/passwd", &out, &reason));
    CHECK(!reason.empty());
}

TEST(calc_precedence) {
    double v = 0;
    std::string err;
    REQUIRE(eval("2 + 3 * 4", &v, &err));
    CHECK(v == 14);
}

TEST(calc_division_by_zero) {
    double v = 0;
    std::string err;
    CHECK(!eval("1 / 0", &v, &err));
    CHECK(err == "division by zero");
}

// ------------------------------------------------------------- your tests
//
// Still to write (spec §6 has the full list). Suggested names:
//   reg_null_rejected, reg_for_each_visits_each_once
//   reg_10000_tools               REQUIRED: print bucket_count / load_factor /
//                                 max_probe_length, check the bound
//   reg_alternative_hash          REQUIRED: longest chain of a weak hash
//   ws_<case>                     REQUIRED: every row of the table in spec §4.4
//   calc_<case>                   left-to-right, parentheses, negatives

// registry tests

//Test that the registry rejects a null tool:
TEST(reg_null_rejected) {
	ToolRegistry r; //make an empty registry
	CHECK(!r.add(nullptr)); //check that adding nullptr returns false
	CHECK(r.size() == 0); //check that the size is still 0
}


//Test registry for visiting each tool exactly once:
TEST(reg_for_each_visits_each_once) {
	//make a registry and add three NamedTools named "a", "b", and "c"
	ToolRegistry r;
	REQUIRE(r.add(std::make_unique<NamedTool>("a"))); // REQUIRE is used to ensure the add succeeds
	REQUIRE(r.add(std::make_unique<NamedTool>("b")));
	REQUIRE(r.add(std::make_unique<NamedTool>("c")));
	//call for_each with a lambda that pushes each tool's name into a vector
	std::vector<std::string> names; // vector to hold the names of the tools
	r.for_each([&names](const Tool& t) {
		names.push_back(t.name());
	});
	//sort the vector and check that it holds exactly "a", "b", and "c"
	std::sort(names.begin(), names.end());//sort the names alphabetically
	CHECK((names == std::vector<std::string>{"a", "b", "c"})); //check that the order is correct - exact match (extra parentheses keep the commas out of the macro)
}


//Test the registry with 10,000 tools:
TEST(reg_10000_tools) {
	ToolRegistry r; //make an empty registry
	REQUIRE(r.add(std::make_unique<NamedTool>("tool_0"))); //add "tool_0" and save the pointer that find("tool_0") returns
	const Tool* tool_0_ptr = r.find("tool_0"); //init
	REQUIRE(tool_0_ptr != nullptr); // ensure that find("tool_0") returned a valid pointer
	for (int i=1; i<10000; ++i) { //for all tools "tool_1" through "tool_9999"
		std::string name = "tool_" + std::to_string(i); // construct the tool name "tool_i"
		bool added = r.add(std::make_unique<NamedTool>(name)); // add the tool to the registry
		REQUIRE(added); // require each add to succeed
		CHECK(r.load_factor() <= 1.0); // check that the load factor is at most 1.0 after every add
	}
	for (int i=0; i<10000; ++i) { //for every tool "tool_0" through "tool_9999"
		std::string name = "tool_" + std::to_string(i); //construct the tool name "tool_i"
		const Tool* t = r.find(name); //find the tool by name
		CHECK(t != nullptr); //make sure find() returned a valid pointer
	}
	//check that find("tool_0") still returns the pointer saved at the start:
	CHECK(r.find("tool_0") == tool_0_ptr);
	//check that max_probe_length() is at most 64:
	CHECK(r.max_probe_length() <= 64);
	//print bucket_count(), load_factor(), and max_probe_length():
	std::cout << "bucket_count: " << r.bucket_count() << ", load_factor: " << r.load_factor() << ", max_probe_length: " << r.max_probe_length() << std::endl;
}


//Test how the weak hash s[0] + s.size() would spread the same 10,000 names:
TEST(reg_alternative_hash) {
	//fill a registry with "tool_0" through "tool_9999" and get its final bucket count:
	ToolRegistry r; //make an empty registry
	for (int i=0; i<10000; ++i) { //for all tools "tool_0" through "tool_9999"
		std::string name = "tool_" + std::to_string(i); //construct the tool name "tool_i"
		bool added = r.add(std::make_unique<NamedTool>(name)); //check if added
		REQUIRE(added);
	}
	size_t bucket_count = r.bucket_count(); //get the final bucket count
	std::vector<int> counters(bucket_count, 0); //one counter per bucket, all starting at 0
	//for each name, add one to the counter at (first character + name length) % bucket count
	for (int i=0; i<10000; ++i) { //for each name "tool_0" through "tool_9999"
		std::string name = "tool_" + std::to_string(i); //construct the tool name "tool_i"
		std::size_t first_char = static_cast<unsigned char>(name[0]); //get the first character as an unsigned value
		std::size_t weak = first_char + name.length(); //compute the weak hash
		std::size_t bucket = weak % bucket_count; //compute the bucket index
		++counters[bucket]; //increment the counter for that bucket
	}
	//find the largest counter (the longest chain the weak hash would make)
	int max_counter = 0; //set a default value for the largest counter - start at 0
	for (std::size_t i = 0; i < counters.size(); ++i) { //for all buckets...
		if (counters[i] > max_counter) { //check if the counter is larger than max_counter
			max_counter = counters[i]; //change max counter if it is
		}
	}
	//print it next to the registry's real max_probe_length()
	std::cout << "weak hash max chain: " << max_counter << ", real max_probe_length: " << r.max_probe_length() << std::endl;
	std::size_t weak_longest = static_cast<std::size_t>(max_counter); //convert the largest counter to size_t for comparison
	CHECK(weak_longest >= r.max_probe_length()); //the weak hash's longest chain should be at least as long as the real max probe length
}

// workspace tests
//these cover the rows of the spec §4.4 table
//("notes/todo.txt" and "../etc/passwd" are already in the example above)
//each test makes a TempDir, a Workspace rooted at the TempDir's path,
//an fs::path for the output, and a string for the reason
//for an accepted path, require resolve() to return true and check the output path
//for a rejected path, check that resolve() returns false and that the reason is not empty

TEST(ws_accepts_dot_and_double_slash) {
	//"./notes//todo.txt" is accepted as root / "notes" / "todo.txt"
}

TEST(ws_accepts_dot) {
	//"." is accepted as root
}

TEST(ws_accepts_inner_dotdot) {
	//"a/../b" is accepted as root / "b"
}

TEST(ws_accepts_dotdot_back_to_root) {
	//"a/.." is accepted as root
}

TEST(ws_accepts_missing_path) {
	//"new/dir/file.txt" (which does not exist) is accepted as root / "new" / "dir" / "file.txt"
}

TEST(ws_accepts_dots_in_name) {
	//"my..notes.txt" is accepted as root / "my..notes.txt"
}

TEST(ws_rejects_empty) {
	//"" is rejected
}

TEST(ws_rejects_nul) {
	//std::string("notes") + '\0' + "/x" is rejected
	//(a plain string literal would be cut off at the '\0')
}

TEST(ws_rejects_absolute) {
	//"/etc/passwd" is rejected
}

TEST(ws_rejects_dotdot) {
	//".." is rejected
}

TEST(ws_rejects_climb_past_root) {
	//"a/../../b" is rejected
}

TEST(ws_rejects_leave_and_return) {
	//root the Workspace at the TempDir's path / "ws" for this test
	//"notes/../../ws/notes/todo.txt" is rejected, even though it ends up back inside the workspace
}

// calculator tests
//these cover the rows of the spec §4.6 table plus two more
//("2 + 3 * 4" and "1 / 0" are already in the examples above)
//each test makes a double for the value and a string for the error,
//requires eval() to return true, and checks the value

TEST(calc_left_to_right) {
	//"10 - 4 - 3" gives 3
}

TEST(calc_left_to_right_division) {
	//"100 / 10 / 5" gives 2
}

TEST(calc_parentheses) {
	//"(2 + 3) * 4" gives 20
}

TEST(calc_parentheses_then_subtract) {
	//"2 * (3 + 4) - 5" gives 9
}

TEST(calc_negatives) {
	//"-3 * -3" gives 9
}

TEST(calc_decimal_result) {
	//"7 / 2" gives 3.5
}

TEST(calc_division_by_zero_in_parens) {
	//"1 / (2 - 2)" makes eval() return false with the error "division by zero"
}

TEST(calc_long_sum) {
	//build a string of "1" followed by 20,000 copies of "+1"
	//it gives 20001 (this checks that the evaluator uses a loop and not recursion)
}

TEST(calc_deep_nesting) {
	//build a string of 5,000 "(" characters, then "1", then 5,000 ")" characters
	//it gives 1
}

int main(int argc, char** argv) { return mt::run_all(argc, argv); }
