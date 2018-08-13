function [ps, path] = spheres(out_path)
% OUT_PATH -> Path to where the PS file will be written
% PS       -> Full name of the created postscript file (built from OUT_PATH)
% PATH     -> Path to where this file lives (useful for gmtest.m)
%

	full = mfilename('fullpath');
	[pato, fname] = fileparts(full);
	ps = [out_path fname '.ps'];
	path = [pato filesep];

	gmt('destroy'),		gmt('gmtset -Du'),		gmt('destroy')		% Make sure we start with a clean session

	r = 10;		z0 = -15;	ro = 1000;

	%li1 = gmt('sample1d -Fl -I1', [-50 0; 50 0]);
	li1 = [(-50:50)' zeros(101,1)];

	ptodos_g = gmt(['gmtgravmag3d -Tr' path 'sphere.raw -C' num2str(ro) ' -F'], li1);

	% xyzokb solution
	gmt(['psxy -i0,2 -R-50/50/0/0.125 -JX14c/10c -Bx10f5 -By.01 -BWSne+t"Anomaly (mGal)" -W1p -P -K > ' ps], ptodos_g)
	gmt(['psxy -i0,2 -R -JX -Sc.1c -G0 -O -K >> ' ps], ptodos_g)


	% Profile of analytic anomaly
	ztmp = gmt('gmtmath -T-50/50/1 T -15 HYPOT 3 POW INV 6.674e-6 MUL 4 MUL 3 DIV PI MUL 10 3 POW MUL 1000 MUL -15 ABS MUL');
	% Next line fails complaining about T
	%ztmp = gmt('gmtmath -T-50/50/1 T $ HYPOT 3 POW INV 6.674e-6 MUL 4 MUL 3 DIV PI MUL $ 3 POW MUL $ MUL $ ABS MUL', z0, r, ro, z0);
	gmt(['psxy -R -JX -W1p,200/0/0 -O >> ' ps], ztmp)

	builtin('delete','gmt.conf');
