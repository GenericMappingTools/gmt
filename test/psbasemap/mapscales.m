function [ps, path] = mapscales(out_path)
% OUT_PATH -> Path to where the PS file will be written
% PS       -> Full name of the created postscript file (built from OUT_PATH)
% PATH     -> Path to where this file lives (useful for gmtest.m)
%
%	Testing that map scales with various labels have reasonable panels behind them

	full = mfilename('fullpath');
	[pato, fname] = fileparts(full);
	ps = [out_path fname '.ps'];
	path = [pato filesep];

	gmt('destroy'),		gmt('gmtset -Du'),	gmt('destroy')		% Make sure we start with a clean session

	gmt(['psbasemap -R-10/10/-15/15 -JM15c -Bafg90 -BWSne+gazure1 -Lg0/14+c14+f+w1000k+l+ar+jTC' ...
		' -F+gcornsilk1+p0.5p,black -P -K -Xc > ' ps])
	gmt(['psbasemap -R -J -Lg0/11+c11+w1000k+f+l+al+jTC -F+gcornsilk1+p0.5p,black -O -K >> ' ps])
	gmt(['psbasemap -R -J -Lg0/8+c8+w1000k+f+l+at+jTC -F+gcornsilk1+p0.5p,black -O -K >> ' ps])
	gmt(['psbasemap -R -J -Lg0/4+c4+w1.5e6e+f+l"My own map label"+at+u+jTC -F+gcornsilk1+p0.5p,black -O -K >> ' ps])
	gmt(['psbasemap -R -J -Lg4/0+c0+w400n+f+l+al+jTC -F+gcornsilk1+p0.5p,black+s -O -K' ...
		' --FONT_LABEL=32p --FONT_ANNOT_PRIMARY=24p >> ' ps])
	gmt(['psbasemap -R -J -Lg0/-4+c-4+w500M+f+l+al+jTC -F+gcornsilk1+p0.5p,black+i+r -O -K' ...
		' --FONT_LABEL=32p --FONT_ANNOT_PRIMARY=24p >> ' ps])
	gmt(['psbasemap -R -J -L0/-7+c-7+w500M+jTC -F+gcornsilk1+p0.5p,black+s -O -K --FONT_LABEL=32p' ...
		' --FONT_ANNOT_PRIMARY=24p >> ' ps])
	gmt(['psbasemap -R -J -L0/-10+c-10+w500n+u+jTC -F+gcornsilk1+p0.5p,black -O -K --FONT_LABEL=32p' ...
		' --FONT_ANNOT_PRIMARY=24p >> ' ps])
	gmt(['psbasemap -R -J -L0/-12.5+c-12.5+w3e6f+jTC -F+gcornsilk1+p0.5p,black -O -K --FONT_LABEL=32p' ...
		' --FONT_ANNOT_PRIMARY=24p --FORMAT_FLOAT_MAP=%\''.10g >> ' ps])
	% Plot a red cross at the justification point for the scales
	gmt(['psxy -R -J -O -Sx0.2i -W0.5p,red >> ' ps], [
		0	14
		0	11
		0	8
		0	4
		4	0
		0	-4
		0	-7
		0	-10
		0	-12.5]);

	builtin('delete','gmt.conf');
