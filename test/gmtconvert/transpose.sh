#!/usr/bin/env bash
# Test gmt convert -Z option to take a single-segment dataset transpose
cat << EOF > data.txt
# Fake data header from my brain
> Only one data segment allowed
1	2	3	4	5
6	7	8	9	10
11	12	12	14	15
16	17	18	19	20
11	21	31	41	51
61	71	81	91	101
50	51	52	53	54
EOF
cat << EOF > answer.txt
# Command : gmt gmtconvert data.txt -Z -ho
> Only one data segment allowed
1	6	11	16	11	61	50
2	7	12	17	21	71	51
3	8	12	18	31	81	52
4	9	14	19	41	91	53
5	10	15	20	51	101	54
EOF
# Transpose the data file
gmt convert data.txt -Z -ho > result.txt
# Look for differences between results and truth
diff result.txt answer.txt --strip-trailing-cr  > fail
