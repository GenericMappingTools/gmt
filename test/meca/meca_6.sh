#!/bin/bash
#	$Id: meca_3.sh 12114 2013-09-03 19:19:00Z fwobbe $
# Testing large isotropic component case by Jeremy Pesicek
# as indicated by email:
#----- Original Message ----
#From: Jeremy Pesicek <pesicek@GEOLOGY.WISC.EDU>
#To: GMT-HELP@lists.hawaii.edu
#Sent: Mon, November 8, 2010 2:30:19 PM
#Subject: Re: [GMT-HELP] psmeca help
#
#Thank you Dan and Joaquim.
#
#I was hoping to use psmeca to plot moment  tensors with possibly large non-DC 
#components.  But I'm not sure that the  problem is necessarily due only to these 
#large components.
#
#take for  example this tensor:
#
#echo "3 3 0 0.961 0.204 -0.895 1.045 -0.307 0.716 2"  | psmeca -JX6 -R0/6/0/6 
#-Sm6 -N -M -T0 -G200 > MT.ps
#
#It is very close  to being pure DC (~95%).  Its best fit DC is:
#
#echo  "3 3 0  4.319 37.28 145.8  6" | psmeca -JM8 -R0/6/0/6 -Sa6 -G200 >  
#out.ps
#

ps=meca_6.ps

echo "1 3 0 0.961 0.204 -0.895 1.045 -0.307 0.716 2 0 0 95% double-couple [-Sm]"  | \
	gmt psmeca -JX6i -R-1/6/0/6 -Sm6c -N -M -T0 -G200 -K -P -Baf > $ps
echo  "4 3 0  4.319 37.28 145.8  6 0 0 100% double-couple [-Sa]" | gmt psmeca -J -R -Sa5c -O -G200 >> $ps
