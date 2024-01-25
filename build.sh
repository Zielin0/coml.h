#!/bin/bash

set -xe

CC=clang

CFLAGS="-Wall -Wextra -pedantic -ggdb -I."

$CC $CFLAGS -o ./main ./main.c -lm
