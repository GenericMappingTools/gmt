function [ps, path] = GMT_insert(out_path)
% OUT_PATH -> Path to where the PS file will be written
% PS       -> Full name of the created postscript file (built from OUT_PATH)
% PATH     -> Path to where this file lives (useful for gmtest.m)
%
%	$Id: GMT_insert.m 17328 2016-11-08 20:38:56Z pwessel $

	full = mfilename('fullpath');
	[pato, fname] = fileparts(full);
	ps = [out_path fname '.ps'];
	path = [pato filesep];

	gmt('destroy'),		gmt('gmtset -Du'),	gmt('destroy')		% Make sure we start with a clean session

	% Bottom map of Australia
	gmt(['pscoast -R110E/170E/44S/9S -JM6i -P -Baf -BWSne -Wfaint -N2/1p  -EAU+gbisque' ...
		' -Gbrown -Sazure1 -Da -K -Xc --FORMAT_GEO_MAP=dddF > ' ps])
	gmt(['psbasemap -R -J -O -K -DjTR+w1.5i+o0.15i+stmp___.dat -F+gwhite+p1p+c0.1c+s >> ' ps])
	t = load('tmp___.dat');			% read x0 y0 w h < tmp
	gmt(['pscoast -Rg -JG120/30S/' num2str(t(3)) ' -Da -Gbrown -A5000 -Bg -Wfaint -EAU+gbisque -O -K ' ...
		sprintf('-X%f -Y%f', t(1), t(2)) ' >> ' ps])
	gmt(['psxy -R -J -O -T ' sprintf('-X%f -Y%f', -t(1), -t(2)) ' >> ' ps])

	builtin('delete','gmt.conf', 'tmp___.dat');
