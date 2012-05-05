************
x2sys\_solve
************


x2sys\_solve - Determine least-squares systematic correction from crossovers

`Synopsis <#toc1>`_
-------------------

**x2sys\_solve** **-C**\ *column* **-T**\ *TAG* **-E**\ *mode* [
*COE\_list.d* ] [ **-V**\ [*level*\ ] ] [ **-W** ] [
**-bi**\ [*ncol*\ ][**t**\ ] ]

`Description <#toc2>`_
----------------------

**x2sys\_solve** will use the supplied crossover information to solve
for systematic corrections that can then be applied per track to improve
data quality. Several systematic corrections can be solved for using a
least-squares approach. Note: Only one data column can be processed at
the time.

`Common Arguments And Specifications <#toc3>`_
----------------------------------------------

All options marked with an asterisk (\*) are common GMT command-line
options. Their full syntax as well as how to specify pens, pattern
fills, colors, and fonts can be found in the **gmt** man page. Note: No
space is allowed between the option flag and the associated arguments.

`Required Arguments <#toc4>`_
-----------------------------

*COE\_list.d*
    Name of file with the required crossover columns as produced by
    **x2sys\_list**. NOTE: If **-bi**\ [*ncol*\ ][**t**\ ] is used then
    the first two columns are expected to hold the integer track IDs;
    otherwise we expect those columns to hold the text string names of
    the two tracks. If no file is given we will read from *stdin*.
**-T**\ *TAG*
    Specify the x2sys *TAG* which tracks the attributes of this data
    type.
**-C**\ *column*
    Specify which data column you want to process. Needed for proper
    formatting of the output correction table and must match the same
    option used in **x2sys\_list** when preparing the input data.
**-E**\ *mode*
    The correction type you wish to model. Choose among the following
    functions `*f*\ (*p*) <f.p.html>`_ , where *p* are the *m*
    parameters per track that we will fit simultaneously using a least
    squares approach:
    **c** will fit `*f*\ (*p*) <f.p.html>`_ = *a* (a constant offset);
    records must contain cruise ID1, ID2, COE.
    **d** will fit `*f*\ (*p*) <f.p.html>`_ = *a* + *b* \* *d* (linear
    drift; *d* is distance; records must contain cruise ID1, ID2, d1,
    d2, COE.
    **g** will fit `*f*\ (*p*) <f.p.html>`_ = *a* + *b* sin(y)^2
    (1980-1930 gravity correction); records must contain cruise ID1,
    ID2, latitude y, COE.
    **h** will fit `*f*\ (*p*) <f.p.html>`_ = *a* + *b* cos(H) + *c*
    `cos(2H) <cos.2H.html>`_ + *d* sin(H) + *e* `sin(2H) <sin.2H.html>`_
    (magnetic heading correction); records must contain cruise ID1, ID2,
    heading H, COE.
    **s** will fit `*f*\ (*p*) <f.p.html>`_ = *a* \* z (a unit scale
    correction); records must contain cruise ID1, ID2, z1, z2.
    **t** will fit `*f*\ (*p*) <f.p.html>`_ = *a* + *b* \* (*t - t0*)
    (linear drift; *t0* is the start time of the track); records must
    contain cruise ID1, ID2, t1-t0, t2-t0, COE.

`Optional Arguments <#toc5>`_
-----------------------------

**-V**\ [*level*\ ] (\*)
    Select verbosity level [1].
**-W**
    Means that each input records has an extra column with the composite
    weight for each crossover record. These are used to obtain a
    weighted least squares solution [no weights].
**-bi**\ [*ncol*\ ][**t**\ ] (\*)
    Select binary input.
**-^** (\*)
    Print a short message about the syntax of the command, then exits.
**-?** (\*)
    Print a full usage (help) message, including the explanation of
    options, then exits.

`Examples <#toc6>`_
-------------------

To fit a simple bias offset to faa for all tracks under the MGD77 tag,
try

**x2sys\_list** COE\_data.txt **-V** **-T**\ MGD77 **-C**\ faa
**-F**\ nc > faa\_coe.txt
**x2sys\_solve** faa\_coe.txt **-V** **-T**\ MGD77 **-C**\ faa
**-E**\ c > coe\_table.txt

To fit a faa linear drift with time instead, try

**x2sys\_list** COE\_data.txt **-V** **-T**\ MGD77 **-C**\ faa
**-F**\ nTc > faa\_coe.txt
**x2sys\_solve** faa\_coe.txt **-V** **-T**\ MGD77 **-C**\ faa
**-E**\ t > coe\_table.txt

To estimate heading corrections based on magnetic crossovers associated
with the tag MGD77 from the file COE\_data.txt, try

**x2sys\_list** COE\_data.txt **-V** **-T**\ MGD77 **-C**\ mag
**-F**\ nhc > mag\_coe.txt
**x2sys\_solve** mag\_coe.txt **-V** **-T**\ MGD77 **-C**\ mag
**-E**\ h > coe\_table.txt

To estimate unit scale corrections based on bathymetry crossovers, try

**x2sys\_list** COE\_data.txt **-V** **-T**\ MGD77 **-C**\ depth
**-F**\ nz > depth\_coe.txt
**x2sys\_solve** depth\_coe.txt **-V** **-T**\ MGD77 **-C**\ depth
**-E**\ s > coe\_table.txt

`See Also <#toc7>`_
-------------------

`*x2sys\_binlist*\ (1) <x2sys_binlist.1.html>`_ ,
`*x2sys\_cross*\ (1) <x2sys_cross.1.html>`_ ,
`*x2sys\_datalist*\ (1) <x2sys_datalist.1.html>`_ ,
`*x2sys\_get*\ (1) <x2sys_get.1.html>`_ ,
`*x2sys\_init*\ (1) <x2sys_init.1.html>`_ ,
`*x2sys\_list*\ (1) <x2sys_list.1.html>`_ ,
`*x2sys\_put*\ (1) <x2sys_put.1.html>`_ ,
`*x2sys\_report*\ (1) <x2sys_report.1.html>`_

