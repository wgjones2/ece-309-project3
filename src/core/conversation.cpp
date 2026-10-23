// src/conversation.cpp
//
// Implementation of Conversation, our hand-written growable array of
// Messages (see include/core/conversation.h for the overview).
//
// This is the only file in the whole project that uses raw new[] and
// delete[]. Every new[] here is matched by exactly one delete[]:
//   - the destructor deletes the current array,
//   - copy assignment and move assignment delete the array being replaced,
//   - append() deletes the old array after moving into a bigger one,
//   - copy_messages() deletes its new array if copying fails part-way.

#include "core/conversation.h"

#include <stdexcept>  // For std::out_of_range, which at() throws for a bad index.
#include <string>     // For std::to_string, used to build at()'s error message.
#include <utility>    // For std::move.

// Everything inside an unnamed namespace is private to this .cpp file
// (the provided harness.cpp and main.cpp use the same pattern).
namespace {

// Creates a new heap array with room for `capacity` Messages, copies the
// first `count` Messages from `source` into it, and returns the new array.
// The caller becomes the owner of the array (and must delete[] it later).
//
// Both the copy constructor and the copy assignment operator use this, so
// the copying code is written only once.
//
// Exception safety: copying a Message copies its std::string, and that can
// throw std::bad_alloc if the program runs out of memory. If that happens
// part-way through, the half-filled array is deleted before the exception
// continues on to the caller, so no memory is leaked.
Message* copy_messages(const Message* source, std::size_t count, std::size_t capacity) {
    // Allocate the new array. new[] also runs the default constructor for
    // every slot (each one starts as an empty System Message). This is why
    // Message must have a default constructor.
    Message* copy = new Message[capacity];

    try {
        // Walk through the real messages, oldest first.
        for (std::size_t i = 0; i < count; ++i) {
            copy[i] = source[i];  // Copy assignment: slot i gets its own copy of the role and text.
        }
    } catch (...) {     // "..." means: catch any kind of exception.
        delete[] copy;  // Free the half-filled array so it is not leaked...
        throw;          // ...then re-throw the same exception to our caller.
    }

    return copy;  // Every message was copied successfully; hand the new array back.
}

}  // namespace

// ---------------------------------------------------------------------------
// Constructor and destructor
// ---------------------------------------------------------------------------

// Default constructor: an empty conversation that owns no memory yet.
Conversation::Conversation()
    : data_(nullptr),  // No array allocated yet...
      size_(0),        // ...no messages stored...
      capacity_(0) {   // ...and no slots available.
    // Nothing else to do: memory is only requested by the first append().
}

// Destructor: give back the memory we own.
Conversation::~Conversation() {
    // delete[] runs the destructor of every Message in the array (which frees
    // each message's text) and then frees the array itself. Deleting nullptr
    // does nothing, so this is also safe for an empty or moved-from
    // Conversation.
    delete[] data_;
}

// ---------------------------------------------------------------------------
// Copy operations (deep copy)
// ---------------------------------------------------------------------------

// Copy constructor: build a brand-new Conversation holding copies of every
// message in `other`.
Conversation::Conversation(const Conversation& other)
    : data_(nullptr),  // Start out empty; filled in below if other has an array.
      size_(0),
      capacity_(0) {
    // If other has no array, there is nothing to copy: stay empty (and
    // allocate nothing), just like a newly constructed Conversation.
    if (other.capacity_ == 0) {
        return;
    }

    // Make our OWN array holding copies of other's messages. We must never
    // simply copy the pointer other.data_ (a "shallow copy"): both objects
    // would then delete[] the same array when destroyed, which is a double
    // free.
    data_ = copy_messages(other.data_, other.size_, other.capacity_);

    size_ = other.size_;          // Same number of messages...
    capacity_ = other.capacity_;  // ...and the same capacity, so the copy is an exact clone.
}

// Copy assignment: throw away our current messages and replace them with
// copies of other's (used by statements like "a = b;").
Conversation& Conversation::operator=(const Conversation& other) {
    // Self-assignment ("a = a;"): we already hold exactly these messages, so
    // there is nothing to do.
    if (this == &other) {
        return *this;
    }

    // Step 1: build the new array FIRST, while our old array is still intact.
    // If copying throws, we leave this function before anything has changed,
    // so *this keeps its old, valid contents (the "strong" exception
    // guarantee).
    Message* new_data = nullptr;  // Stays nullptr if other owns no array.
    if (other.capacity_ > 0) {
        new_data = copy_messages(other.data_, other.size_, other.capacity_);
    }

    // Step 2: the copy worked, so it is now safe to free our old array.
    // (Leaving this line out would leak the old array.)
    delete[] data_;

    // Step 3: take ownership of the new array and copy the bookkeeping.
    data_ = new_data;
    size_ = other.size_;
    capacity_ = other.capacity_;

    return *this;  // Returning *this lets assignments be chained, e.g. "a = b = c;".
}

// ---------------------------------------------------------------------------
// Move operations (steal the array instead of copying it)
// ---------------------------------------------------------------------------

// Move constructor: take over other's array. No Message is copied or moved
// one by one; only a pointer and two numbers change hands.
Conversation::Conversation(Conversation&& other) noexcept
    : data_(other.data_),           // Steal other's array pointer...
      size_(other.size_),           // ...its message count...
      capacity_(other.capacity_) {  // ...and its capacity.
    // Leave other valid and empty. Setting its pointer to nullptr is what
    // stops other's destructor from freeing the array we now own.
    other.data_ = nullptr;
    other.size_ = 0;
    other.capacity_ = 0;
}

// Move assignment: free our own array, then take over other's
// (used by statements like "a = std::move(b);").
Conversation& Conversation::operator=(Conversation&& other) noexcept {
    // Self-move ("a = std::move(a);"): going ahead would delete[] the very
    // array we are supposed to keep, so do nothing instead.
    if (this == &other) {
        return *this;
    }

    // Free the array we currently own, because it is being replaced.
    delete[] data_;

    // Steal other's array and bookkeeping.
    data_ = other.data_;
    size_ = other.size_;
    capacity_ = other.capacity_;

    // Leave other valid and empty so its destructor does not free our array.
    other.data_ = nullptr;
    other.size_ = 0;
    other.capacity_ = 0;

    return *this;  // Return *this so assignments can be chained.
}

// ---------------------------------------------------------------------------
// Adding messages
// ---------------------------------------------------------------------------

// Adds m to the end of the conversation (it becomes the newest message).
//
// Messages are only ever added at the end - never inserted in front,
// reordered, or removed - so whichever message was appended first (the
// System message, when there is one) stays at index 0 for good.
void Conversation::append(Message m) {
    // Step 1: if every slot is already in use, grow the array first.
    if (size_ == capacity_) {
        // Growth factor 2: the new array has twice as many slots as the old
        // one. An array with 0 slots cannot be doubled, so it grows to 1.
        std::size_t new_capacity = 0;
        if (capacity_ == 0) {
            new_capacity = 1;
        } else {
            new_capacity = capacity_ * 2;
        }

        // Allocate the bigger array (every slot starts as an empty Message).
        // If this throws std::bad_alloc, nothing has been changed yet, so the
        // Conversation is still exactly as it was before the call.
        Message* new_data = new Message[new_capacity];

        // Move every existing message into the same position in the new
        // array. Moving a Message moves its std::string, which hands over the
        // string's internal buffer instead of copying the characters, and it
        // cannot throw.
        for (std::size_t i = 0; i < size_; ++i) {
            new_data[i] = std::move(data_[i]);
        }

        // Free the old, smaller array (its Messages are empty shells now).
        delete[] data_;

        // Start using the new array.
        data_ = new_data;
        capacity_ = new_capacity;
    }

    // Step 2: slot number size_ is now guaranteed to be free, so put m there.
    // m is our own copy (it was passed by value), so we may move from it.
    data_[size_] = std::move(m);

    // One more message is stored.
    ++size_;
}

// ---------------------------------------------------------------------------
// Reading messages
// ---------------------------------------------------------------------------

// Returns how many messages are stored.
std::size_t Conversation::size() const noexcept {
    return size_;
}

// Returns how many slots the array currently has (used by the tests).
std::size_t Conversation::capacity() const noexcept {
    return capacity_;
}

// Returns message number i (0 = the oldest), after checking that it exists.
const Message& Conversation::at(std::size_t i) const {
    // Valid indexes are 0 .. size_-1. Slots from size_ onwards are spare
    // space, not real messages, so they count as out of range too.
    if (i >= size_) {
        // Chosen behavior for a bad index: throw std::out_of_range (the same
        // exception std::string::at uses). The tests catch it to prove it.
        throw std::out_of_range("Conversation::at: index " + std::to_string(i) +
                                " is out of range (size is " + std::to_string(size_) + ")");
    }

    return data_[i];  // The index is valid: return a read-only reference to that message.
}

// Returns a pointer to the first (oldest) message, for range-for loops.
const Message* Conversation::begin() const noexcept {
    return data_;  // nullptr when nothing has been allocated yet.
}

// Returns a pointer just PAST the last real message, for range-for loops.
// A loop runs from begin() up to, but not including, end().
const Message* Conversation::end() const noexcept {
    // With no array at all, return nullptr so that end() == begin() and a
    // loop over an empty conversation runs zero times.
    if (data_ == nullptr) {
        return nullptr;
    }

    // Otherwise step over the size_ real messages. (Not capacity_: the spare
    // slots at the end are not part of the conversation.)
    return data_ + size_;
}
