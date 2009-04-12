#!/bin/sh
# Test sample1d interpolation with NaNs
. ../functions.sh

makeps () {

# Must redirect sample1d's stderr messages to avoid seeing them for the 3 bad records
gmtset NAN_RECORDS skip
psbasemap -R-1/15/-3/3 -JX6i/3i -P -K -Y6i -B5f1g1:".Skipping NaNs and interpolating through": --HEADER_FONT_SIZE=18
psxy $tmp -R-1/15/-3/3 -JX6i/3i -Sc0.1i -W0.25p -Ggreen -O -K 2> /dev/null
gmtmath $tmp ISNAN 4 SUB = | psxy -R -J -O -K -St0.2i -Gblack -W0.25p
psxy $tmp -R -J -O -K -W2p,red 2> /dev/null
(sample1d $tmp -I0.1 -Fl | psxy -R -J -O -K -W2p,yellow,.) 2> /dev/null
(sample1d $tmp -I0.1 -Fc | psxy -R -J -O -K -W0.5p,blue,-) 2> /dev/null
(sample1d $tmp -I0.1 -Fa | psxy -R -J -O -K -W0.5p,blue  ) 2> /dev/null

# New behavior with upper case switches
gmtset NAN_RECORDS pass
psbasemap -R -J -O -K -Y-4.5i -B5f1g1:".Honoring NaNs as segment indicators": --HEADER_FONT_SIZE=18
psxy $tmp -R -J -Sc0.1i -W0.25p -Ggreen -O -K
gmtmath $tmp ISNAN 4 SUB = | psxy -R -J -O -K -St0.2i -Gblack -W0.25p
psxy $tmp -R -J -O -K -W2p,red
sample1d $tmp -I0.1 -Fl | psxy -R -J -O -K -W2p,yellow,.
sample1d $tmp -I0.1 -Fc | psxy -R -J -O -K -W0.5p,blue,-
sample1d $tmp -I0.1 -Fa | psxy -R -J -O -K -W0.5p,blue
echo "0 -4.5 16 0 0 LB Black triangles indicate NaN locations" | pstext -R -J -O -K -N

psxy -R -J -O /dev/null
}

ncgen -o $$.nc <<EOF
netcdf gaps {
dimensions:
        nr = 15 ;
variables:
        int x(nr) ;
        int y(nr) ;
                y:_FillValue = 999;
data:

 x = 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14 ;

 y = 0, 1, 1, 1, 999, 999, -1, 0, 1, 2, 999, 0, 0, 2, 1 ;
}
EOF

# First test with ASCII input
header "Test sample1d and psxy with NaNs indicating line gaps (ASCII)"
ps=gaps.ps
tmp=$$.txt
gmtconvert -bic $$.nc > $tmp
makeps > $ps
pscmp

# Do the same with netCDF input
header "Test sample1d and psxy with NaNs indicating line gaps (netCDF)"
tmp="-bic $$.nc"
makeps > $ps
pscmp
rm -f $$.nc $$.txt
