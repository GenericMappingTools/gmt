#!/bin/bash
# Exploring NY traffic across V-N bridge, 2018-2020
# First just show data, with gaps > 6 hours not connected
# Notice May 2018 spikes, data gaps in the summer of 2018, Covid drop.
gmt begin GMT_cycle_6
	gmt set GMT_THEME cookbook
	gmt set TIME_WEEK_START Monday
	gmt subplot begin 2x2 -Fs15c/8c -M14p/6p -A -T"Verrazzano-Narrows Bridge @%34%\337@%% Brooklyn Traffic [2018-2020]" -BWSrt
 		# Raw time-series
		gmt plot @NY_traffic.txt -Wfaint,red -By+l"Vehicles/hour" -gx21600 -c
		# Weekly wrapped time-series
 		gmt plot @NY_traffic.txt -R0/7/0/17000 -Wfaint,red -ww -gx0.25 -c
		# Weekly histogram normalized for number of hours for a weekday in the 3-year data
		n_week_hours=$(gmt convert @NY_traffic.txt -ww | gmt select -Z0/0.999+c0 | gmt info -Fi -o2)
 		gmt histogram @NY_traffic.txt -R0/7/2000/4000 -Wthick -Gred -T1 -Z0+w -By+l"Vehicles/hour" -i0,1+d${n_week_hours} -ww -c
		# Daily histogram normalized for number of days in the 3-year data
		n_mondays=$(gmt convert @NY_traffic.txt -wd | gmt select -Z-0.5/0.5+c0 | gmt info -Fi -o2)
 		gmt histogram @NY_traffic.txt -R0/24/0/8000 -Wthick -Gred -T1 -Z0+w -i0,1+d${n_mondays} -wd -c
 	gmt subplot end
gmt end show
