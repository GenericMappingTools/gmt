*******
makecpt
*******


makecpt - Make GMT color palette tables

`Synopsis <#toc1>`_
-------------------

**makecpt** [ **-A**\ [**+**\ ]\ *transparency* ] [ **-C**\ *table* ] [
**-D**\ [**i**\ \|\ **o**] ] [
**-F**\ [**R**\ \|\ **r**\ \|\ **h**\ \|\ **c** ] [ **-I** ] [ **-M** ]
[ **-N** ] [ **-Q**\ [**i**\ \|\ **o**] ] [ **-T**\ *z0*/*z1*\ [/*dz*]
\| **-T**\ *ztable* ] [ **-V**\ [*level*\ ] ] [ **-W** ] [ **-Z** ]

`Description <#toc2>`_
----------------------

**makecpt** is a utility that will help you make color palette tables
(cpt files). You define an equidistant set of contour intervals or pass
your own z-table, and create a new cpt file based on an existing master
cpt file. The resulting cpt file can be reversed relative to the master
cpt, and can be made continuous or discrete.
The color palette includes three additional colors beyond the range of
z-values. These are the background color (B) assigned to values lower
than the lowest *z*-value, the foreground color (F) assigned to values
higher than the highest *z*-value, and the NaN color (N) painted
whereever values are undefined.
If the master cpt file includes B, F, and N entries, these will be
copied into the new master file. If not, the parameters
**COLOR\_BACKGROUND**, **COLOR\_FOREGROUND**, and **COLOR\_NAN** from
the **gmt.conf** file or the command line will be used. This default
behavior can be overruled using the options **-D**, **-M** or **-N**.
The color model (RGB, HSV or CMYK) of the palette created by
**makecpt** will be the same as specified in the header of the master
cpt file. When there is no **COLOR\_MODEL** entry in the master cpt
file, the **COLOR\_MODEL** specified in the **gmt.conf** file or on the
command line will be used.

`Common Arguments And Specifications <#toc3>`_
----------------------------------------------

All options marked with an asterisk (\*) are common GMT command-line
options. Their full syntax as well as how to specify pens, pattern
fills, colors, and fonts can be found in the **gmt** man page. Note: No
space is allowed between the option flag and the associated arguments.

`Required Arguments <#toc4>`_
-----------------------------

None.

`Optional Arguments <#toc5>`_
-----------------------------

**-A**\ [**+**\ ]\ *transparency*
    Sets a constant level of transparency (0-100) for all color slices.
    Prepend **+** to also affect the fore-, back-, and nan-colors
    [Default is no transparency, i.e. 0 (opaque)].
**-C**\ *table*
    Selects the master color table *table* to use in the interpolation.
    Choose among the built-in tables (type **makecpt** to see the list)
    or give the name of an existing cpt file [Default gives a rainbow
    cpt file].
**-D**\ [**i**\ \|\ **o**]
    Select the back- and foreground colors to match the colors for
    lowest and highest *z*-values in the output cpt file [Default uses
    the colors specified in the master file, or those defined by the
    parameters **COLOR\_BACKGROUND**, **COLOR\_FOREGROUND**, and
    **COLOR\_NAN**]. Append **i** to match the colors for the lowest and
    highest values in the input (instead of the output) cpt file.
**-F**\ [**R**\ \|\ **r**\ \|\ **h**\ \|\ **c**]
    Force output cpt file to written with r/g/b codes, gray-scale values
    or color name (**R**, default) or r/g/b codes only (**r**), or h-s-v
    codes (**h**), or c/m/y/k codes (**c**).
**-I**
    Reverses the sense of color progression in the master cpt file. Also
    exchanges the foreground and background colors, including those
    specified by the parameters **COLOR\_BACKGROUND** and
    **COLOR\_FOREGROUND**.
**-M**
    Overrule background, foreground, and NaN colors specified in the
    master cpt file with the values of the parameters
    **COLOR\_BACKGROUND**, **COLOR\_FOREGROUND**, and **COLOR\_NAN**
    specified in the **gmt.conf** file or on the command line. When
    combined with **-D**, only **COLOR\_NAN** is considered.
**-N**
    Do not write out the background, foreground, and NaN-color fields
    [Default will write them].
**-Q**\ [**i**\ \|\ **o**]
    Selects a logarithmic interpolation scheme [Default is linear].
    **-Qi** expects input z-values to be log10(z), assigns colors, and
    writes out z [Default]. **-Qo** takes log10(z) first, assigns
    colors, and writes out z.
**-T**\ *z0*/*z1*\ [/*dz*] \| **-T**\ *ztable*
    Defines the range of the new cpt file by giving the lowest and
    highest z-value and interval. When used with **-C** and the interval
    is not specified, the number of intervals remains the same as in the
    input palette. Alternatively, give the name of a ASCII file that has
    one z-value per record. If **-T** is not given, the existing range
    in the master cpt file will be used intact.
**-V**\ [*level*\ ] (\*)
    Select verbosity level [1].
**-W**
    Do not interpolate the input color table but pick the output colors
    starting at the beginning of the map. This is particularly useful in
    combination with a categorical color table. Cannot be used in
    combination with **-Z**.
**-Z**
    Creates a continuous cpt file [Default is discontinuous, i.e.
    constant colors for each interval].
**-^** (\*)
    Print a short message about the syntax of the command, then exits.
**-?** (\*)
    Print a full usage (help) message, including the explanation of
    options, then exits.

`Examples <#toc6>`_
-------------------

To make a cpt file with z-values from -200 to 200, with discrete color
changes every 25, and using a polar blue-white-red colortable:

makecpt -Cpolar -T-200/200/25 > colors.cpt

To make an equidistant cpt file from z = -2 to 6, in steps of 1, using
continuous default rainbow colors:

makecpt -T-2/6/1 -Z > rainbow.cpt

To make a GEBCO look-alike cpt file for bathymetry, run

makecpt -Cgebco > my\_gebco.cpt

`Bugs <#toc7>`_
---------------

Since **makecpt** will also interpolate from any existing .cpt file you
may have in your directory, you cannot use one of the listed cpt names
as an output filename; hence the my\_gebco.cpt in the example.

`See Also <#toc8>`_
-------------------

`*gmt*\ (1) <gmt.1.html>`_ , `*grd2cpt*\ (1) <grd2cpt.1.html>`_

