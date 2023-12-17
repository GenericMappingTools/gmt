#!/usr/bin/env bash
# Remote grid with a soft-hinge cpt with/without +h to enable and set it
# Bottom panel: Polar CPT with activated soft-hinge at 0, so z = 0 is white
# Middle panel: Polar CPT with activated soft-hinge at 2000, so z = 2000 is white
# Top panel:    Polar CPT with deactivated soft-hinge, so z_mid = --1368.5 is white

gmt begin cpt_hinge ps
	# Pass CPT and activate soft hinge at 0
	gmt grdimage @earth_relief_06m -RPT -Cpolar+h -B
	gmt coast -W0.25p
	gmt colorbar -DJRM -Bxa1000 -By+l"Hinge = 0"
	# Create new CPT and activate soft hinge at 2000
	gmt makecpt -Cpolar+h2000 -T-5766/3029 -H > t.cpt
	gmt grdimage @earth_relief_06m -RPT -Ct.cpt -B -Y8c
	gmt coast -W0.25p
	gmt colorbar -DJRM -Ct.cpt -Bxa1000 -By+l"Hinge = 2000"
	# Pass CPT and do not activate any hinge
	gmt grdimage @earth_relief_06m -RPT -Cpolar -B -Y8c
	gmt coast -W0.25p
	gmt colorbar -DJRM -Bxa1000 -By+l"No hinge set"
gmt end show
