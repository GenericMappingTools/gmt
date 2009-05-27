function fixcoast (id1,idfix)
% FIXCOAST
%	$Id: fixcoast.m,v 1.4 2009-05-27 23:45:46 guru Exp $
%
% Give the id of the polygon to be fixed.  The polygon.id
% file will be overwritten with the changed polygon,  Optionally,
% Give a second id; that polygon is just plotted for reference.
%
% The following click-types are supported:
%   L = Delete nearest point
%   M = Move the nearest point to clicked point
%   R = Insert the clicked point bewteen the two nearest points
%   <space> = ends the editing and saves file

file1 = ['polygon.' num2str(id1)];
load (file1)
x1 = polygon(:,1);
y1 = polygon(:,2);
if (nargin == 2)
	file2 = ['polygon.' num2str(idfix)];
	load (file2)
	x2 = polygon(:,1);
	y2 = polygon(:,2);
end
figure(1)
clf
pl = plot (x1, y1, 'g-');
hold on
pp = plot (x1, y1, '.');
if (nargin == 2)
	plot (x2, y2, 'k-');
end
title ('Zoom in if you need to - then hit return to continue')
pause
title ('Click to start editing - hit any key to quit')
hit = waitforbuttonpress;
while (hit == 0)   % Until a key is pressed
    title ('Click L to delete, M to move, and R to insert')
    [x0, y0, key] = ginput(1);
   n = length(x1);
    numbers = 1:n;
    k = find_near (x1, y1, x0, y0);
    if (key == 1)
        use = find (numbers ~= k(1));
        x1 = x1(use);
        y1 = y1(use);
    elseif (key == 2)
        x1(k(1)) = x0;
        y1(k(1)) = y0;
    elseif (key == 3)
        x1 = [ x1(1:min(k)) x0 x1(max(k):n) ];
        y1 = [ y1(1:min(k)) y0 y1(max(k):n) ];
        end
    set (pl, 'XData', x1, 'YData', y1);
    set (pp, 'XData', x1, 'YData', y1);
    title ('Click to start editing - hit any key to quit')
    drawnow
    hit = waitforbuttonpress;
end
file = ['polygon.' num2str(id1) '.new'];
fp = fopen (file, 'wt');
A = [ x1'; y1'];
fprintf (fp, '%.6f\t%.6f\n', A);
fclose (fp);

function pair = find_near (x, y, x0, y0)
r = abs ((x - x0).*cosd(0.5*(y+y0)) + sqrt(-1).*(y - y0));
[rsort, i] = sort (r);
pair = i(1:2);
