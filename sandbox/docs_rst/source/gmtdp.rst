*****
gmtdp
*****


gmtdp - Line reduction using the Douglas-Peucker algorithm

`Synopsis <#toc1>`_
-------------------

**gmtdp** [ *table* ] **-T**\ *tolerance*\ [*unit*\ ] [
**-V**\ [*level*\ ] ] [
**-b**\ [*ncol*\ ][**t**\ ][\ **+L**\ \|\ **+B**] ] [
**-bo**\ [*ncol*\ ][**t**\ ] ] [ **-f**\ [**i**\ \|\ **o**]\ *colinfo* ] [
**-g**\ [**a**\ ]\ **x**\ \|\ **y**\ \|\ **d**\ \|\ **X**\ \|\ **Y**\ \|\ **D**\ \|[*col*\ ]\ **z**\ [+\|-]\ *gap*\ [**u**\ ]
] [ **-h**\ [**i**\ \|\ **o**][*n*\ ] ] [
**-i**\ *cols*\ [**l**\ ][\ **s**\ *scale*][\ **o**\ *offset*][,\ *...*]
] [ **-o**\ *cols*\ [,*...*] ] [ **-:**\ [**i**\ \|\ **o**] ]

`Description <#toc2>`_
----------------------

**gmtdp** reads one or more data files and apply the Douglas-Peucker
line simplification algorithm. The method recursively subdivides a
polygon until a run of points can be replaced by a straight line
segment, with no point in that run deviating from the straight line by
more than the tolerance. Have a look at this site to get a visual
insight on how the algorithm works
(http://geometryalgorithms.com/Archive/algorithm\_0205/algorithm\_0205.htm)

`Common Arguments And Specifications <#toc3>`_
----------------------------------------------

All options marked with an asterisk (\*) are common GMT command-line
options. Their full syntax as well as how to specify pens, pattern
fills, colors, and fonts can be found in the **gmt** man page. Note: No
space is allowed between the option flag and the associated arguments.

`Required Arguments <#toc4>`_
-----------------------------

**-T**\ *tolerance*\ [*unit*\ ]
    Specifies the maximum mismatch tolerance in the user units. If the
    data is not Cartesian then append the distance unit (see UNITS).

`Optional Arguments <#toc5>`_
-----------------------------

*table*
    One or more ASCII (or binary, see **-bi**\ [*ncol*\ ][**t**\ ]) data
    table file(s) holding a number of data columns. If no tables are
    given then we read from standard input.
**-V**\ [*level*\ ] (\*)
    Select verbosity level [1].
**-bi**\ [*ncol*\ ][**t**\ ] (\*)
    Select binary input. [Default is 2 input columns].
**-bo**\ [*ncol*\ ][**t**\ ] (\*)
    Select binary output. [Default is same as input].
**-f**\ [**i**\ \|\ **o**]\ *colinfo* (\*)
    Specify data types of input and/or output columns.
**-g**\ [**a**\ ]\ **x**\ \|\ **y**\ \|\ **d**\ \|\ **X**\ \|\ **Y**\ \|\ **D**\ \|[*col*\ ]\ **z**\ [+\|-]\ *gap*\ [**u**\ ] (\*)
    Determine data gaps and line breaks.
**-h**\ [**i**\ \|\ **o**][*n*\ ] (\*)
    Skip or produce header record(s).
**-i**\ *cols*\ [**l**\ ][\ **s**\ *scale*][\ **o**\ *offset*][,\ *...*] (\*)
    Select input columns.
**-o**\ *cols*\ [,*...*] (\*)
    Select output columns.
**-:**\ [**i**\ \|\ **o**] (\*)
    Swap 1st and 2nd column on input and/or output.
**-^** (\*)
    Print a short message about the syntax of the command, then exits.
**-?** (\*)
    Print a full usage (help) message, including the explanation of
    options, then exits.

`Units <#toc6>`_
----------------

For map distance units, append *unit* **d** for arc degrees, **m** for
arc minutes, and **s** for arc seconds, or **e** for meters [Default],
**f** for feet, **k** for km, **M** for statute miles, and **n** for
nautical miles. By default we compute such distances using a spherical
approximation with great circles. Prepend **-** to a distance (or the
unit is no distance is given) to perform "Flat Earth" calculations
(quicker but less accurate) or prepend **+** to perform exact geodesic
calculations (slower but more accurate).

`Ascii Format Precision <#toc7>`_
---------------------------------

The ASCII output formats of numerical data are controlled by parameters
in your **gmt.conf** file. Longitude and latitude are formatted
according to **FORMAT\_GEO\_OUT**, whereas other values are formatted
according to **FORMAT\_FLOAT\_OUT**. Be aware that the format in effect
can lead to loss of precision in the output, which can lead to various
problems downstream. If you find the output is not written with enough
precision, consider switching to binary output (**-bo** if available) or
specify more decimals using the **FORMAT\_FLOAT\_OUT** setting.

`Examples <#toc8>`_
-------------------

To reduce the geographic line segment.d using a tolerance of 2 km, run

gmtdp segment.d -T2k > new\_segment.d

To reduce the Cartesian lines xylines.d using a tolerance of 0.45 and
write the reduced lines to file new\_xylines.d, run

gmtdp xylines.d -T0.45 > new\_xylines.d

`Bugs <#toc9>`_
---------------

One known issue with the Douglas-Peucker has to do with crossovers.
Specifically, it cannot be guaranteed that the reduced line does not
cross itself. Depending on how many lines you are considering it is also
possible that reduced lines may intersect other reduced lines. Finally,
the current implementation only does Flat Earth calculations even if you
specify spherical; **gmtdp** will issue a warning and reset the
calculation mode to Flat Earth.

`References <#toc10>`_
----------------------

Douglas, D. H., and T. K. Peucker, Algorithms for the reduction of the
number of points required to represent a digitized line of its
caricature, *Can. Cartogr.*, **10**, 112-122, 1973.
This implementation of the algorithm has been kindly provided by Dr.
Gary J. Robinson, Environmental Systems Science Centre, University of
Reading, Reading, UK (gazza@mail.nerc-essc.ac.uk); his subroutine forms
the basis for this program.

`See Also <#toc11>`_
--------------------

`*gmt*\ <gmt.html>`_

