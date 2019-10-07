.. index:: ! grdhisteq

*********
grdhisteq
*********

.. only:: not man

    Perform histogram equalization for a grid

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt grdhisteq** *in_grdfile* [ |-G|\ *out_grdfile* ]
[ |-C|\ *n_cells* ] [ |-D|\ [*file*] ] [ |-N|\ [*norm*] ]
[ |-Q| ]
|SYN_OPT-R|
|SYN_OPT-V|
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**grdhisteq** allows the user to find the data values which divide a
given grid file into patches of equal area. One common use of
**grdhisteq** is in a kind of histogram equalization of an image. In
this application, the user might have a grid of flat topography with a
mountain in the middle. Ordinary gray shading of this file (using
:doc:`grdimage` or :doc:`grdview`) with a linear mapping from topography to graytone will
result in most of the image being very dark gray, with the mountain
being almost white. One could use **grdhisteq** to write to stdout or file an
ASCII list of those data values which divide the range of the data into
*n_cells* segments, each of which has an equal area in the image. Using
**awk** or :doc:`makecpt` one can take this output and build a CPT;
using the CPT with :doc:`grdimage` will result in an image with all levels
of gray occurring equally. Alternatively, see :doc:`grd2cpt`.

The second common use of **grdhisteq** is in writing a grid with
statistics based on some kind of cumulative distribution function. In
this application, the output has relative highs and lows in the same
(x,y) locations as the input file, but the values are changed to reflect
their place in some cumulative distribution. One example would be to
find the lowest 10% of the data: Take a grid, run **grdhisteq** and make
a grid using *n_cells* = 10, and then contour the result to trace the 1
contour. This will enclose the lowest 10% of the data, regardless of
their original values. Another example is in equalizing the output of
:doc:`grdgradient`. For shading purposes it is desired that the data have a
smooth distribution, such as a Gaussian. If you run **grdhisteq** on
output from :doc:`grdgradient` and make a grid file output with the
Gaussian option, you will have a grid whose values are distributed
according to a Gaussian distribution with zero mean and unit variance.
The locations of these values will correspond to the locations of the
input; that is, the most negative output value will be in the (x,y)
location of the most negative input value, and so on. 

Required Arguments
------------------

*in_grdfile*
    2-D grid file to be equalized. (See GRID FILE FORMATS below).

Optional Arguments
------------------

.. _-C:

**-C**\ *n_cells*
    Sets how many cells (or divisions) of data range to make [16].

.. _-D:

**-D**
    Dump level information to *file*, or standard output if no file is provided.

.. _-G:

**-G**\ *out_grdfile*
    Name of output 2-D grid file. Used with **-N** only. (See GRID FILE FORMATS below).

.. _-N:

**-N**\ [*norm*]
    Gaussian output. Use with **-G** to make an output grid with
    standard normal scores. Append *norm* to force the scores to fall in
    the <-1,+1> range [Default is standard normal scores].

.. _-Q:

**-Q**
    Quadratic output. Selects quadratic histogram equalization. [Default is linear]. 

.. _-R:

.. |Add_-R| replace:: Using the **-R** option
    will select a subsection of *in_grdfile* grid. If this subsection
    exceeds the boundaries of the grid, only the common region will be extracted. 
.. include:: explain_-R.rst_

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. include:: explain_help.rst_

.. include:: explain_grd_inout_short.rst_

Examples
--------

To find the height intervals that divide the file heights.nc into 16
divisions of equal area:

   ::

    gmt grdhisteq heights.nc -C16 -D > levels.d

To make the poorly distributed intensities in the file raw_intens.nc
suitable for use with :doc:`grdimage` or :doc:`grdview`, run

   ::

    gmt grdhisteq raw_intens.nc -Gsmooth_intens.nc -N -V

Notes
------

#. For geographical grids we do a weighted histogram equalization since the
   area of each node varies with latitude.
#. If you use **grdhisteq** to make a Gaussian output for gradient shading
   in :doc:`grdimage` or :doc:`grdview`, you should be aware of the following:
   the output will be in the range [-x, x], where x is based on the number
   of data in the input grid (nx \* ny) and the cumulative Gaussian
   distribution function F(x). That is, let N = nx \* ny. Then x will be
   adjusted so that F(x) = (N - 1 + 0.5)/N. Since about 68% of the values
   from a standard normal distribution fall within ±\ 1, this will be true
   of the output grid. But if N is very large, it is possible for x to be
   greater than 4. Therefore, with the :doc:`grdview` program clipping
   gradients to the range [-1, 1], you will get correct shading of 68% of
   your data, while 16% of them will be clipped to -1 and 16% of them
   clipped to +1. If this makes too much of the image too light or too
   dark, you should take the output of **grdhisteq** and rescale it using
   :doc:`grdmath` and multiplying by something less than 1.0, to shrink the
   range of the values, thus bringing more than 68% of the image into the
   range [-1, 1]. Alternatively, supply a normalization factor with **-N**.

See Also
--------

:doc:`gmt`, :doc:`gmt.conf`,
:doc:`grd2cpt`,
:doc:`grdgradient`,
:doc:`grdimage`, :doc:`grdmath`,
:doc:`grdview`, :doc:`makecpt`
