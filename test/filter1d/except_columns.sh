#!/usr/bin/env bash
# Testing gmt filter1d -N<t_col>+e<cols>: exclude extra columns from filtering
# (e.g., keep X,Y untouched while smoothing Elevation vs Distance).

AWK=${AWK:-awk}
data=except_columns_data.txt
out=except_columns_out.txt

cat << EOF > $data
1000	2000	0	100
1015	1980	25	120
1030	1960	50	90
1045	1940	75	150
1060	1920	100	80
1075	1900	125	130
1090	1880	150	95
1105	1860	175	110
1120	1840	200	140
1135	1820	225	90
1150	1800	250	105
1165	1780	275	120
1180	1760	300	100
EOF

# Column 2 (Distance) is independent; columns 0,1 (X,Y) are excluded from
# filtering along with it, so only column 3 (Elevation) gets filtered.
gmt filter1d $data -Fl250 -N2+e0,1 -E > $out

# Row count must be preserved.
n_in=$(wc -l < $data)
n_out=$(wc -l < $out)
if [ "$n_in" != "$n_out" ]; then
	echo "row count mismatch: input has $n_in rows but output has $n_out" > fail
fi

# X,Y (columns 1-2) must be byte-identical to the input.
$AWK '{print $1, $2}' $data > xy_in.txt
$AWK '{print $1, $2}' $out > xy_out.txt
diff xy_in.txt xy_out.txt >> fail

# Without +e, gmt filter1d would also filter X,Y (this is the behavior +e
# fixes) - confirm that still holds so we know the test data is meaningful.
gmt filter1d $data -Fl250 -N2 -E > no_except_out.txt
if diff -q xy_in.txt <($AWK '{print $1, $2}' no_except_out.txt) > /dev/null; then
	echo "expected X,Y to change without +e, but they did not" >> fail
fi

# A range must mean the same thing as the equivalent explicit list.
gmt filter1d $data -Fl250 -N2+e0-1 -E > range_out.txt
diff $out range_out.txt >> fail

# Bad column lists must be rejected, not silently mis-parsed: a non-numeric
# token, an open-ended or reversed range, an empty list, a column beyond the
# data, and excluding every column so nothing is left to filter.
for bad in "+eabc" "+e1-" "+e3-1" "+e" "+e0,1,99" "+e0,1,3"; do
	if gmt filter1d $data -Fl250 -N2$bad -E > /dev/null 2>&1; then
		echo "-N2$bad should have failed but did not" >> fail
	fi
done

# +e cannot be combined with -T since excluded columns only pass through at
# the input abscissae.
if gmt filter1d $data -Fl250 -N2+e0,1 -T0/300/25 > /dev/null 2>&1; then
	echo "-N2+e0,1 with -T should have failed but did not" >> fail
fi
