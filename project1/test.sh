#!/bin/sh
cc prog1.c -pthread -o 1.out
cc prog2.c -pthread -o 2.out
date
./1.out
date
./2.out
date
exit