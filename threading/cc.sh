#! /bin/sh

CFLAGS="-Wall -g -c"
LFLAGS="-lpthread"

echo "compiling..."
gcc $CFLAGS prime.c || exit

echo "linking..."
gcc -o prime prime.o $LFLAGS || exit

echo "successfull"

