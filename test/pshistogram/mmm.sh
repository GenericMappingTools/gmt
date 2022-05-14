#!/usr/bin/env bash
# Test all the ways of labeling the bars with -D
ps=mmm.ps
# Use blockmean to compute the mean
data=`gmt which -Gc @Pacific_Depths.txt`
mean=`awk '{print 0,0,$1}' $data | gmt blockmean -R-1/1/-1/1 -I2 -r -o2`
# Use blockmedian to compute the median
median=`awk '{print 0,0,$1}' $data | gmt blockmedian -R-1/1/-1/1 -I2 -r -o2`
# Use blockmode to compute the mode
mode=`awk '{print 0,0,$1}' $data | gmt blockmode -R-1/1/-1/1 -I2 -r -o2`
peak=`awk '{print 0,0,$1}' $data | gmt blockmode -R-1/1/-1/1 -I2 -r -D50+c+l -o2`
gmt pshistogram $data -W50 -JX9i/6i -Bafg -BWSne+glightblue+t"Peak, Mode, Median, Mean" -R-6100/-500/0/700 -Gred -K -F > $ps
cat << EOF > tmp
> The mean
$mean	0
$mean	700
> The median
$median	0
$median	700
> The mode
$mode	0
$mode	700
> The peak
$peak	0
$peak	700
EOF
gmt psxy -R -J -O -K -W2p tmp >> $ps
gmt psbasemap -R -J -O -K -D-4600/-600/280/690 -F+glightgreen+p1p >> $ps
gmt pshistogram $data -W50 -JX5.5i/3i -Bafg -BWSne -R-6100/-4500/0/700 -L0.25p -Gred -O -K -F -X3i -Y2.75i >> $ps
gmt psxy -R -J -O -K -W2p tmp >> $ps
gmt pstext -R -J -O -F+jCM+a90+f12p -Gwhite -W0.25p << EOF >> $ps
$peak	600	peak
$mode	600	mode
$median	600	median
$mean	600	mean
EOF
rm -f tmp
