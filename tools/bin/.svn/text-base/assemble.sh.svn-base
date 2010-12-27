#!/bin/sh -i
echo $1 | as -o out
objdump --disassemble out | egrep "[ ]*[0-9a-fA-F]+:"
rm out
