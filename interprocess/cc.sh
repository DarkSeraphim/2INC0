#! /bin/sh

CFLAGS="-Wall -g -c"
LFLAGS="-lrt -lX11"

echo "compiling..."
gcc $CFLAGS farmer.c || exit
gcc $CFLAGS worker.c || exit
gcc $CFLAGS output.c || exit

echo "linking..."
gcc -o farmer farmer.o output.o $LFLAGS || exit
gcc -o worker worker.o $LFLAGS || exit

echo "successfull"

