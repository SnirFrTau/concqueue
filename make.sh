# General idea: Remove ./queue to not accidently run the previous version,
#               as if the compilation failed the previous version is still available.

rm ./queue
gcc -O3 -D_POSIX_C_SOURCE=200809 -Wall -std=c11 -pthread queue.c queue.h testsqueue.c -o queue
./queue
