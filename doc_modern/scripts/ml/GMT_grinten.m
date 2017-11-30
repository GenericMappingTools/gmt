function [ps, path] = GMT_grinten(out_path)
% OUT_PATH -> Path to where the PS file will be written
% PS       -> Full name of the created postscript file (built from OUT_PATH)
% PATH     -> Path to where this file lives (useful for gmtest.m)
%
%	$Id: GMT_grinten.m 17328 2016-11-08 20:38:56Z pwessel $

	full = mfilename('fullpath');
	[pato, fname] = fileparts(full);
	ps = [out_path fname '.ps'];
	path = [pato filesep];

	gmt('destroy'),		gmt('gmtset -Du'),	gmt('destroy')		% Make sure we start with a clean session
	gmt(['pscoast -Rg -JV4i -Bg -Dc -Glightgray -Scornsilk -A10000 -Wthinnest -P > ' ps])
	builtin('delete','gmt.conf');
