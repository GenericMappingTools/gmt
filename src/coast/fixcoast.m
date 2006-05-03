function fixcoast (id)
% FIXCOAST
%	$Id: fixcoast.m,v 1.2 2006-05-03 03:53:49 pwessel Exp $
%
% Give the id of the polygon to be fixed.  The polygon.id
% file will be overwritten with the changed polygon
%
% The following click-types are supported:
%   L = Delete nearest point
%   M = Move the nearest point to clicked point
%   R = Insert the clicked point bewteen the two nearest points
%   <space> = ends the editing and saves file

file = ['polygon.' num2str(id)];
load (file)
x = polygon(:,1);
y = polygon(:,2);

figure(1)
clf
pl = plot (x, y, 'g-');
hold on
pp = plot (x, y, '.');
title ('Click to start editing - hit any key to quit')

hit = waitforbuttonpress;
while (hit == 0)   % Until a key is pressed
    title ('Click L to delete, M to move, and R to insert')
    [x0, y0, key] = ginput(1);
   n = length(x);
    numbers = 1:n;
    k = find_near (x, y, x0, y0);
    if (key == 1)
        use = find (numbers ~= k(1));
        x = x(use);
        y = y(use);
    elseif (key == 2)
        x(k(1)) = x0;
        y(k(1)) = y0;
    elseif (key == 3)
        x = [ x(1:min(k)) x0 x(max(k):n) ];
        y = [ y(1:min(k)) y0 y(max(k):n) ];
        end
    set (pl, 'XData', x, 'YData', y);
    set (pp, 'XData', x, 'YData', y);
    title ('Click to start editing - hit any key to quit')
    drawnow
    hit = waitforbuttonpress;
end
file = ['polygon.' num2str(id) '.new'];
A = [ x y];
cmd = ['save ' file ' A -tabs -ascii -double'];
eval (cmd);

function pair = find_near (x, y, x0, y0)
r = abs ((x - x0).*cosd(0.5*(y+y0)) + sqrt(-1).*(y - y0));
[rsort, i] = sort (r);
pair = i(1:2);
