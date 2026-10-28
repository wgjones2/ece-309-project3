// src/tools/tool_registry.cpp — YOU IMPLEMENT (spec §4.3).
//
// A hand-written hash table. No std::unordered_map / std::map / std::set.

#include "tools/tool_registry.h"
#include <cstdint>
#include <utility>


// Anonymous namespace for internal constants and helper functions
namespace {
// FNV-1a constants - given
	static constexpr std::uint64_t kFnvOffsetBasis = 1469598103934665603; //a 64-bit unsigned offset basis of 1469598103934665603
	static constexpr std::uint64_t kFnvPrime = 1099511628211; //a 64-bit unsigned prime of 1099511628211
}


//create the starting buckets (all empty) and set the tool count to 0
ToolRegistry::ToolRegistry() {
	buckets_.resize(kInitialBucketCount);
	size_ = 0; // initial count
}
ToolRegistry::~ToolRegistry() = default; // unique_ptrs in your buckets free the tools


//Hash - computes the FNV-1a hash of a string key
std::size_t ToolRegistry::hash(std::string_view key) noexcept {
	//start the hash at the FNV-1a offset basis
	std::uint64_t hash = kFnvOffsetBasis;
	//for each character in the key:
	for (unsigned char c : key) {
		hash ^= c; //xor the character into the hash
		hash *= kFnvPrime; //multiply the hash by the FNV-1a prime
	}
	return static_cast<std::size_t>(hash);
}


//Add - adds a new tool to the registry if it doesn't already exist, returns true on success, false otherwise
bool ToolRegistry::add(std::unique_ptr<Tool> tool) { // add a new tool to the registry
	if (tool == nullptr) {// if the tool is null
		return false;  // return false
	}
	std::string name = tool->name(); //get the tool's name
	Tool* existing_tool = find(name); // check if a tool with the same name already exists
	if (existing_tool != nullptr) { // if a tool with that name is already stored
		return false; //return false
	} //note: the new tool is destroyed on its own when it goes out of scope
	//if adding one more tool would push the load factor above the max load factor, rehash to double the bucket count first
	double new_count = static_cast<double>(size_ + 1); // get a new count variable without touching the actual size_
	double bucket_total = static_cast<double>(buckets_.size()); // get the total number of buckets as a double (also static cast)
	double next_load_factor = new_count / bucket_total; //find the load factor of the table if this tool were added
	if (next_load_factor > kMaxLoadFactor) { //if it is too big
		rehash(buckets_.size() * 2); // double the bucket count
	}
	std::size_t bucket_index = hash(name) & (buckets_.size() - 1); //turn the hash of the name into a bucket index
	//make a new node, move the tool into it, and put the node at the front of that bucket's chain
	std::unique_ptr<Node> new_node = std::make_unique<Node>();
	new_node->tool = std::move(tool); //transfer ownership of the tool to the new node
	new_node->next = std::move(buckets_[bucket_index]); //the old first node becomes the next node after the new node
	buckets_[bucket_index] = std::move(new_node); //new node becomes the first node in the bucket's chain
	++size_; //add one to the tool count
    return true; // return true on successful addition
}


//Find - searches for a tool by name and returns a pointer to it if found, nullptr otherwise
Tool* ToolRegistry::find(std::string_view name) const {
	//turn the hash of the name into a bucket index
	std::size_t index = hash(name) % buckets_.size(); //turn the hash of the name into a bucket index
	Node* current = buckets_[index].get(); //start at the first node in the bucket's chain
	while (current != nullptr) { //walk down the chain
		if (current->tool->name() == name) { //if the tool's name matches
			return current->tool.get(); //return a plain pointer to the tool
		}
		current = current->next.get(); // move to the next node
	}
    return nullptr;  // if not found, return nullptr
}


//Size - returns the number of tools in the registry
std::size_t ToolRegistry::size() const noexcept {
    return size_; // return the current tool count
}


//For each - applies a function to each tool in the registry
void ToolRegistry::for_each(const std::function<void(const Tool&)>& fn) const {
	for (std::size_t i = 0; i < buckets_.size(); ++i) { // for every bucket
		Node* current = buckets_[i].get(); //get the bucket's first node
		while (current != nullptr) { //while there are more nodes in the chain
			fn(*current->tool); // call the function on the current tool
			current = current->next.get(); //move to the next node
		}
	}
}


// Rehash private helper:
//this function rehashes the registry to have a new number of buckets, redistributing all nodes according to the new bucket count
void ToolRegistry::rehash(std::size_t new_bucket_count) {
	std::vector<std::unique_ptr<Node>> new_buckets(new_bucket_count);
	for (std::size_t i = 0; i < buckets_.size(); ++i) {
		Node* current = buckets_[i].release(); // take the head node away from the old bucket so it has one owner
		while (current != nullptr) { // while the current node is not null
			Node* next = current->next.release();
			std::size_t new_index = hash(current->tool->name()) % new_bucket_count; // compute the new bucket index
			current->next = std::move(new_buckets[new_index]); // insert at the front of the new bucket's chain
			new_buckets[new_index].reset(current); // set the new bucket's head to the current node
			current = next; // move to the next node in the old chain
		}
	}
	buckets_ = std::move(new_buckets); //replace the old buckets with the new ones
}


//Bucket count - returns the number of buckets in the registry
std::size_t ToolRegistry::bucket_count() const noexcept {
    return buckets_.size();   // return the number of buckets
}


//Load factor - returns the current load factor of the registry
double ToolRegistry::load_factor() const noexcept {
	//return the tool count divided by the bucket count, as a double
    return (static_cast<double>(size_) / static_cast<double>(buckets_.size())); //the load factor
}


//Max probe length - returns the length of the longest chain in the registry
std::size_t ToolRegistry::max_probe_length() const {
	std::size_t max_length = 0; // keep track of the longest chain - 0 by default
	for (std::size_t i = 0; i < buckets_.size(); ++i) { // for each bucket
		std::size_t length = 0; //set current chain length to 0 - init
		Node* current = buckets_[i].get(); //get first node of the current bucket's chain
		while (current != nullptr) { //count the nodes in the chain until null
			++length; // increment the current chain length
			current = current->next.get(); // move to the next node in the chain
		} //once done counting the nodes in the current chain
		if (length > max_length) { //if longer than the current max
			max_length = length; // update the maximum length if needed
		}
	} //every bucket has been checked
	return max_length; // return the longest chain length found
}
    