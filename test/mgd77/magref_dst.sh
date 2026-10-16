#!/usr/bin/env bash
#
# gmt mgd77magref -D<dst> with a numeric Dst value used to segfault (the value was stored
# before its buffer was allocated) and dropped the sign of a negative value.

rec="-30 45 0 2000.08700533"
echo "$rec" | gmt mgd77magref -A+y -Fxyz/3 -D8   > pos.txt || exit 1
echo "$rec" | gmt mgd77magref -A+y -Fxyz/3 -D-8  > neg.txt || exit 1
# The sign must matter and the output must be finite
cmp -s pos.txt neg.txt && { echo "-D-8 same as -D8: sign was dropped"; exit 1; }
test -s pos.txt && test -s neg.txt

# With several records the single Dst value must apply to every row, not index past it
printf -- "-30 45 0 2000.08700533\n-30 45 0 2000.5\n-30 45 0 2001.2\n" > multi.txt
gmt mgd77magref multi.txt -A+y -Fxyz/3 -D8 > multi_all.txt || exit 1
for i in 1 2 3; do
	sed -n ${i}p multi.txt | gmt mgd77magref -A+y -Fxyz/3 -D8
done > multi_one.txt || exit 1
diff multi_all.txt multi_one.txt

# A segment rejected for lying outside the Dst table must not break the next segment
printf -- "> bad\n-30 45 0 1900.5\n-30 45 0 1900.6\n> good\n-30 45 0 2000.5\n" > seg.txt
gmt mgd77magref seg.txt -A+y -Fxyz/3 -Vq > seg_out.txt || exit 1
grep -v '^>' seg_out.txt | wc -l | grep -q '^ *1$'
