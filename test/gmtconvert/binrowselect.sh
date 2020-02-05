#!/usr/bin/env bash
# Test gmt convert with binary data and using -qi
# Key difference is that all the multisegment header text is lost of course
# Build two ascii files
echo "> Seg 1 File 1" > one.txt
gmt math -T2010-12-01T/2010-12-31T/1d TROW = >> one.txt
echo "> Seg 2 File 1" >> one.txt
gmt math -T2011-01-01T/2011-01-311T/1d TROW 100 ADD = >> one.txt
echo "> Seg 1 File 2" > two.txt
gmt math -T2011-02-01T/2011-02-28T/1d TROW 200 ADD = >> two.txt
echo "> Seg 2 File 2" >> two.txt
gmt math -T2011-03-01T/2011-03-31T/1d TROW 100 ADD = >> two.txt
# Then convert to native binary
gmt convert one.txt -bo2d > one.bin
gmt convert two.txt -bo2d > two.bin
rm -f one.txt two.txt
# Case 1: Only pull two first data records per segment
gmt convert one.bin two.bin -bi2d -q:1+s -f0T > answer1.txt
# This is what the first output should look like
cat << EOF > truth1.txt
>
2010-12-01T00:00:00	0
2010-12-02T00:00:00	1
>
2011-01-01T00:00:00	100
2011-01-02T00:00:00	101
>
2011-02-01T00:00:00	200
2011-02-02T00:00:00	201
>
2011-03-01T00:00:00	100
2011-03-02T00:00:00	101
EOF
diff truth1.txt answer1.txt --strip-trailing-cr > fail
# Case 2: Same, but per file
gmt convert one.bin two.bin -bi2d -q:1+f -f0T > answer2.txt
# This is what the first output should look like
cat << EOF > truth2.txt
>
2010-12-01T00:00:00	0
>
2011-02-01T00:00:00	200
EOF
diff truth2.txt answer2.txt --strip-trailing-cr >> fail
# Case 3: Limit to just Dec 30-Jan 2 inclusive
gmt convert one.bin two.bin -bi2d -q2010-12-30T/2011-01-02T+c0 -f0T > answer3.txt
cat << EOF > truth3.txt
>
2010-12-30T00:00:00	29
2010-12-31T00:00:00	30
>
2011-01-01T00:00:00	100
2011-01-02T00:00:00	101
EOF
diff truth3.txt answer3.txt --strip-trailing-cr >> fail
# Case 4: Exclude rows with data values
gmt convert one.bin two.bin -bi2d -q~200-400,0-127+c1 -f0T > answer4.txt
cat << EOF > truth4.txt
>
2011-01-29T00:00:00	128
2011-01-30T00:00:00	129
2011-01-31T00:00:00	130
>
2011-03-29T00:00:00	128
2011-03-30T00:00:00	129
2011-03-31T00:00:00	130
EOF
diff truth4.txt answer4.txt --strip-trailing-cr >> fail
