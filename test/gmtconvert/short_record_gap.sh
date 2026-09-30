#!/usr/bin/env bash
# Test gmt convert -g with a record too short that is skipped
printf '1 2 3\n100 200\n' > gap_short.txt
gmt convert gap_short.txt -gx10 > answer1.txt
cat << EOF > truth1.txt
1	2	3
EOF
diff truth1.txt answer1.txt --strip-trailing-cr > fail

printf '1 2 3\n100 200\n3 4 5\n' > gap_near.txt
gmt convert gap_near.txt -gx10 > answer2.txt
cat << EOF > truth2.txt
1	2	3
3	4	5
EOF
diff truth2.txt answer2.txt --strip-trailing-cr >> fail

printf '1 2 3\n100 200\n101 201 301\n' > gap_far.txt
gmt convert gap_far.txt -gx10 > answer3.txt
cat << EOF > truth3.txt
>
1	2	3
> Data gap detected via -g; Segment header inserted
101	201	301
EOF
diff truth3.txt answer3.txt --strip-trailing-cr >> fail

printf '50 1 3\n100 200\n50 3 5\n' > gap_swap.txt
gmt convert gap_swap.txt -: -gx10 > answer4.txt
cat << EOF > truth4.txt
50	1	3
50	3	5
EOF
diff truth4.txt answer4.txt --strip-trailing-cr >> fail
