REM               GMT EXAMPLE 44
REM
REM Purpose:      Illustrate use of map insets
REM GMT modules:  coast, basemap, mapproject
REM

gmt begin ex44
	gmt subplot begin 2x1 -Fs6i/4i -B -BWSne
		gmt coast -R10W/5E/35N/44N -JM6i -EES+gbisque -Gbrown -Wfaint -N1/1p -Sazure1 -Df --FORMAT_GEO_MAP=dddF -c0
		gmt inset begin -DjTR+w2i/0.93i+o0.15i/0.1i -F+gwhite+p1p+c0.1c+s
			gmt coast -R15W/35E/30N/48N -JM? -Da -Gbrown -B0 -EES+gbisque --MAP_FRAME_TYPE=plain
		gmt inset end

		gmt coast -R110E/170E/44S/9S -JM6i -Wfaint -N2/1p -EAU+gbisque -Gbrown -Sazure1 -Da --FORMAT_GEO_MAP=dddF -c1
		gmt inset begin -DjTR+w1.5i+o0.15i/0.1i -F+gwhite+p1p+c0.1c+s
			gmt coast -Rg -JG120/30S/? -Da -Gbrown -A5000 -Bg -Wfaint -EAU+gbisque
		gmt inset end
	gmt subplot end
gmt end show
