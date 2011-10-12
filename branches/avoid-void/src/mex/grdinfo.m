% GRDINFO Get header information from a GMT grd-file
%
%	GRDINFO('filename') will display the contents of the file's
%	header, including title, remarks, and range information.
%
%	D = GRDINFO('filename') will in addition return a vector
%	containing (xmin, xmax, ymin, ymax, zmin, zmax, format, xinc, yinc).
%	Format is 1 for pixel registration and 0 for grid node reg-
%	istration.
%	
%	See also GRDREAD, GRDWRITE
	
%  $Id$
