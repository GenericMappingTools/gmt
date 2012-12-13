*******
colmath
*******

colmath - Do mathematics on columns from data tables

`Synopsis <#toc1>`_
-------------------

**colmath** [ *table* ] [ **-A** ] [ **-N** ] [ **-Q**\ *seg* ] [
**-S**\ [**~**\ ]\ *"search string"* ] [ **-T** ] [ **-V**\ [*level*\ ]
] [ **-b**\ [*ncol*\ ][**t**\ ][\ **+L**\ \|\ **+B**] ] [
**-f**\ [**i**\ \|\ **o**]\ *colinfo* ] [
**-g**\ [**a**\ ]\ **x**\ \|\ **y**\ \|\ **d**\ \|\ **X**\ \|\ **Y**\ \|\ **D**\ \|[*col*\ ]\ **z**\ [+\|-]\ *gap*\ [**u**\ ]
] [ **-h**\ [**i**\ \|\ **o**][*n*\ ] ] [
**-i**\ *cols*\ [**l**\ ][\ **s**\ *scale*][\ **o**\ *offset*][,\ *...*]
] [ **-o**\ *cols*\ [,*...*] ] [ **-s**\ [*cols*\ ][\ **a**\ \|\ **r**]
] [ **-:**\ [**i**\ \|\ **o**] ]

`Description <#toc2>`_
----------------------

**colmath** reads its standard input [or inputfiles] does mathematics in
RPN on the columns and then writes the result to standard output. It can
do a combination of four tasks: (1) convert between binary and ASCII
data tables, (2) paste corresponding records from multiple files
horizontally into a single file, (3) extract a subset of the available
columns, (4) do mathematics on the columns. Input (and hence output) may
have multiple sub-headers, and ASCII tables may have regular headers as
well.

`Common Arguments And Specifications <#toc3>`_
----------------------------------------------

All options marked with an asterisk (\*) are common GMT command-line
options. Their full syntax as well as how to specify pens, pattern
fills, colors, and fonts can be found in the **gmt** man page. Note: No
space is allowed between the option flag and the associated arguments.

`Required Arguments <#toc4>`_
-----------------------------

`Optional Arguments <#toc5>`_
-----------------------------

*table*
    One or more ASCII (or binary, see **-bi**\ [*ncols*\ ][*type*\ ])
    data table file(s) holding a number of data columns. If no tables
    are given then we read from standard input.
**-A**
    The records from the input files should be pasted horizontally, not
    appended vertically [Default]. All files must have the same number
    of segments and number of rows per segment. Note for binary input,
    all the files you want to paste must have the same number of columns
    (as set with **-bi**\ [*ncols*\ ][*type*\ ]); ascii tables can have
    different number of columns.
**-N**
    Do not write records that only contain NaNs in every field [Default
    writes all records].
**-Q**\ *seg*
    Only write segment number *seg* and skip all others. Cannot be used
    with **-S**.
**-S**\ [**~**\ ]\ *"search string"*
    Only output those segments whose header record contains the
    specified text string. To reverse the search, i.e., to output
    segments whose headers do *not* contain the specified pattern, use
    **-S~**. Should your pattern happen to start with ~ you need to
    escape this character with a backslash  [Default output all
    segments]. Cannot be used with **-Q**.
**-T**
    Suppress the writing of segment headers on output.
**-V**\ [*level*\ ] (\*)
    Select verbosity level [c].
**-bi**\ [*ncols*\ ][*type*\ ] (\*)
    Select binary input.
**-bo**\ [*ncols*\ ][*type*\ ] (\*)
    Select binary output. [Default is same as input].
**-f**\ [**i**\ \|\ **o**]\ *colinfo* (\*)
    Specify data types of input and/or output columns.
**-g**\ [**a**\ ]\ **x**\ \|\ **y**\ \|\ **d**\ \|\ **X**\ \|\ **Y**\ \|\ **D**\ \|[*col*\ ]\ **z**\ [+\|-]\ *gap*\ [**u**\ ]
(\*)
    Determine data gaps and line breaks.
**-h**\ [**i**\ \|\ **o**][*n*\ ] (\*)
    Skip or produce header record(s).
**-i**\ *cols*\ [**l**\ ][\ **s**\ *scale*][\ **o**\ *offset*][,\ *...*](\*)
    Select input columns.
**-o**\ *cols*\ [,*...*] (\*)
    Select output columns.
**-s**\ [*cols*\ ][\ **a**\ \|\ **r**] (\*)
    Set handling of NaN records.
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

`Ascii Format Precision <#toc6>`_
---------------------------------

The ASCII output formats of numerical data are controlled by parameters
in your **gmt.conf** file. Longitude and latitude are formatted
according to **FORMAT\_GEO\_OUT**, whereas other values are formatted
according to **FORMAT\_FLOAT\_OUT**. Be aware that the format in effect
can lead to loss of precision in the output, which can lead to various
problems downstream. If you find the output is not written with enough
precision, consider switching to binary output (**-bo** if available) or
specify more decimals using the **FORMAT\_FLOAT\_OUT** setting.

`Examples <#toc7>`_
-------------------

To convert the binary file test.b (single precision) with 4 columns to
ASCII:

colmath test.b -bi4f > test.dat

To convert the multiple segment ASCII table test.d to a double precision
binary file:

colmath test.d -bo > test.b

You have an ASCII table with 6 columns and you want to plot column 5
versus column 0. Try

colmath table.d -o5,0 \| psxy ...

If the file instead is the binary file results.b which has 9
single-precision values per record, we extract the last column and
columns 4-6 and write ASCII with the command

colmath results.b -o8,4-6 -bi9s \| psxy ...

You want to plot the 2nd column of a 2-column file left.d versus the
first column of a file right.d:

colmath left.d right.d -A -o1,2 \| psxy ...

To extract all segments in the file big\_file.d whose headers contain
the string "RIDGE AXIS", try

colmath big\_file.d -S"RIDGE AXIS" > subset.d

`See Also <#toc8>`_
-------------------

`*gmt*\ (1) <gmt.html>`_ , `*minmax*\ (1) <minmax.html>`_
