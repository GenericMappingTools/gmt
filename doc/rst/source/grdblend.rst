.. index:: ! grdblend

********
grdblend
********

.. only:: not man

    Blend several partially over-lapping grids into one large grid

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt grdblend** [ *blendfile* \| *grid1* *grid2* ... ] |-G|\ *outgrid*
|SYN_OPT-I|
|SYN_OPT-R|
[ |-C|\ **f**\ \|\ **l**\ \|\ **o**\ \|\ **u**\ ][**+n**\ \|\ **p**\ ] [ |-N|\ *nodata* ]
[ |-Q| ] [ |-Z|\ *scale* ]
[ |SYN_OPT-V| ]
[ |-W|\ [**z**] ]
[ |SYN_OPT-f| ]
[ |SYN_OPT-n| ]
[ |SYN_OPT-r| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**grdblend** reads a listing of grid files and blend parameters and
creates a binary grid file by blending the other grids using
cosine-taper weights. **grdblend** will report if some of the nodes are
not filled in with data. Such unconstrained nodes are set to a value
specified by the user [Default is NaN]. Nodes with more than one value
will be set to the weighted average value. Any input grid that does not
share the final output grid's node registration and grid spacing will
automatically be resampled via calls to :doc:`grdsample`. Note: Due to the
row-by-row i/o nature of operations in **grdblend** we only support the
netCDF and native binary grid formats for both input and output. 

Required Arguments
------------------

.. _-G:

**-G**\ *outgrid*
    *outgrid* is the name of the binary output grid file. (See GRID FILE
    FORMATS below). Only netCDF and native binary grid formats are can
    be written directly. Other output format choices will be handled by
    reformatting the output once blending is complete. 

.. _-I:

.. include:: explain_-I.rst_

.. _-R:

.. |Add_-R| unicode:: 0x20 .. just an invisible code
.. include:: explain_-R.rst_

Optional Arguments
------------------

*blendfile*
    ASCII file with one record per grid file to include in the blend.
    Each record may contain up to three items, separated by spaces or tabs:
    the gridfile name (required), the **-R**-setting for the interior region
    (optional), and the relative weight *wr* (optional). In the combined weighting scheme, this
    grid will be given zero weight outside its domain, weight = *wr*
    inside the interior region, and a 2-D cosine-tapered weight between
    those end-members in the boundary strip. However, if a negative *wr*
    is given then the sense of tapering is inverted (i.e., zero weight
    inside its domain). If the inner region should instead exactly match
    the grid region then specify a - instead of the **-R**-setting, or
    leave it off entirely.  Likewise, if a weight *wr* is not specified
    we default to a weight of 1.  If the ASCII *blendfile* file is not
    given **grdblend** will read standard input. Alternatively, if you
    have more than one grid file to blend and you wish (a) all input
    grids to have the same weight (1) and (b) all grids
    should use their actual region as the interior region, then you may simply
    list all the grids on the command line instead of providing a
    *blendfile*. You must specify at least 2 input grids for this
    mechanism to work. Any grid that is not co-registered with the
    desired output layout implied by **-R**, **-I** (and |SYN_OPT-r|) will
    first be resampled via :doc:`grdsample`. Also, grids that are not in
    netCDF or native binary format will first be reformatted via :doc:`grdconvert`.

.. _-C:

**-C**\ **f**\ \|\ **l**\ \|\ **o**\ \|\ **u**\ ][**+n**\ \|\ **p**\ ]
    Clobber mode: Instead of blending, simply pick the value of one of
    the grids that covers a node. Select from the following modes: **f**
    for the first grid to visit a node; **o** for the last grid to visit
    a node; **l** for the grid with the lowest value, and **u** for the
    grid with the uppermost value. For modes **f** and **o** the
    ordering of grids in the *blendfile* will dictate which grid
    contributes to the final result. Weights and cosine tapering are not
    considered when clobber mode is active. Optionally, append **+p** or **+n**.
    Then, we always initialize output to equal the first grid but then
    for subsequent grids we only consider them in the decision if the
    values are >= 0 or <= 0, respectively.

.. _-N:

**-N**\ *nodata*
    No data. Set nodes with no input grid to this value [Default is NaN].

.. _-Q:

**-Q**
    Create plain header-less grid file (for use with external tools).
    Requires that the output grid file is a native format (i.e., not netCDF). 

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. _-W:

**-W**\ [**z**]
    Do not blend, just output the weights used for each node [Default makes the blend].
    Append **z** to write the weight*z sum instead.

.. _-Z:

**-Z**\ *scale*
    Scale output values by *scale* before writing to file. [1]. 

.. |Add_-f| unicode:: 0x20 .. just an invisible code
.. include:: explain_-f.rst_

.. include:: explain_-n.rst_

.. |Add_nodereg| unicode:: 0x20 .. just an invisible code
.. include:: explain_nodereg.rst_

.. include:: explain_help.rst_

.. include:: explain_grd_output.rst_

.. include:: explain_grd_coord.rst_

Tapering
--------

While the weights computed are tapered from 1 to 0, we are computing weighted
averages, so if there is only a single grid given then the weighted output
will be identical to the input.  If you are looking for a way to taper your
data grid, see grdmath's TAPER operator.

Examples
--------

To create a grid file from the four grid files piece\_?.nc, giving them each the different
weights, make the blendfile like this

   ::

    piece_1.nc -R<subregion_1> 1
    piece_2.nc -R<subregion_2> 1.5
    piece_3.nc -R<subregion_3> 0.9
    piece_4.nc -R<subregion_4> 1

Then run

   ::

    gmt grdblend blend.job -Gblend.nc -R<full_region> -I<dx/dy> -V

To blend all the grids called MB\_\*.nc given them all equal weight, try

   ::

    gmt grdblend MB_*.nc -Gblend.nc -R<full_region> -I<dx/dy> -V

Warning on large file sets
--------------------------

While **grdblend** can process any number of files, it works by keeping those
files open that are being blended, and close files as soon as they are finished.
Depending on your session, many files may remain open at the same time.
Some operating systems set fairly modest
default limits on how many concurrent files can be open, e.g., 256.  If you
run into this problem then you can change this limit; see your operating system
documentation for how to change system limits.

See Also
--------

:doc:`gmt`,
:doc:`grd2xyz`,
:doc:`grdconvert`,
:doc:`grdedit`,
:doc:`grdsample`
