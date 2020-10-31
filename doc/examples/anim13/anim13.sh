#!/usr/bin/env bash
#
# Movie of seismic waveforms from ....
#
# DEM:  
# Data: @waveform_AV.DO.txt
# 
# The finished movie is available in our YouTube channel as well:
# https://youtu.be/xxxxxxxxxx
# The movie took ~x.xx hour to render on a 24-core MacPro 2013.

# 0. Create an include file with key parameters
cat << EOF > inc.sh
TRANGE=2020-10-06T05:55:00/2020-10-06T05:56:15
DOMAIN=2020-10-06T05:55:00/2020-10-06T05:56:15/-2600000/2800000
DPI=200i
SPLINE=linear
EOF
# 1. Create a dense point-file from the waveform data
cat << EOF > pre.sh
gmt begin
  # 1a. Create dense point-files from the waveform data components
  gmt psevents waveform_AV.DO.txt -R\${DOMAIN} -JX20cT/4c -Ar\${DPI} -i0,1,0 -f2T --GMT_INTERPOLANT=\${SPLINE} > E.txt
  gmt psevents waveform_AV.DO.txt -R\${DOMAIN} -JX20cT/4c -Ar\${DPI} -i0,2,0 -f2T --GMT_INTERPOLANT=\${SPLINE} > N.txt
  gmt psevents waveform_AV.DO.txt -R\${DOMAIN} -JX20cT/4c -Ar\${DPI} -i0,3,0 -f2T --GMT_INTERPOLANT=\${SPLINE} > Z.txt
  # 1b. Setup the desired output times (once per second)
  gmt math -T\${TRANGE}/1 -o0 T --TIME_UNIT=s = times.txt
gmt end
EOF
# 2. Set up main movie script
cat << EOF > main.sh
gmt begin
  gmt set FORMAT_DATE_MAP "o dd yyyy" FORMAT_CLOCK_MAP hh:mm FONT_ANNOT_PRIMARY +9p TIME_INTERVAL_FRACTION 0.01 FORMAT_CLOCK_MAP hh:mm:ss
    gmt subplot begin 3x1 -Fs20c/4c -A -M2p -R\${DOMAIN} -X2c -Y1.5c
      gmt subplot set 0 -A"Z"
      gmt events Z.txt -Sc0.5p -Gred -Es+d0.5+r0.5 -Ms4+c1 -Mi0.5 -Mt+c0 -T\${MOVIE_COL0} -Bpxafg -Bsxa1D -Bpyafg1 -BWSrt 
      gmt subplot set 1 -A"N"
      gmt events N.txt -Sc0.5p -Gred -Es+d0.5+r0.5 -Ms4+c1 -Mi0.5 -Mt+c0 -T\${MOVIE_COL0} -Bpxafg -Bsxa1D -Bpyafg1 -BWSrt 
      gmt subplot set 2 -A"E"
      gmt events E.txt -Sc0.5p -Gred -Es+d0.5+r0.5 -Ms4+c1 -Mi0.5 -Mt+c0 -T\${MOVIE_COL0} -Bpxafg -Bsxa1D -Bpyafg1 -BWSrt 
    gmt subplot end
gmt end show
EOF
# 3. Run the movie, requesting a fade in/out via white
#gmt movie main.sh -Iinc.sh -Sbpre.sh -CHD -Ttimes.txt -Nanim13 -Lc0 -Ls"Waveform due to Earthquake"+jTC -H8 -Pb+w1c+jBL -Fmp4 -V -W -Zs --FORMAT_DATE_MAP=-
#gmt movie main.sh -Iinc.sh -Sbpre.sh -CHD -Ttimes.txt -Nanim13 -Lc0 -H8 -Pb+w1c+jBL -M45,png -Fnone -V -W -Zs --FORMAT_DATE_MAP=-
