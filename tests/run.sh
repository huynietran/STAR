#!/bin/sh
# Host-side tests for the Turtle Board's link parser and pet logic.
# Needs any C++ compiler (g++ or clang++). Run from the repo root: sh tests/run.sh
set -e
cd "$(dirname "$0")"
${CXX:-c++} -std=gnu++11 -Wall -Istubs -o /tmp/star_tests pet_logic_test.cpp
/tmp/star_tests
