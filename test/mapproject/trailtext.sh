#!/usr/bin/env bash
# Another test from forum message
# http://gmt.soest.hawaii.edu/boards/1/topics/6027
# Expanded to test a few of the -o settings

cat << EOF > line.txt
1 2
3 4
EOF
echo -e "0\t0\tnot important text" > input.txt

gmt mapproject -Lline.txt input.txt > result.txt
gmt mapproject -Lline.txt input.txt -on >> result.txt
gmt mapproject -Lline.txt input.txt -ot >> result.txt
gmt mapproject -Lline.txt input.txt -o2,4 >> result.txt
gmt mapproject -Lline.txt input.txt -o2,4,t >> result.txt
cat << EOF > answer.txt
0	0	246419.226859	1	2	not important text
0	0	246419.226859	1	2
not important text
246419.226859	2
246419.226859	2	not important text
EOF
diff answer.txt result.txt --strip-trailing-cr > fail
