function [ps, path] = flexure_e(out_path)
% OUT_PATH -> Path to where the PS file will be written
% PS       -> Full name of the created postscript file (built from OUT_PATH)
% PATH     -> Path to where this file lives (useful for gmtest.m)
%

	full = mfilename('fullpath');
	[pato, fname] = fileparts(full);
	ps = [out_path fname '.ps'];
	path = [pato filesep];

	gmt('destroy'),		gmt('gmtset -Du')
	gmt('gmtset MAP_FRAME_TYPE plain'),		gmt('destroy')		% Make sure we start with a clean session

	%lon lat azimuth, semi-major, semi-minor, height
	t = [300	200	70	70	30	5000];

	Gsmt = gmt('grdseamount -Rk0/600/0/400 -I1000 -G -Dk -E -F0.2 -Cg', t);
	% WE DON'T HAVE SYNTAX FOR "smt.nc+Uk"
gmt(['grdcontour smt.nc+Uk -Jx0.01i -Xc -P -A1 -GlLM/RM -Bafg -K -Z0.001 > ' ps])
	Gflex_e = gmt('grdflexure -D3300/2700/2400/1030 -E5k -G', Gsmt);
gmt(['grdcontour flex_e.nc+Uk -J -O -K -C0.2 -A1 -Z0.001 -GlLM/RM -Bafg -BWsNE+t"Elastic Plate Flexure, T@-e@- = 5 km" -Y4.4i >> ' ps])
	gmt(['psxy -R -J -O -T >> ' ps])
	builtin('delete','gmt.conf');
