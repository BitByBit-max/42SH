#!/bin/sh

cfiles=$(tree -fi | grep "\.c$" | tr '\n' ' ')
hfiles=$(tree -fi | grep "\.h$" | tr '\n' ' ')

for file in $cfiles; do
    clang-format -i "$file"
done

for file in $hfiles; do
    clang-format -i "$file"
done
