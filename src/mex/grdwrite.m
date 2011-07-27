% GRDWRITE Write matrix to a GMT grd-file
%
%	GRDWRITE(Z, D, 'filename') will write the matrix Z using the
%	GMT grdfile format.  The array D contains (xmin, xmax, ymin,
%	ymax, zmin, zmax, format) for this data set.  Format is 1 for
%	pixel registration and 0 for grid node registration.
%	
%	GRDWRITE(Z, D, 'filename', 'title') will in addition set the
%	title field in the grd file.
%
%	GRDWRITE(X, Y, Z, 'filename', 'title', [1]) expects to get the x-
%	and y- arrays in addition to the matrix. It assumes grid node
%	registration unless a sixth argument is passed (as 1).
%
%	See also GRDREAD, GRDINFO
	
%	$Id$
