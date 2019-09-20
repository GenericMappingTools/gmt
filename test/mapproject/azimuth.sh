#!/usr/bin/env bash
#
# Tests mapproject for azimuth calculation
# This is failing since the vectors are not parallel to the oblique
# meridians and parallels.  Yet the reported azimuth match calculations
# in MATLAB with the same equations as well as some online azimuth calculator
# Maybe the plotting of arrows are wrong?

gmt begin azimuth ps
  plon=186.3
  plat=-60.1
  omega=1.08
  lon=156.293957716
  lat=-8.06651671195
  # Get az from point to pole
  az=`echo $lon $lat | gmt mapproject -Af${plon}/${plat} -o2 -fg`
  # Add 90 to aziumth:
  az2=`gmt math -Q $az 90 ADD 360 FMOD =`
  gmt basemap -R140/210/-65/15 -JM6i -Bafg2 -BWEsn+o${plon}/${plat}+t"Parallel az = $az2 -Xc"
  echo ${plon} ${plat} | gmt plot -Sc0.1i -Gred
  echo ${lon} ${lat}   | gmt plot -Sc0.1i -Ggreen
  cat << EOF | gmt plot -W1p
${lon} ${lat}
${lon} 15
EOF
cat << EOF | gmt plot -W1p,red
${lon} ${lat}
${plon} ${plat}
EOF
  echo ${lon} ${lat} $az 1000 | gmt plot -S=0.15i+e -W1p,blue -Ggreen
  echo ${lon} ${lat} $az2 1000 | gmt plot -S=0.15i+e -W1p,blue -Ggreen
gmt end
