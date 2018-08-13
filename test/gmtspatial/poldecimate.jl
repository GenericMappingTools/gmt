function poldecimate(out_path::ASCIIString="")
# OUT_PATH -> Path to where the PS file will be written. If not in input must be set via the "outPath" global var
# PS       -> Full name of the created postscript file (built from OUT_PATH)
# PATH     -> Path to where this file lives (useful for gmtest.m)
#
#	Testing gmt gmtspatial decimation near poles
#
# Input was made this way.
#gmt gmtmath -T0/2000/1 -o1 0 360 RAND = x
#gmt gmtmath -T0/2000/1 -o1 60 90 RAND = y
#gmt gmtmath -T0/2000/1 -o1 0 100 RAND = z
#paste x y z > polar.txt

	pato, fname = fileparts(@__FILE__)
	ps = out_path * fname * ".ps"
	path = pato * "/"

	gmt("destroy"),		gmt("gmtset -Du"),		gmt("destroy")		# Make sure we start with a clean session

	# NN averaging
	results = gmt("gmtspatial -Aa100k -fg " * path * "polar.txt")
	gmt("psxy -R0/360/60/90 -JA0/90/4.5i -P -K -Bafg -BWsne " * path * "polar.txt -Sc0.05i -Ggreen -X1.5i -Y0.75i > " * ps)
	gmt("pstext -R -J -O -K -N -F+f12+jLM -Dj0.25i >> " * ps, "90 60 N = 2000")
	gmt("psxy -R -J -O -K " * path * "polar.txt -Sc0.05i -Bafg -BWsne -Gdarkseagreen1 -Y5i >> " * ps)
	gmt("psxy -R -J -O -K -Sc0.05i -Gred >> " * ps, results)
	gmt("pstext -R -J -O -K -N -F+f12+jLM -Dj0.25i >> " * ps, @sprintf("90 60 N = %d", size(results,1)))
	gmt("psxy -R -J -O -T >> " * ps)
	
	rm("gmt.conf")

	return ps, path
end

poldecimate() = poldecimate("")
