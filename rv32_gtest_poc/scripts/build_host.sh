#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
mkdir -p build/host
g=vendor/googletest-1.15.2/googletest
for f in src/product.c src/product_adapter.c; do
 gcc -std=c11 -Wall -Wextra -Werror -O2 -Isrc -c "$f" -o "build/host/$(basename "${f%.c}").o"
done
# GoogleTest include path must precede FFF 1.1, which includes an old gtest copy.
g++ -std=c++14 -O2 -pthread -Isrc -Ifixtures -I"$g/include" -I"$g" -Ivendor/fff-1.1 tests/product_test.cc tests/test_main.cc "$g/src/gtest-all.cc" build/host/product.o build/host/product_adapter.o -o build/host/product_tests
