#!/usr/bin/env bash
#	Testing gpsgridder for large region with weights
#	Work is being done by run_GPS_case.sh
# Due to hairline differences in many gridlines between Linux and macOS we need a
# higher rms threshold for this test to pass
# GRAPHICSMAGICK_RMS = 0.0435
ps=case_largeR_withW.ps
# Use real GPS data with uncertainties
data=$(gmt which -G @wus_gps_final_crowell.txt)
#  Large region
R=122.5W/115W/33N/38N
# blockmean interval
I=10m
# Gridding interval
G=0.2
# vector decimation
D=1
#  Select no weights:
W="-W"
${src:-.}/run_GPS_case_sub $data $R $I $G $D $W > $ps
