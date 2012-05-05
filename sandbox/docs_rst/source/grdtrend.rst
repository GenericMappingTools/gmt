********
grdtrend
********


grdtrend - Fit trend surface to grids and compute residuals

`Synopsis <#toc1>`_
-------------------

**grdtrend** *grdfile* **-N**\ *n\_model*\ [**r**\ ] [ **-D**\ *diff.nc*
] [ **-R**\ *west*/*east*/*south*/*north*\ [**r**\ ] ] [
**-T**\ *trend.nc* ] [ **-V**\ [*level*\ ] ] [ **-W**\ *weight.nc* ]

`Description <#toc2>`_
----------------------

**grdtrend** reads a 2-D grid file and fits a low-order polynomial trend
to these data by [optionally weighted] least-squares. The trend surface
is defined by:

m1 + m2\*x + m3\*y + m4\*x\*y + m5\*x\*x + m6\*y\*y + m7\*x\*x\*x +
m8\*x\*x\*y + m9\*x\*y\*y + m10\*y\*y\*y.

The user must specify **-N**\ *n\_model*, the number of model parameters
to use; thus, **-N**\ *4* fits a bilinear trend, **-N**\ *6* a quadratic
surface, and so on. Optionally, append **r** to the **-N** option to
perform a robust fit. In this case, the program will iteratively
reweight the data based on a robust scale estimate, in order to converge
to a solution insensitive to outliers. This may be handy when separating
a "regional" field from a "residual" which should have non-zero mean,
such as a local mountain on a regional surface.

If data file has values set to NaN, these will be ignored during
fitting; if output files are written, these will also have NaN in the
same locations.

`Common Arguments And Specifications <#toc3>`_
----------------------------------------------

All options marked with an asterisk (\*) are common GMT command-line
options. Their full syntax as well as how to specify pens, pattern
fills, colors, and fonts can be found in the **gmt** man page. Note: No
space is allowed between the option flag and the associated arguments.

`Required Arguments <#toc4>`_
-----------------------------

*grdfile*
    The name of a 2-D binary grid file.
**-N**\ *n\_model*\ [**r**\ ]
    [**r**\ ]\ *n\_model* sets the number of model parameters to fit.
    Append **r** for robust fit.

`Optional Arguments <#toc5>`_
-----------------------------

**-D**\ *diff.nc*
    Write the difference (input data - trend) to the file *diff.nc*.
**-R**\ *xmin*/*xmax*/*ymin*/*ymax*\ [**r**\ ] (\*)
    Specify the region of interest. Using the **-R** option will select
    a subsection of the input grid. If this subsection exceeds the
    boundaries of the grid, only the common region will be extracted.
**-T**\ *trend.nc*
    Write the fitted trend to the file *trend.nc*.
**-V**\ [*level*\ ] (\*)
    Select verbosity level [1].
**-W**\ *weight.nc*
    If *weight.nc* exists, it will be read and used to solve a weighted
    least-squares problem. [Default: Ordinary least-squares fit.] If the
    robust option has been selected, the weights used in the robust fit
    will be written to *weight.nc*.
**-^** (\*)
    Print a short message about the syntax of the command, then exits.
**-?** (\*)
    Print a full usage (help) message, including the explanation of
    options, then exits.

`Remarks <#toc6>`_
------------------

The domain of x and y will be shifted and scaled to [-1, 1] and the
basis functions are built from Legendre polynomials. These have a
numerical advantage in the form of the matrix which must be inverted and
allow more accurate solutions. NOTE: The model parameters listed with
**-V** are Legendre polynomial coefficients; they are not numerically
equivalent to the m#s in the equation described above. The description
above is to allow the user to match **-N** with the order of the
polynomial surface. See **grdmath** if you need to evaluate the trend
using the reported coefficients.

`Grid File Formats <#toc7>`_
----------------------------

By default **GMT** writes out grid as single precision floats in a
COARDS-complaint netCDF file format. However, **GMT** is able to produce
grid files in many other commonly used grid file formats and also
facilitates so called "packing" of grids, writing out floating point
data as 2- or 4-byte integers. To specify the precision, scale and
offset, the user should add the suffix
**=**\ *id*\ [**/**\ *scale*\ **/**\ *offset*\ [**/**\ *nan*]], where
*id* is a two-letter identifier of the grid type and precision, and
*scale* and *offset* are optional scale factor and offset to be applied
to all grid values, and *nan* is the value used to indicate missing
data. When reading grids, the format is generally automatically
recognized. If not, the same suffix can be added to input grid file
names. See `**grdreformat**\ (1) <grdreformat.1.html>`_ and Section 4.17
of the GMT Technical Reference and Cookbook for more information.

When reading a netCDF file that contains multiple grids, **GMT** will
read, by default, the first 2-dimensional grid that can find in that
file. To coax **GMT** into reading another multi-dimensional variable in
the grid file, append **?**\ *varname* to the file name, where *varname*
is the name of the variable. Note that you may need to escape the
special meaning of **?** in your shell program by putting a backslash in
front of it, or by placing the filename and suffix between quotes or
double quotes. The **?**\ *varname* suffix can also be used for output
grids to specify a variable name different from the default: "z". See
`**grdreformat**\ (1) <grdreformat.1.html>`_ and Section 4.18 of the GMT
Technical Reference and Cookbook for more information, particularly on
how to read splices of 3-, 4-, or 5-dimensional grids.

`Examples <#toc8>`_
-------------------

To remove a planar trend from hawaii\_topo.nc and write result in
hawaii\_residual.nc:

grdtrend hawaii\_topo.nc -N3 -Dhawaii\_residual.nc

To do a robust fit of a bicubic surface to hawaii\_topo.nc, writing the
result in hawaii\_trend.nc and the weights used in hawaii\_weight.nc,
and reporting the progress:

grdtrend hawaii\_topo.nc -N10r -Thawaii\_trend.nc -Whawaii\_weight.nc -V

`See Also <#toc9>`_
-------------------

`*gmt*\ (1) <gmt.1.html>`_ , `*grdfft*\ (1) <grdfft.1.html>`_ ,
`*grdfilter*\ (1) <grdfilter.1.html>`_

