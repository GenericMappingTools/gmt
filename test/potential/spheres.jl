function spheres(out_path::ASCIIString="")
# OUT_PATH -> Path to where the PS file will be written. If not in input must be set via the "outPath" global var
# PS       -> Full name of the created postscript file (built from OUT_PATH)
# PATH     -> Path to where this file lives (useful for gmtest.m)
#
#	$Id$

	global outPath
	if (isempty(out_path))	out_path = outPath;		end

	pato, fname = fileparts(@__FILE__)
	ps = out_path * fname * ".ps"
	path = pato * "/"

	gmt("destroy"),		gmt("gmtset -Du"),		gmt("destroy")		# Make sure we start with a clean session

	r = 10;		z0 = -15;	ro = 1000;

	li1 = gmt("sample1d -Fl -I1", [-50 0; 50 0])

	ptodos_g = gmt("gmtgravmag3d -Tr" * path * "sphere.raw -C" * "$ro" * " -F", li1)

	# xyzokb solution
	gmt("psxy -i0,2 -R-50/50/0/0.125 -JX14c/10c -Bx10f5 -By.01 -BWSne+t\"Anomaly (mGal)\" -W1p -P -K > " * ps, ptodos_g)
	gmt("psxy -i0,2 -R -JX -Sc.1c -G0 -O -K >> " * ps, ptodos_g)

	# Profile of analytic anomaly
	ztmp = gmt("gmtmath -T-50/50/1 T -15 HYPOT 3 POW INV 6.674e-6 MUL 4 MUL 3 DIV PI MUL 10 3 POW MUL 1000 MUL -15 ABS MUL")
	gmt("psxy -R -JX -W1p,200/0/0 -O >> " * ps, ztmp)

	rm("gmt.conf")

	return ps, path
end

spheres() = spheres("")