.. index:: ! makecpt

*******
makecpt
*******

.. only:: not man

    makecpt - Make GMT color palette tables

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**makecpt** [ **-A**\ [**+**\ ]\ *transparency* ] [ **-C**\ *table* ]
[ **-D**\ [**i**\ \|\ **o**] ]
[ **-F**\ [**R**\ \|\ **r**\ \|\ **h**\ \|\ **c** ] [ **-G**\ *zlo*\ /\ *zhi* ] [ **-I** ] [ **-M** ]
[ **-N** ] [ **-Q**\ [**i**\ \|\ **o**] ]
[ **-T**\ *z_min*/*z_max*\ [/*z_inc*\ [+]] \| **-T**\ *ztable* ]
[ **-V**\ [*level*\ ] ] [ **-W** ] [ **-Z** ]

|No-spaces|

Description
-----------

**makecpt** is a utility that will help you make color palette tables
(cpt files). You define an equidistant set of contour intervals or pass
your own z-table, and create a new cpt file based on an existing master
cpt file. The resulting cpt file can be reversed relative to the master
cpt, and can be made continuous or discrete.  For color tables beyond the
standard GMT offerings, visit cpt-city:
http://soliton.vm.bytemark.co.uk/pub/cpt-city/.

The color palette includes three additional colors beyond the range of
z-values. These are the background color (B) assigned to values lower
than the lowest *z*-value, the foreground color (F) assigned to values
higher than the highest *z*-value, and the NaN color (N) painted
whereever values are undefined.

If the master cpt file includes B, F, and N entries, these will be
copied into the new master file. If not, the parameters
:ref:`COLOR_BACKGROUND <COLOR_BACKGROUND>`, :ref:`COLOR_FOREGROUND <COLOR_FOREGROUND>`,
and :ref:`COLOR_NAN <COLOR_NAN>` from
the :doc:`gmt.conf` file or the command line will be used. This default
behavior can be overruled using the options **-D**, **-M** or **-N**.

The color model (RGB, HSV or CMYK) of the palette created by **makecpt**
will be the same as specified in the header of the master cpt file. When
there is no :ref:`COLOR_MODEL <COLOR_MODEL>` entry in the master cpt file, the
:ref:`COLOR_MODEL <COLOR_MODEL>` specified in the :doc:`gmt.conf` file or on the command
line will be used. 

Required Arguments
------------------

None.

Optional Arguments
------------------

**-A**\ [**+**\ ]\ *transparency*
    Sets a constant level of transparency (0-100) for all color slices.
    Prepend **+** to also affect the fore-, back-, and nan-colors
    [Default is no transparency, i.e., 0 (opaque)].
**-C**\ *table*
    Selects the master color table *table* to use in the interpolation.
    Choose among the built-in tables (type **makecpt** to see the list)
    or give the name of an existing cpt file [Default gives a rainbow
    cpt file].
**-D**\ [**i**\ \|\ **o**]
    Select the back- and foreground colors to match the colors for
    lowest and highest *z*-values in the output cpt file [Default uses
    the colors specified in the master file, or those defined by the
    parameters :ref:`COLOR_BACKGROUND <COLOR_BACKGROUND>`, :ref:`COLOR_FOREGROUND <COLOR_FOREGROUND>`, and
    :ref:`COLOR_NAN <COLOR_NAN>`]. Append **i** to match the colors for the lowest and
    highest values in the input (instead of the output) cpt file.
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
**-T**\ *z_min*/*z_max*\ [/*z_inc*\ [+]] \| **-T**\ *ztable*
    Defines the range of the new cpt file by giving the lowest and
    highest z-value and interval. When used with **-C** and *z_inc* is
    not specified, the number of intervals remains the same as in the
    input palette. If *z_inc* is specified with a trailing **+** we
    interpret *z_inc* as the number of desired intervals instead.
    Alternatively, give the name of a ASCII file that has one z-value
    per record. If **-T** is not given, the existing range in the master
    cpt file will be used intact. 

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

**-W**
    Do not interpolate the input color table but pick the output colors
    starting at the beginning of the color table, until colors for all
    intervals are assigned. This is particularly useful in combination
    with a categorical color table, like "categorical". Cannot be used
    in combination with **-Z**.
**-Z**
    Creates a continuous cpt file [Default is discontinuous, i.e.,
    constant colors for each interval]. 

.. include:: explain_help.rst_

Examples
--------

To make a cpt file with z-values from -200 to 200, with discrete color
changes every 25, and using a polar blue-white-red colortable:

   ::

    gmt makecpt -Cpolar -T-200/200/25 > colors.cpt

To make an equidistant cpt file from z = -2 to 6, in steps of 1, using
continuous default rainbow colors:

   ::

    gmt makecpt -T-2/6/1 -Z > rainbow.cpt

To make a GEBCO look-alike cpt file for bathymetry, run

   ::

    gmt makecpt -Cgebco > my_gebco.cpt

Bugs
----

Since **makecpt** will also interpolate from any existing .cpt file you
may have in your directory, you cannot use one of the listed cpt names
as an output filename; hence the my_gebco.cpt in the example.

See Also
--------

:doc:`gmt`, :doc:`grd2cpt`
