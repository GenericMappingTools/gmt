.. index:: ! gmtinfo
.. include:: module_core_purpose.rst_

****
info
****

|gmtinfo_purpose|

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt info** [ *table* ]
[ |-A|\ **a**\|\ **t**\|\ **s** ]
[ |-C| ]
[ |-D|\ [*dx*\ [/*dy*]] ]
[ |-E|\ **L**\|\ **l**\|\ **H**\|\ **h**\ [*col*] ]
[ |-F|\ [**i**\|\ **d**\|\ **t**] ]
[ |-I|\ [**b**\|\ **e**\|\ **f**\|\ **p**\|\ **s**]\ *dx*\ [/*dy*\ [/*dz*...][**+e**\|\ **r**\|\ **R**\ *incs*] ]
[ |-L| ]
[ |-T|\ *dz*\ [**w**\|\ **d**\|\ **h**\|\ **m**\|\ **s**][**+c**\ *col*] ]
[ |SYN_OPT-V| ]
[ |SYN_OPT-a| ]
[ |SYN_OPT-bi| ]
[ |SYN_OPT-di| ]
[ |SYN_OPT-e| ]
[ |SYN_OPT-f| ]
[ |SYN_OPT-g| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-o| ]
[ |SYN_OPT-qi| ]
[ |SYN_OPT-r| ]
[ |SYN_OPT-s| ]
[ |SYN_OPT-w| ]
[ |SYN_OPT-:| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**info** reads its standard input [or from files] and finds the
extreme values in each of the columns reported as slash-separated min/max
pairs. It recognizes NaNs and will print warnings if the number of columns
vary from record to record. The pairs can be split into two separate columns
by using the |-C| option.  As another option, **info** can find the extent
of data in the first two columns rounded up and down to the nearest multiple of the
supplied increments given by |-I|. Such output will be in the text form
|-R|\ *w/e/s/n*, which can be used directly on the command line for other
modules (hence only *dx* and *dy* are needed).  If |-C| is combined with
|-I| then the output will be in column form and rounded up/down for as many
columns as there are increments provided in |-I|. A similar option (|-T|)
will provide a |-T|\ *zmin/zmax/dz* string for :doc:`makecpt`.

Required Arguments
------------------

.. |Add_intables| unicode:: 0x20 .. just an invisible code
.. include:: explain_intables.rst_

Optional Arguments
------------------

.. _-A:

**-A**\ **a**\|\ **t**\|\ **s**
    Specify how the range should be reported. Choose |-A|\ **a** for the
    range of all tables combined, |-A|\ **t** to report the range for each
    table separately, and |-A|\ **s** to report the range for each segment
    (in multisegment tables) separately. [Default is |-A|\ **a**].

.. _-C:

**-C**
    Report the min/max values per column in separate columns [Default
    uses <min/max> format]. When used, users may also use **-o** to
    limit which output columns should be reported [all].

.. _-D:

**-D**\ [*dx*\ [/*dy*]]
    Modifies results obtained by |-I| by shifting the region to better
    align with the center of the data.  Optionally, append granularity
    for this shift [Default performs an exact shift].

.. _-E:

**-EL**\|\ **l**\|\ **H**\|\ **h**\ [*col*]
    Returns the record whose column *col* contains the minimum
    (**l**) or maximum (**h**) value. Upper case
    (**L\|H**) works on absolute value of the data. In case of multiple
    matches, only the first record is returned. If *col* is not
    specified we default to the last column in the data.

.. _-F:

**-F**\ [**i**\|\ **d**\|\ **t**]
    Returns the counts of various records depending on the appended mode:
    **i** returns a single record with the total number of tables, segments,
    data records, header records, and overall records.  In contrast, **d** returns
    information for each segment in the virtual data set: *tbl_number, seg_number,
    n_rows, start_rec, stop_rec*. Mode **t** does the same but honors the input
    table organization and thus resets *seg_number, start_rec, stop_rec* at the
    start of each new table [Default is **i**].

.. _-I:

**-I**\ [**b**\|\ **e**\|\ **f**\|\ **p**\|\ **s**]\ *dx*\ [/*dy*\ [/*dz*...][**+e**\|\ **r**\|\ **R**\ *incs*]
    Compute the *min*\ /*max* values of the first *n* columns to the nearest multiple
    of the provided increments (separate the *n* increments by slashes) [default is 2 columns].
    By default, output results in the string |-R|\ *w/e/s/n* or |-R|\ *xmin/xmax/ymin/ymax*,
    unless |-C| is set in which case we output each *min* and *max* value in separate
    output columns. If only one increment is given we also use it for the second
    column. Several directives are available:

    - **b** - Write the bounding box of the data table or segments (see |-A|)
      as a closed polygon segment. 
    - **e** - The exact *min*\ /*max* of the input is given in the **-R** string.
      If you only want either the *x-* or *y-*\ range to be exact and the other
      range rounded, give one of the increments as zero.
    - **f** - Append *dx*\ [/*dy*] to report an extended region optimized
      to give grid dimensions for fastest results in programs using FFTs.
    - **p** - Append *dx*. This directive overrides use of a single *dx* for two columns.
    - **s** - Append *dx*\ [/*dy*] to report an extended region optimized to
      give grid dimensions for fastest results in programs like :doc:`surface`.

    A few modifiers can adjust the determined region further:

    - **+e** - Similar to **+r**, but ensures that the bounding box extends by at
      least 0.25 times the increment(s) [no extension].
    - **+r** - Modify the *min*\ /*max* of the first *n* columns further:
      Append *inc*, *xinc*/*yinc*, or *winc*/*einc*/*sinc*/*ninc* to adjust the
      region to be a multiple of these steps [no adjustment].
    - **+R** - Extend the region outward by adding and subtracting these increments instead.

    **Note**: If the input *x*- and *y*-coordinates all have the
    same phase shift relative to the *dx* and *dy* increments then we
    use those phase shifts in determining the region, and you may use
    |SYN_OPT-r| to switch from gridline-registration to pixel-registration.
    For irregular data both phase shifts are set to 0 and the |SYN_OPT-r| is ignored.

.. _-L:

**-L**
    Determines common limits across tables (|-A|\ **t**) or segments (|-A|\ **s**).
    If used with |-I| it will round inwards so that the resulting bounds
    lie within the actual data domain.

.. _-T:

**-T**\ *dz*\ [**w**\|\ **d**\|\ **h**\|\ **m**\|\ **s**][**+c**\ *col*]
    Report the min/max of the first (0'th) column to the nearest multiple of *dz* and output this as the
    string |-T|\ *zmin/zmax/dz*. To use another column, append **+c**\ *col*. Cannot be used together with |-I|.
    **Note**: If your column has absolute time then you may append a valid fixed time unit to *dz*
    (i.e., choose from **w**\ eek, **d**\ ay, **h**\ our, **m**\ inute, or **s**\ econd), or rely
    on the current setting of :term:`TIME_UNIT` [**s**].

.. |Add_-V| replace:: |Add_-V_links|
.. include:: explain_-V.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. include:: explain_-aspatial.rst_

.. |Add_-bi| replace:: [Default is 2 input columns].
.. include:: explain_-bi.rst_

.. |Add_-di| unicode:: 0x20 .. just an invisible code
.. include:: explain_-di.rst_

.. |Add_-e| unicode:: 0x20 .. just an invisible code
.. include:: explain_-e.rst_

.. |Add_-f| unicode:: 0x20 .. just an invisible code
.. include:: explain_-f.rst_

.. |Add_-g| unicode:: 0x20 .. just an invisible code
.. include:: explain_-g.rst_

.. |Add_-h| unicode:: 0x20 .. just an invisible code
.. include:: explain_-h.rst_

.. include:: explain_-icols.rst_

.. include:: explain_-ocols.rst_

.. include:: explain_-qi.rst_

.. |Add_nodereg| unicode:: 0x20 .. just an invisible code
.. include:: explain_nodereg.rst_

.. include:: explain_-s.rst_

.. include:: explain_-w.rst_

.. include:: explain_colon.rst_

.. include:: explain_help.rst_

.. include:: explain_precision.rst_

Examples
--------

.. include:: explain_example.rst_

To find the extreme values in the remote file @ship_15.txt::

    gmt info @ship_15.txt

Output should look like::

    ship_15.txt: N = 82651	<245/254.705>	<20/29.99131>	<-4504/-9>

To find the extreme values in @ship_15.txt to the nearest 5 units
but shifted to within 1 unit of the data center, and use this region to
plot all the points as small black circles using :doc:`plot`, run

::

  gmt plot $(gmt info -I5 -D1 @ship_15.txt$) @ship_15.txt -B -Sc2p -pdf map

To find the min and max values for each of the first 3 columns, but
rounded to integers, and return the result individually for each data
file, use

::

  gmt info @ship_15.txt -C -I1/1/1

Given seven profiles with different start and stop positions, we
want to find a range of positions, with increment of 5, that are
common to all the profiles.  We use

::

  gmt info profile_[123567].txt -L -I5

The file magprofs.txt contains a number of magnetic profiles stored
as separate data segments.  We need to know how many segments there
are and use

::

  gmt info magprofs.txt -Fi

Bugs
----

The |-I| option does not yet work properly with time series data
(e.g., **-f**\ 0T). Thus, such variable intervals as months and years
are not calculated. Instead, specify your interval in the same units as
the current setting of :term:`TIME_UNIT`.

See Also
--------

:doc:`gmt`,
:doc:`gmtconvert`,
:doc:`plot`
