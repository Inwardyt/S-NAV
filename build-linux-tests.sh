#!/bin/sh
set -eu
cd "$(dirname "$0")"
gcc -std=c99 -Wall -Wextra -Werror -pedantic tests/test_core.c src/snav.c src/render.c -lm -o /tmp/snav-tests
/tmp/snav-tests
