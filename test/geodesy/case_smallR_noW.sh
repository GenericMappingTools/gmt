#!/usr/bin/env bash
#	Testing gpsgridder for small region without weights
#	Work is being done by run_GPS_case.sh
# Due to hairline differences in many gridlines between Linux and macOS we need a
# higher rms threshold for this test to pass
# GRAPHICSMAGICK_RMS = 0.0655
ps=case_smallR_noW.ps
# Use real GPS data with uncertainties
data=$(gmt which -G @wus_gps_final_crowell.txt)
#  Small region
R=118.5W/115.2W/33.0N/34.5N
# blockmean interval
I=1m
# Gridding interval
G=0.01
# vector decimation
D=10
#  Select no weights:
W=""
${src:-.}/run_GPS_case_sub $data $R $I $G $D $W > $ps
