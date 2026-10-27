# Project 3 Design Log

**Author:** Walter Jones · **P2 code used:** My own from project 2 and professor's code referenced.
**Tools used:** Claude used to review / suggest changes to pseudocode structure after my first draft before starting to write code - VSCode autocomplete utilized while writing code
**Additional Note:** I neglected to commit the changes during the pseudocode stage to git while I was doing them. However, the change log is still present and shows the gradual set up work.

<!-- 500–800 words. An argument, not a diary: say what you decided, what
     you rejected, and what evidence backs it up. Delete these comments
     before submitting. -->

## 1. Hash table

**The Collision Strategy:** I utilized seperate chaining. Each bucket holds the head of a single linked list of nodes. Each node holds a tool through a unique smart pointer as well as a unique pointer to the next node. A new tool goes at the front of the chain, therefore, inserting is constant time. I chose chaining over linear probing because it is simple, not impactful to size, quick to search, and because this project never removes tools. The main downside being the extra memory per node doesn't really matter at the size this project uses. It's quicker to search than probing because there are no long probe runs.

**Hash Functions:** The hash is 64 bit FNV-1a. The hash function needs a starting state before it reads any data and it starts it. For each charactor, it XORs the byte, then multiplies by FNV prime. In this way, every character changes the result and it happens after each XOR so order matters. The hash function returns the full value and the bucket index is seperately taken with "hash & (bucket_count - 1)" which is bitwise and quick. For that, bucket_count is strictly a power of 2 but is much quicker than hash % 8. 

**Load Factor Threshold:** The threshold is a load factor of 0.75. The default table starts with 8 buckets. Each add increases utilization. If another tool addition would push the ratio of the number of tools to bucket count over 0.75, then the bucket count doubles first. Every node is moved into the new buckets with a recomputed index. Because it checks before it never goes above 0.75 after the add function returns. 0.75 was chosen because its the lower safer limit of the suggested range in the spec. The overall memory impact is small and so it is not a big deal to have higher overhead.

**Measurements Table:**

<!-- Chaining or open addressing, and why. Your hash function, load-factor
     threshold, and resize policy. Paste your 10,000-tool numbers
     (bucket_count, load_factor, max_probe_length) next to the alternative
     hash's longest chain. Explain briefly why add() is amortized O(1). -->

| hash              | bucket_count | load_factor | longest chain |
| ----------------- | ------------ | ----------- | ------------- |
| FNV-1a            |              |             |               |
| `s[0] + s.size()` |              |             |               |

**Amoritized O(1):** Most adds just push a node onto the front of a chain. A resize costs O(n) because every node is relinked, but the table doubles each time so as it becomes a longer operation, it also becomes increasingly infrequent to perform. The total work for all resizes up to n tools is about 2n.

## 2. Pointer stability

<!-- Why does a Tool* returned by find() survive a resize in your design? -->

## 3. Calculator trace

<!-- Trace the values and ops stacks for 2 * (3 + 4) - 5, one token at a
     time, in your own words. -->

| token | action                                                                 | values (bottom → top) | ops (bottom → top) |
| ----- | ---------------------------------------------------------------------- | --------------------- | ------------------ |
| `2`   | push 2 onto values                                                     | 2                     | (nothing)          |
| `*`   | ops is empty so push * to ops                                          | 2                     | *                  |
| `(`   | push ( to ops                                                          | 2                     | * (                |
| `3`   | push 3 to values                                                       | 2 3                   | * (                |
| `+`   | top is (, so nothing is yet applied and push +                         | 2 3                   | * ( +              |
| `4`   | push 4                                                                 | 2 3 4                 | * ( +              |
| `)`   | apply the + (3+4=7), push 7, and pop (                                 | 2 7                   | *                  |
| `-`   | * has higher precedence so it is applied (2*7=14), store 14 and push - | 14                    | -                  |
| `5`   | push 5                                                                 | 14 5                  | -                  |
| end   | apply the - (14-5=9) and push 9 to values                              | 9                     | (nothing)          |

Therefore, the answer is 9

## 4. Errors as values

<!-- Why do tools, resolve() and calc_evaluate() return errors instead of
     throwing? When IS an exception the right choice? -->

## 5. What I would design differently
