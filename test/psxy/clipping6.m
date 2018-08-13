function [ps, path] = clipping6(out_path)
% OUT_PATH -> Path to where the PS file will be written
% PS       -> Full name of the created postscript file (built from OUT_PATH)
% PATH     -> Path to where this file lives (useful for gmtest.m)
%

	full = mfilename('fullpath');
	[pato, fname] = fileparts(full);
	ps = [out_path fname '.ps'];
	path = [pato filesep];

	gmt('destroy'),		gmt('gmtset -Du'),	gmt('destroy')		% Make sure we start with a clean session
	badpol = [-70	-55
			-50	-30
			-50	50
			-120	67
			-170	60
			-80	0];
	gmt(['psxy -R-90/270/-60/75 -JM5i -A -Ggreen -W0.25p,red -Bag -K -P -Xc > ' ps], badpol)
	gmt(['psxy -R-180/180/-60/75 -JR90/5i -A -Ggreen -W0.25p,red -Bag -O -K -Y3.5i >> ' ps], badpol)
	gmt(['psxy -R -JR-90/5i -A -Ggreen -W0.25p,red -Bag -O -Y3.5i >> ' ps], badpol)
	builtin('delete','gmt.conf');
