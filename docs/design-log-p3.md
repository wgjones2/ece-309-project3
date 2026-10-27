# Project 3 Design Log

**Author:** Walter Jones · **P2 code used:** My own from project 2 and professor's code referenced.
**Tools used:** Claude used to review / suggest changes to pseudocode structure - VSCode autocomplete utilized while writing code

<!-- 500–800 words. An argument, not a diary: say what you decided, what
     you rejected, and what evidence backs it up. Delete these comments
     before submitting. -->

## 1. Hash table

**The Collision Strategy:**

**Hash Functions:**

**Load Factor Threshold:** The threshold is a load factor of 0.75. The default table starts with 8 buckets. Each add  increases utilization. If another tool addition would push the ratio of utilization to bucket count over 0.75, then the bucket count doubles first. Every node is moved into the new buckets with a recomputed index. Because it checks before it never goes above 0.75 after the add function returns. 0.75 was choosen because its the lower safer limit of the suggest range in the spec. The overall memory impact is small and so it is not a big deal to have higher overhead.

<!-- Chaining or open addressing, and why. Your hash function, load-factor
     threshold, and resize policy. Paste your 10,000-tool numbers
     (bucket_count, load_factor, max_probe_length) next to the alternative
     hash's longest chain. Explain briefly why add() is amortized O(1). -->

| hash              | bucket_count | load_factor | longest chain |
| ----------------- | ------------ | ----------- | ------------- |
| FNV-1a            |              |             |               |
| `s[0] + s.size()` |              |             |               |

## 2. Pointer stability

<!-- Why does a Tool* returned by find() survive a resize in your design? -->

## 3. Calculator trace

<!-- Trace the values and ops stacks for 2 * (3 + 4) - 5, one token at a
     time, in your own words. -->

| token | action | values (bottom → top) | ops (bottom → top) |
| ----- | ------ | --------------------- | ------------------ |
| `2`   |        |                       |                    |
| `*`   |        |                       |                    |
| `(`   |        |                       |                    |
| `3`   |        |                       |                    |
| `+`   |        |                       |                    |
| `4`   |        |                       |                    |
| `)`   |        |                       |                    |
| `-`   |        |                       |                    |
| `5`   |        |                       |                    |
| end   |        |                       |                    |

## 4. Errors as values

<!-- Why do tools, resolve() and calc_evaluate() return errors instead of
     throwing? When IS an exception the right choice? -->

## 5. What I would design differently
