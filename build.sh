#!/bin/bash

set -ue
CC="clang++ -g -O1 -std=c++14 -Iinclude"
CC="$CC -Wfatal-errors -Wall -Wextra -Wno-missing-field-initializers"
CC="$CC -fsanitize=address -fno-omit-frame-pointer -fsanitize=undefined -fsanitize=integer -fsanitize-blacklist=src/sanitize_blacklist.txt"
FLAGS="$(pkg-config yaml-0.1 --cflags --libs)"
$CC dump/dump.cpp src/tapi.cpp $FLAGS -o tapi-dump
