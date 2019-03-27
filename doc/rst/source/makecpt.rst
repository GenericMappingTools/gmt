.. index:: ! makecpt

*******
makecpt
*******

.. only:: not man

    makecpt - Make GMT color palette tables

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt makecpt** [ |-A|\ *transparency*\ [**+a**\ ] ]
[ |-C|\ *cpt* ]
[ |-D|\ [**i**\ \|\ **o**] ]
[ |-E|\ [*nlevels*] ]
[ |-F|\ [**R**\ \|\ **r**\ \|\ **h**\ \|\ **c**\ ][**+c**\ ]]
[ |-G|\ *zlo*\ /\ *zhi* ]
[ |-I|\ [**c**][**z**] ] [ |-M| ]
[ |-N| ] [ |-Q| ]
[ |-S|\ *mode* ]
[ |-T|\ [*min*/*max*/*inc*\ [**+n**\ ]\|\ *file*\ \|\ *list*\ ] ]
[ |-V|\ [*level*\ ] ]
[ |-W|\ [**w**] ]
[ |-Z| ]
[ |SYN_OPT-bi| ]
[ |SYN_OPT-di| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**makecpt** is a module that will help you make static color palette tables
(CPTs). You define an equidistant set of contour intervals or pass
your own z-table or list, and create a new CPT based on an existing master (dynamic)
CPT. The resulting CPT can be reversed relative to the master
cpt, and can be made continuous or discrete.  For color tables beyond the
standard GMT offerings, visit `cpt-city <http://soliton.vm.bytemark.co.uk/pub/cpt-city/>`_ and
`Scientific Colour-Maps <http://www.fabiocrameri.ch/colourmaps.php>`_.

The CPT includes three additional colors beyond the range of
z-values. These are the background color (B) assigned to values lower
than the lowest *z*-value, the foreground color (F) assigned to values
higher than the highest *z*-value, and the NaN color (N) painted
wherever values are undefined.

If the master CPT includes B, F, and N entries, these will be
copied into the new master file. If not, the parameters
:ref:`COLOR_BACKGROUND <COLOR_BACKGROUND>`, :ref:`COLOR_FOREGROUND <COLOR_FOREGROUND>`,
and :ref:`COLOR_NAN <COLOR_NAN>` from
the :doc:`gmt.conf` file or the command line will be used. This default
behavior can be overruled using the options **-D**, **-M** or **-N**.

The color model (RGB, HSV or CMYK) of the palette created by **makecpt**
will be the same as specified in the header of the master CPT. When
there is no :ref:`COLOR_MODEL <COLOR_MODEL>` entry in the master CPT, the
:ref:`COLOR_MODEL <COLOR_MODEL>` specified in the :doc:`gmt.conf` file or on the command
line will be used.

Required Arguments
------------------

None.

Optional Arguments
------------------

.. _-A:

**-A**\ *transparency*\ [**+a**\ ]
    Sets a constant level of transparency (0-100) for all color slices.
    Append **+a** to also affect the fore-, back-, and nan-colors
    [Default is no transparency, i.e., 0 (opaque)].

.. _-C:

**-C**\ *cpt*
    Selects the master color table CPT to use in the interpolation.
    Choose among the built-in tables (type **makecpt** to see the list)
    or give the name of an existing CPT [Default gives a rainbow CPT].
    Yet another option is to specify -Ccolor1,color2[,color3,...]
    to build a linear continuous cpt from those colors automatically.
    In this case *color*\ **n** can be a r/g/b triplet, a color name,
    or an HTML hexadecimal color (e.g. #aabbcc ).

.. _-D:

**-D**\ [**i**\ \|\ **o**]
    Select the back- and foreground colors to match the colors for
    lowest and highest *z*-values in the output CPT [Default uses
    the colors specified in the master file, or those defined by the
    parameters :ref:`COLOR_BACKGROUND <COLOR_BACKGROUND>`, :ref:`COLOR_FOREGROUND <COLOR_FOREGROUND>`, and
    :ref:`COLOR_NAN <COLOR_NAN>`]. Append **i** to match the colors for the lowest and
    highest values in the input (instead of the output) CPT.

.. _-E:

**-E**\ [*nlevels*]
    Implies reading data table(s) from given command-line files or standard input.
    We use the last data column to determine the data range; use **-i** to
    select another column, and use **-bi** if your data table is native binary.
    This z-range information is used instead of providing the **-T** option.
    We create a linear color table by dividing the table data z-range into
    *nlevels* equidistant slices.  If *nlevels* is not given it defaults to
    the number of levels in the chosen CPT.

.. _-F:

**-F**\ [**R**\ \|\ **r**\ \|\ **h**\ \|\ **c**][**+c**\ ]]
    Force output CPT to be written with r/g/b codes, gray-scale values
    or color name (**R**, default) or r/g/b codes only (**r**), or h-s-v
    codes (**h**), or c/m/y/k codes (**c**).  Optionally or alternatively,
    append **+c** to write discrete palettes in categorical format.

.. _-G:

**-G**\ *zlo*\ /\ *zhi*
    Truncate the incoming CPT so that the lowest and highest z-levels
    are to *zlo* and *zhi*.  If one of these equal NaN then
    we leave that end of the CPT alone.  The truncation takes place
    before any resampling. See also :ref:`manipulating_CPTs`

.. _-I:

**-I**\ [**c**][**z**]
    Append **c** [Default] to reverse the sense of color progression in the master CPT. Also
    exchanges the foreground and background colors, including those
    specified by the parameters :ref:`COLOR_BACKGROUND <COLOR_BACKGROUND>` and
    :ref:`COLOR_FOREGROUND <COLOR_FOREGROUND>`.
    Append **z** to reverse the sign of z-values in the color table.  Note that
    this change of *z*-direction happens before **-G** and **-T** values are used
    so the latter much be compatible with the changed *z*-range.
    See also :ref:`manipulating_CPTs`

.. _-M:

**-M**
    Overrule background, foreground, and NaN colors specified in the
    master CPT with the values of the parameters
    :ref:`COLOR_BACKGROUND <COLOR_BACKGROUND>`, :ref:`COLOR_FOREGROUND <COLOR_FOREGROUND>`,
    and :ref:`COLOR_NAN <COLOR_NAN>`
    specified in the :doc:`gmt.conf` file or on the command line. When
    combined with **-D**, only :ref:`COLOR_NAN <COLOR_NAN>` is considered.

.. _-N:

**-N**
    Do not write out the background, foreground, and NaN-color fields [Default will write them].

.. _-Q:

**-Q**
    For logarithmic interpolation scheme with input given as logarithms.
    Expects input z-values provided via **-T** to be log10(*z*\ ), assigns colors, and
    writes out *z*.

.. _-S:

**-S**\ *mode*
    Determine a suitable range for the **-T** option from the input table(s) (or stdin).
    Choose from several types of range determinations: 
    **-Sr** will use the data range min/max, **-S**\ *inc*\ [**+d**\ ] will use the data min/max but rounded
    to nearest *inc* (append **+d** to resample to a discrete CPT), **-Sa**\ *scl* will
    make a symmetric range around the average (i.e., mean)
    and ±\ *scl* * *sigma*, **-Sm**\ *scl* will make a symmetric range around the median
    and ±\ *scl* * *L1_scale*, **-Sp**\ *scl* will make symmetric range around mode and
    ±\ *scl* * *LMS_scale*, while **-Sq**\ *low/high* sets the range from *low* quartile
    to *high* quartile (in percentages).  We use the last data column for this calculation;
    use **i** if you need to adjust the column orders.

.. _-T:

**-T**\ [*min*/*max*/*inc*\ [**+b**\ \|\ **l**\ \|\ **n**\ ]\|\ *file*\ \|\ *list*\ ]
    Defines the range of the new CPT by giving the lowest and
    highest z-value (and optionally an interval).  If **-T** is
    not given, the existing range in the master CPT will be used intact.
    The values produces defines the color slice boundaries.  If **+n** is
    used it refers to the number of such boundaries and not the number of slices.
    For details on array creation, see `Generate 1D Array`_.

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. _-W:

**-W**\ [**w**]
    Do not interpolate the input color table but pick the output colors
    starting at the beginning of the color table, until colors for all
    intervals are assigned. This is particularly useful in combination
    with a categorical color table, like "categorical". Cannot be used
    in combination with **-Z**.  Alternatively, use **-Ww** to produce
    a wrapped (cyclic) color table that endlessly repeats its range.

.. _-Z:

**-Z**
    Creates a continuous CPT [Default is discontinuous, i.e.,
    constant colors for each interval]. This option has no effect when no **-T**
    is used, or when using **-T**\ *z_min*/*z_max*; in the first case the input
    CPT remains untouched, in the second case it is only scaled to match the
    range *z_min*/*z_max*.

.. |Add_-bi| replace:: [Default is the required number of columns given the chosen settings].
.. include:: explain_-bi.rst_

.. |Add_-di| unicode:: 0x20 .. just an invisible code
.. include:: explain_-di.rst_

.. include:: explain_-icols.rst_

.. include:: explain_help.rst_

.. include:: explain_transparency.rst_

.. include:: explain_array.rst_

Color Hinges
------------

Some of the GMT master dynamic CPTs are actually two separate CPTs
meeting at a *hinge*.  Usually, colors may change dramatically across
the hinge, which is used to separate two different domains (e.g., land
and ocean across the shoreline, for instance).  CPTs with a hinge will
have their two parts stretched to the required range separately, i.e.,
the bottom part up to the hinge will be stretched independently of the
part from the hinge to the top, according to the prescribed new range.
If the selected range does not include the hinge then no such partitioning
takes place.

Color Aliasing
--------------

For best result when **-T -Z** is used we recommend you do no append
a specific *z_inc*.  This way the original CPT is used exactly
as is but the *z* boundaries are adjusted to match the stated limits.
Otherwise you may, depending on the nature of the input CPT, miss
aspects of the color changes by aliasing the signal.

Examples
--------

To make a CPT with z-values from -200 to 200, with discrete color
changes every 25, and using a polar blue-white-red colortable:

   ::

    gmt makecpt -Cpolar -T-200/200/25 > colors.cpt

To make an equidistant CPT from z = -2 to 6 using the
continuous default rainbow of colors:

   ::

    gmt makecpt -T-2/6 -Z > rainbow.cpt

To use the GEBCO look-alike CPT with its default range for bathymetry, run

   ::

    gmt makecpt -Cgebco > my_gebco.cpt

or simply use -Cgebco directly in the application that needs the color table.
To create a 24-level rainbow color table suitable for plotting the depths in
the data table depths.txt (with lon, lat, depths), run

   ::

    gmt makecpt -Cgebco depths.txt -i2 -Z -E24 > my_depths.cpt

To use the gebco color table but reverse the z-values so it can be used for
positive depth values, try

   ::

    gmt makecpt -Cgebco -Iz > my_positive_gebco.cpt

To create a 24-level rainbow color table suitable for plotting the depths in
the data table depths.txt (with lon, lat, depths), run

To make a custom discrete color table for depth of seismicity, using red color for
hypocenters between 0 and 100 km, green for 100-300 km, and blue for deep (300-1000 km)
earthquakes, use

   ::
   
    gmt makecpt -Cred,green,blue -T0,80,300,1000 -N > seis.cpt

To make a continuous CPT from white to blue as z goes from
3 to 10, try

   ::
   
    gmt makecpt -Cwhite,blue -T3,10 -Z > cold.cpt

To make a wrapped (cyclic) CPT from the jet table over the interval
0 to 500, i.e., the color will be wrapped every 500 z-units so that
we always get a color regardless of the *z* value, try

   ::

    gmt makecpt -Cjet -T0/500 -Ww > wrapped.cpt

Bugs
----

Since **makecpt** will also interpolate from any existing CPT you
may have in your directory, you should not use one of the listed cpt names
as an output filename; hence the my_gebco.cpt in the example.  If you
do create a CPT of such a name, e.g., rainbow.cpt, then **makecpt** will
read that file first and not look for the master CPT in the shared GMT
directory.

See Also
--------

:doc:`gmt`, :doc:`grd2cpt`

References
----------

Crameri, F., (2018). Scientific colour-maps. Zenodo. http://doi.org/10.5281/zenodo.1243862

Crameri, F. (2018), Geodynamic diagnostics, scientific visualisation and StagLab 3.0,
*Geosci. Model Dev.*, 11, 2541-2562, doi:10.5194/gmd-11-2541-2018.
