function GMT_insert(out_path::ASCIIString="")
# OUT_PATH -> Path to where the PS file will be written. If not in input must be set via the "outPath" global var
# PS       -> Full name of the created postscript file (built from OUT_PATH)
# PATH     -> Path to where this file lives (useful for gmtest.m)
#
#	$Id: GMT_insert.jl 17328 2016-11-08 20:38:56Z pwessel $

	global outPath
	if (isempty(out_path))	out_path = outPath;		end

	pato, fname = fileparts(@__FILE__)
	ps = out_path * fname * ".ps"
	path = pato * "/"

	gmt("destroy"),		gmt("gmtset -Du"),		gmt("destroy")		# Make sure we start with a clean session

	# Bottom map of Australia
	gmt("pscoast -R110E/170E/44S/9S -JM6i -P -Baf -BWSne -Wfaint -N2/1p  -EAU+gbisque" *
		" -Gbrown -Sazure1 -Da -K -Xc --FORMAT_GEO_MAP=dddF > " * ps)
	gmt("psbasemap -R -J -O -K -DjTR+w1.5i+o0.15i+stmp___.dat -F+gwhite+p1p+c0.1c+s >> " * ps)
	t = readdlm("tmp___.dat");			# read x0 y0 w h < tmp
	gmt("pscoast -Rg -JG120/30S/" * "$(t[3])" * " -Da -Gbrown -A5000 -Bg -Wfaint -EAU+gbisque -O -K " *
		@sprintf("-X%f -Y%f", t[1], t[2]) * " >> " * ps)
	gmt(@sprintf("psxy -R -J -O -T -X%f -Y%f >> %s", -t[1], -t[2], ps))

	rm("gmt.conf"),	rm("tmp___.dat")

	return ps, path
end

GMT_insert() = GMT_insert("")