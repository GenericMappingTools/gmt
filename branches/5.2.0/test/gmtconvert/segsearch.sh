#!/bin/bash
#	$Id$
# Test the gmtconvert -Q search option for all possibilities

cat << EOF > data.txt
> This is just junk
-9	-9
-9	-9
> -L"Segment 100-A"
0	0
1	1
> -L"Segment 100-B"
0	0
1	1
> -L"Segment 110-A"
0	0
1	1
> -L"Segment 110-B"
0	0
1	1
> -L"Segment 120-A"
0	0
1	1
> -L"Segment 120-B"
0	0
1	1
EOF
rm -f fail
# 1. This test should yield answer1.txt
cat << EOF > answer1.txt
> This is just junk
-9	-9
-9	-9
EOF
gmtconvert data.txt -Sjunk > result1.txt
diff answer1.txt result1.txt >> fail
# 2. This test should yield answer2.txt
cat << EOF > answer2.txt
> -L"Segment 100-A"
0	0
1	1
> -L"Segment 100-B"
0	0
1	1
> -L"Segment 110-A"
0	0
1	1
> -L"Segment 110-B"
0	0
1	1
> -L"Segment 120-A"
0	0
1	1
> -L"Segment 120-B"
0	0
1	1
EOF
gmtconvert data.txt -S~junk > result2.txt
diff answer2.txt result2.txt >> fail
# 3. This test should yield answer3.txt
cat << EOF > answer3.txt
> -L"Segment 100-A"
0	0
1	1
> -L"Segment 100-B"
0	0
1	1
> -L"Segment 120-A"
0	0
1	1
> -L"Segment 120-B"
0	0
1	1
EOF
gmtconvert data.txt -S/"Segment 1[02]0"/ > result3.txt
diff answer3.txt result3.txt >> fail
# 4. This test should yield answer4.txt
cat << EOF > answer4.txt
> -L"Segment 100-B"
0	0
1	1
> -L"Segment 120-B"
0	0
1	1
EOF
gmtconvert data.txt -S/"Segment 1[02]0-B"/ > result4.txt
diff answer4.txt result4.txt >> fail
# 5. This test should yield answer5.txt
cat << EOF > answer5.txt
> This is just junk
-9	-9
-9	-9
> -L"Segment 100-A"
0	0
1	1
> -L"Segment 110-A"
0	0
1	1
> -L"Segment 110-B"
0	0
1	1
> -L"Segment 120-A"
0	0
1	1
EOF
gmtconvert data.txt -S~/"Segment 1[02]0-B"/ > result5.txt
diff answer5.txt result5.txt >> fail
cat << EOF > list
/"Segment 1[02]0-B"/
junk
EOF
# 6. This test should yield answer6.txt
cat << EOF > answer6.txt
> -L"Segment 100-A"
0	0
1	1
> -L"Segment 110-A"
0	0
1	1
> -L"Segment 110-B"
0	0
1	1
> -L"Segment 120-A"
0	0
1	1
EOF
gmtconvert data.txt -S~+flist > result6.txt
diff answer6.txt result6.txt >> fail
# 7. This test should yield answer7.txt
cat << EOF > answer7.txt
> This is just junk
-9	-9
-9	-9
> -L"Segment 100-B"
0	0
1	1
> -L"Segment 120-B"
0	0
1	1
EOF
gmtconvert data.txt -S+flist > result7.txt
diff answer7.txt result7.txt >> fail
