******
grdokb
******

grdokb - Compute the gravity effect of a grid by the method of Okabe

`Synopsis <#toc1>`_
-------------------

**grdokb** [ **-C**\ *density* ] [ **-D** ] [ **-F**\ *xy\_file* ] [
**-G**\ *outputgrid.nc* ] [ **-L**\ *z\_observation* ] [ **-M** ]
**-R**\ *west*/*east*/*south*/*north*\ [**r**\ ] [ **-S**\ *radius* ] [
**-Z**\ *level* ] [ **-V**\ [*level*\ ] ] [
**-f**\ [**i**\ \|\ **o**]\ *colinfo* ]

`Description <#toc2>`_
----------------------

**grdokb** will compute the gravity anomaly of a body described by one
or (optionally) two grids The output can either be along a given set of
xy locations or on a grid. This method is not particularly fast but
allows computing the anomaly of arbitrarily complex shapes.

`Common Arguments And Specifications <#toc3>`_
----------------------------------------------

All options marked with an asterisk (\*) are common GMT command-line
options. Their full syntax as well as how to specify pens, pattern
fills, colors, and fonts can be found in the **gmt** man page. Note: No
space is allowed between the option flag and the associated arguments.

`Required Arguments <#toc4>`_
-----------------------------

**-C**\ *density*
    Sets body density in SI. This option is mutually exclusive with
    **-H**.
**-F**\ *xy\_file*
    Provide locations where the anomaly will be computed. Note this
    option is mutually exlusive with **-G**.
**-G**\ *outgrid.nc*
    Output the gravity anomaly at nodes of this grid file.
**-R**\ [*unit*\ ]\ *xmin*/*xmax*/*ymin*/*ymax*\ [**r**\ ] (\*)
    Specify the region of interest.

`Optional Arguments <#toc5>`_
-----------------------------

**-V**\ [*level*\ ] (\*)
    Select verbosity level [c].
**-L**
    sets level of observation [Default = 0]. That is the height (z) at
    which anomalies are computed.
**-M**
    Map units TRUE; x,y in degrees, dist units in m. [Default dist unit
    = x,y unit]
**-Z**
    level of reference plane [Default = 0]. Use this option when the
    triangles describe a non-closed surface and the volume is deffined
    from each triangle and this reference level. An example will be the
    whater depth to compute a Bouguer anomaly.
**-f**\ [**i**\ \|\ **o**]\ *colinfo* (\*)
    Specify data types of input and/or output columns.
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

Suppose you want to compute the gravity effect of the phantom "Sandy
Island" together with its not phantom seamount

grdokb sandy\_bat.grd -C1700 -Z-4300 -M -I1m -Gsandy\_okb.grd -V

`See Also <#toc7>`_
-------------------

`*GMT*\ (1) <GMT.html>`_

`Reference <#toc8>`_
--------------------

Okabe, M., Analytical expressions for gravity anomalies due to
polyhedral bodies and translation into magnetic anomalies, *Geophysics*,
44, (1979), p 730-741.
