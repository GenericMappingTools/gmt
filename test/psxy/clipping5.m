function [ps, path] = clipping5(out_path)
% OUT_PATH -> Path to where the PS file will be written
% PS       -> Full name of the created postscript file (built from OUT_PATH)
% PATH     -> Path to where this file lives (useful for gmtest.m)
%

	full = mfilename('fullpath');
	[pato, fname] = fileparts(full);
	ps = [out_path fname '.ps'];
	path = [pato filesep];

	gmt('destroy'),		gmt('gmtset -Du'),	gmt('destroy')		% Make sure we start with a clean session
	gmt(['psxy -R -J -O -K -SE -W5p,blue >> ' ps], {'60	-68	0	4500	4500'})
	gmt(['psxy -R -J -O -K -SE -W5p,blue --PS_LINE_CAP=square >> ' ps], {'00	68	0	4500	4500'})
	gmt(['pstext -R -J -O -K -F+jCM+f18p >> ' ps], {'10 50 D'})

	p = [
		%> clipped at bottom
		NaN	Nan
		100	-45
		130	-40
		140	-47
		150	-54
		130	-65
		90	-60
		%> clipped at left/right
		NaN	Nan
		30	20
		60	23
		80	35
		70	38
		40	35
		20	33
		5	29
		-15	24
		-25	20
		-40	10
		-33	5
		-15	15
		-5	18
		5	19];

	gmt(['psxy -R -J -O -K -Baf -BWSne -Gorange -W5p,blue -Y3i >> ' ps], p)
	gmt(['pstext -R -J -O -K -F+jCM+f18p >> ' ps], {'10 50 C'})
	s = [
		0	1	c
		1	0	s
		2	1	t
		1	2	g
		1	1	a];

	gmt(['psxy -R0/2/0/2 -JX2.75i -O -K -Baf -BWSne -S1i -Gyellow -W5p,blue -Y3i >> ' ps], s)
	gmt(['pstext -R -J -O -K -F+jCM+f18p >> ' ps], {'0.2 1.8 A'})
	v = [
		-0.9	-0.9	93	135
		-0.9	-0.9	60	270
		-0.9	-0.9	30	270
		-0.9	-0.9	-3	160];

	gmt(['psxy -R-1/1/-1/1 -JM2.75i -O -K -Baf -BwSnE -S=0.5i+e -Gcyan -W5p,brown -X3.25i >> ' ps], v)
	gmt(['pstext -R -J -O -K -F+jCM+f18p >> ' ps], {'-0.8 0.8 B'})
	gmt(['psxy -R -J -O -T >> ' ps])

	builtin('delete','gmt.conf');
