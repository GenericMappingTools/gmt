#!/usr/bin/env bash
# Test various effects of soft hinges with use selection
gmt begin cpthinges ps
	# No hinge selected, discrete CPT
	gmt makecpt -Cpolar -T100/300/20
	gmt colorbar -Baf -Dx0/0+w16c+h -Y2c
	# No hinge selected, continuous CPT
	gmt makecpt -Cpolar -T100/300
	gmt colorbar -Baf -Dx0/0+w16c+h -Y2c
	# hinge selected at z=0 which is outside range so only use top half - discrete CPT
	gmt makecpt -Cpolar+h -T100/300/20
	gmt colorbar -Baf -Dx0/0+w16c+h -Y2c
	# hinge selected at z=0 which is outside range so only use top half - continuous CPT
	gmt makecpt -Cpolar+h -T100/300
	gmt colorbar -Baf -Dx0/0+w16c+h -Y2c
	# hinge selected at z=200 becoming a hard hinge - discrete CPT
	gmt makecpt -Cpolar+h200 -T100/300/20
	gmt colorbar -Baf -Dx0/0+w16c+h -Y2c
	# hinge selected at z=200 becoming a hard hinge - continuous CPT
	gmt makecpt -Cpolar+h200 -T100/300
	gmt colorbar -Baf -Dx0/0+w16c+h -Y2c
	# hinge selected at z=150 becoming a hard hinge - discrete CPT
	gmt makecpt -Cpolar+h150 -T100/300/20
	gmt colorbar -Baf -Dx0/0+w16c+h -Y2c
	# hinge selected at z=150 becoming a hard hinge - continuous CPT
	gmt makecpt -Cpolar+h150 -T100/300
	gmt colorbar -Baf -Dx0/0+w16c+h -Y2c
	# hinge selected at z=-50 becoming a hard hinge - discrete CPT
	gmt makecpt -Cpolar+h-50 -T-100/100/10
	gmt colorbar -Baf -Dx0/0+w16c+h -Y2c
	# hinge selected at z=-50 becoming a hard hinge - continuous CPT
	gmt makecpt -Cpolar+h-50 -T-100/100
	gmt colorbar -Baf -Dx0/0+w16c+h -Y2c
	# hinge selected at z=50 becoming a hard hinge - discrete CPT
	gmt makecpt -Cpolar+h50 -T-100/100/10
	gmt colorbar -Baf -Dx0/0+w16c+h -Y2c
	# hinge selected at z=50 becoming a hard hinge - continuous CPT
	gmt makecpt -Cpolar+h50 -T-100/100
	gmt colorbar -Baf -Dx0/0+w16c+h -Y2c
gmt end show
