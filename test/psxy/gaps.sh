#!/usr/bin/env bash
# Test gmt sample1d interpolation with NaNs

makeps () {

# Must redirect gmt sample1d's stderr messages to avoid seeing them for the 3 bad records
gmt set IO_NAN_RECORDS skip
R=-R-1/15/-3/3
gmt psbasemap $R -JX6i/3i -P -K -Y6i -B5f1g1 -B+t"Skipping NaNs and interpolating through" --FONT_TITLE=18p
gmt psxy $tmp $R -JX6i/3i -Sc0.1i -W0.25p -Ggreen -O -K 2> /dev/null
gmt math $tmp ISNAN 4 SUB = | gmt psxy -R -J -O -K -St0.2i -Gblack -W0.25p
gmt psxy $tmp $R -J -O -K -W2p,red 2> /dev/null
(gmt sample1d $tmp -I0.1 -Fl | gmt psxy $R -J -O -K -W2p,yellow,.) 2> /dev/null
(gmt sample1d $tmp -I0.1 -Fc | gmt psxy $R -J -O -K -W0.5p,blue,-) 2> /dev/null
(gmt sample1d $tmp -I0.1 -Fa | gmt psxy $R -J -O -K -W0.5p,blue  ) 2> /dev/null

# New behavior with upper case switches
gmt set IO_NAN_RECORDS pass
gmt psbasemap $R -J -O -K -Y-4.5i -B5f1g1 -B+t"Honoring NaNs as segment indicators" --FONT_TITLE=18p
gmt psxy $tmp $R -J -Sc0.1i -W0.25p -Ggreen -O -K
gmt math $tmp ISNAN 4 SUB = | gmt psxy $R -J -O -K -St0.2i -Gblack -W0.25p
gmt psxy $tmp $R -J -O -K -W2p,red
gmt sample1d $tmp -I0.1 -Fl | gmt psxy -R -J -O -K -W2p,yellow,.
gmt sample1d $tmp -I0.1 -Fc | gmt psxy -R -J -O -K -W0.5p,blue,-
gmt sample1d $tmp -I0.1 -Fa | gmt psxy -R -J -O -K -W0.5p,blue
gmt pstext $R -J -F+f16p+jBL -O -K -N <<< "0 -4.5 Black triangles indicate NaN locations"

gmt psxy $R -J -O -T
}

# First test with ASCII input
ps=gaps_ascii.ps
tmp=tt.txt
gmt convert gaps.nc > $tmp
makeps > $ps

# Do the same with netCDF input
ps=gaps_netcdf.ps
tmp=gaps.nc
makeps > $ps

psref=gaps.ps
