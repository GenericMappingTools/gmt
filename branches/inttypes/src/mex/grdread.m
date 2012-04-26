% GRDREAD Read matrix from a GMT grd-file
%
%	Z = GRDREAD('filename') will return Z, the matrix stored in the
%	GMT grdfile format.
%
%	[Z,D] = GRDREAD('filename') will also return an array D that
%	contains (xmin, xmax, ymin, ymax, zmin, zmax, format, xinc, yinc)
%	for this data set.  Format is 1 for pixel registration and 0 for
%	grid node registration.
%	
%	[X,Y,Z] = GRDREAD('filename') will return the x and y arrays
%	defining the grid as well as the data matrix Z.
%	
%	[X,Y,Z,D] = GRDREAD('filename') will return the x and y arrays,
%	the data matrix Z, and the info vector D.
%	
%	See also GRDWRITE, GRDINFO
	
%	$Id$
