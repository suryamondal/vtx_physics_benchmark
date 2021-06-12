#!/bin/bash
WARN_FLAGS="-Wall -pedantic -Wno-literal-suffix"
g++ *.cc -o ana $WARN_FLAGS `root-config --cflags --glibs`
