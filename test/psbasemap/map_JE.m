function [ps, path] = map_JE(out_path)
% OUT_PATH -> Path to where the PS file will be written
% PS       -> Full name of the created postscript file (built from OUT_PATH)
% PATH     -> Path to where this file lives (useful for gmtest.m)
%

	full = mfilename('fullpath');
	[pato, fname] = fileparts(full);
	ps = [out_path fname '.ps'];
	path = [pato filesep];

	gmt('destroy'),		gmt('gmtset -Du'),	gmt('destroy')		% Make sure we start with a clean session
	gmt(['psbasemap -B10g10 -Je-70/-90/1:10000000 -R-95/-75/-60/-55r -P -Xc > ' ps])
	builtin('delete','gmt.conf');
