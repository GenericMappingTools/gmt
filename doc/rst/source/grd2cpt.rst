.. index:: ! grd2cpt

*******
grd2cpt
*******

.. only:: not man

    grd2cpt - Make linear or histogram-equalized color palette table from grid

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**grd2cpt** *grid* [ **-A**\ [**+**\ ]\ *transparency* ]
[ **-C**\ *cptfile* ] [ **-D**\ [**i**\ \|\ **o**] ] [ **-E**\ *nlevels* ]
[ **-F**\ [**R**\ \|\ **r**\ \|\ **h**\ \|\ **c** ] [ **-G**\ *zlo*\ /\ *zhi* ] [ **-I** ]
[ **-L**\ *minlimit/maxlimit* ] [ **-M** ] [ **-N** ]
[ **-Q**\ [**i**\ \|\ **o**] ]
[ |SYN_OPT-R| ]
[ **-S**\ *zstart/zstop/zinc* ]
[ **-T**\ **-**\ \|\ **+**\ \|\ **\_**\ \|\ **=** ]
[ |SYN_OPT-V| ]
[ **-W** ]
[ **-Z** ]

|No-spaces|

Description
-----------

**grd2cpt** reads one or more grid files and writes a color palette
(cpt) file to standard output. The cpt file is based on an existing
master cpt file of your choice, and the mapping from data value to
colors is through the data's cumulative distribution function (CDF), so
that the colors are histogram equalized. Thus if the grid(s) and the
resulting cpt file are used in :doc:`grdimage` with a linear projection,
the colors will be uniformly distributed in area on the plot. Let z be
the data values in the grid. Define CDF(Z) = (# of z < Z) / (# of z in
grid). (NaNs are ignored). These z-values are then normalized to the
master cpt file and colors are sampled at the desired intervals.

The color palette includes three additional colors beyond the range of
z-values. These are the background color (B) assigned to values lower
than the lowest *z*-value, the foreground color (F) assigned to values
higher than the highest *z*-value, and the NaN color (N) painted
wherever values are undefined.  For color tables beyond the
standard GMT offerings, visit cpt-city:
http://soliton.vm.bytemark.co.uk/pub/cpt-city/.

If the master cpt file includes B, F, and N entries, these will be
copied into the new master file. If not, the parameters
:ref:`COLOR_BACKGROUND <COLOR_BACKGROUND>`, :ref:`COLOR_FOREGROUND <COLOR_FOREGROUND>`,
and :ref:`COLOR_NAN <COLOR_NAN>` from
the :doc:`gmt.conf` file or the command line will be used. This default
behavior can be overruled using the options **-D**, **-M** or **-N**.

The color model (RGB, HSV or CMYK) of the palette created by :doc:`makecpt`
will be the same as specified in the header of the master cpt file. When
there is no :ref:`COLOR_MODEL <COLOR_MODEL>` entry in the master cpt file, the
:ref:`COLOR_MODEL <COLOR_MODEL>` specified in the :doc:`gmt.conf` file or on the command
line will be used. 

Required Arguments
------------------

*grid*
    Names of one or more grid files used to derive the color palette
    table. All grids need to have the same size and dimensions. (See
    GRID FILE FORMATS below).

Optional Arguments
------------------

**-A**\ [**+**\ ]\ *transparency*
    Sets a constant level of transparency (0-100) for all color slices.
    Prepend **+** to also affect the fore-, back-, and nan-colors
    [Default is no transparency, i.e., 0 (opaque)].

**-C**\ *cptfile*
    Selects the master color table to use in the interpolation. Choose
    among the built-in tables (type **grd2cpt** to see the list) or give
    the name of an existing cpt file [Default gives a rainbow cpt file].
    Yet another option is to specify -Ccolor1,color2[,color3,...]
    to build a linear continuous cpt from those colors automatically.  
    In this case *color*\ **n** can be a r/g/b triplet, a color name,
    or an HTML hexadecimal color (e.g. #aabbcc ).

**-D**\ [**i**\ \|\ **o**]
    Select the back- and foreground colors to match the colors for
    lowest and highest *z*-values in the output cpt file [Default uses
    the colors specified in the master file, or those defined by the
    parameters :ref:`COLOR_BACKGROUND <COLOR_BACKGROUND>`, :ref:`COLOR_FOREGROUND <COLOR_FOREGROUND>`, and
    :ref:`COLOR_NAN <COLOR_NAN>`]. Append **i** to match the colors for the lowest and
    highest values in the input (instead of the output) cpt file.

**-E**\ *nlevels*
    Create a linear color table by dividing the grid z-range into
    *nlevels* equidistant slices.

**-F**\ [**R**\ \|\ **r**\ \|\ **h**\ \|\ **c**]
    Force output cpt file to written with r/g/b codes, gray-scale values
    or color name (**R**, default) or r/g/b codes only (**r**), or h-s-v
    codes (**h**), or c/m/y/k codes (**c**).

**-G**\ *zlo*\ /\ *zhi*
    Truncate the incoming CPT so that the lowest and highest z-levels
    are to *zlo* and *zhi*.  If one of these equal NaN then
    we leave that end of the CPT alone.  The truncation takes place
    before any resampling.

**-I**
    Reverses the sense of color progression in the master cpt file. Also
    exchanges the foreground and background colors, including those
    specified by the parameters :ref:`COLOR_BACKGROUND <COLOR_BACKGROUND>` and
    :ref:`COLOR_FOREGROUND <COLOR_FOREGROUND>`.

**-L**\ *minlimit/maxlimit*
    Limit range of cpt file to *minlimit/maxlimit*, and don't count data
    outside range when estimating CDF(Z). [Default uses min and max of data.]

**-M**
    Overrule background, foreground, and NaN colors specified in the
    master cpt file with the values of the parameters
    :ref:`COLOR_BACKGROUND <COLOR_BACKGROUND>`, :ref:`COLOR_FOREGROUND <COLOR_FOREGROUND>`,
    and :ref:`COLOR_NAN <COLOR_NAN>`
    specified in the :doc:`gmt.conf` file or on the command line. When
    combined with **-D**, only :ref:`COLOR_NAN <COLOR_NAN>` is considered.

**-N**
    Do not write out the background, foreground, and NaN-color fields
    [Default will write them].

**-Q**\ [**i**\ \|\ **o**]
    Selects a logarithmic interpolation scheme [Default is linear].
    **-Qi** expects input z-values to be log10(z), assigns colors, and
    writes out z [Default]. **-Qo** takes log10(z) first, assigns
    colors, and writes out z. 

.. |Add_-R| unicode:: 0x20 .. just an invisible code
.. include:: explain_-R.rst_

**-S**\ *zstart/zstop/zinc* or **-S**\ *n*
    Set steps in cpt file. Calculate entries in cpt file from *zstart*
    to *zstop* in steps of (*zinc*). Default chooses arbitrary values by
    a crazy scheme. Use **-S**\ *n* to select *n* points from a
    cumulative normal distribution.

**-T**\ **-**\ \|\ **+**\ \|\ **\_**\ \|\ **=**
    Force the color table to be symmetric about zero (from -R to +R).
    Append flag to set the range R: **-** for R =\|zmin\|, **+** for R =
    \|zmax\|, **\_** for R = min(\|zmin\|, \|zmax\|), or **=** for R =
    max(\|zmin\|, \|zmax\|).

**-V**
    Verbose operation. This will write CDF(Z) estimates to stderr. [Default is silent.]

**-W**
    Do not interpolate the input color table but pick the output colors
    starting at the beginning of the map. This is particularly useful in
    combination with a categorical color table. Cannot be used in
    combination with **-Z**.

**-Z**
    Will create a continuous color palette. [Default is discontinuous,
    i.e., constant color intervals] 

.. include:: explain_help.rst_

.. include:: explain_grd_inout.rst_

Examples
--------

Sometimes you don't want to make a cpt file (yet) but would find it
helpful to know that 90% of your data lie between z1 and z2, something
you cannot learn from :doc:`grdinfo`. So you can do this to see some points
on the CDF(Z) curve (use **-V** option to see more):

   ::

    gmt grd2cpt mydata.nc -V > /dev/null

To make a cpt file with entries from 0 to 200 in steps of 20, and ignore
data below zero in computing CDF(Z), and use the built-in master cpt
file relief, run

   ::

    gmt grd2cpt mydata.nc -Crelief -L0/10000 -S0/200/20 > mydata.cpt

See Also
--------

:doc:`gmt`, :doc:`gmt.conf`,
:doc:`grdhisteq`, :doc:`grdinfo`, :doc:`makecpt`
