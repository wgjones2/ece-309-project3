// src/tools/workspace.cpp — YOU IMPLEMENT (spec §4.4).
//
// Workspace::resolve: a purely lexical check, steps 1–6 of the spec.
// Do not touch the filesystem here (no exists(), canonical(), ...).
// The constructor, root() and check_symlinks() are provided in
// workspace_provided.cpp.

#include "tools/workspace.h"

namespace fs = std::filesystem;

bool Workspace::resolve(std::string_view /*user_path*/, fs::path* /*out*/,
                        std::string* reason) const {
	//step 1: if the path is empty, set the reason to "empty path" and return false
	//step 2: if the path contains a '\0' character, set the reason to "path contains NUL"
	//and return false
	//step 3: build an fs::path from the text of the path
	//if it is absolute or has a root path, set the reason to "absolute path" and return false
	//step 4: normalize the path with lexically_normal()
	//(this removes . parts and double slashes, and folds x/.. pairs)
	//step 5: if the first part of the normalized path is "..", set the reason to
	//"path escapes workspace" and return false
	//step 6: if the normalized path is just ".", the result is root()
	//otherwise the result is root() joined with the normalized path
	//store the result in out and return true
    *reason = "Workspace::resolve not implemented yet";   // TODO
    return false;
}
