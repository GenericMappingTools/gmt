.. index:: ! gmtconvert

**********
gmtconvert
**********

.. only:: not man

    gmtconvert - Convert, Paste, and/or Extract columns from data tables

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmtconvert** [ *table* ] [ **-A** ] [ **-D**\ [*template*] ]
[ **-E**\ [**f**\ \|\ **l**\ \|\ **m**\ *stride*] ] [ **-L** ]
[ **-I**\ [**tsr**\ ] ] [ **-Q**\ [**~**\ ]*selection* ]
[ **-S**\ [**~**\ ]\ *"search string"* \|
**-S**\ [**~**\ ]/\ *regexp*/[**i**\ ] ]
[ **-T** ]
[ |SYN_OPT-V| ]
[ |SYN_OPT-a| ]
[ |SYN_OPT-b| ]
[ |SYN_OPT-f| ]
[ |SYN_OPT-g| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-o| ]
[ |SYN_OPT-s| ]
[ |SYN_OPT-:| ]

|No-spaces|

Description
-----------

**gmtconvert** reads its standard input [or input files] and writes out
the desired information to standard output. It can do a combination of
seven tasks: (1) convert between binary and ASCII data tables, (2) paste
corresponding records from multiple files horizontally into a single
file, (3) extract a subset of the available columns, (4) only extract
segments whose header record matches a text pattern search, (5) only
list segment headers and no data records, (6) extract first and/or last
data record for each segment, and (7) reverse the order
of items on output. Input (and hence output) may have multiple
sub-headers, and ASCII tables may have regular headers as well. 

Required Arguments
------------------

None

Optional Arguments
------------------

.. |Add_intables| unicode:: 0x20 .. just an invisible code
.. include:: explain_intables.rst_

**-A**
    The records from the input files should be pasted horizontally, not
    appended vertically [Default]. All files must have the same number
    of segments and number of rows per segment. Note for binary input,
    all the files you want to paste must have the same number of columns
    (as set with **-bi**); ascii tables can have different number of columns.

**-D**\ [*template*]
    For multiple segment data, dump each segment to a separate output
    file [Default writes a multiple segment file to stdout]. Append a
    format template for the individual file names; this template
    **must** contain a C format specifier that can format an integer
    argument (the running segment number across all tables); this is
    usually %d but could be %08d which gives leading zeros, etc.
    [Default is gmtconvert_segment\_%d.{txt\|bin}, depending on
    **-bo**]. Alternatively, give a template with
    two C format specifiers and we will supply the table number and the
    segment number within the table to build the file name.

**-E**\ [**f**\ \|\ **l**\ \|\ **m**\ *stride*]
    Only extract the first and last record for each segment of interest
    [Default extracts all records]. Optionally, append **f** or **l** to
    only extract the first or last record of each segment, respectively.
    Alternatively, append **m**\ *stride* to extract only one out of *stride* records.

**-I**
    Invert the order of items, i.e., output the items in reverse order,
    starting with the last and ending up with the first item [Default
    keeps original order]. Append up to three items that should be
    reversed: **t** will reverse the order of tables, **s** will reverse
    the order of segments within each table, and **r** will reverse the
    order of records within each segment [Default].

**-L**
    Only output a listing of all segment header records and no data
    records (requires ASCII data).

**-Q**\ [**~**\ ]*selection*
    Only write segments whose number is included in *selection* and skip
    all others. Cannot be used with **-S**. The *selection* syntax is
    *range*[,*range*,...] where each *range* of items is either a single
    segment *number*, a range of segment numbers *start-stop*, or a range with
    stepped increments given via *start:step:stop*.   A leading **~** will
    invert the selection and write all segments but the ones listed.  Instead
    of a list of ranges, use **+f**\ *file* to supply a file list with one
    *range* per line.

**-S**\ [**~**]\ *"search string"* or **-S**\ [**~**]/\ *regexp*/[**i**]
    Only output those segments whose header record contains the
    specified text string. To reverse the search, i.e., to output
    segments whose headers do *not* contain the specified pattern, use
    **-S~**. Should your pattern happen to start with ~ you need to
    escape this character with a backslash  [Default output all
    segments]. Cannot be used with **-Q**. For matching segments based
    on aspatial values (via OGR/GMT format), give the search string as
    *varname*\ =\ *value* and we will compare *value* against the value
    of *varname* for each segment. Note: If the features are polygons
    then a match of a particular polygon perimeter also means that any
    associated polygon holes will also be matched. For matching segment
    headers against extended regular expressions enclose the expression
    in slashes. Append **i** for caseless matching.
    For a list of such patterns, give **+f**\ *file* with one pattern per line.
    To give a single pattern starting with +f, escape it with a backslash.

**-T**
    Suppress the writing of segment headers on output. 

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_

.. include:: explain_-aspatial.rst_

.. |Add_-bi| unicode:: 0x20 .. just an invisible code
.. include:: explain_-bi.rst_

.. |Add_-bo| replace:: [Default is same as input].
.. include:: explain_-bo.rst_

.. |Add_-f| unicode:: 0x20 .. just an invisible code
.. include:: explain_-f.rst_

.. |Add_-g| unicode:: 0x20 .. just an invisible code
.. include:: explain_-g.rst_

.. |Add_-h| unicode:: 0x20 .. just an invisible code
.. include:: explain_-h.rst_

.. include:: explain_-icols.rst_

.. include:: explain_-ocols.rst_

.. include:: explain_-s.rst_

.. include:: explain_colon.rst_

.. include:: explain_help.rst_

.. include:: explain_precision.rst_


Examples
--------

To convert the binary file test.b (single precision) with 4 columns to ASCII:

   ::

    gmt gmtconvert test.b -bi4f > test.dat

To convert the multiple segment ASCII table test.d to a double precision binary file:

   ::

    gmt gmtconvert test.d -bo > test.b

You have an ASCII table with 6 columns and you want to plot column 5 versus column 0. Try

   ::

    gmt gmtconvert table.d -o5,0 | psxy ...

If the file instead is the binary file results.b which has 9
single-precision values per record, we extract the last column and
columns 4-6 and write ASCII with the command

   ::

    gmt gmtconvert results.b -o8,4-6 -bi9s | psxy ...

You want to plot the 2nd column of a 2-column file left.d versus the
first column of a file right.d:

   ::

    gmt gmtconvert left.d right.d -A -o1,2 | psxy ...

To extract all segments in the file big_file.d whose headers contain
the string "RIDGE AXIS", try

   ::

    gmt gmtconvert big_file.d -S"RIDGE AXIS" > subset.d

To invert the selection of segments whose headers begin with "profile "
followed by an integer number and any letter between "g" and "l", try

   ::

    gmt gmtconvert -S~"/^profile [0-9]+[g-l]$/"

To reverse the order of segments in a file without reversing the order
of records within each segment, try

   ::

    gmt gmtconvert lots_of_segments.txt -Is > last_segment_first.txt

To extract segments 20 to 40 in steps of 2, plus segment 0 in a file, try

   ::

    gmt gmtconvert lots_of_segments.txt -Q0,20:2:40 > my_segments.txt


To extract the attribute ELEVATION from an ogr gmt file like this

   ::

    # @VGMT1.0 @GPOINT
    ...
    # @NELEVATION|DISPX|DISPY
    # @Tdouble|double|double
    # FEATURE_DATA
    # @D4.945000|-106500.00000000|-32700.00000000
    -9.36890245902635 39.367156766570389

do

   ::

    gmt gmtconvert file.gmt -a2=ELEVATION > xyz.dat

or just

   ::

    gmt gmtconvert file.gmt -aELEVATION > xyz.dat

See Also
--------

:doc:`gmt`,
:doc:`gmtinfo`,
:doc:`gmtselect`
