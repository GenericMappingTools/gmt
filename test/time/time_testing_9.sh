#!/bin/bash
#	$Id$
# Test various input date formats
cat << EOF > tt9.answer
9.00820793434
9.00820793434
9.00820793434
9.00136798906
9.00136798906
9.00136798906
9.00136798906
9.00136798906
9.00136798906
EOF
echo "2000-10-20T12:00:00  0" | mapproject -R1999T/2001T/0/1 -JX10cT/10c -o0      > tt9.result
echo "2000-OCT-20T12:00:00 0" | mapproject -R -J -o0 --FORMAT_DATE_IN=yyyy-o-dd  >> tt9.result
echo "2000-20-OCTT12:00:00 0" | mapproject -R -J -o0 --FORMAT_DATE_IN=yyyy-dd-o  >> tt9.result
echo "2000-10-20T 	   0" | mapproject -R -J -o0                             >> tt9.result
echo "2000-10-20	   0" | mapproject -R -J -o0                             >> tt9.result
echo "2000-OCT-20T	   0" | mapproject -R -J -o0 --FORMAT_DATE_IN=yyyy-o-dd  >> tt9.result
echo "2000-OCT-20	   0" | mapproject -R -J -o0 --FORMAT_DATE_IN=yyyy-o-dd  >> tt9.result
echo "2000-20-OCT	   0" | mapproject -R -J -o0 --FORMAT_DATE_IN=yyyy-dd-o  >> tt9.result
echo "2000-20-10	   0" | mapproject -R -J -o0 --FORMAT_DATE_IN=yyyy-dd-mm >> tt9.result
diff tt9.result tt9.answer --strip-trailing-cr > fail
