#!/usr/bin/env bash
# Testing 1-D greenspline with mix of data and gradients
# This also exercises -Q for 1-D and plots the solution slopes
# to show the match the input slopes (arrows).

# Create 7 data points
cat <<- EOF > oneD_data.txt
-2.5	4
-2	2
-1	-0.5
0	0
1	1
1.5	0
2	-2
EOF
# Create 2 gradient constraints
cat <<-EOF > oneD_grad.txt
0.5	1
2.5	-0.5
EOF
# Fit the data and gradients
gmt greenspline oneD_data.txt -AoneD_grad.txt+f0 -Z0 -R-3/3 -I0.01 -Sc -GoneD_out.txt
# Just fit the data
gmt greenspline oneD_data.txt  -Z0 -R-3/3 -I0.01 -Sc -GoneD_noA.txt
# Estimate gradient of solution at gradient constraint locations
gmt greenspline oneD_data.txt -AoneD_grad.txt+f0 -Z0 -NoneD_grad.txt -Sc -Q -GoneD_slp.txt
paste oneD_grad.txt oneD_slp.txt > oneD_grad2.txt

# Create data and text for slope lines and annotations
gmt math -T-2/2/0.5 T = | awk '{printf ">\n-1\t%s\n1\t%s\n", $1, -$1}' > lines.txt
cat <<- EOF >> lines.txt
> vertical line
0	-2
0	+2
EOF
gmt math -T-2/2/0.5 T = | awk '{printf "1 %s %.1lf\n", $1, $1}' > labels.txt

gmt begin gspline_7 ps
	gmt plot oneD_data.txt -R-4/4/-5/5 -Jx1i -Sc10p -Gblack -Baf -l"Values"+jBL
	gmt plot oneD_noA.txt -W3p,gray -l"No slopes used"
	gmt plot oneD_out.txt -W3p,red -l"With slope constraints"
	gmt plot oneD_out.txt -W0.25p
	LEG="-lData-Slopes"
	LEGS=-l"Solution-Slopes"
	# Loop over gradients and place grid and labels
	while read X S X2 Sout; do
		pattern="^${X}\t"	# Search pattern for finding y-value at gradient
		Y=$(grep ${pattern} oneD_out.txt | awk '{print $2}')
		# Lay down grid/labels after shifting origin to tangent point
		gmt plot -W0.25p,blue -X${X}i -Y${Y}i lines.txt
		the_Sout=$(gmt math -Q ${Sout} NEG =)
		gmt plot -W0.25p,green ${LEGS} <<- EOF
		-1	${the_Sout}
		1	${Sout}
		EOF
		gmt text -F+f12p+jML -Dj8p labels.txt
		echo 0 0 1 ${S} | gmt plot -Sv12p+e+s -W1p -Gblack
		the_X=$(gmt math -Q ${X} NEG =)
		the_Y=$(gmt math -Q ${Y} NEG =)
		echo 0 2 s = ${S} | gmt text -F+f12p+jCB -DJ3p
		# Undo shift and place gradient circle
		echo $X $Y | gmt plot -Sc10p -W0.5p ${LEG} -X${the_X}i -Y${the_Y}i
		LEG=
		LEGS=
	done < oneD_grad2.txt
gmt end
