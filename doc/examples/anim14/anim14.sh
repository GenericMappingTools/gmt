#!/usr/bin/env bash
#
# Movie of focal mechanisms from 09-1981 to 01-2018 in the flat subduction area
# in western Argentina and central Chile. Shows a map with a profile.
# Global CMT web page: https://www.globalcmt.org/
#
# DEM:  @earth_relief_03s
# Data: @GCMT_1976-2017_meca.gmt 
#
# The original data file jan76_dec17.ndk can be downloaded and rearranged to a gmt format with the followings commands:
# URL="https://www.ldeo.columbia.edu/~gcmt/projects/CMT/catalog/jan76_dec17.ndk"
# gmt which $URL -G 
# gawk '/^PDE/ {Date=$2; Time=$3; Lat=$4; Long=$5; Depth=$6; getline; Name=$1; getline; getline; Exp=$1; getline; mant=$11; strike1=$12; dip1=$13; rake1=$14; strike2=$15; dip2=$16; rake2=$17; print Long, Lat, Depth, strike1, dip1, rake1, strike2, dip2, rake2, mant, Exp, Date "T" Time, Name}' jan76_dec17.ndk | sed 's/\//-/g' > meca.gmt
# gmt select meca.gmt -R-75.1/-63/-34.44/-30.35 > GCMT_1976-2017_meca.gmt
# The GCMT_1976-2017_meca.gmt file is in the GMT Dataserver cache and can be accessed via @GCMT_1976-2017_meca.gmt
# The finished movie is available in our YouTube channel as well:
# https://youtu.be/Wk58r72g_nk
# The 79-sec, 1896 frame movie took ~50 minutes to render on a 2017 iMac Pro.
#
# Author: ESTEBAN, Federico D.
# Profile coordinates (in degrees) and perpendicular distance to filter the data and to show in the cross section.
Long1=-75.02
Long2=-63.65
Lat1=-33.5
Lat2=-31
Dist=100k
REGION=-75.1/-63/-34.44/-30.35	# Map Region
# Angles (min/max/inc) for the profile inset
Angles=15/60/15 
DEM=@earth_relief_03s
# Other Variables
W=22.73   			# Width Profile
H=2.7c                   	# Profile Height
PROJ=M23.78c			# Map projection and width
X=0.115c			# Offset in X
Y=0.91c				# Offset in Y
# Create Profile
cat << EOF > tmp_profile
$Long1 $Lat1
$Long2 $Lat2
EOF
# Filter Focal Mechanism. Create file with data Inside/Outside.
gmt select @GCMT_1976-2017_meca.gmt -Ltmp_profile+d$Dist+p -fg > coupe_I.gmt
gmt select @GCMT_1976-2017_meca.gmt -Ltmp_profile+d$Dist+p -fg > coupe_O.gmt -Il
# Calculate variables (profile length, maximum depth of events (+10 to avoid clipping in the profile), vertical exaggeration, table of angles).
KM=$(echo $Long1 $Lat1 | gmt mapproject -G$Long2/$Lat2+uk -o2)
DepthMax=$(gmt info coupe_I.gmt -C2 -o5 | gmt math -Q STDIN 10 ADD =)
VE=$(gmt math -Q $KM $H MUL $W DIV $DepthMax DIV = --FORMAT_FLOAT_OUT=%.2g)
gmt math -T$Angles T SIND -o0,1,0 = | gmt math STDIN -T -C0 COSD = | gawk '$0=$0"@."' > angles.txt
Ha=$(gmt math -Q $H 0.7c SUB =) # Height of inset with angles
Wa=$(gmt math -Q $Ha $VE DIV =) # Width of inset with angles (calculated from vertical exaggeration to have same deformation as the profile)
#       -----------------------------------------------------------------------------------------------------------
cat << EOF > pre.sh
	gmt set FONT_LABEL 10p FONT_ANNOT_PRIMARY 7p MAP_FRAME_PEN thin,black MAP_GRID_PEN faint,gray
gmt begin
	# 1a. Create list of dates for the animation
	gmt math -o0 -T1981-09-01T/2018-01-02T/7d T = times.txt
	# 1b. Create cpt to paint the focal mechanism
	gmt makecpt -T0/$DepthMax -Chot -I -H > q.cpt
	# 1c. Draw profile
	gmt basemap -R0/$KM/0/$DepthMax -JX$W/-$H -Bg25 -BwESn -Bxaf+l"Distance (km)" -Byaf+l"Depth (km)" -X$X -Y$Y
	gmt inset begin -DjLB+w$Wa/$Ha+o0.1c/0.2c -F+p+gwhite+s+r -C0.15c
		gmt plot angles.txt -Wthinnest,red -Bnw -Bg -R0/1/0/1 -JX?/-? --MAP_FRAME_TYPE=graph -Fr0/0 -i0-1+s0.9
		gmt text angles.txt -F+f6 -N
	gmt inset end
	echo Vert. Ex. = $VE | gmt text -F+cBR+f9p -Gwhite -W1p
	echo SW | gmt text -F+cTL+f12p -Gwhite -W1p
	echo NE | gmt text -F+cTR+f12p -Gwhite -W1p
	# 1d. Draw map
	gmt grdimage $DEM -Coleron -I+nt1.2 -R$REGION -J$PROJ -Yh+0.3c
	gmt coast -Df -N1/0.5p -Bf
	gmt plot tmp_profile -W2p,red
	gmt plot tmp_profile -Sc0.25c -Gred
	gmt colorbar -Cq.cpt -DjBR+o1.75c/0.6c+w-6c/0.5c -Baf+l"Focal Depth (km)" -F+p+gwhite+s+r
	gmt inset begin -DjTL+w3.5c+o-0.08c
		gmt coast -Rg -JG-68.025/-32.01/? -Da -Gwhite -A5000 -Bg -W1/faint -Sdodgerblue2 -C- -N1
		gmt basemap -R$REGION -J$PROJ -A | gmt plot -Wthin,darkred 
	gmt inset end
gmt end
EOF
# 2. Set up main script
cat << EOF > main.sh
gmt begin
	gmt set TIME_UNIT d
	gmt events coupe_I.gmt -Wfaint -T\${MOVIE_COL0} -Es+r21+p42+d35+f1 -Mi0.6 -Ms1.5+c0.8 -Mt+c10 -R0/$KM/0/$DepthMax -JX$W/-$H -Y$Y -X$X -Z"coupe -Aa$Long1/$Lat1/$Long2/$Lat2+w$Dist -Q -Sc0.3c+f0" -Cq.cpt 
	gmt events coupe_O.gmt -Wfaint -T\${MOVIE_COL0} -Es+r21+p42+d35+f1 -Mi0.6 -Ms1.5+c0.8 -Mt+c10 -R$REGION -J$PROJ -Z"meca -Sc0.3c+f0" -Yh+0.3c
	gmt events coupe_I.gmt -Wfaint -T\${MOVIE_COL0} -Es+r21+p42+d35+f1 -Mi0.6 -Ms1.5+c0.8 -Mt+c10 -R$REGION -J$PROJ -Z"meca -Sc0.3c+f0" -Cq.cpt
gmt end
EOF
# 3. Run the movie
gmt movie main.sh -Sbpre.sh -Chd -Ttimes.txt -Nanim14 -H2 -Fmp4 -D24 -Ml,png -Ve -Pd+ac0+jRM+w5.9c+o2.7/0.8c+P3,white+p1,red+a1+f11p,2,white \
--FORMAT_CLOCK_MAP=- --FORMAT_DATE_MAP=dd-mm-yyyy --TIME_EPOCH=0000-01-01 -Zs
rm coupe_*.gmt tmp_profile angles.txt
