#!/bin/bash
# Test sample1d interpolation with NaNs
. functions.sh

makeps () {

# Must redirect sample1d's stderr messages to avoid seeing them for the 3 bad records
gmtset IO_NAN_RECORDS skip
psbasemap -R-1/15/-3/3 -JX6i/3i -P -K -Y6i -B5f1g1:".Skipping NaNs and interpolating through": --FONT_TITLE=18p
psxy $tmp -R-1/15/-3/3 -JX6i/3i -Sc0.1i -W0.25p -Ggreen -O -K 2> /dev/null
gmtmath $tmp ISNAN 4 SUB = | psxy -R -J -O -K -St0.2i -Gblack -W0.25p
psxy $tmp -R -J -O -K -W2p,red 2> /dev/null
(sample1d $tmp -I0.1 -Fl | psxy -R -J -O -K -W2p,yellow,.) 2> /dev/null
(sample1d $tmp -I0.1 -Fc | psxy -R -J -O -K -W0.5p,blue,-) 2> /dev/null
(sample1d $tmp -I0.1 -Fa | psxy -R -J -O -K -W0.5p,blue  ) 2> /dev/null

# New behavior with upper case switches
gmtset IO_NAN_RECORDS pass
psbasemap -R -J -O -K -Y-4.5i -B5f1g1:".Honoring NaNs as segment indicators": --FONT_TITLE=18p
psxy $tmp -R -J -Sc0.1i -W0.25p -Ggreen -O -K
gmtmath $tmp ISNAN 4 SUB = | psxy -R -J -O -K -St0.2i -Gblack -W0.25p
psxy $tmp -R -J -O -K -W2p,red
sample1d $tmp -I0.1 -Fl | psxy -R -J -O -K -W2p,yellow,.
sample1d $tmp -I0.1 -Fc | psxy -R -J -O -K -W0.5p,blue,-
sample1d $tmp -I0.1 -Fa | psxy -R -J -O -K -W0.5p,blue
pstext -R -J -F+f16p+jBL -O -K -N <<< "0 -4.5 Black triangles indicate NaN locations"

psxy -R -J -O -T
}

# First test with ASCII input
header "Test sample1d and psxy with NaNs indicating line gaps (ASCII)"
ps=gaps.ps
tmp=tt.txt
gmtconvert $src/gaps.nc > $tmp
makeps > $ps
pscmp

# Do the same with netCDF input
header "Test sample1d and psxy with NaNs indicating line gaps (netCDF)"
tmp=$src/gaps.nc
makeps > $ps
pscmp
