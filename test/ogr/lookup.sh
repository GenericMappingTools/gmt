#!/bin/bash
# Test that @D records after a new segment header are properly
#   processed so metadata is used to set color, etc.  Based on
# example in issue # 539.  Original plot faked with direct psxy plot.

ps=lookup.ps

cat << EOF > ogr.txt
# @VGMT1.0 @GPOLYGON
# @R-180.000002508/185.593483405/-53.6649710199/82.1554661247             
# @Jp"+proj=longlat +datum=WGS84 +no_defs " 
# @Jw"GEOGCS[\"wgs84\",DATUM[\"WGS_1984\",SPHEROID[\"WGS_1984\",6378137,298.257223563]],PRIMEM[\"Greenwich\",0],UNIT[\"degree\",0.0174532925199433]]" 
# @Ncat|minor_group|major_group|name|grassrgb|gmtrgb|major_group_name
# @Tinteger|integer|integer|string|string|string|string
# FEATURE_DATA
>
# @D5|43|5|Egypt|110:186:103|110/186/103|"Urbanized societies / kingdoms" 
# @P
22.3 32
24.2 32.3
26.0 34.7
24.4 34.6
22.3 32
>
# @D6|41|4|Norway|173:216:230|lightblue|"Urbanized societies / kingdoms" 
# @P
24.3 35
26.2 36.3
27.8 37.7
26.4 37.6
24.3 35
EOF
gmt psxy ogr.txt -R22/28/31/38 -JM6i -W1p -Baf -A -aG=gmtrgb -P > $ps
#
# For test to fail, orig was made with
#cat << EOF > ogr.txt
#> -G110/186/103
#22.3 32
#24.2 32.3
#26.0 34.7
#24.4 34.6
#22.3 32
#> -Glightblue
#24.3 35
#26.2 36.3
#27.8 37.7
#26.4 37.6
#24.3 35
#EOF
#gmt psxy ogr.txt -R22/28/31/38 -JM6i -W1p -Baf -A -Gred -P > $ps
