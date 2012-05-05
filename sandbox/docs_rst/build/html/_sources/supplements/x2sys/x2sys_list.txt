***********
x2sys\_list
***********


x2sys\_list - Extract subset from crossover data base

`Synopsis <#toc1>`_
-------------------

**x2sys\_list** **-C**\ *column* **-T**\ *TAG* [ *coedbase.txt* ] [
**-A**\ *asymm\_max* ] [ **-F**\ *acdhiInNtTvxyz* ] [ **-I**\ [*list*\ ]
] [ **-L**\ [*corrtable*\ ] ] [ **-N**\ *nx\_min* ] [ **-Qe**\ \|\ **i**
] [ **-R**\ *west*/*east*/*south*/*north*\ [**r**\ ] ] [ **-S**\ *track*
] [ **-V**\ [*level*\ ] ] [ **-W**\ [*list*\ ] ] [
**-bo**\ [*ncol*\ ][**t**\ ] ]

`Description <#toc2>`_
----------------------

**x2sys\_list** will read the crossover ASCII data base *coedbase.txt*
(or *stdin*) and extract a subset of the crossovers based on the other
arguments. The output may be ASCII or binary.

`Common Arguments And Specifications <#toc3>`_
----------------------------------------------

All options marked with an asterisk (\*) are common GMT command-line
options. Their full syntax as well as how to specify pens, pattern
fills, colors, and fonts can be found in the **gmt** man page. Note: No
space is allowed between the option flag and the associated arguments.

`Required Arguments <#toc4>`_
-----------------------------

**-C**\ *column*
    Specify which data column you want to process. Crossovers related to
    this column name must be present in the crossover data base.
**-T**\ *TAG*
    Specify the x2sys *TAG* which tracks the attributes of this data type.

`Optional Arguments <#toc5>`_
-----------------------------

*coedbase.txt*
    The name of the input ASCII crossover error data base as produced by
    **x2sys\_cross**. If not given we read standard input instead.
**-A**\ *asymm\_max*
    Specifies maximum asymmetry in the distribution of crossovers
    relative to the mid point in time (or distance, if not time is
    available). Asymmetry is computed as (n\_right - n\_left)/(n\_right
    + n\_left), referring the the number of crossovers that falls in the
    left or right half of the range. Symmetric distributions will have
    values close to zero. If specified, we exclude tracks whose
    asymmetry exceeds the specify cutoff in absolute value [1, i.e.,
    include all].
**-F**\ *acdhiInNtTvxyz*
    Specify your desired output using any combination of
    *acdhiInNtTvwxyz*, in any order. Do not use space between the
    letters, and note your selection is case-sensitive. The output will
    be ASCII (or binary, **-bo**\ [*ncol*\ ][**t**\ ]) columns of
    values. Description of codes: **a** is the angle (< 90) defined by
    the crossing tracks, **c** is crossover value of chosen observation
    (see **-C**), **d** is distance along track, **h** is heading along
    track, **i** is the signed time interval between the visit at the
    crossover of the two tracks involved, **I** is same as **i** but is
    unsigned, **n** is the names of the two tracks, **N** is the id
    numbers of the two tracks, **t** is time along track in
    *date*\ **T**\ *clock* format (NaN if not available), **T** is
    elapsed time since start of track along track (NaN if not
    available), **v** is speed along track, **w** is the composite
    weight, **x** is *x*-coordinate (or longitude), **y** is
    *y*-coordinate (or latitude), and **z** is observed value (see
    **-C**) along track. If **-S** is not specified then
    **d**,\ **h**,\ **n**,\ **N**,\ **t**,\ **T**,\ **v** results in two
    output columns each: first for track one and next for track two (in
    lexical order of track names); otherwise, they refer to the
    specified track only (except for **n**,\ **N** which then refers to
    the other track). The sign convention for **c**,\ **i** is track one
    minus track two (lexically sorted). Time intervals will be returned
    according to the **TIME\_UNIT** GMT defaults setting.
**-I**\ [*list*\ ]
    Name of ASCII file with a list of track names (one per record) that
    should be excluded from consideration [Default includes all tracks].
**-L**\ [*corrtable*\ ]
    Apply optimal corrections to the chosen observable. Append the
    correction table to use [Default uses the correction table
    *TAG*\ \_corrections.txt which is expected to reside in the
    **$X2SYS\_HOME**/*TAG* directory]. For the format of this file, see
    **x2sys\_solve**.
**-N**\ *nx\_min*
    Only report data from pairs that generated at least *nx\_min*
    crossovers between them [use all pairs].
**-Qe**\ \|\ **i**
    Append **e** for external crossovers or **i** for internal
    crossovers only [Default is all crossovers].
**-R**\ *west*/*east*/*south*/*north*\ [/*zmin*/*zmax*][**r**\ ]
    *west*, *east*, *south*, and *north* specify the region of interest,
    and you may specify them in decimal degrees or in
    [+-]dd:mm[:ss.xxx][W\|E\|S\|N] format. Append **r** if lower left
    and upper right map coordinates are given instead of w/e/s/n. The
    two shorthands **-Rg** and **-Rd** stand for global domain (0/360
    and -180/+180 in longitude respectively, with -90/+90 in latitude).
    Alternatively, specify the name of an existing grid file and the
    **-R** settings (and grid spacing, if applicable) are copied from
    the grid. For Cartesian data just give *xmin/xmax/ymin/ymax*. This
    option limits the COEs to those that fall inside the specified
    domain.
**-S**\ *track*
    Name of a single track. If given we restrict output to those
    crossovers involving this track [Default output is crossovers
    involving any track pair].
**-V**\ [*level*\ ] (\*)
    Select verbosity level [1].
**-W**\ [*list*\ ]
    Name of ASCII file with a list of track names and their relative
    weights (one track per record) that should be used to calculate the
    composite crossover weight (output code **w** above). [Default sets
    weights to 1].
**-bo**\ [*ncol*\ ][**t**\ ] (\*)
    Select binary output.
**-m**
    Multiple segment output format. Segments with crossovers for a
    single track pair are separated by a record whose first character is
    *flag* and contains the two track names. [Default is ’>’].
**-^** (\*)
    Print a short message about the syntax of the command, then exits.
**-?** (\*)
    Print a full usage (help) message, including the explanation of
    options, then exits.

`Examples <#toc6>`_
-------------------

To find all the magnetic crossovers associated with the tag MGD77 from
the file COE\_data.txt, restricted to occupy a certain region in the
south Pacific, and return location, time, and crossover value, try

**x2sys\_list** COE\_data.txt **-V** **-T**\ MGD77
**-R**\ 180/240/-60/-30 **-C**\ mag **-F**\ xytz > mag\_coe.txt

To find all the faa crossovers globally that involves track 12345678 and
output time since start of the year, using a binary double precision
format, try

**x2sys\_list** COE\_data.txt **-V** **-T**\ MGD77 **-C**\ faa
**-S**\ 12345678 **-F**\ Tz **-bod** > faa\_coe.b

`See Also <#toc7>`_
-------------------

`*x2sys\_binlist*\ (1) <x2sys_binlist.1.html>`_ ,
`*x2sys\_cross*\ (1) <x2sys_cross.1.html>`_ ,
`*x2sys\_datalist*\ (1) <x2sys_datalist.1.html>`_ ,
`*x2sys\_get*\ (1) <x2sys_get.1.html>`_ ,
`*x2sys\_init*\ (1) <x2sys_init.1.html>`_ ,
`*x2sys\_put*\ (1) <x2sys_put.1.html>`_ ,
`*x2sys\_report*\ (1) <x2sys_report.1.html>`_ ,
`*x2sys\_solve*\ (1) <x2sys_solve.1.html>`_

