function [ps, path] = vectors(out_path)
% OUT_PATH -> Path to where the PS file will be written
% PS       -> Full name of the created postscript file (built from OUT_PATH)
% PATH     -> Path to where this file lives (useful for gmtest.m)
%

	full = mfilename('fullpath');
	[pato, fname] = fileparts(full);
	ps = [out_path fname '.ps'];
	path = [pato filesep];

	gmt('destroy'),		gmt('gmtset -Du'),	gmt('destroy')		% Make sure we start with a clean session
	Gr  = gmt('grdmath -Rg -I30 -r 0.5 Y COSD ADD =');
	Gaz = gmt('grdmath -Rg -I30 -r X =');
	t_cpt = gmt('makecpt -T0.5/1.5/0.1 -Z');

	gmt(['grdvector -A -C -JG45/45/4.5i -Q0.2i+e -W2p -Si2500 -P -K -B30g30' ...
		' -Xc -Y0.75i --MAP_VECTOR_SHAPE=0.5 > ' ps], Gr, Gaz, t_cpt)
	gmt(['grdvector -A -C -J -W2p -Si2500 -O -B30g30 -Y5i >> ' ps], Gr, Gaz, t_cpt)
	builtin('delete','gmt.conf');
