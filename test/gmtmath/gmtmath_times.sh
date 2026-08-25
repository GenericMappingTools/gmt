#!/usr/bin/env bash
# Test that the -T option for gmt math works with appended time units

# Testing year interval
gmt math -o0 -T1980-01-01T/1988-12-31T/4y+t T = out
echo "1980-01-01T00:00:00" > in
echo "1984-01-01T00:00:00" >> in
echo "1988-01-01T00:00:00" >> in
# Testing month interval
gmt math -o0 -T1980-01-01T/1980-12-31T/4o T = >> out
echo "1980-01-01T00:00:00" >> in
echo "1980-05-01T00:00:00" >> in
echo "1980-09-01T00:00:00" >> in
# Testing day interval
gmt math -o0 -T1980-01-01T/1980-01-12T/4d T = >> out
echo "1980-01-01T00:00:00" >> in
echo "1980-01-05T00:00:00" >> in
echo "1980-01-09T00:00:00" >> in
# Testing hour interval
gmt math -o0 -T1980-01-01T00:00:00/1980-01-01T04:00:00/2h T = >> out
echo "1980-01-01T00:00:00" >> in
echo "1980-01-01T02:00:00" >> in
echo "1980-01-01T04:00:00" >> in
# Testing minute interval
gmt math -o0 -T1980-01-01T00:09:00/1980-01-01T00:11:00/1m T = >> out
echo "1980-01-01T00:09:00" >> in
echo "1980-01-01T00:10:00" >> in
echo "1980-01-01T00:11:00" >> in
# Testing second interval
gmt math -o0 -T1980-01-01T00:00:10/1980-01-01T00:00:30/10s T = >> out
echo "1980-01-01T00:00:10" >> in
echo "1980-01-01T00:00:20" >> in
echo "1980-01-01T00:00:30" >> in

diff out in --strip-trailing-cr > fail
