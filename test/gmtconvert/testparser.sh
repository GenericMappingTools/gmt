#!/usr/bin/env bash
# Test gmt convert with strings.txt and determine if we match those

gmt convert -/ strings.txt | grep -v '#' > output.txt
$AWK '{if ($2 != $3) print $0}' output.txt > fail
