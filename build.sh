#!/usr/bin/env bash
i686-w64-mingw32-c++ -static -static-libgcc -static-libstdc++ -Wall -Wextra -std=c++20 ./*.cpp  -o main.exe
