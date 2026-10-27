// src/tools/workspace.cpp — YOU IMPLEMENT (spec §4.4).
//
// Workspace::resolve: a purely lexical check, steps 1–6 of the spec.
// Do not touch the filesystem here (no exists(), canonical(), ...).
// The constructor, root() and check_symlinks() are provided in
// workspace_provided.cpp.

#include "tools/workspace.h"

namespace fs = std::filesystem;

bool Workspace::resolve(std::string_view user_path, fs::path* out, std::string* reason) const {
	//step 1: if the path is empty, set the reason to "empty path" and return false
	if (user_path.empty()) { //if the user path is empty
		*reason = "empty path"; //mark the reason as empty path
		return false; // return false if the path is empty
	}
	//step 2: if the path contains a '\0' character, set the reason to "path contains NUL" and return false
	for (std::size_t i = 0; i < user_path.size(); ++i) {
		if (user_path[i] == '\0') { //if the path contains a NUL character
			*reason = "path contains NUL"; //mark the reason as path contains NUL
			return false; //return false
		}
	}
	//step 3: build an fs::path from the text of the path
	std::string path_str(user_path); //convert the user path to a std::string
	fs::path path(path_str); //build an fs::path from the text of the path
	if (path.is_absolute() || path.has_root_path()) { //if it is absolute or has a root path
		*reason = "absolute path"; //mark the reason as absolute path
		return false; //return false
	}
	//step 4: normalize the path with lexically_normal()
	fs::path normalized = path.lexically_normal(); //normalize the path with lexically_normal()
	//step 5: if the first part of the normalized path is "..", set the reason to "path escapes workspace" and return false
	auto it = normalized.begin(); //get an iterator to the first part of the normalized path
	fs::path first_part = (it != normalized.end()) ? *it : ""; //get the first part of the normalized path
	if (first_part == "..") { //if the first part is ".."
		*reason = "path escapes workspace"; //mark the reason as path escapes workspace
		return false; //return false
	}
	//step 6: if the normalized path is just ".", the result is root(), otherwise join root() with the normalized path
    if (normalized == ".") { //if the normalized path is just "."
        *out = root(); //set the result to root()
    } else { //otherwise,
        *out = root() / normalized; //join root() with the normalized path
    }
    return true;//return true if the path was successfully resolved
}
