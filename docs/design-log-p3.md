# Project 3 Design Log

**Author:** Walter Jones · **P2 code used:** My own conversation.h, sentinel_scanner.h, conversation.cpp, and sentinel_scanner.cpp, copied unchanged from Project 2.
**Tools used:** Claude to review my pseudocode structure before I started writing code; VS Code autocomplete while writing code; Claude for final debugging.
**Additional Note:** I neglected to commit during the pseudocode stage, but the commits show the history despite its late addition to github.

## 1. Hash table

**The Collision Strategy:** I utilized separate chaining. Each bucket holds the head of a singly linked list of nodes, and each node owns its tool and the next node through a std::unique_ptr. A new tool goes at the front of its chain, so inserting is constant time. I chose chaining over linear probing because it is simple, avoids long probe runs, and never runs out of room. The main downside, the extra memory per node, doesn't really matter at this size.

**Hash Function:** The hash is 64-bit FNV-1a. It starts from the FNV offset basis, and for each character it XORs in the byte and then multiplies by the FNV prime. Every character changes the result, and because the multiply happens after each XOR, order matters. The function returns the full value; add() reduces it with "hash & (bucket_count - 1)", while find() and rehash() use the equivalent "hash % bucket_count". These match because bucket_count is always a power of 2.

**Load Factor Threshold:** The threshold is 0.75, and the table starts with 8 buckets. If adding another tool would push the ratio of tools to buckets over 0.75, the bucket count doubles first and every node is moved into the new buckets with a recomputed index. Because it checks before inserting, the load factor never goes above 0.75 after add() returns. I chose 0.75 because it is the lower, safer end of the range the spec suggests for chaining.

**Measurements Table:**

| hash              | bucket_count | load_factor | longest chain |
| ----------------- | ------------ | ----------- | ------------- |
| FNV-1a            | 16384        | 0.6104      | 6             |
| `s[0] + s.size()` | 16384        | 0.6104      | 9000          |

With FNV-1a, the longest chain is 6, so a lookup checks at most 6 nodes. The weak hash only produces four different values for these names, so one chain would hold 9000 tools and find() would become a linear search. FNV-1a makes the worst-case lookup 1500 times shorter.

**Amortized O(1):** Most adds just push a node onto the front of a chain. A resize costs O(n) because every node is relinked, but since the table doubles each time, resizes become rarer as they become more expensive. The total work for all resizes up to n tools is about 2n, so each add is O(1) on average.

## 2. Pointer stability

find() returns "node->tool.get()", which is the address of the Tool itself, not of the node or bucket. During a rehash, only the nodes are relinked into the new bucket vector; the Tool objects are never moved or copied. Because a Tool's address never changes, any pointer taken before a resize stays valid. The "reg_10000_tools" test checks this by comparing the pointer for "tool_0" before and after all 10,000 adds.

## 3. Calculator trace

| token | action                                                | values (bottom → top) | ops (bottom → top) |
| ----- | ----------------------------------------------------- | --------------------- | ------------------ |
| `2`   | push 2 onto values                                    | 2                     | (nothing)          |
| `*`   | ops is empty, so push * onto ops                      | 2                     | *                  |
| `(`   | push ( onto ops                                       | 2                     | * (                |
| `3`   | push 3 onto values                                    | 2 3                   | * (                |
| `+`   | top is (, so nothing is applied; push +               | 2 3                   | * ( +              |
| `4`   | push 4 onto values                                    | 2 3 4                 | * ( +              |
| `)`   | apply + (3+4=7), push 7, and pop (                    | 2 7                   | *                  |
| `-`   | * has higher precedence, so apply it (2*7=14); push - | 14                    | -                  |
| `5`   | push 5 onto values                                    | 14 5                  | -                  |
| end   | apply - (14-5=9) and push 9                           | 9                     | (nothing)          |

Therefore, the answer is 9.

## 4. Errors as values

Errors are returned as values rather than thrown as exceptions because, in an agentic AI loop, a tool failing is not abnormal. The failure is fed back into the conversation, so the model can read why it failed and try something different. Exceptions are for actual bugs in the code. For example, json_value.cpp throws std::logic_error when as_string() is called on a value that is not a string, since only a programming mistake can cause that. The dispatcher still catches anything a tool throws as a last line of defense. My code returns failures by design: add() returns false for a duplicate, calc_evaluate() reports "division by zero", etc.

## 5. What I would design differently

I hand-built a linked list for each chain. Upon review, I noticed a std::vector in each bucket, which the spec allows, would mean fewer allocations and a simpler rehash that just moves unique_ptrs. I would also add operators like exponents, modulo, and square root, which the Project 1 C harness supported.
