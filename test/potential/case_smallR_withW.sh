#!/bin/bash
#	$Id$
#	Testing gpsgridder for small region with weights
#	Work is being done by run_GPS_case.sh
#
ps=case_smallR_withW.ps
# Use real GPS data with uncertainties
data=wus_gps_final_crowell.dat 
#  Select small region
R=118.5W/115.2W/33.0N/34.5N
# blockmean interval
I=1m
# Gridding interval
G=0.01
# vector decimation
D=10
#  Select weights:
W="-W"
run_GPS_case.sh $data $R $I $G $D $W > $ps
