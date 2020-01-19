#!/usr/bin/env bash
gmt begin GMT_tut_9
	gmt makecpt -Cred,green,blue -T0,100,300,10000
	gmt coast -R130/150/35/50 -JM6i -B -Ggray
	gmt plot @tut_quakes.ngdc -Wfaint -i4,3,5,6+s0.1 -h3 -Scc -C -Vl
gmt end show
