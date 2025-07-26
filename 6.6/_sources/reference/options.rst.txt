Standardized command line options
=================================

Most of the programs take many of the same arguments such as those related
to setting the data region, the map projection, etc. The switches in
Table :ref:`switches <tbl-switches>` have the same meaning in all the programs (although
some programs may not use all of them). These options will be described
here as well as in the manual pages, as is vital that you understand how
to use these options. We will present these options in order of
importance (some are used a lot more than others).

.. _tbl-switches:

+--------------------------------+--------------------------------------------------------------------+
| Standard option                | Description                                                        |
+================================+====================================================================+
| :ref:`-B <option_-B>`          | Define tick marks, annotations, and labels for basemaps and axes   |
+--------------------------------+--------------------------------------------------------------------+
| :ref:`-J <option_-J>`          | Select a map projection or coordinate transformation               |
+--------------------------------+--------------------------------------------------------------------+
| :ref:`-R <option_-R>`          | Define the extent of the map/plot region                           |
+--------------------------------+--------------------------------------------------------------------+
| :ref:`-U <option_-U>`          | Plot a time-stamp, by default in the lower left corner of page     |
+--------------------------------+--------------------------------------------------------------------+
| :ref:`-V <option_-V>`          | Select verbose operation; reporting on progress                    |
+--------------------------------+--------------------------------------------------------------------+
| :ref:`-X <option_-X>`          | Set the *x*-coordinate for the plot origin on the page             |
+--------------------------------+--------------------------------------------------------------------+
| :ref:`-Y <option_-Y>`          | Set the *y*-coordinate for the plot origin on the page             |
+--------------------------------+--------------------------------------------------------------------+
| :ref:`-a <option_-a>`          | Associate aspatial data from OGR/GMT files with data columns       |
+--------------------------------+--------------------------------------------------------------------+
| :ref:`-b <option_-binary>`     | Select binary input and/or output                                  |
+--------------------------------+--------------------------------------------------------------------+
| :ref:`-c <option_-c>`          | Advance plot focus to selected (or next) subplot panel             |
+--------------------------------+--------------------------------------------------------------------+
| :ref:`-d <option_-d>`          | Replace user *nodata* values with IEEE NaNs                        |
+--------------------------------+--------------------------------------------------------------------+
| :ref:`-e <option_-e>`          | Only process data records that match a *pattern*                   |
+--------------------------------+--------------------------------------------------------------------+
| :ref:`-f <option_-f>`          | Specify the data content on a per column basis                     |
+--------------------------------+--------------------------------------------------------------------+
| :ref:`-g <option_-g>`          | Identify data gaps based on supplied criteria                      |
+--------------------------------+--------------------------------------------------------------------+
| :ref:`-h <option_-h>`          | Specify that input/output tables have header record(s)             |
+--------------------------------+--------------------------------------------------------------------+
| :ref:`-i <option_-i>`          | Specify which input columns to read and optionally transform       |
+--------------------------------+--------------------------------------------------------------------+
| :ref:`-j <option_-j_distcalc>` | Specify how spherical distances should be computed                 |
+--------------------------------+--------------------------------------------------------------------+
| :ref:`-l <option_-l>`          | Add a legend entry for the symbol or line being plotted            |
+--------------------------------+--------------------------------------------------------------------+
| :ref:`-n <option_-n>`          | Specify grid interpolation settings                                |
+--------------------------------+--------------------------------------------------------------------+
| :ref:`-o <option_-o>`          | Specify which output columns to optionally transform and write     |
+--------------------------------+--------------------------------------------------------------------+
| :ref:`-p <option_-p>`          | Control perspective views for plots                                |
+--------------------------------+--------------------------------------------------------------------+
| :ref:`-q <option_-q>`          | Specify which input rows to read or output rows to write           |
+--------------------------------+--------------------------------------------------------------------+
| :ref:`-r <option_nodereg>`     | Set grid registration [Default is gridline]                        |
+--------------------------------+--------------------------------------------------------------------+
| :ref:`-s <option_-s>`          | Control output of records containing one or more NaNs              |
+--------------------------------+--------------------------------------------------------------------+
| :ref:`-t <option_-t>`          | Change layer transparency                                          |
+--------------------------------+--------------------------------------------------------------------+
| :ref:`-w <option_-w>`          | Convert selected coordinate to repeating cycles                    |
+--------------------------------+--------------------------------------------------------------------+
| :ref:`-x <option_-x_core>`     | Set number of cores to be used in multi-threaded applications      |
+--------------------------------+--------------------------------------------------------------------+
| :ref:`-: <option_colon>`       | Assume input geographic data are (*lat, lon*) and not (*lon, lat*) |
+--------------------------------+--------------------------------------------------------------------+

.. _option_-R:

Data domain or map region: The **-R** option
--------------------------------------------

.. |Add_-R| unicode:: 0x20 .. just an invisible code
.. include:: ../explain_-R.rst_
    :start-after: ^^^^^^^^^^^^^^^^^

For rectilinear projections the first two forms give identical results. Depending on the selected map projection (or
the kind of expected input data), the boundary coordinates may take on several different formats:

Geographic coordinates:
    These are longitudes and latitudes and may be given in decimal degrees (e.g., -123.45417) or in the
    [±]\ *ddd*\ [:*mm*\ [:*ss*\ [*.xxx*]]][**W**\|\ **E**\|\ **S**\|\ **N**] format (e.g., 123:27:15W). **-Rg** and
    **-Rd** are shorthands for "global domain" **-R**\ *0*/*360*/*-90*/*90* and **-R**\ *-180*/*180*/*-90*/*90*
    respectively.

    When used in conjunction with the Cartesian Linear Transformation (**-Jx** or **-JX**) — which can be used to map
    floating point data, geographical coordinates, as well as time coordinates — it is prudent to indicate that you are
    using geographical coordinates in one of the following ways:

    -  Use **-Rg** or **-Rd** to indicate the global domain.

    -  Use **-Rg**\ *xmin*/*xmax*/*ymin*/*ymax* to indicate a limited geographic domain.

    -  Add **W**, **E**, **S**, or **N** to the coordinate limits (e.g., **-R**\ *15W*/*30E*/*10S*/*15N*).

    Alternatively, you may indicate geographical coordinates by supplying **-fg**; see Section
    `Data type selection: The -f option`_.

Projected coordinates:
    These are Cartesian projected coordinates compatible with the chosen projection and are given in a length *unit*
    set via the **+u** modifier, (e.g., -200/200/-300/300\ **+uk** for a 400 by 600 km rectangular area centered
    on the projection center (0, 0), shorthand: -R200/300+uk.
    These coordinates are internally converted to the corresponding geographic
    (longitude, latitude) coordinates for the lower left and upper right corners. This form is convenient when you want
    to specify a region directly in the projected units (e.g., UTM meters). For allowable units, see Table
    :ref:`Distance units <tbl-distunits>`. **Note**: For the UTM, TM and Stereographic projections we will guess the
    units in your grid to be meter if the domain exceeds the range of geographical longitude and latitude.

Calendar time coordinates:
    These are absolute time coordinates referring to a Gregorian or ISO calendar. The general format is
    [*date*]\ **T**\ [*clock*], where *date* must be in the [-]\ *yyyy*\ [*-mm*\ [*-dd*]] (year, month, day-of-month) or
    *yyyy*\ [*-jjj*] (year and day-of-year) for Gregorian calendars and *yyyy*\ [*-*\ **W**\ *ww*\ [*-d*]] (year,
    week, and day-of-week) for the ISO calendar. **Note**: This format requirement only applies to command-line
    arguments and not time coordinates given via data files.  If no *date* is given we assume the current day. The
    **T** flag is required if a *clock* is given.

    The optional *clock* string is a 24-hour clock in *hh*\ [*:mm*\ [*:ss*\ [*.xxx*]]] format. If no *clock* is given it
    implies 00:00:00, i.e., the start of the specified day. Note that not all of the specified entities need be present
    in the data. All calendar date-clock strings are internally represented as double precision seconds since proleptic
    Gregorian date Monday January 1 00:00:00 0001. Proleptic means we assume that the modern calendar can be
    extrapolated forward and backward; a year zero is used, and Gregory's reforms [11]_ are extrapolated backward. Note
    that this is not historical. The use of delimiters and their type and positions for *date* and *clock* must be
    exactly as indicated; however, these are customizable using :ref:`FORMAT parameters <FORMAT Parameters>`

Relative time coordinates:
    These are coordinates which count seconds, hours, days or years relative to a given epoch. A combination of the
    parameters :term:`TIME_EPOCH` and :term:`TIME_UNIT` define the epoch and time unit. The parameter
    :term:`TIME_SYSTEM` provides a few shorthands for common combinations of epoch and unit, like **j2000** for days
    since noon of 1 Jan 2000. The default relative time coordinate is that of UNIX computers: seconds since 1 Jan 1970.
    Denote relative time coordinates by appending the optional lower case **t** after the value. When it is otherwise
    apparent that the coordinate is relative time (for example by using the **-f** switch), the **t** can be omitted.

Radians:
    For angular regions (and increments) specified in radians you may use a set of forms indicating multiples or
    fractions of :math:`\pi`.  Valid forms are [±][*s*]\ **pi**\ [*f*], where *s* and *f* are any integer or floating
    point numbers, e.g., -2\ **pi**\ /2\ **pi**\ 3 goes from -360 to 120 degrees (but in radians).  When GMT parses one
    of these forms we alert the labeling machinery to look for certain combinations of **pi**, limited to *n*\
    **pi**\ , 3/2 **pi** (3\ **pi**\ 2), and fractions 3/4 (3\ **pi**\ 4), 2/3 (2\ **pi**\ 3), 1/2 (1\ **pi**\ 2), 1/3
    (1\ **pi**\ 3), and 1/4 (1\ **pi**\ 4) in the *interval* given to the **-B** axes settings.  When an annotated value
    is within roundoff-error of these combinations we typeset the label using the Greek letter :math:`\pi` and required
    multiples or fractions.

Other coordinates:
    These are simply any coordinates that are not related to geographic or calendar time or relative time and are
    expected to be simple floating point values such as
    [±]\ *xxx.xxx*\ [**E**\|\ **e**\|\ **D**\|\ **d**\ [±]\ *xx*],
    i.e., regular or exponential notations, with the enhancement to understand FORTRAN double precision output which
    may use **D** instead of **E** for exponents. These values are simply converted as they are to internal
    representation. [12]_

**Examples**

.. _gmt_region:

.. figure:: /_images/GMT_-R.*
   :width: 500 px
   :align: center

   The plot region can be specified in two different ways. (a) Extreme values for each dimension, or (b) coordinates of
   lower left and upper right corners.

.. toggle::

   Here is the source script for the figure above:

   .. literalinclude:: /_verbatim/GMT_-R.txt


.. _option_-J:

Coordinate transformations and map projections: The **-J** option
-----------------------------------------------------------------

This option selects the coordinate transformation or map projection. The general format is

-  **-J**\ :math:`\delta`\ [*parameters*/]\ *scale*. Here, :math:`\delta` is a *lower-case* letter of the alphabet that
   selects a particular map projection, the *parameters* is zero or more slash-delimited projection parameter, and
   *scale* is map scale given in :ref:`plot-units <plt-units>` /degree or as 1:xxxxx.

-  **-J**\ :math:`\Delta`\ [*parameters*/]\ *width*. Here, :math:`\Delta` is an *upper-case* letter of the alphabet that
   selects a particular map projection, the *parameters* is zero or more slash-delimited projection parameter, and
   *width* is map width in :ref:`plot-units <plt-units>` (map height is automatically computed from the implied map scale
   and region).

The over 30 map projections and coordinate transformations available in GMT are represented in the Figure
:ref:`GMT Projections <gmt_projections>`.

.. _gmt_projections:

.. figure:: /_images/GMT_-J.*
   :width: 500 px
   :align: center

   The over-30 map projections and coordinate transformations available in GMT

.. toggle::

   Here is the source script for the figure above:

   .. literalinclude:: /_verbatim/GMT_-J.txt

.. _proj-codes:

Projections specifications
^^^^^^^^^^^^^^^^^^^^^^^^^^
GMT offers 31 map projections specified using the **-J** option. The projection codes are tabulated
below along with the associated *parameters* and links to the cookbook sections that describe the projection syntax and
usage.

.. include:: ../proj-codes.rst_

.. _option_-B:

Map frame and axes annotations: The **-B** option
-------------------------------------------------

.. |Add_-B| unicode:: 0x20 .. just an invisible code
.. include:: ../explain_-B.rst_
    :start-after: ^^^^^^^^^^^^^^^^^

Geographic basemaps
^^^^^^^^^^^^^^^^^^^

Geographic basemaps may differ from regular plot axis in that some
projections support a "fancy" form of axis and is selected by the
:term:`MAP_FRAME_TYPE` setting. The annotations will be formatted
according to the :term:`FORMAT_GEO_MAP` template and
:term:`MAP_DEGREE_SYMBOL` setting. A simple example of part of a basemap
is shown in Figure :ref:`Geographic map border <basemap_border>`.

.. _basemap_border:

.. figure:: /_images/GMT_-B_geo_1.*
   :width: 500 px
   :align: center

   Geographic map border using separate selections for annotation,
   frame, and grid intervals.  Formatting of the annotation is controlled by
   the parameter :term:`FORMAT_GEO_MAP` in your :doc:`/gmt.conf`.

.. toggle::

   Here is the source script for the figure above:

   .. literalinclude:: /_verbatim/GMT_-B_geo_1.txt

The machinery for primary and secondary annotations introduced for
time-series axes can also be utilized for geographic basemaps. This may
be used to separate degree annotations from minutes- and
seconds-annotations. For a more complicated basemap example using
several sets of intervals, including different intervals and pen
attributes for grid lines and grid crosses, see Figure :ref:`Complex basemap
<complex_basemap>`.

.. _complex_basemap:

.. figure:: /_images/GMT_-B_geo_2.*
   :width: 500 px
   :align: center

   Geographic map border with both primary (P) and secondary (S) components.

.. toggle::

   Here is the source script for the figure above:

   .. literalinclude:: /_verbatim/GMT_-B_geo_2.txt

Cartesian linear axes
^^^^^^^^^^^^^^^^^^^^^

For non-geographic axes, the :term:`MAP_FRAME_TYPE` setting is implicitly
set to plain. Other than that, cartesian linear axes are very similar to
geographic axes. The annotation format may be controlled with the
:term:`FORMAT_FLOAT_OUT` parameter. By default, it is set to "%g", which
is a C language format statement for floating point numbers [13]_, and
with this setting the various axis routines will automatically determine
how many decimal points should be used by inspecting the *stride*
settings. If :term:`FORMAT_FLOAT_OUT` is set to another format it will be
used directly (e.g., "%.2f" for a fixed, two decimals format). Note that
for these axes you may use the *unit* setting to add a unit string to
each annotation (see Figure :ref:`Axis label <axis_label_basemap>`).

.. _axis_label_basemap:

.. figure:: /_images/GMT_-B_linear.*
   :width: 500 px
   :align: center

   Linear Cartesian projection axis.  Long tick-marks accompany
   annotations, shorter ticks indicate frame interval. The axis label is
   optional. For this example we used ``-R0/12/0/0.95 -JX7.5c/0.75c -Ba4f2g1+lFrequency+u" %" -BS``

.. toggle::

   Here is the source script for the figure above:

   .. literalinclude:: /_verbatim/GMT_-B_linear.txt

For axes with angles in radians you can also specify annotation, tick, and gridline
intervals using multiples or fractions of :math:`\pi` (using *pi*)
(e.g., as in Figure :ref:`Axis label <axis_radians_basemap>`)

.. _axis_radians_basemap:

.. figure:: /_images/GMT_-B_radians.*
   :width: 500 px
   :align: center

   Linear Cartesian projection axis with annotations and ticks in radians.

.. toggle::

   Here is the source script for the figure above:

   .. literalinclude:: /_verbatim/GMT_-B_radians.txt

There are occasions when the length of the annotations are such that placing them
horizontally (which is the default) may lead to overprinting or too few annotations.
One solution is to request slanted annotations for the x-axis (e.g., Figure :ref:`Axis label <axis_slanted_basemap>`)
via the **+a**\ *angle* modifier.

.. _axis_slanted_basemap:

.. figure:: /_images/GMT_-B_slanted.*
   :width: 500 px
   :align: center

   Linear Cartesian projection axis with slanted annotations.
   For this example we used ``-R2000/2020/35/45 -JX12c -Bxa2f+a-30 -BS``.
   For the y-axis only the modifier **+ap** for parallel is allowed.

.. toggle::

   Here is the source script for the figure above:

   .. literalinclude:: /_verbatim/GMT_-B_slanted.txt

Cartesian log\ :sub:`10` axes
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Due to the logarithmic nature of annotation spacings, the *stride*
parameter takes on specific meanings. The following concerns are
specific to log axes (see Figure :ref:`Logarithmic projection axis
<Log_projection>`):

*  *stride* must be 1, 2, 3, or a negative integer -n.
   Annotations/ticks will then occur at 1, 1-2-5, or 1,2,3,4,...,9,
   respectively, for each magnitude range. For *-n* the
   annotations will take place every *n*\ 'th magnitude.

*  Append **l** to *stride*. Then, log\ :sub:`10` of the annotation
   is plotted at every integer log\ :sub:`10` value (e.g.,
   *x = 100* will be annotated as "2") [Default annotates *x* as is].

*  Append **p** to *stride*. Then, annotations appear as 10 raised to
   log\ :sub:`10` of the value (e.g., 10\ :sup:`-5`).

.. _Log_projection:

.. figure:: /_images/GMT_-B_log.*
   :width: 500 px
   :align: center

   Logarithmic projection axis using separate values for annotation,
   frame, and grid intervals.  (top) Here, we have chosen to annotate the actual
   values.  Interval = 1 means every whole power of 10, 2 means 1, 2, 5 times
   powers of 10, and 3 means every 0.1 times powers of 10.  We used
   -R1/1000/0/1 -JX7.5cl/0.6c -Ba1f2g3. (middle) Here, we have chosen to
   annotate :math:`\log_{10}` of the actual values, with -Ba1f2g3l.
   (bottom) We annotate every power of 10 using :math:`\log_{10}` of the actual
   values as exponents, with -Ba1f2g3p.

.. toggle::

   Here is the source script for the figure above:

   .. literalinclude:: /_verbatim/GMT_-B_log.txt

Cartesian exponential axes
^^^^^^^^^^^^^^^^^^^^^^^^^^

Normally, *stride* will be used to create equidistant (in the user's
unit) annotations or ticks, but because of the exponential nature of the
axis, such annotations may converge on each other at one end of the
axis. To avoid this problem, you can append **p** to *stride*, and the
annotation interval is expected to be in transformed units, yet the
annotation itself will be plotted as un-transformed units (see Figure
:ref:`Power projection axis <Pow_projection>`). E.g., if
*stride* = 1 and power = 0.5 (i.e., sqrt), then equidistant annotations
labeled 1, 4, 9, ... will appear.

.. _Pow_projection:

.. figure:: /_images/GMT_-B_pow.*
   :width: 500 px
   :align: center

   Exponential or power projection axis. (top) Using an exponent of 0.5
   yields a :math:`sqrt(x)` axis.  Here, intervals refer to actual data values,
   in -R0/100/0/0.9 -JX3ip0.5/0.25i -Ba20f10g5.
   (bottom) Here, intervals refer to projected values, although the annotation
   uses the corresponding unprojected values, as in -Ba3f2g1p.

.. toggle::

   Here is the source script for the figure above:

   .. literalinclude:: /_verbatim/GMT_-B_pow.txt

.. _cartesian_time_axes:

Cartesian time axes
^^^^^^^^^^^^^^^^^^^

What sets time axis apart from the other kinds of plot axes is the
numerous ways in which we may want to tick and annotate the axis. Not
only do we have both primary and secondary annotation items but we also
have interval annotations versus tick-mark annotations, numerous time
units, and several ways in which to modify the plot. We will demonstrate
this flexibility with a series of examples. While all our examples will
only show a single *x*\ -axis (south, selected via **-BS**), time-axis
annotations are supported for all axes.

Our first example shows a time period of almost two months in Spring
2000. We want to annotate the month intervals as well as the date at the start of each week:

.. literalinclude:: /_verbatim/GMT_-B_time1.txt

These commands result in Figure :ref:`Cartesian time axis <cartesian_axis1>`.
Note the leading hyphen in the :term:`FORMAT_DATE_MAP`
removes leading zeros from calendar items (e.g., 02 becomes 2).

.. _cartesian_axis1:

.. figure:: /_images/GMT_-B_time1.*
   :width: 500 px
   :align: center

   Cartesian time axis, example 1

The next example shows two different ways to annotate an axis portraying 2 days in July 1969:

.. literalinclude:: /_verbatim/GMT_-B_time2.txt

The lower example (Figure :ref:`cartesian_axis2`) chooses to annotate the weekdays (by
specifying **a**\ 1\ **K**) while the upper example choses dates (by
specifying **a**\ 1\ **D**). Note how the clock format only selects
hours and minutes (no seconds) and the date format selects a month name,
followed by one space and a two-digit day-of-month number.

.. _cartesian_axis2:

.. figure:: /_images/GMT_-B_time2.*
   :width: 500 px
   :align: center

   Cartesian time axis, example 2

The third example (Figure :ref:`cartesian_axis3`) presents two years, annotating
both the years and every 3rd month.

.. literalinclude:: /_verbatim/GMT_-B_time3.txt

Note that while the year annotation is centered on the 1-year interval,
the month annotations must be centered on the corresponding month and
*not* the 3-month interval. The :term:`FORMAT_DATE_MAP` selects month name
only and :term:`FORMAT_TIME_PRIMARY_MAP` selects the 1-character, upper
case abbreviation of month names using the current language (selected by
:term:`GMT_LANGUAGE`).

.. _cartesian_axis3:

.. figure:: /_images/GMT_-B_time3.*
   :width: 500 px
   :align: center

   Cartesian time axis, example 3

The fourth example (Figure :ref:`cartesian_axis4`) only shows a few hours of a day, using
relative time by specifying **t** in the **-R** option while the
:term:`TIME_UNIT` is **d** (for days). We select both primary and secondary
annotations, ask for a 12-hour clock, and let time go from right to left:

.. literalinclude:: /_verbatim/GMT_-B_time4.txt

.. _cartesian_axis4:

.. figure:: /_images/GMT_-B_time4.*
   :width: 500 px
   :align: center

   Cartesian time axis, example 4

The fifth example shows a few weeks of time (Figure :ref:`cartesian_axis5`). The lower axis
shows ISO weeks with week numbers and abbreviated names of the weekdays.
The upper uses Gregorian weeks (which start at the day chosen by
:term:`TIME_WEEK_START`); they do not have numbers.

.. literalinclude:: /_verbatim/GMT_-B_time5.txt

.. _cartesian_axis5:

.. figure:: /_images/GMT_-B_time5.*
   :width: 500 px
   :align: center

   Cartesian time axis, example 5

Our sixth example (Figure :ref:`cartesian_axis6`) shows the first five months of
1996, and we have annotated each month with an abbreviated, upper case name and
2-digit year. Only the primary axes information is specified.

.. literalinclude:: /_verbatim/GMT_-B_time6.txt

.. _cartesian_axis6:

.. figure:: /_images/GMT_-B_time6.*
   :width: 500 px
   :align: center

   Cartesian time axis, example 6

Our seventh and final example (Figure :ref:`cartesian_axis7`) illustrates
annotation of year-days. Unless we specify the formatting with a leading hyphen
in :term:`FORMAT_DATE_MAP` we get 3-digit integer days. Note that
in order to have the two years annotated we need to allow for the annotation of
small fractional intervals; normally such truncated interval must be at
least half of a full interval.

.. literalinclude:: /_verbatim/GMT_-B_time7.txt

.. _cartesian_axis7:

.. figure:: /_images/GMT_-B_time7.*
   :width: 500 px
   :align: center

   Cartesian time axis, example 7

.. _custom_axes:

Custom axes
^^^^^^^^^^^

Irregularly spaced annotations or annotations based on
look-up tables can be implemented using the *custom* annotation
mechanism. Here, we have given the **c** (custom) type to the **-B** option
followed by a filename that contains the annotations (and
tick/grid-lines specifications) for one axis. The file can contain any
number of comments (lines starting with #) and any number of records of
the format

| *coord* *type* [*label*]

The *coord* is the location of the desired annotation, tick, or
grid-line, whereas *type* is a string composed of letters from **a**
(annotation), **i** interval annotation, **f** frame tick, and **g**
gridline. You must use either **a** or **i** within one file; no mixing
is allowed. The coordinates should be arranged in increasing order. If
*label* is given it replaces the normal annotation based on the *coord*
value. Our last example (Figure :ref:`Custom and irregular annotations
<Custom_annotations>`) shows such a custom basemap with an interval
annotations on the *x*-axis and irregular annotations on the *y*-axis.

.. literalinclude:: /_verbatim/GMT_-B_custom.txt

.. _Custom_annotations:

.. figure:: /_images/GMT_-B_custom.*
   :width: 500 px
   :align: center

   Custom and irregular annotations, tick-marks, and gridlines.

.. _option_-U:

Timestamps on plots: The **-U** option
--------------------------------------

.. |Add_-U| unicode:: 0x20 .. just an invisible code
.. include:: ../explain_-U.rst_
    :start-after: ^^^^^^^^^^^^^^^^^

**Examples**

.. _fig_-U:

.. figure:: /_images/GMT_-U.*
   :width: 500 px
   :align: center

   The -U option makes it easy to date a plot.

.. toggle::

   Here is the source script for the figure above:

   .. literalinclude:: /_verbatim/GMT_-U.txt

.. _option_-V:

Verbose feedback: The **-V** option
-----------------------------------

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: ../explain_-V.rst_
    :start-after: ^^^^^^^^^^^^^^^^^

.. _option_-X:
.. _option_-Y:

Plot positioning and layout: The **-X** **-Y** options
------------------------------------------------------

.. |Add_-XY| unicode:: 0x20 .. just an invisible code
.. include:: ../explain_-XY.rst_
    :start-after: ^^^^^^^^^^^^^^^^^^^^^^^^^

**Examples**

.. _XY_options:

.. figure:: /_images/GMT_-XY.*
   :width: 300 px
   :align: center

   Plot origin can be translated freely with -X -Y.

.. toggle::

   Here is the source script for the figure above:

   .. literalinclude:: /_verbatim/GMT_-XY.txt

.. _option_-a:

OGR/GMT GIS i/o: The **-a** option
----------------------------------

.. include:: ../explain_-aspatial_full.rst_
    :start-after: ^^^^^^^^^^^^^^^^^

.. _option_-binary:

Binary table i/o: The **-b** option
-----------------------------------

Binary input with **-bi** option
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. include:: ../explain_-bi_full.rst_
    :start-after: ^^^^^^^^^^^^^^^^^^

Binary output with **-bo** option
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. include:: ../explain_-bo_full.rst_
    :start-after: ^^^^^^^^^^^^^^^^^^

.. _option_-c:

Selecting subplot panels: The **-c** option
-------------------------------------------

.. include:: ../explain_-c_full.rst_
    :start-after: ^^^^^^^^^^^^^^^^^

.. _option_-d:

Missing data conversion: The **-d** option
------------------------------------------

.. include:: ../explain_-d_full.rst_
    :start-after: ^^^^^^^^^^^^^^^^^

.. _option_-e:

Data record pattern matching: The **-e** option
-----------------------------------------------

.. include:: ../explain_-e_full.rst_
    :start-after: ^^^^^^^^^^^^^^^^^

.. _option_-f:

Data type selection: The **-f** option
--------------------------------------

.. include:: ../explain_-f_full.rst_
    :start-after: ^^^^^^^^^^^^^^^^^

.. _option_-g:

Data gap detection: The **-g** option
-------------------------------------

.. include:: ../explain_-g_full.rst_
    :start-after: ^^^^^^^^^^^^^^^^^

.. _option_-h:

Header data records: The **-h** option
--------------------------------------

.. include:: ../explain_-h_full.rst_
    :start-after: ^^^^^^^^^^^^^^^^^

.. _option_-i:

Input columns selection: The **-i** option
------------------------------------------

.. include:: ../explain_-icols_full.rst_
    :start-after: ^^^^^^^^^^^^^^^^^

**Examples**

For example, to use the 4th, 7th, and 3rd data column as the required *x, y, z*
to :doc:`/blockmean` you would specify **-i**\ 3,6,2 (since 0 is the first
column). The chosen data columns will be used as given.

.. _gmt_record:

.. figure:: /_images/GMT_record.png
   :width: 600 px
   :align: center

   The physical, logical (input) and output record in GMT.  Here, we are
   reading a file with 5 numerical columns plus some free-form text at the
   end.  Our module (here :doc:`/plot`) will be used to plot circles at the
   given locations but we want to assign color based on the ``depth`` column
   (which we need to convert from meters to km) and symbol size based on the
   ``mag`` column (but we want to scale the magnitude by 0.01 to get suitable symbol sizes).
   We use **-i** to pull in the desired columns in the required order and apply
   the scaling, resulting in a logical input record with 4 columns.  The **-f** option
   can be used to specify column types in the logical record if it is not clear
   from the data themselves (such as when reading a binary file).  Finally, if
   a module needs to write out only a portion of the current logical record then
   you may use the corresponding **-o** option to select desired columns, including
   the trailing text column **t**.  If you only want to output one word from the
   trailing text, then append the word number (0 is the first word).  Note that
   these column numbers now refer to the logical record, not the physical, since
   after reading the data there is no physical record, only the logical record in memory.

.. _option_-j_distcalc:

Spherical distance calculations: The **-j** option
--------------------------------------------------

.. include:: ../explain_distcalc_full.rst_
    :start-after: ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. _option_-l:

Setting automatic legend entries: The **-l** option
---------------------------------------------------

.. include:: ../explain_-l_full.rst_
    :start-after: ^^^^^^^^^^^^^^^^^

**Examples**

A simple plot with two symbols can obtain a legend by using this option and modifiers and is shown in Figure
:ref:`Auto Legend <auto_legend>`:

.. literalinclude:: /_verbatim/GMT_autolegend.txt

As the script shows, when no *specfile* is given to :doc:`/legend` then we look for the automatically generated one in
the session directory.

.. _auto_legend:

.. figure:: /_images/GMT_autolegend.*
   :width: 500 px
   :align: center

   Each of the two :doc:`/plot` commands use **-l** to add a symbol to the
   auto legend; the first also sets a legend header of given size and draws a horizontal line.

.. _option_-n:

Grid interpolation parameters: The **-n** option
------------------------------------------------

.. include:: ../explain_-n_full.rst_
    :start-after: ^^^^^^^^^^^^^^^^^

.. _option_-o:

Output columns selection: The **-o** option
-------------------------------------------

.. include:: ../explain_-ocols_full.rst_
    :start-after: ^^^^^^^^^^^^^^^^^

**Examples**

To write out just the 4th and 2nd data column to the output, use **-o**\ 3,1 (since 0 is the first column).
To write the 4th, 2nd, and 4th again use **-o**\ 3,1,3. As for **-i** you can transform columns using the
**+d**, **+l**, **+o**, and **+s** modifiers.

.. _option_-p:

Perspective view: The **-p** option
-----------------------------------

.. include:: ../explain_perspective_full.rst_
    :start-after: ^^^^^^^^^^^^^^^^^

.. _option_-q:

Data row selection: The **-q** option
-------------------------------------

.. include:: ../explain_-q_full.rst_
    :start-after: ^^^^^^^^^^^^^^^^^

**Examples**

Use **-q**\ 1\ **+s** to only read the 2nd data record from each of the segments found.

.. _option_nodereg:

Grid registration: The **-r** option
------------------------------------

.. include:: ../explain_nodereg_full.rst_
    :start-after: ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
    :end-before: (Node registrations

Gridline registration
^^^^^^^^^^^^^^^^^^^^^

In this registration, the nodes are centered on the grid line
intersections and the data points represent the average value in a cell
of dimensions (:math:`x_{inc} \cdot y_{inc}`) centered on each node
(left side of Figure :ref:`Grid registration <Grid_registration>`).
In the case of grid line registration the number of nodes are related
to region and grid spacing by

.. math::

   \begin{array}{ccl}
   nx & =  &       (x_{max} - x_{min}) / x_{inc} + 1       \\
   ny & =  &       (y_{max} - y_{min}) / y_{inc} + 1
   \end{array}

which for the example in left side of Figure :ref:`Grid registration
<Grid_registration>` yields nx = ny = 4.

Pixel registration
^^^^^^^^^^^^^^^^^^

Here, the nodes are centered in the grid cells, i.e., the areas
between grid lines, and the data points represent the average values
within each cell (right side of Figure :ref:`Grid registration
<Grid_registration>`). In the case of
pixel registration the number of nodes are related to region and grid
spacing by

.. _Grid_registration:

.. figure:: /_images/GMT_registration.*
   :width: 500 px
   :align: center

   Gridline- and pixel-registration of data nodes.  The red shade indicates the
   areas represented by the value at the node (solid circle).

.. toggle::

   Here is the source script for the figure above:

   .. literalinclude:: /_verbatim/GMT_registration.txt

.. math::

   \begin{array}{ccl}
   nx & =  &       (x_{max} - x_{min}) / x_{inc}   \\
   ny & =  &       (y_{max} - y_{min}) / y_{inc}
   \end{array}

Thus, given the same region (**-R**) and grid spacing, the
pixel-registered grids have one less column and one less row than the
gridline-registered grids; here we find nx = ny = 3.

Switching registrations
^^^^^^^^^^^^^^^^^^^^^^^

.. _Switch_Registrations:

GMT offer ways to convert a pixel-registered grid to a gridline-registered grid.
One way is to simply adjust the region of the grid by half the grid-spacing and
toggle the registration in :doc:`/grdedit` **-T**.  This is a *non-destructive* way to convert the grid,
but it does change the domain which may not be desirable depending on application.
The other is to *resample* the grid at the other set of nodes via :doc:`/grdsample` **-T**.
This approach leaves the region exactly the same but is *destructive* due to the loss
of the higher data frequencies, as shown in Figure :ref:`Registration resampling <Grid_grid2pix>`.

.. _Grid_grid2pix:

.. figure:: /_images/GMT_grid2pix.*
   :width: 500 px
   :align: center

   a) Cross-section of a grid along the *x*-axis for a constant value of *y*, showing just the Nyquist
   *x*-component (heavy line) at its grid nodes (red circles).  Resampling this component half-way
   between nodes (vertical lines) will always give zero (red triangles), hence this signal is lost,
   unlike long wavelength components (thin line), which can be interpolated (blue triangles).
   Intermediate wavelengths will experience attenuated amplitudes as well. b) Transfer function for
   resampling data from a pixel-registered to a gridline-registered grid format illustrates the loss
   of amplitude that will occur.  There is also a linear change in phase from 0 to 90 degrees as a
   function of wavenumber :math:`k_j` [Marks and Smith, 2007] [14]_.

.. toggle::

   Here is the source script for the figure above:

   .. literalinclude:: /_verbatim/GMT_grid2pix.txt

.. _option_-s:

NaN-record treatment: The **-s** option
---------------------------------------

.. include:: ../explain_-s_full.rst_
    :start-after: ^^^^^^^^^^^^^^^^^

.. _option_-t:

Layer transparency: The **-t** option
-------------------------------------

.. include:: ../explain_-t_full.rst_
    :start-after: ^^^^^^^^^^^^^^^^^
.. _option_-w:

Examining data cycles: The **-w** option
----------------------------------------

.. include:: ../explain_-w_full.rst_
    :start-after: ^^^^^^^^^^^^^^^^^
    :end-before: See the cookbook section

**Examples**

To demonstrate the use of **-w** we will make a few plots of the daily discharge rate of
the Mississippi river during the 1930-1940 period.  A simple time series plot is created by

.. literalinclude:: /_verbatim/GMT_cycle_1.txt

which results in the plot in Figure :ref:`Mississippi discharge <gmt_cycle_1>`:

.. _gmt_cycle_1:

.. figure:: /_images/GMT_cycle_1.*
   :width: 500 px
   :align: center

   Regular time-series plot of the daily Mississippi river discharge.

Given the clear annual signal we wish to plot this data using a normalized
yearly coordinate so that all the years are plotted on top of a single normalized year.
We accomplish this feature via **-wy** and use the prescribed 0–1 year range.

.. literalinclude:: /_verbatim/GMT_cycle_2.txt

These commands result in Figure :ref:`Mississippi annual discharge <gmt_cycle_2>`

.. _gmt_cycle_2:

.. figure:: /_images/GMT_cycle_2.*
   :width: 500 px
   :align: center

   Daily Mississippi river discharge data plotted over the duration of a single,
   normalized year.

In this representation, both regular and leap years are normalized by their respective lengths.
If we prefer to examine the discharge variation as a function of calendar month,
then we want all the values belonging to a particular month to fall into the same bin,
even though the bins represent variable ranges (28-31 days).  For such analyses we are better
off using **-wa** which normalizes the data per month, then adds the integer month number.
In other words, all timestamps in March of any year are converted by taking the time since the
start of March normalized by the length of March, and then add 2.  Thus, all March data from
any year will result in coordinates 2.00000–2.999999..... This allows us to easily make a
histogram of monthly discharge shown in Figure :ref:`Mississippi monthly discharge <gmt_cycle_3>`.

.. literalinclude:: /_verbatim/GMT_cycle_3.txt

.. _gmt_cycle_3:

.. figure:: /_images/GMT_cycle_3.*
   :width: 500 px
   :align: center

   Monthly Mississippi river discharge for the 10-year period, from September to September.

Quarterly discharge would similarly be obtained by using **-T**\ 3 in the :doc:`/histogram`
command.  As can be seen in Figure :ref:`Mississippi monthly discharge <gmt_cycle_3>`, the
annual cycle axis (as well as a weekly cycle axis) is considered a *temporal* axis and hence
the settings related to the appearance of month and weekday names, such as :term:`GMT_LANGUAGE`,
:term:`FORMAT_TIME_PRIMARY_MAP` and :term:`TIME_WEEK_START`, may be used.

Note that the **-w** option can also be applied to the *y*-coordinate instead (or any other
coordinate) via the **+c**\ *col* modifier.  Below we demonstrate this using the same
Mississippi river data but read it in so that time is the *y* coordinate. The following
script generates a subplot with two illustrations similar to the ones above but (basically)
transposed:

.. literalinclude:: /_verbatim/GMT_cycle_4.txt

.. _gmt_cycle_4:

.. figure:: /_images/GMT_cycle_4.*
   :width: 500 px
   :align: center

   a) Daily Mississippi river discharge data plotted over the duration of a single,
   normalized year. b) Monthly Mississippi river discharge for the 10-year period,
   from September to September.

Because **-w** is a global GMT option it is available in all modules that read
tables. Because of this, we can grid the data and make an image of the wrapped
dataset:

.. literalinclude:: /_verbatim/GMT_cycle_5.txt

.. _gmt_cycle_5:

.. figure:: /_images/GMT_cycle_5.*
   :width: 500 px
   :align: center

   Daily Mississippi river discharge data converted to a grid and imaged with
   the default (turbo) color table.

Our final example will explore the weekly and daily time-cycles using a three-year data
set of traffic (vehicles/hour) crossing the Verrazano-Narrows bridge between
Staten Island and Brooklyn (i.e., mostly inbound traffic destined for Manhattan).
We will show the raw time-series, wrap it to a weekly period, make a weekly histogram
and finally present an hourly histogram.  The figure caption under Figure :ref:`NY traffic <gmt_cycle_6>`
discusses the four panels resulting from running the script below:

.. literalinclude:: /_verbatim/GMT_cycle_6.txt

.. _gmt_cycle_6:

.. figure:: /_images/GMT_cycle_6.*
   :width: 500 px
   :align: center

   a) Time-series of traffic over a three-year period.  Note the dramatic drop as
   Covid-19 became a major issue in mid-March 2020, as well as some data-gaps
   and possibly a spike in May 2018. We use -g to skip drawing lines across data
   gaps exceeding 6 hours. b) Wrapped weekly signals make it easy
   to see the bimodal morning and evening rush-hour pattern for the weekdays
   with a different pattern on the weekends (the "spike" in (a) turned out to be
   data from a single Thursday and Sunday that seem problematic). Again, we use -g to
   skip data gaps exceeding 6 hours. c) A weekly histogram
   shows how traffic slowly builds up towards the weekend then drops off towards
   Sunday. The script calculates the number of repeated hours to normalize the plots.
   d) A daily histogram is created by wrapping to a daily cycle, then normalizing
   by the number of days.

.. _option_-x_core:

Selecting number of CPU cores: The **-x** option
------------------------------------------------

.. include:: ../explain_core_full.rst_
    :start-after: ^^^^^^^^^^^^^^^^^

.. _option_colon:

Latitude/Longitude or Longitude/Latitude?: The **-:** option
------------------------------------------------------------

.. include:: ../explain_colon_full.rst_
    :start-after: ^^^^^^^^^^^^^^^^^

Footnotes
---------

.. [11]
   The Gregorian Calendar is a revision of the Julian Calendar which was
   instituted in a papal bull by Pope Gregory XIII in 1582. The reason
   for the calendar change was to correct for drift in the dates of
   significant religious observations (primarily Easter) and to prevent
   further drift in the dates. The important effects of the change were
   (a) Drop 10 days from October 1582 to realign the Vernal Equinox with
   21 March, (b) change leap year selection so that not all years ending
   in "00" are leap years, and (c) change the beginning of the year to 1
   January from 25 March. Adoption of the new calendar was essentially
   immediate within Catholic countries. In the Protestant countries,
   where papal authority was neither recognized not appreciated,
   adoption came more slowly. England finally adopted the new calendar
   in 1752, with eleven days removed from September. The additional day
   came because the old and new calendars disagreed on whether 1700 was
   a leap year, so the Julian calendar had to be adjusted by one more
   day.

.. [12]
   While UTM coordinates clearly refer to points on the Earth, in this
   context they are considered "other". Thus, when we refer to
   "geographical" coordinates herein we imply longitude, latitude.

.. [13]
   Please consult the man page for *printf* or any book on C.

.. [14]
   Marks, K. M., and W. H. F. Smith, 2007, Some remarks on resolving seamounts in satellite gravity, Geophys. Res. Lett., 34 (L03307),
   https://doi.org/10.1029/2006GL028857.
