function [ps, path] = trimvector(out_path)
% OUT_PATH -> Path to where the PS file will be written
% PS       -> Full name of the created postscript file (built from OUT_PATH)
% PATH     -> Path to where this file lives (useful for gmtest.m)
%

	full = mfilename('fullpath');
	[pato, fname] = fileparts(full);
	ps = [out_path fname '.ps'];
	path = [pato filesep];

	gmt('destroy'),		gmt('gmtset -Du')		% Make sure we start with a clean session
	gmt('destroy'),		gmt('set MAP_VECTOR_SHAPE 0.5'),	gmt('destroy')

	p = [0.3 0.3; 1.5 1.0; 0.7 1.6];
	v = gmt('convert -Fv' , p, p);
	gmt(['psxy -R0/7/0/9 -Jx1i -P -K -T -Xc > ' ps])
	% Vectors at various ends and one with no vector
	plot(ps, p, v, '-Sv0.2i+t0.1i+s+e')
	plot(ps, p, v, '-Sv0.2i+t0.1i+s+b -X1.75i')
	plot(ps, p, v, '-Sv0.2i+t0.1i+s+b+e -X1.75i')
	plot(ps, p, v, '-Sv0.2i+t0.1i+s -X1.75i')
	% Mid-point vectors and half vectors
	plot(ps, p, v, '-Sv0.2i+t0.1i+s+mf -X-5.25i -Y1.8i')
	plot(ps, p, v, '-Sv0.2i+t0.1i+s+mr -X1.75i')
	plot(ps, p, v, '-Sv0.2i+t0.1i+s+mfl -X1.75i')
	plot(ps, p, v, '-Sv0.2i+t0.1i+s+mfr -X1.75i')
	% Mid-point circles + terminal
	plot(ps, p, v, '-Sv0.2i+t0.1i+s+mc -X-5.25i -Y1.8i')
	plot(ps, p, v, '-Sv0.2i+t0.1i+s+mcl -X1.75i')
	plot(ps, p, v, '-Sv0.2i+t0.1i+s+mcr -X1.75i')
	plot(ps, p, v, '-Sv0.2i+t0.1i+s+mt -X1.75i')
	% Mid-point terminals and squares
	plot(ps, p, v, '-Sv0.2i+t0.1i+s+mtl -X-5.25i -Y1.8i')
	plot(ps, p, v, '-Sv0.2i+t0.1i+s+mtr -X1.75i')
	plot(ps, p, v, '-Sv0.2i+t0.1i+s+ms -X1.75i')
	plot(ps, p, v, '-Sv0.2i+t0.1i+s+msl -X1.75i')
	% Vectors at various ends with extra trim
	plot(ps, p, v, '-Sv0.2i+t0.1i+s+msr -X-5.25i -Y1.8i')
	plot(ps, p, v, '-Sv0.2i+t0.15i+s+e -X1.75i')
	plot(ps, p, v, '-Sv0.2i+t0.15i+s+b -X1.75i')
	plot(ps, p, v, '-Sv0.2i+t0.15i+s+b+e -X1.75i')
	gmt(['psxy -R -J -O -T >> ' ps])

	builtin('delete','gmt.conf');
	
function plot(ps, p, v, str)		% The args are -X -Y and -Sv<something>
	gmt(['psxy -R0/1.75/0/1.8 -J -O -K -Gred -W0.5p -B0 ' str ' >> ' ps], v)
	gmt(['psxy -R -J -O -K -Sc0.2i -Wfaint >> ' ps], p)

