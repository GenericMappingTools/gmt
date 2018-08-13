function [ps, path] = oblique(out_path)
% OUT_PATH -> Path to where the PS file will be written
% PS       -> Full name of the created postscript file (built from OUT_PATH)
% PATH     -> Path to where this file lives (useful for gmtest.m)
%

	full = mfilename('fullpath');
	[pato, fname] = fileparts(full);
	ps = [out_path fname '.ps'];
	path = [pato filesep];

	gmt('destroy'),		gmt('gmtset -Du')		% Make sure we start with a clean session
	gmt('gmtset MAP_ANNOT_OBLIQUE 14 MAP_ANNOT_MIN_SPACING 0.5i'),	gmt('destroy')
	gmt(['psbasemap -R-100/100/-60/60 -JOa1/0/45/5.5i -B30g30 -P -K -Xc > ' ps])
	gmt(['psbasemap -R -JOa0/0.1/45/5.5i -B30g30 -O -Y5i >> ' ps])
	builtin('delete','gmt.conf');
