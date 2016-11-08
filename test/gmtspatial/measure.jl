function measure(out_path::ASCIIString="")
# OUT_PATH -> Path to where the PS file will be written. If not in input must be set via the "outPath" global var
# PS       -> Full name of the created postscript file (built from OUT_PATH)
# PATH     -> Path to where this file lives (useful for gmtest.m)
#
#	$Id$

	pato, fname = fileparts(@__FILE__)
	ps = out_path * fname * ".ps"
	path = pato * "/"

	area = [0 0; 1 0; 1 1; 0 1; 0 0];
	# Cartesian centroid and area
	#echo "0.5	0.5	1" > answer
	answer = [0.5 0.5 1]
	result = gmt("gmtspatial -Q", area)
	pass = (sum(abs(result - answer)) < 1e-12)
	if (!pass)
		fid = open(out_path * fname * "_fail1.dat", "w")
		@printf(fid, "Should be\n%f\t%f\t%f\n", answer[1], answer[2], answer[3])
		@printf(fid, "It is\n%f\t%f\t%f", result[1], result[2], result[3])
		close(fid)
	end
	# Geographic centroid and area
	answer = [0.5 0.5000195463076722 12225.940899448242]
	result = gmt("gmtspatial -Q -fg", area)
	pass2 = (sum(abs(result - answer)) < 1e-12)
	if (!pass2)
		fid = open(out_path * fname * "_fail2.dat", "w")
		@printf(fid, "Should be\n%f\t%f\t%f\n", answer[1], answer[2], answer[3])
		@printf(fid, "It is\n%f\t%f\t%f", result[1], result[2], result[3])
		close(fid)
	end
	pass = pass && pass2

	return pass, path
end

measure() = measure("")