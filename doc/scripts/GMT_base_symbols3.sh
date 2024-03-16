#!/usr/bin/env bash
#
# Plot psxy text symbol for the man page

gmt begin GMT_base_symbols3
    gmt set GMT_THEME cookbook
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
grep -v '>' tmp.txt | gmt plot -R0.5/5.5/0.5/1.5 -B0g1 -B+n -Jx2c -Sc1.5c -W0.25p --PROJ_LENGTH_UNIT=cm -i0,1 --MAP_GRID_PEN_PRIMARY=default,dashed
gmt plot tmp.txt -S -Glightred -W1p --PROJ_LENGTH_UNIT=cm
gmt end show
