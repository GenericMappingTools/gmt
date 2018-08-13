function [ps, path] = geosegmentize(out_path)
% OUT_PATH -> Path to where the PS file will be written
% PS       -> Full name of the created postscript file (built from OUT_PATH)
% PATH     -> Path to where this file lives (useful for gmtest.m)
%

	full = mfilename('fullpath');
	[pato, fname] = fileparts(full);
	ps = [out_path fname '.ps'];
	path = [pato filesep];

	gmt('destroy'),		gmt('gmtset -Du'),	gmt('destroy')		% Make sure we start with a clean session
	gmt('destroy'),		gmt('set MAP_FRAME_TYPE plain'),	gmt('destroy')

	t1 = [10	10
		48	15
		28	20
		NaN NaN
		40	40
		30	5
		5	15];

	t2 = [7	20
		29	11
		8	4];

	scl = '0.06id';
	% Show the data and its natural connectivity
	gmt(['psxy -R0/50/0/45 -Jx' scl ' -P -Ba10 -BWSne -W0.25p,- -K > ' ps], t1, t2)
	plotpts(t1, t2, 'TWO DATA TABLES', ps)
 	% Lines from dataset origin
	gmt(['psxy -R -J -Ba10 -BwSnE -W0.25p,- -O -K -X3.25i >> ' ps], t1, t2)
	gmt(['psxy -R -J -W1p -Fra -O -K >> ' ps], t1, t2)
 	plotpts(t1, t2, 'DATASET ORIGIN', ps)
	% Lines from table origin
	gmt(['psxy -R -Jx' scl ' -Ba10 -BWSne -W0.25p,- -O -K -X-3.25i -Y3.15i >> ' ps], t1, t2)
	gmt(['psxy -R -J -W1p -Frf -O -K >> ' ps], t1, t2)
	plotpts(t1, t2, 'TABLE ORIGIN', ps)
	% Lines from segment origin
	gmt(['psxy -R -J -Ba10 -BwSnE -W0.25p,- -O -K -X3.25i >> ' ps], t1, t2)
	gmt(['psxy -R -J -W1p -Frs -O -K >> ' ps], t1, t2)
	plotpts(t1, t2, 'SEGMENT ORIGIN', ps)
	% Lines from fixed origin
	gmt(['psxy -R -J -Ba10 -BWSne  -W0.25p,- -O -K -X-3.25i -Y3.15i >> ' ps], t1, t2)
	gmt(['psxy -R -J -W1p -Fr10/35 -O -K >> ' ps], t1, t2)
	plotpts(t1, t2, 'FIXED ORIGIN', ps)
	gmt(['psxy -R -J -O -K -Sa0.4c -Gred -Wfaint >> ' ps], [10 35])
	% Lines for network
	gmt(['psxy -R -J -Ba10 -BwSnE -W0.25p,- -O -K -X3.25i >> ' ps], t1, t2)
	gmt(['psxy -R -J -W1p -Fna -O -K >> ' ps], t1, t2)
	plotpts(t1, t2, 'NETWORK', ps)
	gmt(['psxy -R -J -O -T >> ' ps])

	builtin('delete','gmt.conf');

% ------------------------------------------------------------------
function plotpts(t1, t2, str, ps)
	% Plots the two data tables and places given text
	gmt(['psxy -R -J -O -K -Sc0.2c -Ggreen -Wfaint >> ' ps], t1)
	gmt(['psxy -R -J -O -K -Sc0.2c -Gblue -Wfaint >> ' ps], t2)
	gmt(['pstext -R -J -O -K -F+cTL+jTL+f12p -Dj0.05i >> ' ps], {str})
