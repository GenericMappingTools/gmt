#!/usr/bin/env bash
# Test that -i and -o works the same way for the same arguments
cat << EOF > t.txt
1	2	3	A
4	5	6	B
7	8	9	C
EOF

# 1. Explore -i
gmt convert t.txt -i2,1,0 > answer_i.txt	# Reverse numerical order, skip trailing text
gmt convert t.txt -i2,1,0,t >> answer_i.txt	# Same with trailing text
gmt convert t.txt -i2+s2,1+o5 >> answer_i.txt	# Reverse numerical order, but select two cols scaled
gmt convert t.txt -i2+s2,1+o5,t >> answer_i.txt	# Reverse numerical order, but scaled

# 1. Explore -o
gmt convert t.txt -o2,1,0 > answer_o.txt	# Reverse numerical order, skip trailing text
gmt convert t.txt -o2,1,0,t >> answer_o.txt	# Same with trailing text
gmt convert t.txt -o2+s2,1+o5 >> answer_o.txt	# Reverse numerical order, but select two cols scaled
gmt convert t.txt -o2+s2,1+o5,t >> answer_o.txt	# Reverse numerical order, but scaled

diff answer_i.txt answer_o.txt --strip-trailing-cr > fail
