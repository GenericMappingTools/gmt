#!/usr/bin/env bash
# a is intercept, b is slope of fat line, m is slope of orthogonal
# xp, yp is nearest point on line to red point
# y is point on line at x = 4.5 and x is point on line at y = 1
b=0.397058823529
a=0.160294117647
m=-2.51851851852
angle=$(gmt math -Q $b ATAN R2D =)
y=$(gmt math -Q $a $b 4.5 MUL ADD =)
x=$(gmt math -Q 1 $a SUB $b DIV =)
xm=$(gmt math -Q $x 4.5 ADD 2 DIV =)
ym=$(gmt math -Q $y 1 ADD 2 DIV =)
xp=$(gmt math -Q 4.5 $b 1 MUL ADD  $b $a MUL SUB $b $b MUL 1 ADD DIV =)
yp=$(gmt math -Q 4.5 $b 1 MUL ADD $b MUL $a ADD $b $b MUL 1 ADD DIV =)
xn=$(gmt math -Q $xp 4.5 ADD 2 DIV =)
yn=$(gmt math -Q $yp 1 ADD 2 DIV =)
xa=$(gmt math -Q $x 0.4 ADD =)
a1=$(gmt math -Q $angle 180 ADD =)
a2=$(gmt math -Q $angle 270 ADD =)
gmt begin GMT_misfit ps
	gmt set GMT_THEME cookbook
	gmt plot -R1/6/0/3 -Jx2c -Glightgreen@35 <<- EOF
	$x	1
	4.5	1
	4.5	$y
	$x	$y
	EOF
	gmt plot -W <<- EOF
	> -W4p
	0.1 0.2
	6.9 2.9
	> -W0.5p
	4.5	1
	4.5	$y
	> -W0.5p
	4.5	1
	$x 1
	> -W1p
	4.5	1
	$xp	$yp
	EOF
	echo $x 1 0.8c 0 $angle | gmt plot -Sm8p -Gblack -W0.5p
	angle=$(gmt math -Q $b ATAN R2D 90 ADD =)
	echo 4.5 1 0.8c 90 $angle | gmt plot -Sm8p -Gblack -W0.5p
	echo $xp $yp 0.4c $a1 $a2 | gmt plot -SM8p -Gblack -W0.5p
	echo 4.5 1 | gmt plot -Sc0.3c -Gred
	echo $xp $yp | gmt plot -Sc4p -Gred
	gmt text -F+f10p,Times-Italic+j -Dj0.15c <<- EOF
	4.5 1 LT (X@-i@-,Y@-i@-)
	$xp $yp BR (x@-i@-,y@-i@-)
	4.5 $ym LM e@-y@-
	$xm 1 CT e@-x@-
	$xn $yn RM e@-o@-
	$xa 0.96 BL @~a@~
	EOF
gmt end show
