#!/usr/bin/env bash
# Make a figure illustrating the approximation done in grdflexure
# when infill density is < load density.
# Data file @Wessel_GJI_Fig_5.txt has these items:
# col 0 = x, col 1 = exact with shol != rhoi, 2 = exact solution (rhol), 3 = exact solution (rhoi), 4 approximate solution
# Modified from Wessel [2016, GJI].
gmt begin grdflexure_approx
	gmt set GMT_THEME cookbook
	gmt set FONT_ANNOT_PRIMARY 9p,Helvetica,black FONT_LABEL 12p,Helvetica,black PS_MEDIA letter MAP_VECTOR_SHAPE 0.5
	# Normal
	cat <<- EOF > line.txt
	-330	75.5
	330	75.5
	EOF
	cat <<- EOF >> load.txt
	-15	-500
	15	-500
	15	300
	-15	300
	EOF
	cat <<- EOF >> load2.txt
	-15	-700
	15	-700
	15	300
	-15	300
	EOF
	# Exact FFT when rhol = rhoi on middle row
	gmt plot -R-330/270/-1000/500 -JX6i/2i -T -X1.5i -Y4i
	gmt plot @Wessel_GJI_Fig_5.txt -i0,1o+73 -Gperu -W0.25p
	gmt plot -Gperu -W0.25p load2.txt
	gmt plot @Wessel_GJI_Fig_5.txt -i0,1 -W14p,gray
	gmt plot line.txt -W0.25p
	gmt plot -W0.25p,. <<- EOF
	0	-667.5
	205	-667.5
	EOF
	echo -195.5 120 195.5 120 | gmt plot -Sv0.2i+s+b+e -Gblack -W2p -N
	echo 195.5 75.7 195.5 -667.5 | gmt plot -Sv0.2i+s+b+e -Gblack -W2p -N
	echo 0 300 0 600 | gmt plot -Sv0.3i+s+b+jb+a60 -Gblack -W10p -N

	gmt text -F+f+j -N <<- EOF | gmt text -F+f+j
	-330 305 16p LB @~r@~@-i@- = @~r@~@-l@-
	-330 225 14p LT [Exact]
	-300 40 11p LT CRUST
	210 -667.5 16p,6 LM w@-l@-
	97.5 250 16p,6 CM @~l@~@-l@- > @~l@~@-exact@-
	40 -50 16p,6 CM @~r@~@-l@-
	EOF
	echo "b)" | gmt text -F+cTR+f18p,Helvetica+jTR -Dj0.0i/0 -N

	# Exact with rhol != rhoi on top row
	gmt plot @Wessel_GJI_Fig_5.txt -i0,2o+73 -Glightbrown -Y1.75i
	gmt plot -Gperu -W0.25p load.txt
	gmt plot  @Wessel_GJI_Fig_5.txt -i0,2 -W14p,gray
	gmt plot line.txt -W0.25p
	gmt plot -W0.25p,. <<- EOF
	0	-455.75
	205	-455.75
	EOF
	echo -164.3 120 164.3 120 | gmt plot -Sv0.2i+s+b+e -Gblack -W2p -N
	echo 164.3 75.7 164.3 -455.75 | gmt plot -Sv0.2i+s+b+e -Gblack -W2p -N
	echo 0 300 0 600 | gmt plot -Sv0.3i+s+b+jb+a60 -Gblack -W10p -N
	gmt text -F+f+j -N <<- EOF
	-330 305 16p LB @~r@~@-i@- @~\271 r@~@-l@-
	-330 225 14p LT [Exact]
	-300 40 11p LT CRUST
	210 -455.75 16p,6 LM w@-exact@-
	82 250 16p,6 CM @~l@~@-exact@-
	0 -50 16p,6 CM @~r@~@-l@-
	40 -50 16p,6 CM @~r@~@-i@-
	EOF
	echo "a)" | gmt text -F+cTR+f18p,Helvetica+jTR -Dj0.0i/0 -N

	# Wessel approx for FFT on bottom
	#gmt plot -R-330/270/-750/500 -JX6i/2.5i @Wessel_GJI_Fig_5.txt -i0,4o+73 -Glightbrown -Bxaf+u" km" -BS -Y-4.15i
	gmt plot -R-330/270/-650/500 -JX6i/2.3i @Wessel_GJI_Fig_5.txt -i0,4o+73 -Glightbrown -Bxaf+u" km" -BS -Y-4.0i
	gmt plot -Glightbrown -W0.25p load.txt
	gmt plot @Wessel_GJI_Fig_5.txt -i0,4+o022 -W14p,gray
	gmt plot @Wessel_GJI_Fig_5.txt -i0,3o73 -W0.75p,-
	gmt plot line.txt -W0.25p
	gmt plot -W0.25p,. <<- EOF
	>
	0	-299.27
	205	-299.27
	>
	0	-455.75
	205	-455.75
	EOF
	echo -164.3 120 164.3 120 | gmt plot -Sv0.2i+s+b+e -Gblack -W2p -N
	echo 164.3 75.7 164.3 -455.75 | gmt plot -Sv0.2i+s+b+e -Gblack -W2p -N
	echo 195.5 75.7 195.5 -299.27 | gmt plot -Sv0.2i+s+b+e -Gblack -W2p -N
	echo 0 300 0 500 | gmt plot -Sv0.3i+s+b+jb+a60 -Gblack -W10p -N
	gmt text -F+f+j -N <<- EOF
	-330 240 16p LB @~r@~@-a@- = @~r@~@-i@-
	-330 180 14p LT [Approximate]
	-300 50 11p LT CRUST
	210 -435 16p,6 LM w@-a@- = @~g@~w@-i@-
	210 -299.27 16p,6 LM w@-i@-
	82 205 16p,6 CM @~l@~@-a@- ~ @~l@~@-exact@-
	40 -50 16p,6 CM @~r@~@-a@-
	EOF
	echo "105 -455.75 w@-a@- ~ w@-exact@-" | gmt text -F+f16p,6+jCB -Dj0/0.075i
	echo "c)" | gmt text -F+cTR+f18p,Helvetica+jTR -Dj0.0i/-0.2i -N
	gmt plot -R-330/270/0/1 -JX6i/6.25i -W0.25p,. <<- EOF
	>
	164.3	0
	164.3	1
	>
	-164.3	0
	-164.3	1
	>
	195.5	0
	195.5	1
	>
	-195.5	0
	-195.5	1
	EOF
	rm -f line.txt load.txt load2.txt
gmt end show
