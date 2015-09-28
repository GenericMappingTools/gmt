#!/bin/bash
#	$Id
# Place 7 decorated lines with symbols along them
ps=decoratedlines.ps
cat << EOF > vert.txt
90	0
90	6
EOF
echo "> The first curve" > data.txt
gmt math -T0/180/1 T SIND = >> data.txt
echo "> The second curve -S_n5:+ss0.5i+p0.25p+gcyan" >> data.txt
gmt math -T0/180/1 T SIND 1 ADD = >> data.txt
echo "> The third curve -S_N15:+sc0.1i+gblue+a0" >> data.txt
gmt math -T0/180/1 T SIND 2 ADD = >> data.txt
echo "> The fourth curve -S_lLM/RM:+sn0.3i+gblack" >> data.txt
gmt math -T0/180/1 T SIND 2.5 ADD = >> data.txt
echo "> The fifth curve -S_xvert.txt:+si0.3i+p1p+ggreen -W2p" >> data.txt
gmt math -T0/180/1 T SIND 3 ADD = >> data.txt
echo "> The sixth curve -S_N15:+sd0.1i+p0.25p,red -Wfaint" >> data.txt
gmt math -T0/180/1 T SIND 4 ADD = >> data.txt
echo "> The seventh curve -S_n5:+ss0.5i+p0.25p+gpurple+a0 -W1p,orange" >> data.txt
gmt math -T0/180/1 T SIND 4.5 ADD = >> data.txt
gmt psxy -R-5/185/-0.1/6 -JX6i/9i -P -Baf -W1p,red -S_n3:+sa0.5i+p0.25p,green+gblue data.txt > $ps
