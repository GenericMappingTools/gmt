#!/usr/bin/env bash
# Test gmtflexure for basic functionality
flex () {	# $1 is width, $2 is Te, $3 is dy build a 4.05 km tall ridge and compute flexure
	gmt math -T-800/800/1 T ABS $1 LT 4.05 MUL = topo.txt
	gmt flexure -Qttopo.txt -D3300/2700/2300/1030 -E${2}k -Mx > flex.txt
	R=$(gmt info -I100/1 flex.txt topo.txt)
	gmt psxy $R -JX3i/1.4i -O -K -W1p flex.txt -Y1.65i -Bafg1000 $3
	gmt psxy -R -J -O -K -Ggray topo.txt
	gmt pstext -R -J -O -K -F+cTR+f9p+jTR+t"Te = $2, W = $1" -Dj0.03i
}

ps=flex2d.ps
cat << EOF > Te.txt
0
2
5
15
25
50
EOF
cat << EOF > W.txt
10
20
50
80
100
200
EOF
# Plot variability with Te for fixed W = 50
row=0
gmt psxy -R0/6/0/9 -Jx1 -P -X1i -K -T -Y-1.0i > $ps
B=-BWSne
while read Te; do
	flex 50 $Te $B >> $ps
	let row=row+1
	B=-BWsne
done < Te.txt
gmt psxy -R0/6/0/9 -Jx1 -O -K -T -Y-9.9i -X3.4i >> $ps
# Plot variability with W for fixed Te = 20
row=0
B=-BwSnE
while read W; do
	flex $W 20 $B >> $ps
	let row=row+1
	B=-BwsnE
done < W.txt
gmt psxy -R -J -O -T >> $ps
