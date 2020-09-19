.. index:: ! grd2cpt
.. include:: module_core_purpose.rst_

*******
grd2cpt
*******

|grd2cpt_purpose|

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt grd2cpt** *grid* [ |-A|\ *transparency*\ [**+a**] ]
[ |-C|\ *cpt* ] [ |-D|\ [**i**] ]
[ |-E|\ [*nlevels*] ]
[ |-F|\ [**R**\|\ **r**\|\ **h**\|\ **c**][**+c**] ]
[ |-G|\ *zlo*\ /\ *zhi* ]
[ |-H| ]
[ |-I|\ [**c**][**z**] ]
[ |-L|\ *minlimit/maxlimit* ]
[ |-M| ]
[ |-N| ]
[ |-Q|\ [**i**\|\ **o**] ]
[ |SYN_OPT-R| ]
[ |-S|\ **h**\|\ **l**\|\ **m**\|\ **u** ]
[ |-T|\ *start/stop/inc* ]
[ |SYN_OPT-V| ]
[ |-W|\ [**w**] ]
[ |-Z| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**grd2cpt** reads one or more grid files and writes a static color palette
(CPT) file. In classic mode we write the CMT to standard output, while under
modern mode we simply save the CPT as the current session CPT (but see **-H**).
The CPT is based on an existing dynamic
master CPT of your choice, and the mapping from data value to
colors is through the data's cumulative distribution function (CDF), so
that the colors are histogram equalized. Thus if the grid(s) and the
resulting CPT are used in :doc:`grdimage` with a linear projection,
the colors will be uniformly distributed in area on the plot. Let z be
the data values in the grid. Define CDF(Z) = (# of z < Z) / (# of z in
grid). (NaNs are ignored). These z-values are then normalized to the
master CPT and colors are sampled at the desired intervals.

The color palette includes three additional colors beyond the range of
z-values. These are the background color (B) assigned to values lower
than the lowest *z*-value, the foreground color (F) assigned to values
higher than the highest *z*-value, and the NaN color (N) painted
wherever values are undefined.  For color tables beyond the
standard GMT offerings, visit `cpt-city <http://soliton.vm.bytemark.co.uk/pub/cpt-city/>`_ and
`Scientific Colour-Maps <http://www.fabiocrameri.ch/colourmaps.php>`_.

If the master CPT includes B, F, and N entries, these will be
copied into the new master file. If not, the parameters
:term:`COLOR_BACKGROUND`, :term:`COLOR_FOREGROUND`,
and :term:`COLOR_NAN` from
the :doc:`gmt.conf` file or the command line will be used. This default
behavior can be overruled using the options **-D**, **-M** or **-N**.

The color model (RGB, HSV or CMYK) of the palette created by :doc:`makecpt`
will be the same as specified in the header of the master CPT. When
there is no :term:`COLOR_MODEL` entry in the master CPT, the
:term:`COLOR_MODEL` specified in the :doc:`gmt.conf` file or on the command
line will be used.

Required Arguments
------------------

*grid*
    Names of one or more grid files used to derive the color palette
    table. All grids need to have the same size and dimensions. (See
    GRID FILE FORMATS below).

Optional Arguments
------------------

.. _-A:

**-A**\ *transparency*\ [**+a**]
    Sets a constant level of transparency (0-100) for all color slices.
    Append **+a** to also affect the fore-, back-, and nan-colors
    [Default is no transparency, i.e., 0 (opaque)].

.. _-C:

.. include:: create_cpt.rst_

.. _-D:

**-D**\ [**i**]
    Select the back- and foreground colors to match the colors for
    lowest and highest *z*-values in the output CPT [Default uses
    the colors specified in the master file, or those defined by the
    parameters :term:`COLOR_BACKGROUND`, :term:`COLOR_FOREGROUND`, and
    :term:`COLOR_NAN`]. Append **i** to match the colors for the lowest and
    highest values in the input (instead of the output) CPT.

.. _-E:

**-E**\ [*nlevels*]
    Create a linear color table by using the grid z-range as the new
    limits in the CPT.  Alternatively, append *nlevels* and we will
    resample the color table into *nlevels* equidistant slices.

.. _-F:

**-F**\ [**R**\|\ **r**\|\ **h**\|\ **c**][**+c**]
    Force output CPT to written with r/g/b codes, gray-scale values
    or color name (**R**, default) or r/g/b codes only (**r**), or h-s-v
    codes (**h**), or c/m/y/k codes (**c**).  Optionally or alternatively,
    append **+c** to write discrete palettes in categorical format.

.. _-G:

**-G**\ *zlo*\ /\ *zhi*
    Truncate the incoming CPT so that the lowest and highest z-levels
    are to *zlo* and *zhi*.  If one of these equal NaN then
    we leave that end of the CPT alone.  The truncation takes place
    before any resampling. See also :ref:`manipulating_CPTs`

.. _-H:

**-H**\
    Modern mode only: Write the CPT to standard output as well [Default saves
    the CPT as the session current CPT].  Required for scripts used to make
    animations via :doc:`movie` where we must pass named CPT files.

.. _-I:

**-I**\ [**c**][**z**]
    Append **c** [Default] to reverse the sense of color progression in the master CPT. Also
    exchanges the foreground and background colors, including those
    specified by the parameters :term:`COLOR_BACKGROUND` and
    :term:`COLOR_FOREGROUND`.
    Append **z** to reverse the sign of z-values in the color table.  Note that
    this change of *z*-direction happens before **-G** and **-S** values are used
    so the latter much be compatible with the changed *z*-range. See also :ref:`manipulating_CPTs`

.. _-L:

**-L**\ *minlimit/maxlimit*
    Limit range of CPT to *minlimit/maxlimit*, and don't count data
    outside this range when estimating CDF(Z). [Default uses min and max of data.]

.. _-M:

**-M**
    Overrule background, foreground, and NaN colors specified in the
    master CPT with the values of the parameters
    :term:`COLOR_BACKGROUND`, :term:`COLOR_FOREGROUND`,
    and :term:`COLOR_NAN`
    specified in the :doc:`gmt.conf` file or on the command line. When
    combined with **-D**, only :term:`COLOR_NAN` is considered.

.. _-N:

**-N**
    Do not write out the background, foreground, and NaN-color fields
    [Default will write them].

.. _-Q:

**-Q**\ [**i**\|\ **o**]
    Selects a logarithmic interpolation scheme [Default is linear].
    **-Qi** expects input z-values to be log10(z), assigns colors, and
    writes out z [Default]. **-Qo** takes log10(z) first, assigns
    colors, and writes out z.

.. _-R:

.. |Add_-R| unicode:: 0x20 .. just an invisible code
.. include:: explain_-R.rst_

.. _-S:

**-S**\ **h**\|\ **l**\|\ **m**\|\ **u**
    Force the color table to be symmetric about zero (from -*R* to +\ *R*).
    Append flag to set the range *R*: **l** for *R* =\|zmin\|, **u** for *R* =
    \|zmax\|, **m** for *R* = min(\|zmin\|, \|zmax\|), or **h** for *R* =
    max(\|zmin\|, \|zmax\|).

.. _-T:

**-T**\ *start/stop/inc* or **-T**\ *n*
    Set steps in CPT. Calculate entries in CPT from *start*
    to *stop* in steps of (*inc*). Default chooses arbitrary values by
    a crazy scheme based on equidistant values for a Gaussian CDF.
    Use **-T**\ *n* to select *n* points from such a
    cumulative normal distribution [11].

.. _-V:

**-V**
    Verbose operation. This will write CDF(Z) estimates to stderr. [Default is silent.]

.. _-W:

**-W**\ [**w**]
    Do not interpolate the input color table but pick the output colors
    starting at the beginning of the map. This is particularly useful in
    combination with a categorical color table. Alternatively, use **-Ww**
    to produce a wrapped (cyclic) color table that endlessly repeats its range.

.. _-Z:

**-Z**
    Force a continuous CPT [Default is discontinuous].

.. |Add_-h| unicode:: 0x20 .. just an invisible code
.. include:: explain_-h.rst_

.. include:: explain_help.rst_

.. include:: explain_grd_inout_short.rst_

.. include:: explain_transparency.rst_

Color Hinges
------------

Some of the GMT master dynamic CPTs are actually two separate CPTs
meeting at a *hinge*.  Usually, colors may change dramatically across
the hinge, which is used to separate two different domains (e.g., land
and ocean across the shoreline, for instance).  CPTs with a hinge will
have their two parts stretched to the required range separately, i.e.,
the bottom part up to the hinge will be stretched independently of the
part from the hinge to the top, according to the prescribed new range.
Hinges are either *hard* or *soft*.  Soft hinges must be *activated* by
appending **+h**\ [*hinge*] to the CPT name.
If the selected range does not include an activated soft or hard hinge then
we only resample colors from the half of the CPT that pertains to the range.
See :ref:`Of Colors and Color Legends` for more information.

Discrete versus Continuous CPT
------------------------------

All CPTs can be stretched, but only continuous CPTs can be sampled
at new nodes (i.e., by given an increment in **-T**).  We impose this
limitation to avoid aliasing the original CPT.

Examples
--------

.. include:: explain_example.rst_

To get a reasonable and symmetrical color table for the data in the region 0/60/0/60
from the remote 5m relief file, using the geo color table, try::

    gmt grd2cpt @earth_relief_05m -R0/60/0/60 -Cgeo -Su

Sometimes you don't want to make a CPT (yet) but would find it
helpful to know that 90% of your data lie between z1 and z2, something
you cannot learn from :doc:`grdinfo`. So you can do this to see some points
on the CDF(Z) curve (use **-V** option to see more):

   ::

    gmt grd2cpt mydata.nc -V > /dev/null

To make a CPT with entries from 0 to 200 in steps of 20, and ignore
data below zero in computing CDF(Z), and use the built-in master cpt
file relief, run

   ::

    gmt grd2cpt mydata.nc -Crelief -L0/10000 -T0/200/20 > mydata.cpt

.. include:: cpt_notes.rst_

See Also
--------

:doc:`gmt`, :doc:`gmt.conf`,
:doc:`grdhisteq`, :doc:`grdinfo`, :doc:`makecpt`

References
----------

Crameri, F., (2018). Scientific colour-maps. Zenodo. http://doi.org/10.5281/zenodo.1243862

Crameri, F. (2018), Geodynamic diagnostics, scientific visualisation and StagLab 3.0,
*Geosci. Model Dev.*, 11, 2541-2562, doi:10.5194/gmd-11-2541-2018.
