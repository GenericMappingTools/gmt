#!/usr/bin/env bash
# Makes crustal age map for doc/rst/source/datasets/earth-age.rst 
gmt begin GMT_agefig ps
	gmt set GMT_THEME cookbook
  gmt set MAP_TICK_LENGTH 2p MAP_ANNOT_OFFSET 2p MAP_FRAME_PEN 0.5p
  gmt grdimage @earth_age_06m -JQ180/6i -C@age_chrons_GTS2012_2020.cpt -B0 --MAP_FRAME_PEN=1p
  gmt colorbar -C@age_chrons_GTS2012_2020.cpt -G0/164.70  -Dn0.015/0.055+jBL+w5.8i/0.12c+h -S+c --FONT_ANNOT_PRIMARY=5p -F+gwhite+c0p/1p/2p/8p+pfaint
  gmt colorbar -C@age_chrons_GTS2012_2020.cpt -G0/164.70  -Dn0.015/0.055+jBL+w5.8i/0.12c+h+ma -S+n --FONT_ANNOT_PRIMARY=3.5p
gmt end show
