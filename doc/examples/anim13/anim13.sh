#!/usr/bin/env bash
#
# Movie of seismic waveforms from 1020-10-06 Alaska earthquake
# Epicenter 54.662N 159.675W
# UTC time 2020-10-19 20:54:40
#
# DEM:  @earth_relief_06m
# Data: @waveform_AV.DO.txt
# 
# The finished movie is available in our YouTube channel as well:
# https://youtu.be/xxxxxxxxxx
# The 66-sec, 1081 frame movie took 12 minutes to render on a 24-core MacPro 2013.

# 0. Create an include file with key parameters
cat << EOF > inc.sh
TIME_RANGE=2020-10-06T05:55:10/2020-10-06T05:56:15
PLOT_DOMAIN=2020-10-06T05:55:10/2020-10-06T05:56:15/-2.6/2.8
DPI=200i
SPLINE=linear
EOF
# 1. Create a dense point-file from the waveform data
cat << EOF > pre.sh
gmt begin
  # 1a. Create dense point-files from the waveform data components
  gmt psevents waveform_AV.DO.txt -R\${PLOT_DOMAIN} -JX20cT/3.5c -Ar\${DPI} -i0,1+s1e-6,0 -f2T --GMT_INTERPOLANT=\${SPLINE} > E.txt
  gmt psevents waveform_AV.DO.txt -R\${PLOT_DOMAIN} -JX20cT/3.5c -Ar\${DPI} -i0,2+s1e-6,0 -f2T --GMT_INTERPOLANT=\${SPLINE} > N.txt
  gmt psevents waveform_AV.DO.txt -R\${PLOT_DOMAIN} -JX20cT/3.5c -Ar\${DPI} -i0,3+s1e-6,0 -f2T --GMT_INTERPOLANT=\${SPLINE} > Z.txt
  # 1b. Setup the desired output times (24 frames per second)
  gmt math -T\${TIME_RANGE}/24+i -o0 T --TIME_UNIT=s --FORMAT_CLOCK_OUT=hh:mm:ss.xxxxx = times.txt
  # 1c. Create background page with location map and metadata
  gmt grdimage -R190W/130W/30/75 @earth_relief_06m -Ba30+f -JM3.75c -I+d -Cterra --MAP_FRAME_TYPE=plain -X0.75c -Y0.75c -BWNbr --MAP_ANNOT_OBLIQUE=lat_parallel --FONT_ANNOT_PRIMARY=9p
  echo 159.675W 54.662N | gmt plot -Sa0.5c -Gred -Wfaint
gmt end show
EOF
# 2. Set up main movie script
cat << EOF > main.sh
gmt begin
  gmt set FORMAT_DATE_MAP "o dd yyyy" FORMAT_CLOCK_MAP hh:mm FONT_ANNOT_PRIMARY +9p TIME_INTERVAL_FRACTION 0.01 FORMAT_CLOCK_MAP hh:mm:ss
    gmt subplot begin 3x1 -Fs18c/3.5c -A -M0 -R\${PLOT_DOMAIN} -X5.5c -Y0.75c
      gmt subplot set 0 -A"Z"
      gmt plot waveform_AV.DO.txt -Bpxafg -Bsxa1D -Bpyaf -BWSrt -W0.5p,darkgray -i0,3+s1e-6
      gmt events Z.txt -Sc0.5p -Gred -Es+d0.5+r0.5 -Ms4+c1 -Mi0.5 -Mt+c0 -T\${MOVIE_COL0}
      printf "%s -2.6\n%s 2.8\n" \${MOVIE_COL0} \${MOVIE_COL0} | gmt plot -W0.5p
      gmt subplot set 1 -A"N"
      gmt plot waveform_AV.DO.txt -Bpxafg -Bsxa1D -Bpyaf -BWSrt -W0.5p,darkgray -i0,2+s1e-6
      gmt events N.txt -Sc0.5p -Ggreen -Es+d0.5+r0.5 -Ms4+c1 -Mi0.5 -Mt+c0 -T\${MOVIE_COL0}
      printf "%s -2.6\n%s 2.8\n" \${MOVIE_COL0} \${MOVIE_COL0} | gmt plot -W0.5p
      gmt subplot set 2 -A"E"
      gmt plot waveform_AV.DO.txt -Bpxafg -Bsxa1D -Bpyaf -BWSrt -W0.5p,darkgray -i0,1+s1e-6
      gmt events E.txt -Sc0.5p -Gblue -Es+d0.5+r0.5 -Ms4+c1 -Mi0.5 -Mt+c0 -T\${MOVIE_COL0}
      printf "%s -2.6\n%s 2.8\n" \${MOVIE_COL0} \${MOVIE_COL0} | gmt plot -W0.5p
    gmt subplot end
gmt end show
EOF
# 3. Run the movie
gmt movie main.sh -Iinc.sh -Sbpre.sh -CHD -Ttimes.txt -Nanim13 -Lc0+jTR+o0.75c/0.5c+gwhite -H8 -Pb+w0.4c+jBR -Fmp4 -V -W -Zs -M165,png --FORMAT_DATE_MAP=- --FORMAT_CLOCK_OUT=hh:mm:ss.xxxxx
