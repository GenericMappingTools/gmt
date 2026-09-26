#!/usr/bin/env bash
# Test gmt convert on files whose first or only segment has only records too short for -i
printf '30 45\n120 60\n' > short.txt
printf '5 6 7\n8 9 10\n' > good.txt
gmt convert short.txt good.txt -i0:2 > answer1.txt
cat << EOF > truth1.txt
5	6	7
8	9	10
EOF
diff truth1.txt answer1.txt --strip-trailing-cr > fail

printf '1 2\n3 4\n> B\n5 6 7\n8 9 10\n' > headerless.txt
gmt convert headerless.txt -i0:2 > answer2.txt
cat << EOF > truth2.txt
> B
5	6	7
8	9	10
EOF
diff truth2.txt answer2.txt --strip-trailing-cr >> fail

printf '1 2\n100 200 300\n' > gap_first.txt
gmt convert gap_first.txt -i0:2 -gx10 > answer3.txt
cat << EOF > truth3.txt
> Data gap detected via -g; Segment header inserted
100	200	300
EOF
diff truth3.txt answer3.txt --strip-trailing-cr >> fail
