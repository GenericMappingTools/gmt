#!/usr/bin/env bash
#
# Movie of seismic waveforms from 1020-10-06 Alaska earthquake
#
# DEM:  
# Data: @waveform_AV.DO.txt
# 
# The finished movie is available in our YouTube channel as well:
# https://youtu.be/xxxxxxxxxx
# The movie took ~x.xx hour to render on a 24-core MacPro 2013.

# 0. Create an include file with key parameters
cat << EOF > inc.sh
TIME_RANGE=2020-10-06T05:55:00/2020-10-06T05:56:15
PLOT_DOMAIN=2020-10-06T05:55:00/2020-10-06T05:56:15/-2.6/2.8
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
  # 1b. Setup the desired output times (once per second)
  gmt math -T\${TIME_RANGE}/1 -o0 T --TIME_UNIT=s = times.txt
gmt end
EOF
# 2. Set up main movie script
cat << EOF > main.sh
gmt begin
  gmt set FORMAT_DATE_MAP "o dd yyyy" FORMAT_CLOCK_MAP hh:mm FONT_ANNOT_PRIMARY +9p TIME_INTERVAL_FRACTION 0.01 FORMAT_CLOCK_MAP hh:mm:ss
    gmt subplot begin 3x1 -Fs20c/3.5c -A -M0 -R\${PLOT_DOMAIN} -X3.5c -Y0.75c
      gmt subplot set 0 -A"Z"
      gmt plot waveform_AV.DO.txt -Bpxafg -Bsxa1D -Bpyaf -BWSrt -W0.5p,darkgray -i0,3+s1e-6
      gmt events Z.txt -Sc0.5p -Gred -Es+d0.5+r0.5 -Ms4+c1 -Mi0.5 -Mt+c0 -T\${MOVIE_COL0}
      printf "%s -2.6\n%s 2.8\n" \${MOVIE_COL0} \${MOVIE_COL0} | gmt plot -W1p,black
      gmt subplot set 1 -A"N"
      gmt plot waveform_AV.DO.txt -Bpxafg -Bsxa1D -Bpyaf -BWSrt -W0.5p,darkgray -i0,2+s1e-6
      gmt events N.txt -Sc0.5p -Gred -Es+d0.5+r0.5 -Ms4+c1 -Mi0.5 -Mt+c0 -T\${MOVIE_COL0}
      printf "%s -2.6\n%s 2.8\n" \${MOVIE_COL0} \${MOVIE_COL0} | gmt plot -W1p,black
      gmt subplot set 2 -A"E"
      gmt plot waveform_AV.DO.txt -Bpxafg -Bsxa1D -Bpyaf -BWSrt -W0.5p,darkgray -i0,1+s1e-6
      gmt events E.txt -Sc0.5p -Gred -Es+d0.5+r0.5 -Ms4+c1 -Mi0.5 -Mt+c0 -T\${MOVIE_COL0}
      printf "%s -2.6\n%s 2.8\n" \${MOVIE_COL0} \${MOVIE_COL0} | gmt plot -W1p,black
    gmt subplot end
gmt end show
EOF
# 3. Run the movie
#gmt movie main.sh -Iinc.sh -Sbpre.sh -CHD -Ttimes.txt -Nanim13 -Lc0 -H8 -Pb+w1c+jBL -Fmp4 -V -W -Zs --FORMAT_DATE_MAP=-
#gmt movie main.sh -Iinc.sh -Sbpre.sh -CHD -Ttimes.txt -Nanim13 -Lc0 -H8 -Pb+w1c+jBL -M45,png -Fnone -V -W -Zs --FORMAT_DATE_MAP=-
