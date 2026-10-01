#!/usr/bin/env bash
#
# -T's date modifier requires the +d prefix (-Tdcna[+d<date>][+z<TZ>]).
# A bare date glued to the type letters (e.g. -Td2020-01-01T00:00:00,
# missing +d) must be rejected, not silently ignored in favor of today's
# date (found while investigating gh#9220).

rm -f fail

# Missing +d: must now fail instead of silently using today's date.
gmt solar -Td2020-01-01T00:00:00+z08:00 -M > /dev/null 2>&1 && echo "-Td<date> without +d was silently accepted" >> fail

# Correct syntax: must succeed and actually honor the date (Jan vs Jun differ).
gmt solar -Td+d2020-01-01T00:00:00 -M > jan.txt 2>fail_jan
gmt solar -Td+d2020-06-01T00:00:00 -M > jun.txt 2>fail_jun
[ -s fail_jan ] && cat fail_jan >> fail
[ -s fail_jun ] && cat fail_jun >> fail
diff jan.txt jun.txt > /dev/null && echo "Jan and Jun terminators are identical - date is being ignored" >> fail
