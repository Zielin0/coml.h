#!/bin/bash

set -xe

CC=clang
CFLAGS="-Wall -Wextra -pedantic -ggdb -I."

$CC $CFLAGS -o ./demo ./demo.c -lm
