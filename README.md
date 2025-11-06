# mylibs: A Collection of Header-Only C/C++ Libraries
A set of lightweight, reusable libraries designed to simplify common tasks and enhance code modularity.

# Libraries Included
1. network.h: A minimalistic TCP library for straightforward socket communication. 
Ideal for learning and prototyping networking applications.
2. memorytracker: it's simple tracking system for all malloc()'s that are allocated
    and haven't been free()d yet. It helps with memory leaks. It gives you exact filename
    and line number where unfreed malloc is located.

# Getting Started
* Just drop it inside your C or C++ project and that's it.
* Anywhere it's needed, do this
  ````c
  #define NETWORK_IMPLEMENTATION
  #include "network.h"


# Why it exists
I wanted to a central place to commonly used syscalls, data structures and algorithms to reduce the need to rewrite the same things for every new project I work on.

# Roadmap
* Enhance error handling across libraries.
* Add more utility libraries (e.g., for file handling, string manipulation).
* Improve cross-platform compatibility.

# License
This project is licensed under the MIT License.
