function [ps, path] = sph_5(out_path)
% OUT_PATH -> Path to where the PS file will be written
% PS       -> Full name of the created postscript file (built from OUT_PATH)
% PATH     -> Path to where this file lives (useful for gmtest.m)
%
%	Test sphdistance's -En and -Ez modes

	full = mfilename('fullpath');
	[pato, fname] = fileparts(full);
	ps = [out_path fname '.ps'];
	path = [pato filesep];

	gmt('destroy'),		gmt('gmtset -Du'),	gmt('destroy')		% Make sure we start with a clean session

	% Use the locations of a global hotspot file and fake z values
	tt = gmt(['sphtriangulate ' path 'hotspots.d -Qv -T']);
	t_cpt = gmt('makecpt -Ccategorical -T0/55/1');
	% Make a grid with node numbers
	hot = gmt(['gmtconvert ' path 'hotspots.d -i0,1']);		
	Gn = gmt('sphdistance -Rg -I30m -En5 -G', [hot.data (1:size(hot.data,1))']);
	gmt(['grdimage -C -R0/360/-90/0 -JA0/-90/6i -Baf -P -K -nn -Y0.75i > ' ps], Gn, t_cpt)
	%gmt grdimage n.nc -Ct.cpt -JH0/6i -B0 -P -K > $ps
	gmt(['psxy -R -J -O -K -W1p >> ' ps], tt)
	gmt(['psxy -R -J -O -K -SE-250 -Gwhite -Wfaint ' path 'hotspots.d >> ' ps])
	gmt(['psxy -R -J -O -K -SE-100 -Gblack ' path 'hotspots.d >> ' ps])
	% Make a grid with z values numbers
	t_cpt = gmt('makecpt -Crainbow -T0/600/10 -Z');
	Gz = gmt('sphdistance -Rg -I30m -Ez5 -G', [hot.data 10*(1:size(hot.data,1))']);
	gmt(['grdimage -C -JH0/6i -B0 -O -K -Y6.5i -nn >> ' ps], Gz, t_cpt)
	gmt(['psxy -R -J -O -K -W1p >> ' ps], tt)
	gmt(['psxy -R -J -O -K -SE-350 -Gwhite -Wfaint ' path 'hotspots.d >> ' ps])
	gmt(['psxy -R -J -O -SE-150 -Gblack ' path 'hotspots.d >> ' ps])
	
	builtin('delete','gmt.conf');
