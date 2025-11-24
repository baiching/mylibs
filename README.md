# mylibs: A Collection of C/C++ Libraries and tools
A set of lightweight, reusable libraries designed to simplify common tasks and enhance code modularity.

# Libraries Included

1.1 network.h: A minimalistic TCP library for straightforward socket communication. 
Ideal for learning and prototyping networking applications.

1.2 network_win.h: It's the windows version of POSIX api. For now, it doesn't have any IOCP interface but it's on my list to add next.


2. memorytracker: it's simple tracking system for all malloc()'s that are allocated
    and haven't been free()d yet. It helps with memory leaks. It gives you exact filename
    and line number where unfreed malloc is located.

# Getting Started
* For *.h only libs droping it inside C or C++ project and then do the following where it's intended to be used.
* Anywhere it's needed, do this
  ````c
  #define NETWORK_IMPLEMENTATION
  #include "network.h"


# Usage
I wanted to a central place to commonly used syscalls, data structures and algorithms to reduce the need to rewrite the same things for every new project I work on.

# Roadmap
* Enhance error handling across libraries.
* Add more utility libraries (e.g., for file handling, string manipulation).
* Improve cross-platform compatibility.

# License
This project is licensed under the MIT License.
