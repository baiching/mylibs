# mylibs: A Collection of Header-Only C/C++ Libraries
A set of lightweight, reusable libraries designed to simplify common tasks and enhance code modularity.

# Libraries Included
network.h: A minimalistic TCP library for straightforward socket communication. 
Ideal for learning and prototyping networking applications.

# Getting Started
* Just drop it inside your C or C++ project and that's it.
* Anywhere it's needed, do this
  ````c
  #define NETWORK_IMPLEMENTATION
  #include "network.h"

# Features

* Header-Only: No need for separate compilation or linking.
* Minimalistic API: Designed to be intuitive and easy to use.
* No External Dependencies: Everything you need is included.

# Why it exists
I wanted to a central place to commonly used syscalls, data structures and algorithms to reduce the need to rewrite the same things for every new project I work on.

# Roadmap
* Enhance error handling across libraries.
* Add more utility libraries (e.g., for file handling, string manipulation).
* Improve cross-platform compatibility.

# License
This project is licensed under the MIT License.
