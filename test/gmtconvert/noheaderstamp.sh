#!/usr/bin/env bash
# Test that -h+n suppresses the automatic "# Command : ..." header record
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
