#!/bin/bash
#
# $Id$

# test byteswapping with gmtconvert

# increase number of output digits
gmtset FORMAT_FLOAT_OUT %.20g

# generate text file
gmtconvert -V0 << EOF > numbers.txt
# u c H h I i f L l d
171 -85 43981 -21555 3735928559 -559038737 \
-6.259853398707798016e+18 1.6045690981116495872e+19 \
-2.401053092593056256e+18 -1.1885953980619317e+148
EOF

# read big endian file
gmtconvert -bi1u1c1H1h1I1i1f1L1l1d+B "${src}"/binary.be > numbers_from_be.txt

# read little endian file
gmtconvert -bi1u1c1H1h1I1i1f1L1l1d+L "${src}"/binary.le > numbers_from_le.txt

# compare result with text file
diff -q numbers.txt numbers_from_be.txt
diff -q numbers.txt numbers_from_le.txt

# write big endian file
gmtconvert -bo1u1c1H1h1I1i1f1L1l1d+B numbers_from_be.txt > bin_from_num.be

# write little endian file
gmtconvert -bo1u1c1H1h1I1i1f1L1l1d+L numbers_from_le.txt > bin_from_num.le

# compare binary output with original binary files
diff -q "${src}"/binary.be bin_from_num.be
diff -q "${src}"/binary.le bin_from_num.le
