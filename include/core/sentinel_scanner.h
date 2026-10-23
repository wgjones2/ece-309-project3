// include/core/sentinel_scanner.h
//
// SentinelScanner: finds the stop marker (the "sentinel") in a stream of
// text that arrives in pieces ("chunks") of any size.
//
// The model streams its reply a few characters at a time, and the chunk
// boundaries can cut the sentinel "<|end_conversation|>" anywhere, e.g.
//     "Goodbye.<|end_"   followed by   "conversation|>"
// The scanner hands back the text that is definitely NOT part of the
// sentinel (safe to print right away), holds back only the few characters
// at the end that could still turn out to be the start of the sentinel,
// and reports when the whole sentinel has been seen.
//
// Memory stays bounded: pending_ never holds more than sentinel.size() - 1
// characters, however long the stream is. The proof is in
// docs/design-log-p2.md.

#pragma once  // Process this header only once per .cpp file.

#include <cstddef>      // For std::size_t.
#include <string>       // For std::string.
#include <string_view>  // For std::string_view (a read-only view of characters stored somewhere else).

class SentinelScanner {
public:
    // Creates a scanner that looks for the given sentinel text.
    // "explicit" stops a plain string from being turned into a
    // SentinelScanner by accident.
    explicit SentinelScanner(std::string sentinel);

    // What feed() and flush() return.
    struct Out {
        std::string safe_text;  // Text that is NOT part of the sentinel: safe to print now.
        bool sentinel_found;    // True if this call completed the sentinel.
    };

    // Feed the next chunk. Returns text guaranteed NOT to be part of
    // the sentinel (safe to print immediately) and whether the
    // sentinel has now been fully seen.
    //
    // Once sentinel_found is true the reply is over: anything after the
    // sentinel in that chunk is dropped, and the caller should stop
    // feeding (the provided Harness does exactly that).
    Out feed(std::string_view chunk);

    // Call once, after the stream ends, to release any text still
    // being held back.
    Out flush();

    // How many characters are being held back right now. Not part of the
    // spec's required interface: added so the tests can check that pending_
    // never grows past sentinel.size() - 1. It only reads a value.
    std::size_t pending_size() const noexcept;

private:
    std::string sentinel_;  // The marker we are looking for.
    std::string pending_;   // holds back at most sentinel_.size() - 1
                            // trailing characters that could still
                            // become the start of the sentinel
};
