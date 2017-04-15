#!/bin/bash
#	$Id$
#	Testing gpsgridder for large region with weights
#	Work is being done by run_GPS_case.sh
#
ps=case_largeR_withW.ps
# Use real GPS data with uncertainties
data=wus_gps_final_crowell.dat 
#  Large region
R=122.5W/115W/32.7N/38N
# blockmean interval
I=10m
# Gridding interval
G=0.2
# vector decimation
D=1
#  Select no weights:
W="-W"
${src:-.}/run_GPS_case.sub $data $R $I $G $D $W > $ps
