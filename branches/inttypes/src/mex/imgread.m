function [lon lat z ym] = imgread (file, west, east, south, north, scl)
% IMGREAD  Read a section of a Sandwell/Smith Mercator img file
%
% [lon lat z] = imgread (file, west, east, south, north[, scl])
% [lon lat z ym] = imgread (file, west, east, south, north[, scl])
%
% Input: file   Name of *.img file
%        west   West boundary longitude
%        east   East boundary longitude
%        south  South boundary latitude
%        north  North boundary latitude
%        scl    Optional conversion scale (typically 0.1 for FAA
%		0.01 for GEOID, 0.02 for VGG and 1 for TOPO) [1]
%
% Output:
%   lon     Array of longitudes (equidistant)
%   lat     Array of latitudes (variable spacing)
%   z       Data matrix
%   ym	    Optional array of Mercator y-coordinates (equidistant)
%
% W/e/s/n may be rounded off to fit nearest coordinate in the grid.
%
% Example, to pull out data near Hawaii from the FAA grid:
% [lon lat z] = imgreadf ('grav.16.1.img', 170, 220, 10, 40, 0.1);

% $Id$
% P. Wessel, based on img2mergrd.c by Walter H.F. Smith

if (nargin == 5)	% Must specify default scale
	scl = 1;
end

% Determine what kind of img file we are dealing with:

fp = fopen (file, 'r', 'b');
fseek (fp, 0, 1);
bytes = ftell (fp);

if (bytes == 136857600)       % 2 min, ~72 lat
    maxlat = 72.0059773539;
    inc = 2;
elseif (bytes == 186624000)   % 2 min, ~80 lat
    maxlat = 80.738;    
    inc = 2;
elseif (bytes == 547430400)   % 1 min, ~72 lat
    maxlat = 72.0059773539;    
    inc = 1;
elseif (bytes == 746496000)   % 1 min, ~80 lat
    maxlat = 80.738;    
    inc = 1;
end
minlat = -maxlat;

[nx360 radius nytop nyrow] = GMT_img_setup_coord (minlat, maxlat, inc);

% Expected edges of input image based on coordinate initialization (might
% not exactly match user spec):

toplat = GMT_img_ypix_to_lat (0, nytop, radius);
botlat = GMT_img_ypix_to_lat (nyrow, nytop, radius);
dx = 1.0 / (nx360 / 360.0);
if (toplat < north)
    disp (['imgread:  WARNING:  Your top latitude (' num2str(north) ') lies outside top latitude of input (' num2str(toplat) ') - now truncated.']);
    north = toplat - 1.0e-8;	% To ensure proper round-off in calculating ny
end
if (botlat > south)
    disp (['imgread:  WARNING:  Your bottom latitude (' num2str(south) ') lies outside bottom latitude of input (' num2str(botlat) ') - now truncated.']);
    south = botlat + GMT_CONV_LIMIT;	% To ensure proper round-off in calculating ny */
end

% Re-adjust user-selected region so that it falls on pixel coordinate boundaries:

jinstart = floor (GMT_img_lat_to_ypix (north, nytop, radius));
jinstop  = ceil  (GMT_img_lat_to_ypix (south, nytop, radius));
% jinstart <= jinputrow < jinstop
ny = jinstop - jinstart;

iinstart = floor (west/dx);
iinstop  = ceil  (east/dx);
% iinstart <= ipixelcol < iinstop, but modulo all with nx360
% Reset left and right edges of user area:
nx = iinstop - iinstart;

% Set iinstart so that it is non-negative, for use to index pixels.
while (iinstart < 0); iinstart = iinstart + nx360; end

equator = round (GMT_img_lat_to_ypix (0.0, nytop, radius));
x_min = iinstart * dx;
x_max = x_min + nx * dx;
y_max = (nyrow - jinstart - equator) * dx;
y_min = y_max - ny * dx;
if (x_max > 360.0)
    x_max = x_max - 360.0;
    x_min = x_min - 360.0;
end

% Now malloc some space for float grd array, integer pixel index, and short
% integer data buffer.

z = zeros (ny, nx);
ix = mod ((0:(nx-1)) + iinstart, nx360) + 1;

fseek (fp, 0, -1);	% Rewind

if (jinstart > 0 && jinstart < nyrow)
    fseek (fp, (2 * nx360 * jinstart), -1);
end

% Now loop over output points, reading and handling data as needed


for jout = ny:-1:1
    row = fread(fp, nx360, 'int16');	% Read entire row
    k = find (mod(row,2) == 1); % Find odd values and adjust
    row(k) = row(k) - 1;
    z(jout,:) = scl * row(ix);
end
	
fclose (fp);
half = 0.5 * dx;
x = (x_min + half) : dx : (x_max - half);
y = (y_min + half) : dx : (y_max - half);
[lon lat] = merc_inv (x, y);
if (nargout == 4)
	ym = y;
end

function [lon lat] = merc_inv (x, y)
% MERC_INV convert x,y to lon,lat

lon = x;
k = find (lon < 0.0);
if (~isempty(k))
    lon(k) = lon(k) + 360.0;
end
lat = 2*atand (exp (deg2rad(y))) - 90;

function f = GMT_img_gud_fwd (y)
% The Forward Gudermannian function.  Given y, the distance
% from the Equator to a latitude on a spherical Mercator map
% developed from a sphere of unit radius, returns the latitude
% in radians.  Should be called with -oo < y < +oo.  Returned
% value will be in -M_PI_2 < value < +M_PI_2.  */
	 
f = 2.0 * atan(exp(y)) - 0.5*pi;

function f = GMT_img_gud_inv (phi)
% The Inverse Gudermannian function.  Given phi, a latitude
% in radians, returns the distance from the Equator to this
% latitude on a Mercator map tangent to a sphere of unit
% radius.  Should be called with -M_PI_2 < phi < +M_PI_2.
% Returned value will be in -oo < value < +oo.   */
	
f = log(tan(0.25*pi + 0.5 * phi));

function f = GMT_img_lat_to_ypix (lat, nytop, radius)
% Given Latitude in degrees and pointer to coordinate struct,
% return (double) coordinate from top edge of input img file
% measured downward in coordinate pixels.  */
	 
f = nytop - radius * GMT_img_gud_inv(deg2rad(lat));

function f = GMT_img_ypix_to_lat (ypix, nytop, radius)
% Given Y coordinate, measured downward from top edge of 
% input img file in pixels, and pointer to coordinate struct,
% return Latitude in degrees.  */
	
f = rad2deg(GMT_img_gud_fwd ((nytop - ypix) / radius));

function [nx360 radius nytop nyrow] = GMT_img_setup_coord (minlat, maxlat, mpixel)
% Given the RANGE info, set up the COORD values.  Return (-1) on failure;
% 0 on success.  */

nx360  = round (360.0 * 60.0 / mpixel);
radius = nx360 / (2.0 * pi);
nytop  = round (radius * GMT_img_gud_inv(deg2rad(maxlat)));
nyrow  = nytop - round (radius * GMT_img_gud_inv(deg2rad(minlat)));
