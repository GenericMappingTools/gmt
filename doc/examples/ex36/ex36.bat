REM               GMT EXAMPLE 36
REM
REM Purpose:      Illustrate sphinterpolate with Mars radii data
REM GMT modules:  plot, makecpt, grdimage, sphinterpolate
REM
gmt begin ex36
	REM Interpolate data of Mars radius from Mariner9 and Viking Orbiter spacecrafts
	gmt subplot begin 3x1 -Fs14c/0 -JH0/14c -Rg -M0
		gmt makecpt -Crainbow -T-7000/15000
		REM Piecewise linear interpolation; no tension
		gmt sphinterpolate @mars370d.txt -Rg -I1 -Q0 -Gtt.nc
		gmt grdimage tt.nc -Bag -c0

		gmt plot @mars370d.txt -Sc0.1c -G0 -B30g30 -c1

		REM Smoothing
		gmt sphinterpolate @mars370d.txt -Rg -I1 -Q3 -Gtt.nc
		gmt grdimage tt.nc -Bag -c2
	gmt subplot end
	REM cleanup
	del tt.nc
gmt end show
