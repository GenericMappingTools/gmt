#!/usr/bin/env bash
# Test that binary headers are handled properly
gmt math -T1/50/1 -o1       T 10 MUL = t.txt
gmt math -T1/50/1 -o1 -bo1f T 10 MUL = t.b
cat << EOF > header.txt
# This is one line pf junk
# This is another line of junky stuff........................
EOF
# Compute the size of the header in bytes
h=$(ls -l header.txt | awk '{print $5}')
# Make binary file with leading junk header
cat header.txt > junk.b
cat t.b >> junk.b
# Run gmt convert which will strip off the headers on output, then compare
gmt convert junk.b -bi1f -hi${h} > out1.txt
gmt convert t.txt > out2.txt
diff out1.txt out2.txt --strip-trailing-cr > fail
