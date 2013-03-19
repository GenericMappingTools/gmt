******
minmax
******

minmax - Find extreme values in data tables

`Synopsis <#toc1>`_
-------------------

.. include:: common_SYN_OPTs.rst_

**minmax** [ *table*] [ **-A**\ **a**\ \|\ **f**\ \|\ **s** ] [ **-C** ]
[ **-EL**\ \|\ **l**\ \|\ **H**\ \|\ **h**\ *col* ] [
**-I**\ [**p**\ ]\ *dx*\ [/*dy*\ [/*dz*...] ] [
**-S**\ [**x**\ ][**y**\ ] ] [ **-T**\ *dz*\ [/*col*] ] [
**-V**\ [*level*\ ] ] [ **-bi**\ [*ncols*\ ][*type*\ ] ] [
**-f**\ [**i**\ \|\ **o**]\ *colinfo* ] [
**-g**\ [**a**\ ]\ **x**\ \|\ **y**\ \|\ **d**\ \|\ **X**\ \|\ **Y**\ \|\ **D**\ \|[*col*\ ]\ **z**\ [+\|-]\ *gap*\ [**u**\ ]
] [ **-h**\ [**i**\ \|\ **o**][*n*\ ] ] [
**-i**\ *cols*\ [**l**\ ][\ **s**\ *scale*][\ **o**\ *offset*][,\ *...*]
] [ **-o**\ *cols*\ [,*...*] ] [ **-r** ] [ **-:**\ [**i**\ \|\ **o**] ]

`Description <#toc2>`_
----------------------

**minmax** reads its standard input [or from files] and finds the
extreme values in each of the columns. It recognizes NaNs and will print
warnings if the number of columns vary from record to record. As an
option, **minmax** will find the extent of the first *n* columns rounded
up and down to the nearest multiple of the supplied increments. By
default, this output will be in the form **-R**\ *w/e/s/n* which can be
used directly in the command line for other programs (hence only *dx*
and *dy* are needed), or the output will be in column form for as many
columns as there are increments provided. A similar option (**-T**) will
provide a **-T**\ *zmin/zmax/dz* string for makecpt. 

.. include:: explain_commonitems.rst_

`Required Arguments <#toc4>`_
-----------------------------

None.

`Optional Arguments <#toc5>`_
-----------------------------

.. |Add_intables| unicode:: 0x20 .. just an invisible code
.. include:: explain_intables.rst_

**-A**\ **a**\ \|\ **f**\ \|\ **s**
    Specify how the range should be reported. Choose **-Aa** for the
    range of all files combined, **-Af** to report the range for each
    file separately, and **-As** to report the range for each segment
    (in multisegment files) separately. [Default is **-Aa**].
**-C**
    Report the min/max values per column in separate columns [Default
    uses <min/max> format]. When used, users may also use **-o** to
    limit which output columns should be reported [all].
**-EL**\ \|\ **l**\ \|\ **H**\ \|\ **h**\ *col*
    Returns the record whose column *col* contains the `minimum
    (**l**) <minimum.l.html>`_ or maximum (**h**) value. Upper case
    (**L\|H**) works on absolute value of the data. In case of multiple
    matches, only the first record is returned. If *col* is not
    specified we default to the last column in the data.
**-I**\ [**p**\ ]\ *dx*\ [/*dy*\ [/*dz*...]
    Report the min/max of the first *n* columns to the nearest multiple
    of the provided increments (separate the *n* increments by slashes),
    and output results in the form **-R**\ *w/e/s/n* (unless **-C** is
    set). If only one increment is given we also use it for the second
    column (for backwards compatibility). To override this behavior, use
    **-Ip**\ *dx*. If the input *x*- and *y*-coordinates all have the
    same phase shift relative to the *dx* and *dy* increments then we
    use those phase shifts in determining the region, and you may use
    **-r** to switch from gridline-registration to pixel-registration.
    For irregular data both phase shifts are set to 0 and the **-r** is ignored.
**-S**\ [**x**\ ][**y**\ ]
    Add extra space for error bars. Useful together with **-I** option
    and when later plotting with **psxy** **-E**. **-Sx** leaves space
    for horizontal error bars using the values in `third
    (2) <third.2.html>`_ column. **-Sy** leaves space for vertical error
    bars using the values in `third (2) <third.2.html>`_ column. **-S**
    or **-Sxy** leaves space for both error bars using the values in
    third and fourth (2 and 3) columns.
**-T**\ *dz*\ [/*col*]
    Report the min/max of the first (0’th) column to the nearest
    multiple of *dz* and output this in the form **-T**\ *zmin/zmax/dz*.
    To use another column, append /*col*. Only works when **-I** is
    selected. 

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. |Add_-bi| replace:: [Default is 2 input columns]. 
.. include:: explain_-bi.rst_

.. |Add_-f| unicode:: 0x20 .. just an invisible code
.. include:: explain_-f.rst_

.. |Add_-g| unicode:: 0x20 .. just an invisible code
.. include:: explain_-g.rst_

.. |Add_-h| unicode:: 0x20 .. just an invisible code
.. include:: explain_-h.rst_

.. include:: explain_-icols.rst_

.. |Add_nodereg| unicode:: 0x20 .. just an invisible code
.. include:: explain_nodereg.rst_

.. include:: explain_colon.rst_

.. include:: explain_help.rst_

.. include:: explain_precision.rst_

`Examples <#toc7>`_
-------------------

To find the extreme values in the file ship\_gravity.xygd:

    minmax ship\_gravity.xygd

Output should look like

    ship\_gravity.xygd: N = 6992 <326.125/334.684> <-28.0711/-8.6837>
    <-47.7/177.6> <0.6/3544.9>

To find the extreme values in the file track.xy to the nearest 5 units
and use this region to draw a line using psxy, run

    psxy `minmax -I5 track.xy` track.xy -Jx1 -B5 -P > track.ps

To find the min and max values for each of the first 4 columns, but
rounded to integers, and return the result individually for each data
file, use

    minmax profile\_\*.txt -C -I1/1/1/1

`Bugs <#toc8>`_
---------------

The **-I** option does not yet work properly with time series data
(e.g., **-f**\ 0T). Thus, such variable intervals as months and years
are not calculated. Instead, specify your interval in the same units as
the current setting of **TIME\_UNIT**.

`See Also <#toc9>`_
-------------------

`gmt <gmt.html>`_
