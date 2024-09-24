#!/bin/sh
cc "${1:-MazeRunner.c}" $(pkg-config --libs --cflags raylib) -o "${2:-YourGame}"
