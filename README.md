# CacheSim
A Cache Simulator in C++ that accounts for L1 and L2 caches and multiple replacement policies

_____________________________________________________________________________________________________________________________________________

INSTALL & RUN

To run on Windows:
1: open cmd prompt (if you don't already have Windows Subsystem for Linux)
2: run "wsl --install"
3: open your Linux distribution from the Start menu
4: run "sudo apt install make"
5: follow Linux steps 3-6 below

To run on Linux:
1. download the repository
2. open Linux shell
3. navigate to repository directory
4. run "make"
5. type "./sim_cache"
6. Follow the prompts regarding inputs

_____________________________________________________________________________________________________________________________________________

PROGRAM DESCRIPTION

This program simulates L1 and L2 cache with a varying block size, varying cache sizes, and varying associativity.
It allows for LRU, FIFO, or Optimal replacement policy.
It uses the write-back and write-allocate write policy.
It allows for inclusive or non-inclusive caches.

Inputs: 
1. Blocksize (in bytes)
2. Size of each cache (in bytes)
3. Associativity of each cache (as an integer)
4. Inclusion policy (as an integer)
5. Replacement Policy (as an integer)
6. Input file name (this file contains a list of pairs of input; the first input in each pair is 'r' or 'w' for "read" or "write", and the second is a 32 bit address in hexadecimal)

Outputs:
1. Hex blocks stored in each set of each cache (with 'D' displayed if the dirty bit is set)
2. Number of reads, read misses, writes, and write misses for each cache
3. Miss rate for each cache
4. Number of writebacks for L2
5. Number of blocks transferred

