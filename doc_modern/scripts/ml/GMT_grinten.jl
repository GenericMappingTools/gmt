function GMT_grinten(out_path::ASCIIString="")
# OUT_PATH -> Path to where the PS file will be written. If not in input must be set via the "outPath" global var
# PS       -> Full name of the created postscript file (built from OUT_PATH)
# PATH     -> Path to where this file lives (useful for gmtest.m)
#
#	$Id: GMT_grinten.jl 17328 2016-11-08 20:38:56Z pwessel $

	global outPath
	if (isempty(out_path))	out_path = outPath;		end

	pato, fname = fileparts(@__FILE__)
	ps = out_path * fname * ".ps"
	path = pato * "/"

	gmt("destroy"),		gmt("gmtset -Du"),		gmt("destroy")		# Make sure we start with a clean session
	gmt("pscoast -Rg -JV4i -Bg -Dc -Glightgray -Scornsilk -A10000 -Wthinnest -P > " * ps)
	rm("gmt.conf")

	return ps, path
end

GMT_grinten() = GMT_grinten("")