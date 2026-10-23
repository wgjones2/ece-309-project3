// src/sentinel_scanner.cpp
//
// Implementation of SentinelScanner (see include/core/sentinel_scanner.h).
//
// The approach from spec section 3.4: keep at most sentinel_.size() - 1
// characters in pending_. On each feed(), search pending_ + chunk for the
// sentinel. If it is not there, everything except the last
// sentinel_.size() - 1 characters is safe to print, and those last
// characters become the new pending_.

#include "core/sentinel_scanner.h"

#include <utility>  // For std::move.

// Constructor: remember the sentinel. Nothing is held back yet.
SentinelScanner::SentinelScanner(std::string sentinel)
    : sentinel_(std::move(sentinel)),  // Take over the caller's string instead of copying its characters again.
      pending_() {                     // Start with an empty hold-back buffer.
}

// Processes the next chunk of the stream.
SentinelScanner::Out SentinelScanner::feed(std::string_view chunk) {
    Out result;                     // The answer we will return (safe_text starts out empty).
    result.sentinel_found = false;  // Assume "not found" until we actually find it.

    // Step 1: the text to search is the characters held back last time
    // followed by the new chunk. A sentinel that was split across chunks is
    // back in one piece inside this string.
    std::string text = pending_;  // Start with the held-back characters...
    text += chunk;                // ...and add the new chunk (std::string can append a std::string_view).

    // Step 2: look for the whole sentinel anywhere in the text. find() returns
    // the index where it starts, or std::string::npos if it is not there.
    std::size_t position = text.find(sentinel_);

    if (position != std::string::npos) {
        // Step 3a: found it. Everything in front of it is ordinary text.
        result.safe_text = text.substr(0, position);  // The characters before the sentinel are safe to print.
        result.sentinel_found = true;                 // Tell the caller the reply is over.

        // The sentinel itself, and anything that came after it, is dropped:
        // the reply ends at the sentinel. Nothing is held back any more.
        pending_.clear();
        return result;
    }

    // Step 3b: not found. A sentinel that will finish in a LATER chunk needs
    // at least one character from that later chunk, so right now it can only
    // have started somewhere in the last (sentinel_.size() - 1) characters of
    // the text. Those characters must be held back. Everything in front of
    // them can never become part of a sentinel, so it is safe to print.
    //
    // (sentinel_.size() - 1 cannot go below zero here: an empty sentinel is
    // always "found" at position 0 by find(), so an empty sentinel never
    // reaches this line.)
    std::size_t hold_back = sentinel_.size() - 1;

    if (text.size() <= hold_back) {
        // The text is so short that ALL of it could still be the beginning of
        // a sentinel. Hold all of it back; nothing is safe to print yet.
        pending_ = text;
    } else {
        // Keep the last hold_back characters and release everything before them.
        std::size_t safe_length = text.size() - hold_back;  // How many characters are safe.
        result.safe_text = text.substr(0, safe_length);       // The front part: safe to print.
        pending_ = text.substr(safe_length);                  // The last hold_back characters: keep them.
    }

    // At this point pending_ holds at most hold_back = sentinel_.size() - 1
    // characters, whichever branch ran.
    return result;
}

// Called once, when the stream has ended.
SentinelScanner::Out SentinelScanner::flush() {
    Out result;  // The answer we will return.

    // No more characters will ever arrive, so the held-back text can never
    // grow into a sentinel. It is ordinary text: release all of it.
    result.safe_text = pending_;

    // pending_ is shorter than the sentinel, so it cannot contain a whole one.
    result.sentinel_found = false;

    // Nothing is held back any more.
    pending_.clear();

    return result;
}

// Returns how many characters are currently held back (used by the tests).
std::size_t SentinelScanner::pending_size() const noexcept {
    return pending_.size();
}
