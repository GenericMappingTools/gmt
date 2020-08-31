#!/usr/bin/env bash
#
# Plot psxy text symbol for the man page

ps=GMT_base_symbols3.ps

cat << EOF > tmp.txt
# All the basic geometric psxy symbols
> -Gblack
1	1	1.5c l+tA+f1c,Times-Roman
> -Glightred 
2	1	1.5c l+tB+f1c,Helvetica-Bold
> -G-
3	1	1.25c l+tNO+f1c,Palatino-Bold
> -Glightblue
4	1	2c l+t*+f1c,NewCenturySchlbk-Bold
> -Gred -Wfaint
5	1	2c l+tp+f1c,Symbol
EOF
grep -v '>' tmp.txt | gmt psxy -R0.5/5.5/0.5/1.5 -B0g1 -B+n -Jx2c -Sc1.5c -W0.25p -P -K --PROJ_LENGTH_UNIT=cm -i0,1 --MAP_GRID_PEN_PRIMARY=default,dashed > $ps
gmt psxy tmp.txt -R -J -S -Glightred -W1p -O --PROJ_LENGTH_UNIT=cm >> $ps
