#!/usr/bin/env bash
# Test that -h+n suppresses the automatic "# Command : ..." header record so that
# the output header is identical to the input header.  Two code paths write that
# stamp for table output: gmtlib_write_newheaders (e.g. gmtconvert) and
# gmtapi_dataset_comment (modules that call GMT_Set_Comment, e.g. gmtmath).
cat << EOF > data.txt
# Fake data header from my brain
1	2	3
4	5	6
EOF

# Default -h still writes the "# Command :" stamp
gmt convert data.txt -h > with_stamp.txt
# -h+n must reproduce the input header verbatim, with no stamp added
gmt convert data.txt -h+n > no_stamp.txt

cat << EOF > answer_with_stamp.txt
# Fake data header from my brain
# Command : gmt gmtconvert data.txt -h
1	2	3
4	5	6
EOF

diff with_stamp.txt answer_with_stamp.txt --strip-trailing-cr > fail
diff no_stamp.txt data.txt --strip-trailing-cr >> fail

# Same for a module that stamps the command via GMT_Set_Comment
cat << EOF > tdata.txt
# Fake data header from my brain
1	10
2	20
EOF

gmt math tdata.txt -h+n -C1 2 MUL = math_no_stamp.txt

cat << EOF > answer_math.txt
# Fake data header from my brain
1	20
2	40
EOF

diff math_no_stamp.txt answer_math.txt --strip-trailing-cr >> fail
