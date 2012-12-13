******
psclip
******

psclip - Initialize or terminate polygonal clip paths

`Synopsis <#toc1>`_
-------------------

**psclip** [ *table* ] **-J**\ *parameters*
**-R**\ *west*/*east*/*south*/*north*\ [/*zmin*/*zmax*][**r**\ ] [
**-B**\ [**p**\ \|\ **s**]\ *parameters* ] [
**-Jz**\ \|\ **Z**\ *parameters* ] [ **-K** ] [ **-N** ] [ **-O** ] [
**-P** ] [ **-T** ] [ **-U**\ [*just*/*dx*/*dy*/][**c**\ \|\ *label*] ]
[ **-V**\ [*level*\ ] ] [
**-X**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *x-shift*\ [**u**\ ]]
] [
**-Y**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *y-shift*\ [**u**\ ]]
] [ **-bi**\ [*ncols*\ ][*type*\ ] ] [ **-c**\ *copies*] [
**-f**\ *colinfo* ] [
**-g**\ [**a**\ ]\ **x**\ \|\ **y**\ \|\ **d**\ \|\ **X**\ \|\ **Y**\ \|\ **D**\ \|[*col*\ ]\ **z**\ [+\|-]\ *gap*\ [**u**\ ]
] [ **-h**\ [**i**\ \|\ **o**][*n*\ ] ] [
**-i**\ *cols*\ [**l**\ ][\ **s**\ *scale*][\ **o**\ *offset*][,\ *...*]
] [
**-p**\ *azim*/*elev*\ [/*zlevel*][\ **+w**\ *lon0*/*lat0*\ [/*z0*]][\ **+v**\ *x0*/*y0*]
] [ **-t**\ [*transp*\ ] ] [ **-:**\ [**i**\ \|\ **o**] ]

**psclip** **-C**\ [**c**\ \|\ **s**\ \|[\ **a**\ \|\ *n*] [ **-K** ] [
**-O** ]

`Description <#toc2>`_
----------------------

**psclip** reads (x,y) file(s) [or standard input] and draws polygons
that are activated as clipping paths. Several files may be read to
create complex paths consisting of several non-connecting segments. Only
marks that are subsequently drawn inside the clipping path will be
shown. To determine what is inside or outside the clipping path,
**psclip** uses the even-odd rule. When a ray drawn from any point,
regardless of direction, crosses the clipping path segments an odd
number of times, the point is inside the clipping path. If the number is
even, the point is outside. The **-N** option, reverses the sense of
what is the inside and outside of the paths by plotting a clipping path
along the map boundary. After subsequent plotting, which will be clipped
against these paths, the clipping may be deactivated by running
**psclip** a second time with the **-C** option only.

`Common Arguments And Specifications <#toc3>`_
----------------------------------------------

All options marked with an asterisk (\*) are common GMT command-line
options. Their full syntax as well as how to specify pens, pattern
fills, colors, and fonts can be found in the **gmt** man page. Note: No
space is allowed between the option flag and the associated arguments.

`Required Arguments <#toc4>`_
-----------------------------

**-C**\ [**c**\ \|\ **s**\ [**a**\ \|\ *n*]
    Mark end of existing clip path(s). No input file will be processed.
    No projection information is needed unless **-B** has been selected
    as well. Append **c** (for curved text) or **s** (for straight text)
    to plot text previously used to lay down a clip path (e.g., via
    contouring, pstext, or psxy **-Sq**). The curved text option
    (**-Cc**) is only required if psxy **-Sq** was run with the **+v**
    modifier; the pstext and contouring mechanisms use straight text.
    Both **-Cc** and **-Cs** assumes only one level of text clipping was
    initialized and we thus reduce the clip level by one. To undo one
    level of polygon clipping (perhaps initiated by earlier psclip,
    pscoast, or psmask calls) use **-C**. You can undo all clip levels
    with **-Ca** or a specific number with **-C**\ *n*. Also supply
    **-X** and **-Y** settings if you have moved since the clip started.
**-J**\ *parameters* (\*)
    Select map projection.
**-R**\ [*unit*\ ]\ *xmin*/*xmax*/*ymin*/*ymax*\ [**r**\ ] (\*)
    Specify the region of interest.
    For perspective view (**-p**), optionally append /*zmin*/*zmax*.

`Optional Arguments <#toc5>`_
-----------------------------

*table*
    One or more ASCII (or binary, see **-bi**\ [*ncols*\ ][*type*\ ])
    data table file(s) holding a number of data columns. If no tables
    are given then we read from standard input.
**-B**\ [**p**\ \|\ **s**]\ *parameters* (\*)
    Set map boundary intervals.
**-Jz**\ \|\ **Z**\ *parameters* (\*)
    Set z-axis scaling; same syntax as **-Jx**.
**-K** (\*)
    Do not finalize the *PostScript* plot.
**-N**
    Invert the sense of what is inside and outside. For example, when
    using a single path, this means that only points outside that path
    will be shown. Cannot be used together with **-B**.
**-O** (\*)
    Append to existing *PostScript* plot.
**-P** (\*)
    Select "Portrait" plot orientation.
**-T**
    Rather than read any input files, simply turn on clipping for the
    current map region. Basically, **-T** is a convenient way to run
    **psclip** with the arguments **-N** /dev/null (or, under Windows,
    **-N** NUL). Cannot be used together with **-B**.
**-U**\ [*just*/*dx*/*dy*/][**c**\ \|\ *label*] (\*)
    Draw GMT time stamp logo on plot.
**-V**\ [*level*\ ] (\*)
    Select verbosity level [c].
**-X**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *x-shift*\ [**u**\ ]]
**-Y**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *y-shift*\ [**u**\ ]]
(\*)
    Shift plot origin.
**-bi**\ [*ncols*\ ][*type*\ ] (\*)
    Select binary input. [Default is 2 input columns].
**-c**\ *copies* (\*)
    Specify number of plot copies [Default is 1].
**-f**\ [**i**\ \|\ **o**]\ *colinfo* (\*)
    Specify data types of input and/or output columns.
**-g**\ [**a**\ ]\ **x**\ \|\ **y**\ \|\ **d**\ \|\ **X**\ \|\ **Y**\ \|\ **D**\ \|[*col*\ ]\ **z**\ [+\|-]\ *gap*\ [**u**\ ]
(\*)
    Determine data gaps and line breaks.
**-h**\ [**i**\ \|\ **o**][*n*\ ] (\*)
    Skip or produce header record(s).
**-i**\ *cols*\ [**l**\ ][\ **s**\ *scale*][\ **o**\ *offset*][,\ *...*](\*)
    Select input columns.
**-p**\ *azim*/*elev*\ [/*zlevel*][\ **+w**\ *lon0*/*lat0*\ [/*z0*]][\ **+v**\ *x0*/*y0*]
(\*)
    Select perspective view.
**-t**\ [*transp*\ ] (\*)
    Set PDF transparency level.
**-:**\ [**i**\ \|\ **o**] (\*)
    Swap 1st and 2nd column on input and/or output.
**-^** (\*)
    Print a short message about the syntax of the command, then exits.
**-?** (\*)
    Print a full usage (help) message, including the explanation of
    options, then exits.
**--version** (\*)
    Print GMT version and exit.
**--show-sharedir** (\*)
    Print full path to GMT share directory and exit.

`Examples <#toc6>`_
-------------------

To make an overlay *PostScript* file that will set up a complex clip
area to which subsequent plotting will be confined, run:

psclip my\_region.xy -R0/40/0/40 -Jm0.3i -O -K > clip\_mask\_on.ps

To deactivate the clipping in an existing plotfile, run:

psclip -C -O >> complex\_plot.ps

`Bugs <#toc7>`_
---------------

**psclip** cannot handle polygons that contain the south or north pole.
For such polygons, you should split them into two and make each
explicitly contain the polar point. The two clip polygons will combine
to give the desired effect.

`See Also <#toc8>`_
-------------------

`*gmt*\ (1) <gmt.html>`_ , `*grdmask*\ (1) <grdmask.html>`_ ,
`*psbasemap*\ (1) <psbasemap.html>`_ , `*psmask*\ (1) <psmask.html>`_
