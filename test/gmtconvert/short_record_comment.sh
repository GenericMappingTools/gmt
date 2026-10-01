#!/usr/bin/env bash
# Test gmt convert on a short record immediately followed by a comment
printf '1 2 3\n4 5\n# a comment\n6 7 8\n' > short_then_comment.txt
gmt convert short_then_comment.txt > answer1.txt
cat << EOF > truth1.txt
1	2	3
6	7	8
EOF
diff truth1.txt answer1.txt --strip-trailing-cr > fail

printf '1 2 3\n4 5\n# c1\n# c2\n> S2\n6 7 8\n' > short_comments_seg.txt
gmt convert short_comments_seg.txt > answer2.txt
cat << EOF > truth2.txt
>
1	2	3
> S2
6	7	8
EOF
diff truth2.txt answer2.txt --strip-trailing-cr >> fail
