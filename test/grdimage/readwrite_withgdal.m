function [ps, path] = readwrite_withgdal(out_path)
% OUT_PATH -> Path to where the PS file will be written
% PS       -> Full name of the created postscript file (built from OUT_PATH)
% PATH     -> Path to where this file lives (useful for gmtest.m)
%

	full = mfilename('fullpath');
	[pato, fname] = fileparts(full);
	ps = [out_path fname '.ps'];
	path = [pato filesep];

	gmt('destroy')		% Make sure we start with a clean session
	gmt('gmtset -Du'),	gmt('destroy')

	% RGB image
	gmt(['grdimage -D ' path 'gdal/needle.jpg -JX7c/0 -P -Y20c -K > ' ps])

	% Same image as above but as idexed
	gmt(['grdimage -D ' path 'gdal/needle.png -JX7c/0 -X7.5c -O -K >> ' ps])

	% Projected
	gmt(['grdimage -Dr ' path 'gdal/needle.jpg -Rd -JW10c -Bxg30 -Byg15 -X-5.0c -Y-7c -O -K >> ' ps])

	% Illuminated
	Gsomb = gmt('grdmath -R-15/15/-15/15 -I0.2 X Y HYPOT DUP 2 MUL PI MUL 8 DIV COS EXCH NEG 10 DIV EXP MUL =');
	Gillum = gmt('grdgradient -A225 -G -Nt0.75', Gsomb);	clear Gsomb
	gmt(['grdimage -D ' path 'gdal/needle.jpg -I -JM8c -Y-10c -X-2c -O -K >> ' ps], Gillum),	clear Gillum

	% A gray image (one band, no color map)
	gmt(['grdimage -D ' path 'gdal/vader.jpg -JX4c/0 -X9c -Y5c -K -O >> ' ps])

	% Create a .png from a dummy grid and import it
	Glixo = gmt('grdmath -R-5/5/-5/5 -I1 X Y MUL =');
	lixo_cpt = gmt('makecpt -T-25/25/1');
% 	gmt(['grdimage -A' path 'lixo.png=PNG -JX4c -C'], Glixo, lixo_cpt)
% 	gmt(['grdimage -D ' path 'lixo.png -JX4c -Y-5c -O >> ' ps])
	I = gmt('grdimage -A -JX4c -C', Glixo, lixo_cpt);
	gmt(['grdimage -D -JX4c -Y-5c -O >> ' ps], I)

	builtin('delete','gmt.conf');
