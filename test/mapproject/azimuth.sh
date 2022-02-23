#!/usr/bin/env bash
#
# Tests mapproject for azimuth calculation
# Important to use backazimuth in this example!

gmt begin azimuth ps
  plon=186.3
  plat=-60.1
  lon=156.293957716
  lat=-8.06651671195
  # Get az from point to pole
  az=$(echo $lon $lat | gmt mapproject -Ab${plon}/${plat} -o2 -fg)
  # Add 90 to aziumth:
  az2=$(gmt math -Q $az 90 SUB 360 FMOD =)
  gmt basemap -R140/210/-65/15 -JM6i -Bafg2 -BWEsn+o${plon}/${plat}+t"Parallel az = $az2"
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
gmt end show
