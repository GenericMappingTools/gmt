REM             GMT EXAMPLE 35
REM             $Id
REM
REM Purpose:    Illustrate sphtriangulate and sphdistance with GSHHG crude data
REM GMT progs:  pscoast, psxy, makecpt, grdimage, grdcontour, sphtriangulate, sphdistance
REM DOS calls:  del
REM

echo GMT EXAMPLE 35
set ps=example_35.ps

REM Get the crude GSHHS data, select GMT format, and decimate to ~20%:
REM gshhs gshhs_c.b | gawk '{if ($1 == ">" || NR%5 == 0) print $0}' > gshhs_c.txt
REM Get Voronoi polygons
gmt sphtriangulate gshhs_c.txt -Qv -D > tt.pol
REM Compute distances in km
gmt sphdistance -Rg -I1 -Qtt.pol -Gtt.nc -Lk
gmt makecpt -Chot -T0/3500/500 -Z > t.cpt
REM Make a basic image plot and overlay contours, Voronoi polygons and coastlines
gmt grdimage tt.nc -JG-140/30/7i -P -K -Ct.cpt -X0.75i -Y2i > %ps%
gmt grdcontour tt.nc -J -O -K -C500 -A1000+f10p,Helvetica,white -L500 -GL0/90/203/-10,175/60/170/-30,-50/30/220/-5 -Wa0.75p,white -Wc0.25p,white >> %ps%
gmt psxy -R -J -O -K tt.pol -W0.25p,green,. >> %ps%
gmt pscoast -R -J -O -W1p -Gsteelblue -A0/1/1 -B30g30 -B+t"Distances from GSHHG crude coastlines" >> %ps%
REM cleanup
del gmt.conf tt.pol tt.nc t.cpt
