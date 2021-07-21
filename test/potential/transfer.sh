#!/usr/bin/env bash
# Testing firmoviscous flexure code by dumping all the various transfer functions
gmt set GMT_FFT kiss FONT_TAG 7p FONT_ANNOT_PRIMARY 10p

gmt begin transfer ps
	gmt subplot begin 6x3 -Fs5.5c/3c -Srl -Scb -JX5.5cl/3.25c -R1/5000/0/1.1 -Bxaf+ukm -Byafg1 -Bwesn -M3p -Y2c -T"Transfer Functions" -A
		# Elastic
		gmt grdflexure -D3300/2700/2700/1000 -Q -E
		gmt subplot set 0 -A"T@-e@- = 1 km"
		gmt plot grdflexure_transfer_function_te_001_km.txt -i0,2 -W1p
		gmt subplot set 1 -A"T@-e@- = 10 km"
		gmt plot grdflexure_transfer_function_te_010_km.txt -i0,2 -W1p
		gmt subplot set 2 -A"T@-e@- = 100 km"
		gmt plot grdflexure_transfer_function_te_100_km.txt -i0,2 -W1p
		# Viscoelastic
		gmt grdflexure -D3300/2700/2700/1000 -Q -E10 -M20k
		gmt subplot set 3 -A"M@-t@- = 20 ky E = 10 km t = 5 ky"
		gmt plot grdflexure_transfer_function_te_010_km.txt -i0,4 -W1p
		gmt subplot set 4 -A"M@-t@- = 20 ky E = 10 km t = 100 ky"
		gmt plot grdflexure_transfer_function_te_010_km.txt -i0,8 -W1p
		gmt subplot set 5 -A"M@-t@- = 20 ky E = 10 km t = 1 My"
		gmt plot grdflexure_transfer_function_te_010_km.txt -i0,11 -W1p
		# Firmoviscous 1
		gmt grdflexure -D3300/2700/2700/1000 -Q -E10 -F2e20
		gmt subplot set 6 -A"@~h@~ = 2e20 E = 5 km t = 5 ky"
		gmt plot grdflexure_transfer_function_te_005_km.txt -i0,4 -W1p
		gmt subplot set 7 -A"@~h@~ = 2e20 E = 10 km t = 100 ky"
		gmt plot grdflexure_transfer_function_te_010_km.txt -i0,6 -W1p
		gmt subplot set 8 -A"@~h@~ = 2e20 E = 20 km t = 1 My"
		gmt plot grdflexure_transfer_function_te_020_km.txt -i0,8 -W1p
		# Firmoviscous 2
		gmt grdflexure -D3300/2700/2700/1000 -Q -E10 -F2e20/100k/1e21
		gmt subplot set 9 -A"@~h@~ = 2e20/1e21 E = 5 km A = 100 km t = 5 ky"
		gmt plot grdflexure_transfer_function_te_005_km.txt -i0,4 -W1p
		gmt subplot set 10 -A"@~h@~ = 2e20/1e21 E = 10 km A = 100 km t = 100 ky"
		gmt plot grdflexure_transfer_function_te_010_km.txt -i0,6 -W1p
		gmt subplot set 11 -A"@~h@~ = 2e20/1e21 E = 20 km A = 100 km t = 1 My"
		gmt plot grdflexure_transfer_function_te_020_km.txt -i0,8 -W1p
		# Viscous 1
		gmt grdflexure -D3300/2700/2700/1000 -Q -E0 -F2e20
		gmt subplot set 12 -A"@~h@~ = 2e20 E = 0 t = 5 ky"
		gmt plot grdflexure_transfer_function_te_000_km.txt -i0,4 -W1p
		gmt subplot set 13 -A"@~h@~ = 2e20 E = 0 t = 100 ky"
		gmt plot grdflexure_transfer_function_te_000_km.txt -i0,6 -W1p
		gmt subplot set 14 -A"@~h@~ = 2e20 E = 0 t = 1 My"
		gmt plot grdflexure_transfer_function_te_000_km.txt -i0,8 -W1p
		# Viscous 2
		gmt grdflexure -D3300/2700/2700/1000 -Q -E0 -F2e20/100k/1e21
		gmt subplot set 15 -A"@~h@~ = 2e20/1e21 A = 100 km E = 0 t = 5 ky"
		gmt plot grdflexure_transfer_function_te_000_km.txt -i0,4 -W1p
		gmt subplot set 16 -A"@~h@~ = 2e20/1e21 A = 100 km E = 0 t = 100 ky"
		gmt plot grdflexure_transfer_function_te_000_km.txt -i0,6 -W1p
		gmt subplot set 17 -A"@~h@~ = 2e20/1e21 A = 100 km E = 0 t = 1 My"
		gmt plot grdflexure_transfer_function_te_000_km.txt -i0,8 -W1p
	gmt subplot end
gmt end show
