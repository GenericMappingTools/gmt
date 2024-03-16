#!/usr/bin/env bash
# Illustrate the parameters for specifying slides
gmt begin GMT_seamount_specs
	gmt set GMT_THEME cookbook
	gmt plot -R0/2.6/0/1.2 -Jx2.4i/1i -W1p <<- EOF
	0	1
	0.2	1
	1	0
	EOF
	gmt plot -W0.25p,. <<- EOF
	> h1
	0	0.2
	0.84	0.2
	> h2
	0	0.8
	0.36	0.8
	> hc
	0	0.1
	0.92	0.1
	> rf
	0.2	0
	0.2	1
	> r2
	0.36	0
	0.36	0.8
	> r1
	0.84	0
	0.84	0.2
	> rc
	0.92	0
	0.92	0.1
	EOF
	u0=0.1
	gmt math -T0/1/0.01 ${u0} 1 ${u0} ADD T ${u0} ADD DIV 1 SUB MUL 0.8 0.2 SUB MUL 0.2 ADD -C0 0.84 0.36 SUB MUL 0.36 ADD = | gmt plot -Gpink -W1p
	gmt plot -Glightblue -W0.25p <<- EOF
	0.92	0.1
	1	0
	2.5	0
	EOF
	gmt plot -W0.25p,dashed <<- EOF
	>
	0.28	0.9
	0.75	0.9
	> 
	0.44	0.4
	1.05	0.4
	EOF
	gmt text -F+f8p,Times-Italic+j -N -Dj4p -Ba10f1 -BWS --MAP_FRAME_TYPE=graph --FONT_ANNOT_PRIMARY=6p <<- EOF
	0.00	0.1	RM	h@-c@-
	0.50	1.1	CB	Cross-section parameters
	0.00	1.0	RM	h@-0@-
	0.00	0.2	RM	h@-1@-
	0.00	0.8	RM	h@-2@-
	0.00	1.1	RB	h
	0.20	0	TC	r@-f@-
	0.36	0	TC	r@-2@-
	0.84	0	TC	r@-1@-
	0.92	0	TC	r@-c@-
	1.00	0	TC	r@-0@-
	2.50	0	TC	r@-d@-
	2.65	0	TC	r
	1.02	0.4	LM	h@-s@-(r)
	0.72	0.9	LM	h(r)
	2.50	1.1	CB	Map view parameters
	2.26	0.97	LM	@~a@~@-1@-
	2.69	0.78	LM	@~a@~@-2@-
	EOF
	echo 2.5 0.6 | gmt plot -Sw1i/20/140 -Glightgray -N
	echo 2.5 0.6 0.4i 20 140 | gmt plot -Sm10p+b -W0.5p -Gblack -N
	echo 2.5 0.6 | gmt plot -Sc1i -W1p -N
gmt end show
