#!/usr/bin/env bash
# DVC_TEST
gmt begin legend_font ps
	cat <<-EOF > t.txt
	H 48p,Helvetica-Bold Bold Header
	S - - - - thin - Regular text
	L 18p,Times-Roman R Times Label
	EOF
	gmt legend -JX5i -R0/5/0/5 -DjCM+o0.5c -Fi+gwhite+pthick -S1.3 --FONT_ANNOT_PRIMARY=32p,Helvetica t.txt
gmt end show
