.. index:: ! x2sys_datalist

****************
x2sys_datalist
****************

.. only:: not man

    x2sys_datalist - Extract content of track data files

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**gmt x2sys_datalist** *track(s)* **-T**\ *TAG* 
[ |-A| ]
[ |-E| ]
[ |-F|\ *name1*,\ *name2*,... ]
[ |-I|\ [*list*] ]
[ |-L|\ [*corrtable*] ]
[ |SYN_OPT-R| ]
[ |-S| ] [
[ |SYN_OPT-V| ]
[ |SYN_OPT-bo| ]
[ |SYN_OPT-do| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**x2sys_datalist** reads one or more files and produces a single ASCII
[or binary] table. The files can be of any format; this information is
encoded in the TAG set with the |-T| option. You may limit the output to
a geographic region, and insist that the output from several files be
separated by a multiple segment header. Only the named data fields will
be output [Default selects all columns].

Required Arguments
------------------

.. include:: explain_track.rst_
.. include:: explain_tag.rst_

Optional Arguments
------------------

.. _-A:

**-A**
    Eliminate COEs by distributing the COE between the two tracks in
    proportion to track weight. These (dist, adjustment) spline knots
    files for each track and data column are called *track.column*.adj
    and are expected to be in the **$X2SYS_HOME**/*TAG* directory. The
    adjustments are only applied if the corresponding adjust file can be
    found [No residual adjustments]

.. _-E:

**-E**
    Enhance ASCII output by writing GMT segment headers between data
    from each track [no segment headers].

.. _-F:

**-F**\ *name1*,\ *name2*,...
    Give a comma-separated sub-set list of column names defined in the
    format definition file. [Default selects all data columns].

.. _-I:

**-I**\ [*list*]
    Name of ASCII file with a list of track names (one per record) that
    should be excluded from consideration [Default includes all tracks].

.. _-L:

**-L**\ [*corrtable*]
    Apply optimal corrections to columns where such corrections are
    available. Append the correction table to use [Default uses the
    correction table *TAG*\ \_corrections.txt which is expected to
    reside in the **$X2SYS_HOME**/*TAG* directory]. For the format of
    this file, see CORRECTIONS below.

.. _-R:

.. |Add_-Rgeo| replace:: For Cartesian
    data just give *xmin/xmax/ymin/ymax*. This option limits the COEs to
    those that fall inside the specified domain.
.. include:: ../../explain_-Rgeo.rst_

.. _-S:

**-S**
    Suppress output records where all the data columns are NaN [Default
    will output all records].

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-V.rst_

.. |Add_-bo| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-bo.rst_

.. |Add_-do| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-do.rst_

.. |Add_-h| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-h.rst_

.. include:: ../../explain_help.rst_

Examples
--------

To extract all data from the old-style MGG supplement file c2104.gmt,
recognized by the tag GMT:

   ::

    gmt x2sys_datalist c2104.gmt -TGMT > myfile

To make lon,lat, and depth input for :doc:`blockmean </blockmean>` and :doc:`surface </surface>` using
all the files listed in the file tracks.lis and defined by the tag TRK,
but only the data that are inside the specified area, and make output
binary, run

   ::

    gmt x2sys_datalist =tracks.lis -TTRK -Flon,lat,depth -R40/-30/25/35 -bo > alltopo_bin.xyz

Corrections
-----------

The correction table is an ASCII file with coefficients and parameters
needed to carry out corrections. This table is usually produced by
**x2sys_solve**. Comment records beginning with # are allowed. All
correction records are of the form

*trackID observation correction*

where *trackID* is the track name, *observation* is one of the
abbreviations for an observed field contained in files under this TAG,
and *correction* consists of one or more white-space-separated *term*\ s
that will be **subtracted** from the observation before output. Each
*term* must have this exact syntax:

*factor*\ [\*[*function*\ ]([*scale*\ ](\ *abbrev*\ [-*origin*]))[^\ *power*]]

where terms in brackets are optional (the brackets themselves are not
used but regular parentheses must be used exactly as indicated). No
spaces are allowed except between *term*\ s. The *factor* is the
amplitude of the basis function, while the optional *function* can be
one of sin, cos, or exp. The optional *scale* and *origin* can be used
to translate the argument (before giving it to the optional function).
The argument *abbrev* is one of the abbreviations for columns known to
this TAG. However, it can also be one of the three auxiliary terms
**dist** (for along-track distances), **azim** for along-track azimuths,
and **vel** (for along-track speed); these are all sensitive to the
**-C** and **-N** settings used when defining the TAG; furthermore,
**vel** requires **time** to be present in the data. If *origin* is
given as **T** it means that we should replace it with the value of
*abbrev* for the very first record in the file (this is usually only
done for *time*). If the first data record entry is NaN we revert
*origin* to zero. Optionally, raise the entire expression to the given
*power*, before multiplying by *factor*. The following is an example of
fictitious corrections to the track ABC, implying the **z** column
should have a linear trend removed, the field **obs** should be
corrected by a strange dependency on latitude, **weight** needs to have
1 added (hence correction is given as -1), and **fuel** should be
reduced by a linear distance term:

ABC z 7.1 1e-4\*((time-T))

ABC obs 0.5\*exp(-1e-3(lat))^1.5

ABC weight -1

ABC fuel 0.02\*((dist))

See Also
--------

:doc:`blockmean </blockmean>`,
:doc:`gmt </gmt>`,
:doc:`surface </surface>`,
:doc:`x2sys_init`,
:doc:`x2sys_datalist`,
:doc:`x2sys_get`,
:doc:`x2sys_list`,
:doc:`x2sys_put`,
:doc:`x2sys_report`,
:doc:`x2sys_solve`
