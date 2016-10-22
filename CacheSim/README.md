CacheSim
===
Usage
---
stdin | ./cache [cache size(in KB)] [associativity] [block size(in Byte)] [replacement strategy]<br>
replacement strategy support: l for LRU, r for Random Replacement

Example
---
gzip -dc 429.mcf-184B.trace.txt.gz | ./cache 2048 64 64 l
