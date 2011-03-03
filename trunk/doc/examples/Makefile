#	$Id: Makefile,v 1.3 2011-03-03 20:25:45 guru Exp $
.PHONY:
	examples run-examples animations run-animations

EX=example_01.ps example_02.ps example_03.ps example_03a.ps example_03b.ps example_03c.ps example_03d.ps example_03e.ps example_03f.ps \
example_04.ps example_04c.ps example_05.ps example_06.ps \
example_07.ps example_08.ps example_09.ps example_10.ps example_11.ps example_12.ps \
example_13.ps example_14.ps example_15.ps example_16.ps example_17.ps example_18.ps \
example_19.ps example_20.ps example_21.ps example_22.ps example_23.ps example_24.ps \
example_25.ps example_26.ps example_27.ps example_28.ps example_29.ps example_30.ps
ANIM=anim_01.ps anim_02.ps anim_03.ps anim_04.ps

examples run-examples:	$(EX)
animations run-animations:	$(ANIM)
extests tests:	examples animations
	run_ex_tests.sh

clean spotless:
	rm -f $(EX) $(ANIM)

example_01.ps: ex01/job01.sh ex01/osu91a1f_16.nc
	cd ex01 ; ./job01.sh
example_02.ps: ex02/job02.sh ex02/HI_geoid2.nc ex02/HI_topo2.nc
	cd ex02 ; ./job02.sh
example_03.ps example_03a.ps example_03b.ps example_03c.ps example_03d.ps example_03e.ps example_03f.ps: ex03/job03.sh ex03/sat.xyg
	cd ex03 ; ./job03.sh
example_04.ps: ex04/job04.sh ex04/HI_geoid4.nc ex04/HI_topo4.nc ex04/geoid.cpt ex04/topo.cpt
	cd ex04 ; ./job04.sh
example_04c.ps: ex04/job04c.sh ex04/HI_geoid4.nc ex04/HI_topo4.nc ex04/geoid.cpt ex04/topo.cpt
	cd ex04 ; ./job04c.sh
example_05.ps: ex05/job05.sh
	cd ex05 ; ./job05.sh
example_06.ps: ex06/job06.sh ex06/fractures.d ex06/v3206.t
	cd ex06 ; ./job06.sh
example_07.ps: ex07/job07.sh ex07/fz.xy ex07/isochron.xy ex07/quakes.xym ex07/ridge.xy
	cd ex07 ; ./job07.sh
example_08.ps: ex08/job08.sh ex08/guinea_bay.nc
	cd ex08 ; ./job08.sh
example_09.ps: ex09/job09.sh ex09/all.xys ex09/fz.xy ex09/ridge.xy ex09/track_d.107.xys ex09/track_d.109.xys ex09/track_d.111.xys \
ex09/track_d.137.xys ex09/track_d.139.xys ex09/track_d.165.xys ex09/track_d.167.xys ex09/track_d.193.xys ex09/track_d.195.xys ex09/track_d.197.xys \
ex09/track_d.21.xys ex09/track_d.223.xys ex09/track_d.225.xys ex09/track_d.23.xys ex09/track_d.25.xys ex09/track_d.251.xys ex09/track_d.253.xys \
ex09/track_d.279.xys ex09/track_d.281.xys ex09/track_d.283.xys ex09/track_d.309.xys ex09/track_d.311.xys ex09/track_d.337.xys ex09/track_d.339.xys \
ex09/track_d.365.xys ex09/track_d.367.xys ex09/track_d.369.xys ex09/track_d.395.xys ex09/track_d.397.xys ex09/track_d.423.xys ex09/track_d.425.xys \
ex09/track_d.427.xys ex09/track_d.451.xys ex09/track_d.453.xys ex09/track_d.455.xys ex09/track_d.481.xys ex09/track_d.483.xys \
ex09/track_d.49.xys ex09/track_d.51.xys ex09/track_d.53.xys ex09/track_d.79.xys ex09/track_d.81.xys
	cd ex09 ; ./job09.sh
example_10.ps: ex10/job10.sh ex10/agu2008.d
	cd ex10 ; ./job10.sh
example_11.ps: ex11/job11.sh ex11/cut-here.dat ex11/rays.dat
	cd ex11 ; ./job11.sh
example_12.ps: ex12/job12.sh ex12/table_5.11
	cd ex12 ; ./job12.sh
example_13.ps: ex13/job13.sh
	cd ex13 ; ./job13.sh
example_14.ps: ex14/job14.sh ex14/table_5.11
	cd ex14 ; ./job14.sh
example_15.ps: ex15/job15.sh
	cd ex15 ; ./job15.sh
example_16.ps: ex16/job16.sh ex16/circuit.ras ex16/ex16.cpt ex16/table_5.11
	cd ex16 ; ./job16.sh
example_17.ps: ex17/job17.sh ex17/india_geoid.nc ex17/india_topo.nc
	cd ex17 ; ./job17.sh
example_18.ps: ex18/job18.sh ex18/AK_gulf_grav.nc
	cd ex18 ; ./job18.sh
example_19.ps: ex19/job19.sh ex19/circuit.ras
	cd ex19 ; ./job19.sh
example_20.ps: ex20/job20.sh ex20/bullseye.def
	cd ex20 ; ./job20.sh
example_21.ps: ex21/job21.sh ex21/RHAT_price.csv
	cd ex21 ; ./job21.sh
example_22.ps: ex22/job22.sh ex22/USGS.ras ex22/neic_quakes.d
	cd ex22 ; ./job22.sh
example_23.ps: ex23/job23.sh
	cd ex23 ; ./job23.sh
example_24.ps: ex24/job24.sh ex24/oz_quakes.d
	cd ex24 ; ./job24.sh
example_25.ps: ex25/job25.sh
	cd ex25 ; ./job25.sh
example_26.ps: ex26/job26.sh
	cd ex26 ; ./job26.sh
example_27.ps: ex27/job27.sh ex27/tasman_grav.nc
	cd ex27 ; ./job27.sh
example_28.ps: ex28/job28.sh ex28/Kilauea.utm.nc
	cd ex28 ; ./job28.sh
example_29.ps: ex29/job29.sh
	cd ex29 ; ./job29.sh
example_30.ps: ex30/job30.sh ex29/mars370.in
	cd ex30 ; ./job30.sh

anim_01.ps: anim01/anim_01.sh
	cd anim01 ; ./anim_01.sh
anim_02.ps: anim02/anim_02.sh
	cd anim02 ; ./anim_02.sh
anim_03.ps: anim03/anim_03.sh anim03/Iceland.nc
	cd anim03 ; ./anim_03.sh
anim_04.ps: anim04/anim_04.sh anim04/USEast_Coast.nc
	cd anim04 ; ./anim_04.sh
