#!/usr/bin/env bash
# Testing that both ascii and binary read with -i will properly
# access the same files and apply the same col-type tests
# Based on msg7140 by John.

A=$(gmt info -I1d/0.5d -i3,2 tp2.xyz)
B=$(gmt info -I1d/0.5d -i3,2 tp2.bin -bi7d)
if [ ! "X$A" = "X$B" ]; then
	echo "Ascii: $A  Bin: $B" > fail
fi
