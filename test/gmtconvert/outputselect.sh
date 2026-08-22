#!/usr/bin/env bash
# Test gmt convert with ASCII data and using -s

# Build input file
cat << EOF > input.txt
1	1	1	1
2	2	NaN	2
3	3	3	NaN
4	4	NaN	NaN
EOF
echo "# Test -s" > answer.txt
gmt convert input.txt -s >> answer.txt
echo "# test -s+r" >> answer.txt
gmt convert input.txt -s+r >> answer.txt
echo "# Test -s3" >> answer.txt
gmt convert input.txt -s3 >> answer.txt
echo "# test -s3+r" >> answer.txt
gmt convert input.txt -s3+r >> answer.txt
echo "# test -s+a" >> answer.txt
gmt convert input.txt -s+a >> answer.txt
echo "# test -s+a+r" >> answer.txt
gmt convert input.txt -s+a+r >> answer.txt
echo "# test -s2:3" >> answer.txt
gmt convert input.txt -s2:3 >> answer.txt
echo "# test -s2:3+a" >> answer.txt
gmt convert input.txt -s2:3+a >> answer.txt
echo "# test -s2:3+r" >> answer.txt
gmt convert input.txt -s2:3+r >> answer.txt
echo "# test -s2:3+r+a" >> answer.txt
gmt convert input.txt -s2:3+a+r >> answer.txt

cat << EOF > truth.txt
# Test -s
1	1	1	1
3	3	3	NaN
# test -s+r
2	2	NaN	2
4	4	NaN	NaN
# Test -s3
1	1	1	1
2	2	NaN	2
# test -s3+r
3	3	3	NaN
4	4	NaN	NaN
# test -s+a
1	1	1	1
3	3	3	NaN
# test -s+a+r
2	2	NaN	2
4	4	NaN	NaN
# test -s2:3
1	1	1	1
2	2	NaN	2
3	3	3	NaN
# test -s2:3+a
1	1	1	1
# test -s2:3+r
4	4	NaN	NaN
# test -s2:3+r+a
2	2	NaN	2
3	3	3	NaN
4	4	NaN	NaN
EOF

diff truth.txt answer.txt --strip-trailing-cr > fail
