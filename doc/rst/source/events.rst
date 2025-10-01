.. index:: ! events
.. include:: module_core_purpose.rst_

******
events
******

|events_purpose|

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt events**
|-J|\ *parameters*
|SYN_OPT-Rz|
|-T|\ *now*
[ *table* ]
[ |-A|\ **r**\ [*dpu*\ [**c**\|\ **i**][**+v**\ [*value*]]]\|\ **s** ]
[ |SYN_OPT-B| ]
[ |-C|\ *cpt* ]
[ |-D|\ [**j**\|\ **J**]\ *dx*\ [/*dy*][**+v**\ [*pen*]] ]
[ |-E|\ [**s**\|\ **t**\ ][**+o**\|\ **O**\ *dt*][**+r**\ [**c**\|\ **l**\|\ **q**]\ *dt*][**+p**\ *dt*][**+d**\ [**c**\|\ **l**\|\ **q**]\ *dt*][**+f**\ [**c**\|\ **l**\|\ **q**]\ *dt*][**+l**\ *dt*] ]
[ |-F|\ [**+a**\ *angle*][**+f**\ *font*][**+j**\ *justify*][**+r**\ [*first*]\|\ **z**\ [*format*]] ]
[ |-G|\ *color* ]
[ |-H|\ *labelbox* ]
[ |-L|\ [*length*\|\ **t**] ]
[ |-M|\ **i**\|\ **s**\|\ **t**\|\ **v**\ *val1*\ [**+c**\ *val2*] ]
[ |-N|\ [**c**\|\ **r**] ]
[ |-Q|\ *prefix* ]
[ |-S|\ *symbol*\ [*size*] ]
[ |SYN_OPT-U| ]
[ |SYN_OPT-V| ]
[ |-W|\ *pen* ]
[ |SYN_OPT-X| ]
[ |SYN_OPT-Y| ]
[ |-Z|\ *command* ]
[ |SYN_OPT-a| ]
[ |SYN_OPT-bi| ]
[ |SYN_OPT-di| ]
[ |SYN_OPT-e| ]
[ |SYN_OPT-f| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-l| ]
[ |SYN_OPT-p| ]
[ |SYN_OPT-qi| ]
[ |SYN_OPT-tv| ]
[ |SYN_OPT-w| ]
[ |SYN_OPT-:| ]
[ |SYN_OPT--| ]

.. module_common_begins

Description
-----------

Reads (*x*, *y*\ [, *z*] [, *size*], *time* [, *length*] [, *label*]) data from *table* [or standard
input] and plots how the symbol for each event should look for the single specified time *now*.
The reason they may differ is that events may have different *durations* and we may
wish to *accentuate* the symbol attributes in ways to draw attention to an event when
it first appears, and perhaps tone it down after a while to reduce clutter and focus
on more recent events. You may also wish for symbols to disappear completely after they reach
their end time (we call this period the coda), or perhaps remain visible but faded, shrunk, or darkened.
Optionally, each symbol may have a label that can be displayed for a prescribed length of time at the
same time as the symbol or delayed a bit. This module is typically used in conjunction with :doc:`movie`
where the implicit loop over time is used to call **events** over a time-sequence and
thus plot symbols as the events unfold.

..  youtube:: iWt0yZICKlM
    :width: 100%

Animation of two events over the time window -0.5 to 1.5. The events are only active during times
0 to 1. One symbol (blue) is plotted normally only while active (size follows the light-blue time-curve,
while the other (red) is highlighted when it first arrives by adding a rise, plateau, decay, and fade
interval, and finally then let to fade to a persistent but smaller, darker and partly transparent
symbol after it is no longer active; its size follows the red time-curve. In contrast, the
blue symbol just turns on and off.  We also delay the red symbol's label by 0.25 time units.
Red event was plotted using -Es+rq0.25+p0.25+dq0.25+fl0.25 -Et+o0.25 while the blue event used the
default settings (i.e., the light-blue time curve). **Note**: The time curves are plotted here just
to illustrate what is happening under the hood. The movie we make is really about the two events only.

Required Arguments
------------------

.. |Add_intables| unicode:: 0x20 .. just an invisible code
.. include:: explain_intables.rst_

.. |Add_-J| replace:: |Add_-J_links|
.. include:: explain_-J.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. |Add_-R| replace:: |Add_-R_links|
.. include:: explain_-R.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. |Add_-Rz| unicode:: 0x20 .. just an invisible code
.. include:: explain_-Rz.rst_

.. _-T:

**-T**\ *now*
    Set the current plot time. The time shifts and increments set in |-E| are all *relative* to this
    current time.  If an absolute time is given you will need to use :term:`TIME_UNIT` to indicate
    the unit of the values given in options like |-E| and |-L|.

Optional Arguments
------------------

.. _-A:

**-A**\ **r**\ [*dpu*\ [**c**\|\ **i**][**+v**\ [*value*]]]\|\ **s**
    When no |-S| is given we expect to read lines or polygons.  Two different forms for input are
    supported and specified via the chosen |-A| directives:

    - Choose **-Ar** if your input data are *trajectories* (for more details, see `Drawing trajectories`_), the data
      format is given as *x, y, time*, and the "event" is the portion of that trajectories limited by the current
      time (*now*), event duration (via |-L|) and rise/fade periods (via **-Es**). If current time falls between
      two points on the trajectory then we linearly interpolate to find the current end point.
    - Choose **-As** to read each complete segment (i.e., a polygon or line with just *x, y* coordinates)
      from a multisegment file and get a common *time* for each segment via a required **-T**\ *string* specified
      in the segment header.  Here, the entire polygon or line is the "event" if inside the limits of current time
      (*now*), event duration (via |-L|) and rise/fade periods (via **-Es**).  If **-L**\ [**t**] is set then the *string*
      must be either of the format *begin*/*length*\|\ *end* or *begin*,\ *length*\|\ *end*.  Use a comma to
      separate absolute time specifications, otherwise a slash is also supported.

    **Note**: Neither lines nor polygons allow for any labels to be placed.

    Alternatively, you can use **-Ar**\ *dpu* to
    perform no plotting but instead convert your *x, y*\ [, *zcols* ], *time* data into *densely sampled points* so that
    a later call to **events** may use this sampled data set instead and plot it as circles with **-Sc**\ [*width*].
    This way you can plot "lines" that can have variable pen color and fixed or variable pen width as well as
    taking advantage of the full machinery of **-Es** and **-Et** and all the effects controlled by |-M| for symbols.
    The *dpu* must match the intended dpu when running :doc:`movie`. Note that *dpu* means pixels per cm if you are using
    SI units and pixels per inch if you are using US units (hence it depends on your :term:`PROJ_LENGTH_UNIT` setting).
    Alternatively, append **c** or **i** to set the unit used explicitly. Also note that if your movie involves
    changes to the region or the map projection then you may need to run the **-Ar**\ *dpu* as part of the main script.
    Finally, you can use **+v**\ [*value*] to insert a *z*-column (initialized to zero unless another *z* value is appended) in
    the dense point data file for use with **-Mv**.

.. |Add_-B| replace:: |Add_-B_links|
.. include:: explain_-B.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-C:

**-C**\ *cpt*
    Use the *cpt* to convert *z* values given in the optional third column to assign symbol or polygon colors.
    Not allowed if **-Ar** is used.

.. _-D:

**-D**\ [**j**\|\ **J**]\ *dx*\ [/*dy*][**+v**\ [*pen*]]
    Offset the text from the projected (*x*,\ *y*) point by *dx*,\ *dy*
    [0/0]. If *dy* is not specified then it is set equal to *dx*. Use
    **-Dj** to offset the text away from the point instead (i.e., the
    text justification will determine the direction of the shift). Using
    **-DJ** will shorten diagonal offsets at corners by
    sqrt(2). Optionally, append **+v** which will draw
    a line from the original point to the shifted point; append a *pen*
    to change the attributes for this line.

.. _-E:

**-E**\ [**s**\|\ **t**\ ][**+o**\|\ **O**\ *dt*][**+r**\ [**c**\|\ **l**\|\ **q**]\ *dt*][**+p**\ *dt*][**+d**\ [**c**\|\ **l**\|\ **q**]\ *dt*][**+f**\ [**c**\|\ **l**\|\ **q**]\ *dt*][**+l**\ *dt*] ]
    Set the relative time knots for the **s**\ ymbol or **t**\ ext time-functions and note that
    :math:`0 \le \Delta t_x, x = r, p, d, f` (see `The four time-functions`_). Set increments via
    these modifiers:

    - **+o** will shift both the event start and end times by a constant offset (basically delaying
      the event in time by *dt*.
    - **+O** is similar to **+o** but will only shift the start time, effectively shortening
      the duration of the event).
    - **+l** specifies an alternative duration (visibility) of the text (**-Et** only) [same duration as symbol].
    - **+r** sets the duration of the rise phase, hence :math:`t_r = t_b - \Delta t_r`.
      Prepend optional raise curve shape via directive **c**\ osine, **l**\ inear or **q**\ uadratic [Quadratic].
    - **+p** sets the duration of the plateau phase, hence :math:`t_p = t_b + \Delta t_p`.
    - **+d** sets the duration of the decay phase, hence :math:`t_d = t_p + \Delta t_d`.
      Prepend optional decay curve shape via directive **c**\ osine, **l**\ inear or **q**\ uadratic [Quadratic].
    - **+f** sets the duration of the fade phase, hence :math:`t_f = t_e + \Delta t_f`.
      Prepend optional fade curve shape via directive **c**\ osine, **l**\ inear or **q**\ uadratic [Linear].

    These are all optional [and default to zero], and can be set separately for symbols and texts.
    If neither the **s** or **t** directive is given then we set the time knots for both features.
    **Note 1**: The **+p** and **+d** increments do not apply to text labels, lines or polygons.
    **Note 2**: The **-Et** option is required if you wish to plot labels [Default plots symbols only].
    **Note 3**: None of the modifiers are allowed for lines.

.. _-F:

**-F**\ [**+a**\ *angle*][**+f**\ *font*][**+j**\ *justify*][**+r**\ [*first*]\|\ **z**\ [*format*]]
    By default, event label text will be placed horizontally, using the primary annotation font
    attributes, and centered on the data point. Normally, the text to be plotted is the trailing text.
    Use this option's modifiers to override these defaults by specifying alternative attributes:

    - **+f** sets the font (size,fontname,color) [:term:`FONT_ANNOT_PRIMARY`].
    - **+a** sets the angle of the baseline relative to the horizontal [0].
    - **+j** sets the justification of the label relative to the coordinates given [CM].
    - **+r** will use the record number (counting up from *first* [0]) as the text label.
    - **+z** will format incoming *z* value and use it as the text label (requires |-C| and
      the optional *format* [:term:`FORMAT_FLOAT_MAP`]).

.. _-G:

**-G**\ *fill* :ref:`(more ...) <-Gfill_attrib>`
    Set constant shade or color for all symbols or polygons.

.. _-H:

**-H**\ [**+c**\ *dx*/*dy*][**+g**\ *fill*][**+p**\ [*pen*]][**+r**][**+s**\ [[*dx*/*dy*/][*shade*]]]
    Enable bounding boxes around text labels and provide one or more of these specific parameters:

    - **+c** adjusts the clearance between the text and the surrounding box [15% of font size].
    - **+g**\ *fill* will fill the text box [no fill].
    - **+p** draws the box outline with :term:`MAP_DEFAULT_PEN` or append a different pen.
    - **+r** will use a rounded rectangular box [straight rectangular box].
    - **+s** places an offset, background shaded box behind the text box. Here, *dx*/*dy*
      indicates the shift relative to the foreground text box [4\ **p**/-4\ **p**] and *shade* changes
      the fill used for shading [gray50]. **Note**: Modifier **+s** requires **+g** as well.

.. include:: explain_-Jz.rst_

.. _-L:

**-L**\ [*length*\|\ **t**]
    Specify the length (i.e., duration) of the event.  Append a *length* if all events have the same length,
    append **t** if end-times instead of lengths are given in the file after the *time* column,
    or leave blank if lengths can be read from the input file after the *time* column.  If |-L| is not
    given we assume events have an infinite duration. See |-A| for how duration is handled for lines and
    polygons.

.. _-M:

**-M**\ **i**\|\ **s**\|\ **t**\|\ **v**\ *val1*\ [**+c**\ *val2*]

    Controls how each symbol's four attributes (**i**\ ntensity, **s**\ ize, **t**\ ransparency, and **v**\ alue)
    should change from when the symbol first appears, during its active duration, and optionally its fate as time
    moves past its end time. First supply the directive [default *val1* values are given in brackets]:

    - **i** will modify the intensity of the color [1].
    - **s** will modify the relative size of the symbol [1].
    - **t** will modify the transparency [100].
    - **v** will modify the data value (to change symbol color via CPT lookup) during the *rise* interval [0].

    The appended *val1* represents the maximum amplitude that is scaled by the corresponding time-function
    created by **-Es** (see `The four time-functions`_). Option |-M| is repeatable for the different directives;
    here are some further guidelines:

    - The intensity directive (**i**\ ; normally with *val1* in the range ±1, with 0 having no effect) is used to brighten
      (*val1* > 0) or darken (*val1* < 0) the symbol color during this period (the hue is kept fixed).
    - The size directive (**s**) is a magnifying factor that temporarily changes the size of the symbol by the factor *val1*.
    - The transparency directive (**t**) affects temporary changes to the symbol's transparency.
    - The value directive (**v**) temporarily adds *val1* to the data set's *z*-values, scaled by the corresponding time
      function, and thus can change the symbol's *color* via the CPT (hence |-C| is a required option for **-Mv**).

    Optionally, for finite-duration events that should remain visible for all times *after* their event end time has
    been reached you must append **+c** (for coda) to set the corresponding terminal value during the coda. If **+c**
    is not given then the defaults are 0 (intensity), 0 (size), 100 (transparency) and 0 (value), meaning the symbols
    are not plotted unless you change these attributes with one or more **+c** modifiers (one per directive).

   **Note**: Polygons can only use **-Mt** setting.

.. _-N:

**-N**\ [**c**\|\ **r**]
    Do **not** clip symbols that fall outside map border [Default plots points
    whose coordinates are strictly inside the map border only]. For periodic (360-longitude)
    maps we must plot all symbols twice in case they are clipped by the repeating
    boundary. The |-N| will turn off clipping and not plot repeating symbols.
    Use **-Nr** to turn off clipping but retain the plotting of such repeating symbols, or
    use **-Nc** to retain clipping but turn off plotting of repeating symbols. If set, |-N|
    will also apply to any labels.

.. _-Q:

**-Q**\ *prefix*
    Save the intermediate event symbols and labels to permanent files instead of removing them when done.
    Files will be named *prefix*\ _symbols.txt and *prefix*\ _labels.txt.

.. _-S:

**-S**\ *symbol*\ [*size*]
    Specify the symbol to use for the event [Default plots lines or polygons; see |-A|].
    Optionally, append symbol *size* with unit from
    (**c**\|\ **i**\|\ **p**\ ).  If no *size* is given then we read an event-specific size
    from the data file's third column (fourth if |-C| is used). **Note**: Not all the symbols that
    are available in :doc:`plot` can be used here.  At the moment we only support the basic
    symbols and custom symbols; bars, vectors, ellipses, fronts, decorated and quoted lines cannot
    be specified.  Symbols sizes read from a data file will be assumed to be in the unit controlled
    by :term:`PROJ_LENGTH_UNIT` unless they have **c**, **i**, or **p** appended.  If you are only
    plotting labels then |-S| is not required.

.. |Add_-U| replace:: |Add_-U_links|
.. include:: explain_-U.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-:

.. |Add_-V| replace:: |Add_-V_links|
.. include:: explain_-V.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-W:

**-W**\ *pen*
    Specify symbol outline pen attributes [Default is no outline].

.. |Add_-XY| replace:: |Add_-XY_links|
.. include:: explain_-XY.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-Z:

**-Z**\ *command*
    Place symbols generated by either :doc:`/supplements/seis/coupe`,
    :doc:`/supplements/seis/meca`, or :doc:`/supplements/geodesy/velo`.
    The quoted *command* must start with one of these module names, then
    supply all the module-specific options that the selected module requires to
    normally define a plot, excluding any of |-C|, |-G|, |-J|, |-N|, |-R|, and
    |-W| (those options may be required by **events** though).  Also, you may
    not use the |-I| or **-t** options since these will be set automatically
    as part of the variations imposed by **-Mi** and **-Mt**.  As an example, the
    custom command to plot a beachball may be **-Z**\ "meca -Sa5c+f0", while
    displaying beachballs in a crossection may require the more elaborate command
    **-Z**\ "coupe -Q -L -Sc3c -Ab128/11/120/250/90/400/0/100+f -Fa0.1i/cc".
    **Note**: If you are running a simple classic command then you must use
    the corresponding classic module names.

.. include:: explain_-aspatial.rst_

.. |Add_-bi| replace:: [Default is 3 input columns].
.. include:: explain_-bi.rst_

.. |Add_-di| unicode:: 0x20 .. just an invisible code
.. include:: explain_-di.rst_

.. |Add_-e| unicode:: 0x20 .. just an invisible code
.. include:: explain_-e.rst_

.. |Add_-f| unicode:: 0x20 .. just an invisible code
.. include:: explain_-f.rst_

.. |Add_-h| unicode:: 0x20 .. just an invisible code
.. include:: explain_-h.rst_

.. include:: explain_-icols.rst_

.. |Add_-l| unicode:: 0x20 .. just an invisible code
.. include:: explain_-l.rst_

.. |Add_perspective| unicode:: 0x20 .. just an invisible code
.. include:: explain_perspective.rst_

.. include:: explain_-qi.rst_

.. include:: explain_-tv_full.rst_

.. include:: explain_colon.rst_

.. include:: explain_help.rst_

The meaning of time
-------------------

While normally the meaning of "time" (e.g., such as in the argument to |-T|)
is in fact the time of the phenomenon being plotted, it may also be any other monotonically
increasing quantity of convenience, such as elapsed time, model time, frame number, or other physical
quantities such as distance.  It all depends on the context of the movie.  The values
given for delays, fades, etc. in |-E| are assumed to be in the same unit as the
given *time* argument to |-T|.  Thus, if *time* is a monotonically increasing frame
counter then the values in |-E| and |-L| refer to number of frames.  A special
situation occurs when *time* is in fact actual calendar time given by date/clock markers.
Here, the unit of the arguments in |-E| and |-L| is determined by the :term:`TIME_UNIT`
setting [second].  You will need to specify the desired time unit that should be used when
interpreting your |-E| and |-L| arguments.  The example below for seismicity uses
actual earthquake times but we use days as the unit as second is too small.

The four time-functions
-----------------------

Events typically have either a finite length, e.g., the temporal extent of
the reign of a particular monarch, or are infinite.  Here, infinite simply means
its length exceeds the temporal extent of the time we are plotting.  In a movie,
a basic representation of these events as points on a map would be to start plotting
the symbol when the event starts and then keep the symbol visible on all frames until the
event ends (or never stop plotting them in the infinite length case).  However, to draw
attention to the arrival of a new event we can temporarily change four attributes of
the symbol: Its size, color intensity, transparency, and color (via CPT look-up).  You
can think of these four attributes as functions of time.  For some option settings, the
figure below illustrates how the symbol size may vary through the timespan of the movie.

.. figure:: /_images/psevents_size.*
   :width: 400 px
   :align: center

   Implied evolution of one symbol's size-variation as a function of time
   given the time-knots from **-Es** and the magnifications from **-Ms**.
   Here we temporarily magnify the symbol size five times before decaying
   to the intended size and eventually shrink it further down to a small
   size during the fade and coda phases.

Similarly, to draw attention to the newly arrived event you may want to whiten the color
temporarily; this can be done via the **-Mi** option and figure below
illustrates how intensity may vary accordingly.

.. figure:: /_images/psevents_intensity.*
   :width: 400 px
   :align: center

   Implied evolution of one symbol's color intensity as a function of time
   given the time-knots from **-Es** and the intensities from **-Mi**.  Here we
   seek to whiten the symbol during the event arrival, as well as darken it
   during a permanent coda phase.

Next, for finite events you may wish to either fade them out completely or
simply fade them to a constant but still visible transparency; this future stage
is referred to as the coda and levels can be specified via the **-Mt** option.
Figure below illustrates how transparency may vary through time.

.. figure:: /_images/psevents_transparency.*
   :width: 400 px
   :align: center

   Here we show how a symbol may go from being invisible to reaching full opaqueness
   at the event begin time, finally fading to a near-invisible stage after reaching
   its length.

Finally, instead of selecting a fixed color for a symbol you can use |-C| to set a
color table that we will use to modulate the color of the symbol.  It is done by adding
a variable fraction of *dz* to the data's *z*-column and hence the CPT lookup will
return different colors at different times. The amplitude is controlled with the **-Mv**
option which will create *dz* values in the range 0-*val1* [Default is 0-1]. In most
cases the data set will have a zero z-value and hence the CPT will be set up for the
0-1 range (but note that the coda *val2* value may be negative so compose your CPT
accordingly if using coda). Figure below illustrates how *dz* may vary through time.

.. figure:: /_images/psevents_dz.*
   :width: 400 px
   :align: center

   Here we show how a symbol may go from a constant color to changing colors
   at the event begin time, finally fading to a different-colored stage after reaching
   its length. This curve is scaled by the amplitude [1] and added to the *z*-data before
   CPT-lookup occurs.

While shown in these examples, the rise, plateau, fade, and coda periods are all optional
and can be selected or ignored as you wish via |-E|.  You can choose to specify any or none
of the three time histories.  By default the symbol size is constant, there is no intensity
variation, and all symbols are opaque.

If trailing text is plotted (i.e., by selecting **-Et**) then only the transparency time-function
is used and neither the plateau nor decay periods apply.  The appearance and disappearance of a
label is thus dictated by an initial (and optional) rise time (during which the label slowly
becomes fully visible), an normal period (where it remains visible), followed by the optional
fade period (where the text fades out).  The figure below illustrate the setup:

.. figure:: /_images/psevents_labels.*
   :width: 400 px
   :align: center

   Here we show how a label may go from being invisible to reaching full opaqueness
   at the event begin time, staying visible for the given duration, and then finally
   fade away completely. For labels, the plateau and decay periods do not apply.


Sorting the data
----------------

While only the events that should be visible will be plotted at the given time set via |-T|,
the order of plotting is simply given by the order of records in the file.  Thus, if the
file is *not* sorted into ascending time then later events might plot *beneath* earlier events.
To prevent this, you can sort your file into ascending order via the :doc:`gmtconvert` |-N| option.

Drawing trajectories
--------------------

If **-Ar** is used to draw trajectories (lines with *x, y, t* coordinates) then, as stated, we will
use a linear interpolation to find intermediate terminal points.  For geographic data, and especially
if your original point spacing is large, you may wish to first interpolate such trajectories along
great circles via :doc:`sample1d` rather than subject large gaps to linear interpolation.
**Note**: If **-Ar**\ *dpu* is used to discretize the line, then be aware that the resampling
is affected by the current spline interpolator, controlled by :term:`GMT_INTERPOLANT`. To avoid
any departures from straight line segments you will need to select a linear interpolant.

.. module_common_ends

Examples
--------

To show the display of events visible for May 1, 2018 given the catalog of
large (>5) magnitude earthquakes that year, using a 2-day rise time during
which we boost symbol size by a factor of 5 and wash out the color, followed
by a decay over 6 days and then a final shrinking to half size and darken the
color, we may try::

    gmt begin layer
      gmt convert "https://earthquake.usgs.gov/fdsnws/event/1/query.csv?starttime=2018-01-01%2000:00:00&endtime=2018-12-31%2000:00:00&minmagnitude=5&orderby=time-asc" \
         -i2,1,3,4+s50,0 -hi1 > q.txt
      gmt makecpt -Cred,green,blue -T0,70,300,10000
      gmt events -Rg -JG200/5/6i -Baf q.txt -SE- -C --TIME_UNIT=d -T2018-05-01T -Es+r2+d6 -Ms5+c0.5 -Mi1+c-0.6 -Mt+c0
    gmt end show

To convert the time-series seismic_trace.txt (time, amplitude) into a (time, amplitude, time) file that **events** can plot
with a variable pen (by plotting densely placed circles), we use **-i** to ensure we read the time-column twice and then use
a *dpu* of 80 pixels per cm (HD movie) and the projection parameters we will use when making the plot, e.g.,::

      gmt events seismic_trace.txt -R1984-09-10T03:15/1984-09-10T03:45/-15/15 -JX20cT/10c -Ar80c -i0,1,0 > seismic_trace_pts.txt

**Note**: If your :term:`PROJ_LENGTH_UNIT` is set to inch then you need to use the equivalent *dpu* of 200 pixels per inch for HD,
or you specify **-Ar**\ 200\ **i**.

See Also
--------

:doc:`gmt`,
:doc:`gmtcolors`,
:doc:`plot`,
:doc:`movie`
