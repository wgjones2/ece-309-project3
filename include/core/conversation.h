// include/core/conversation.h
//
// Conversation: the full history of a chat, stored as an ordered list of
// Messages (oldest first).
//
// It is our own growable array (std::vector is not allowed in this project).
// A Conversation owns exactly one block of heap memory holding Messages:
//
//     data_ ---> [ msg 0 | msg 1 | msg 2 | spare ]
//                 \____ size_ = 3 _____/
//                 \_______ capacity_ = 4 ________/
//
// When append() finds every slot in use, it moves the messages into a new
// array twice as large (growth factor 2), so appending is amortized O(1).
// The proof is in docs/design-log-p2.md.
//
// This class is the ONLY place in the project that uses raw new/delete.
// Because it owns heap memory, it follows the Rule of Five: it defines its
// own destructor, copy constructor, copy assignment, move constructor, and
// move assignment, so the memory is never leaked, freed twice, or shared by
// two Conversations by accident.

#pragma once  // Process this header only once per .cpp file.

#include "core/message.h"  // The Message type that we store.
#include <cstddef>         // For std::size_t (the standard type for sizes and indexes).

class Conversation {
public:
    // Empty conversation: size() == 0, no allocation yet.
    Conversation();

    // Releases all owned Message storage. No effect if already empty
    // (e.g. moved-from).
    ~Conversation();

    // Deep copy: allocates its own buffer and copies every Message.
    // this->begin() must differ from other.begin() afterward.
    Conversation(const Conversation& other);
    Conversation& operator=(const Conversation& other);

    // Steals other's buffer — no per-element copying. Afterward, other
    // must be left valid and empty (safe to destroy or reassign).
    Conversation(Conversation&& other) noexcept;
    Conversation& operator=(Conversation&& other) noexcept;

    // Appends m, growing the backing array if needed. Amortized O(1).
    // Growth strategy: when the array is full, its capacity doubles
    // (0 -> 1 -> 2 -> 4 -> 8 -> ...). See docs/design-log-p2.md.
    void append(Message m);

    // Number of messages currently stored.
    std::size_t size() const noexcept;

    // Number of message slots currently allocated (always >= size()).
    // Not part of the spec's required interface: added so the tests can
    // check the documented growth factor. It only reads a value.
    std::size_t capacity() const noexcept;

    // Bounds-checked access to message number i (0 = the oldest).
    // Throws std::out_of_range if i >= size().
    const Message& at(std::size_t i) const;

    // Range-for iteration, oldest message first. begin() == end() when
    // size() == 0.
    const Message* begin() const noexcept;
    const Message* end()   const noexcept;

private:
    Message*    data_ = nullptr;  // Start of our heap array (nullptr while nothing is allocated).
    std::size_t size_ = 0;        // How many slots hold real messages.
    std::size_t capacity_ = 0;    // How many slots the heap array has in total.
};
